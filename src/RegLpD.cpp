/*************************************************************************
* This file is part of AgriPoliS
*
* AgriPoliS: An Agricultural Policy Simulator
*
* Copyright (c) 2021, Alfons Balmann, Kathrin Happe, Konrad Kellermann et al.
* (cf. AUTHORS.md) at Leibniz Institute of Agricultural Development in 
* Transition Economies
*
* SPDX-License-Identifier: MIT
**************************************************************************/

// RegLpD.cpp
//---------------------------------------------------------------------------
#include <fstream>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <sstream>
#include <iterator>
#include <istream>
#include <iomanip>
#include <filesystem>

#include "RegLpD.h"
#include "RegManager.h"
#include "RegPlot.h"

#include "textinput.h"
namespace fs = std::filesystem;

static string rtrim(string s, char c) {
	int n = s.size();
	string rs,res;
	rs.resize(n);
	copy(s.rbegin(), s.rend(), rs.begin());
	size_t pos=0;
	while (rs[pos] == '\\') ++pos;
	
	res.resize(n - pos);
	for (unsigned i = 0; i < n-pos; ++i) {
		res[i] = rs[n - 1- i];
	}
	return res;
}

string
RegLpInfo::printVar(double val, int no) {
stringstream o;
if(val>=0)
 o<<" +"<<val;
else
 o<<" "<<val;
o<<" x"<<no;
return o.str();
}

RegLpInfo::RegLpInfo() {
    obj_backup=NULL;
    nzspace = 0;
    ofstream out;
    flat_copy=false;
}

RegLpInfo::~RegLpInfo() {
    if (!flat_copy) {
        for (unsigned i=0;i<invest_links.size();i++)
            if (invest_links[i]) delete invest_links[i];
        for (unsigned i=0;i<market_links.size();i++)
            if (market_links[i])delete market_links[i];
        for (unsigned i=0;i<reference_links.size();i++)
            if (reference_links[i])delete reference_links[i];
        for (unsigned i=0;i<number_links.size();i++)
            if (number_links[i])delete number_links[i];
        for (unsigned i=0;i<land_links.size();i++)
            if (land_links[i]) delete land_links[i];

		for (unsigned i=0;i<yield_links.size();i++)
            if (yield_links[i]) delete yield_links[i];
    }
	yield_links.clear();

    invest_links.clear();
    market_links.clear();
    reference_links.clear();
    number_links.clear();
    land_links.clear();
    mat_links.clear();
    cap_links.clear();
    obj_links.clear();
    incomepay_links.clear();
    if (obj_backup) delete obj_backup;
}
void
RegLpInfo::debug(string filename) {
    ofstream out;
    out.open(filename.c_str(),ios::trunc);
    if (objsen==-1)
        out << "max:\t" ;
    else
        out << "min:\t";
    for (int i=0;i<numcols;i++) {
        out << setprecision(10) << obj[i] << "\t";
    }
    out << "\n\n";
    for (int i=0;i<numrows;i++) {
        out << "\t";
        for (int j=0;j<numcols;j++) {
            out << mat_val[i+j*numrows] << "\t";
        }
        switch (sense[i]) {
        case 'L':
            out << "<=\t";
            break;
        case 'G':
            out << ">=\t";
            break;
        case 'E':
            out << "=\t";
            break;
        default:
            out << "??";
        }
        out << rhs[i] << "\n";
    }

    out << "Activity Level:\t";
    for (int j=0;j<numcols;j++) {
//DCX
        double d = x[j];
        long long int di = (long long int) (d*1E+7+0.5);
        d = di*1E-7;
        out << setprecision(10) << d << "\t";
//        out << x[j] << "\t";
    }
    out << "\n";
    out << "Integer/Continous:\t";
    for (int j=0;j<numcols;j++) {
        out << ctype[j] << "\t";
    }
    out << "\n";
    out << "UB:\t";
    for (int j=0;j<numcols;j++) {
        out << ub[j] << "\t";
    }
    out << "\n";
	bool optimal = false;
#ifdef GNU_SOLVER
	if (stat==GLP_OPT) 
		optimal = true; 
#else 		
    if (stat==101) 
		optimal = true;
#endif
	if (optimal)
        out << "Optimal Solution found!\n";
    else
        out << "Error Code:\t" << stat << "\n";
    out << "Objective Value:\t";
    out << setprecision(10) << objval << "\n";
    out << "nzspace\t";
    out << nzspace <<"\n";
    out.close();
}

static    map <string, int> colindex;
static    map <string, int> rowindex;

static map <pair<int, int>, bool> linkmat;
static map <int, bool> linkobj;

static map <string,int> marketId;
static map < string, int> investId;

static void mkIDs() {
    int msz = marketdata.products.size();
    int isz = investdata.invests.size();
    for (int i = 0 ; i < msz; i++)
        marketId[marketdata.products[i].name]=i;
    for (int i = 0; i< isz; i++)
        investId[investdata.invests[i].name]=i;
    return;
}

int RegLpInfo::getColIndex(string s) {
	return colindex[s];
}

int RegLpInfo::getRowIndex(string s) {
	return rowindex[s];
}

double RegLpInfo::getValOfIndex(int id) {
	return x[id];
}

void
RegLpInfo::setupMatrix(RegGlobalsInfo* G) {
    g = G;

    for (unsigned int i =0 ; i< matrixdata.rownames.size();i++)
        rowindex[matrixdata.rownames[i]]=i;
    for (unsigned int i =0 ; i< matrixdata.colnames.size();i++)
        colindex[matrixdata.colnames[i]]=i;

    if ((rowindex["LIQUIDITY"]!=0) || (rowindex["FINANCING_RULE"]!=1)) {
        cout << "ERROR: The first rows must be LIQUIDITY and FINANCING_RULE ! " << endl;
        exit(2);
    }

   numrows= matrixdata.rownames.size();
   numcols= matrixdata.colnames.size();

  // Read in Matrix
    mat_val.resize(numrows*numcols);
    int index=0;
    nzspace=0;
    for (int i=0;i<numcols;i++) {
        for (int j=0;j<numrows;j++) {
            mat_val[index]=matrixdata.mat[j][i];
            if (mat_val[index]!=0)
                nzspace++;
            index++;
        }
    }

    // Read in Links for CAPACITY
    for (int i=0;i<numrows;i++) {
        // i CapNumber 1 for rhs
        cap_links.push_back(readLink(i,1));
    }

    mkIDs();
    // Read in Link for Objective Function
    for (unsigned int i=0;i< objlinkdata.objlinks.size();i++) {
        // 2 for obj
        int pos = colindex[objlinkdata.objlinks[i].name];
        obj_links.push_back(readLink(i,2));
        linkobj[pos]=true;
    }
    stdObjLink();

    // Read in Links for Matrix
    //0 liquidity, 1 financing_rule
    int msz = matlinkdata.matlinks.size();
	//vector<onematlink> ml = matlinkdata.matlinks;

    for ( int i =0; i< msz; i++) {
        //  0 for mat
        int r= rowindex[matlinkdata.matlinks[i].row];
        int c= colindex[matlinkdata.matlinks[i].col];
        
		RegLinkObject * link = readLink(i,0);
		if (link) { // kann null sein ohne soilservice 
			mat_links.push_back(readLink(i,0));
			linkmat[pair<int,int>(r,c)]=true;
		}
    }
    stdMatLink();

    //------ PREPARATION OF MIP PROBLEM --------

    // prepare matrix data for lp-solver dll

    // from handbook frontsys.dll
    // mat_val is the one-dimensional array of values

    // create arrays
    rhs.resize(numrows);         // capacity values
    obj.resize(numcols);         // array with objective function coefficients
    sense.resize(numrows);         // array containing the sense of each constraint
    lb.resize(numcols);          // array containing the lower bound on each variable
    ub.resize(numcols);          // array containing the upper bounds each variable
    x.resize(numcols);           // array that contains the optimal values of the
    // primal variables
    ctype.resize(numcols);         // array indicating the type of variables in the
    // MIP problems.
    //   "L" =   // <=
    //   "G" =   // >=
    //   "E" =  // =
    //   "I" =  // integer variable
    //   "C" =  // continuous variable
    // ...array (dim numrows) containing the objective function coefficients
    // first assume that all constraints are less than......
    // read in later
    for (int i = 0; i < numrows; i++) {
        sense[i]= 'L';
		if (g->YoungFarmer) {
			if (rowindex["PAY_CONDITION_YOUNG_FARMER"] == i)
				sense[i] = 'E';
		}
    }

    //Integer type or not
    for (int i = 0; i < numcols; i++) {
        if (matrixdata.isInt[i]!=0)
            ctype[i]='I';
        else ctype[i]='C';
    }

    prodcols = marketdata.products.size()+1;    //always right with 2 ? 
    //g->EXCESS_LU = colindex.find("EXCESS_LU")!=colindex.end() ? colindex["EXCESS_LU"] : -1;
    //g->LU_UPPER_LIMIT = colindex.find("LU_UPPER_LIMIT")!=colindex.end() ? colindex["LU_UPPER_LIMIT"] : -1;
    g->stdNameIndexs["EXCESS_LU"] = colindex.find("EXCESS_LU")!=colindex.end() ? colindex["EXCESS_LU"] : -1;
    g->stdNameIndexs["LU_UPPER_LIMIT"] = colindex.find("LU_UPPER_LIMIT")!=colindex.end() ? colindex["LU_UPPER_LIMIT"] : -1;
    for (int i = 0; i < numcols; i++) {
        lb[i] = 0.0;
        if (i < prodcols) { 
            ub[i] = +1E30;
        }
        if (i >= prodcols) { 
            ub[i] = +1E30;
        }
    }
    if (g->stdNameIndexs["EXCESS_LU"] >=0)
        ub[g->stdNameIndexs["EXCESS_LU"]] = 0;
    if (g->stdNameIndexs["LU_UPPER_LIMIT"] >= 0)
        ub[g->stdNameIndexs["LU_UPPER_LIMIT"]] = 0;

    // set to maximistion problem
    objsen=-1;
}

bool
RegLpInfo::updateLpValues() {
    bool changed=false;
    for (unsigned i=0;i<invest_links.size();i++)
        if (invest_links[i]->trigger()) changed=true;
    for (unsigned i=0;i<market_links.size();i++)
        if (market_links[i]->trigger()) changed=true;
    for (unsigned i=0;i<reference_links.size();i++)
        if (reference_links[i]->trigger()) changed=true;
    for (unsigned i=0;i<number_links.size();i++)
        if (number_links[i]->trigger()) changed=true;
	if (g->HAS_SOILSERVICE){
		for (unsigned i=0;i<yield_links.size();i++)
			if (yield_links[i]->trigger()) changed=true;
	}
	return changed;
}
void
RegLpInfo::updateLand() {
    // land_links is pointer to object of type RegLinkObject
    for (unsigned i=0;i < land_links.size();i++)
        land_links[i]->trigger();
}

void
RegLpInfo::updateYield() {
    for (unsigned i=0;i < yield_links.size();i++)
        yield_links[i]->trigger();
}

void
RegLpInfo::updatePaymentEntitlement() {
    for (unsigned i=0;i < incomepay_links.size();i++)
        incomepay_links[i]->trigger();
}
void
RegLpInfo::updateCapacities() {
    for (unsigned i=0;i<cap_links.size();i++) {
        cap_links[i]->trigger();
    }
}
void
RegLpInfo::updateMatrix() {
    for (unsigned i=0;i<mat_links.size();i++)
        mat_links[i]->trigger();
}
void
RegLpInfo::updateObjectiveFunction() {
    for (unsigned i=0;i<obj_links.size();i++)
        obj_links[i]->trigger();
}



void RegLpInfo::resetInvsUbs() {
	for (int i = 0; i < numcols; i++) {
		lb[i] = 0.0;
		if (i >= prodcols) {
			ub[i] = 0;
		}
	}
}

void RegLpInfo::updateBoundsYoungFarmer() {
	setUBound(colindex["MAX_PAY_YOUNG_FARMER"], 1);
	setUBound(colindex["NON_YOUNG_FARMER"], 1);
	setUBoundInf(colindex["PAY_YOUNG_FARMER"]);
	setLBound(colindex["YEARS_YOUNG_FARMER"], 1);
	setUBoundInf(colindex["YEARS_YOUNG_FARMER"]);
}

void RegLpInfo::resetRestrictedInvsUbs(set<string> rs) {
	for (auto x : rs) {
		for (auto p : colindex) {
			if (p.first.rfind(x, 0) == 0) {
				int i = colindex[p.first];
				if (i>= prodcols)
					ub[i] = 0;
			}
		}
	}
}

double
RegLpInfo::Lp(RegProductList* PList, vector<int >& ninv, bool prod, int maxofffarmlu) {
	if (prod) {
		resetInvsUbs();
	}
	if (!prod) {
		// set Bounds
		for (int i = 0; i < numcols; i++) {
			lb[i] = 0.0;
			if (i >= prodcols) {
				ub[i] = 1E30;
			}
		}

		int ReinvestSame = false;
		if (g->RestrictInvestments) {
			if (ReinvestSame) { // reinvest the same disused stalls
				map<string, int> restInvs;
				if (farm->restrictInvestments()) {
					resetRestrictedInvsUbs(g->Livestock_Invs);

					restInvs = farm->getRestrictedInvests();
				}
				else if (!(farm->allowInvest())) {
					restInvs = g->AllRestrictInvs;
				}

				for (auto x : restInvs) {
					ub[colindex[x.first]] = x.second;
					//cout << x.first << "\t" << x.second << " <==> ";
				}
			}else {  //LU cap
				;
			}
		}
	}

	if (g->YoungFarmer) {
		updateBoundsYoungFarmer();
	}

	//#ifndef FRONTMIPISINSTALLED // #define FRONTMIPISINSTALLED is on top of RegGlobals.h
	//    double glpkobject = LpGlpk(PList, ninv, prod, maxofffarmlu );
	// return glpkobject;
	//#else
#ifdef GNU_SOLVER
	glp_solve();

	if (stat != GLP_OPT)
	{
		//g->V++;  //misbrauch von V
		stringstream ss;
		ss << "gdebug" << g->tInd << ".txt";
		debug(ss.str().c_str());
	}
#else

#ifndef NDEBUG
	bool abool = false;
	if (abool) {
		stringstream ss;
		ss << g->tIter << "_" << g->tFarmName << "_" << g->tFarmId << "_debug.txt";
		debug(ss.str().c_str());
	}
#endif
	HPROBLEM lp = loadlp(PROBNAME, numcols, numrows, objsen, &(*obj.begin()), &(*rhs.begin()), (LPBYTEARG)&(*sense.begin()), NULL, NULL,
		NULL, &(*mat_val.begin()), &(*lb.begin()), &(*ub.begin()), NULL, numcols, numrows, nzspace);
	setintparam(lp, PARAM_ARGCK, 1);
	//setdblparam(lp,PARAM_EPGAP,0.01);
	setdblparam(lp, PARAM_TILIM, 60);
	if (!lp) {
		cout << "Error by constructing LP-problem: " << g->tIter << "\t" << g->tFarmId << "\t" << g->tFarmName << endl;
		exit(10);
		//return -1; // -1 = false since return value = double
	}
	// define integer variables
	loadctype(lp, (LPBYTEARG)(&(*ctype.begin())));
	//lprewrite(lp,"test.lp");
	// solve problem, obtain and display the solution

	mipoptimize(lp);
	solution(lp, &stat, &objval, &(*x.begin()), NULL, NULL, NULL);
	// copy x values to correct destination
	// release LP from memory
	unloadprob(&lp);
#ifndef NDEBUG
	//	debug("ldebugx.txt");
#endif

	if (stat != 101 && stat != 102)
		debug("ldebug1.txt");

#endif
	for (int i = 0; i < numcols; i++) {
		// if the objective link is correct, then the type i 0 (market)
		// or 1 (invest)
		int s = obj_links[i]->getSourceNumber();
		int d = obj_links[i]->getDestNumber();

		double td; long long int ti;
		switch (obj_links[i]->getType()) {
		case 0: // units produced
	 //       PList->setUnitsProducedOfNumber(s,x[d]);
			td = x[d];
			ti = (long long int)(td * 1E+7 + 0.5);
			td = ti * 1E-7;
			PList->setUnitsProducedOfNumber(s, td);

			break;
		case 1: // number of investments of investment number
			ninv[s] = (int)(x[d] + 0.5);
			break;
		case 3:
			break;
		default:
			// for debug 
			if (d < prodcols) {
				PList->setUnitsProducedOfNumber(i, 999999);
			}
			if (d >= prodcols) {
				ninv[i] = 999999;
			}
		}
	}

#ifndef NDEBUG1
	if (g->DebMip)	{
		bool mipcond = false;
		bool itercond = g->uIter==-1 || g->uIter == g->tIter;
		bool idcond = g->uFarmId==-1 || g->uFarmId == g->tFarmId;
		bool namecond = g->uFarmName.compare("ALL")==0 || g->uFarmName.compare(g->tFarmName)==0;
		bool phasecond =g->tPhase!=SimPhase::BETWEEN && ( g->uPhase == SimPhase::ALL || g->uPhase == g->tPhase);

		mipcond = itercond && idcond && namecond && phasecond;
		string phasestr;
		switch (g->tPhase) {
		case SimPhase::FUTURE: phasestr = "future"; break;
		case SimPhase::INVEST: phasestr = "invest"; break;
		case SimPhase::LAND: phasestr = "land"; break;
		case SimPhase::PRODUCT: phasestr = "product"; break;
		default:phasestr = "impossible";
		}

		if ( mipcond ) {
			string indstr;
			if (g->tPhase == SimPhase::LAND ){
				int r = g->mapMIP[tuple(g->tIter, g->tFarmId, SimPhase::LAND)]++;
				indstr = to_string(r);// g->tInd_land);
				//++g->tInd_land;
			}else if (g->tPhase == SimPhase::FUTURE) {
				int r = g->mapMIP[tuple(g->tIter, g->tFarmId, SimPhase::FUTURE)]++;
				indstr = to_string(r);// g->tInd_future);
				//++g->tInd_future;
			}

			stringstream ts;
			string debdir = "DebMIPs\\";
		
			string tinputdir;
			tinputdir=rtrim(inputdir, '\\');
				
			fs::path inpdir(tinputdir);
			fs::path pdir = inpdir.parent_path();
			string dstr = pdir.string() + "\\" + debdir;
			fs::path ddir(dstr);
			if (!fs::exists(ddir))
				fs::create_directories(ddir);
			ts << dstr;
			
			ts << "It_" << g->tIter << "_Id_" << g->tFarmId << "_"
				<< g->tFarmName << "_" << phasestr;
			if (indstr.length()>0)
				ts << "_" << indstr << ".txt";
			else
				ts << ".txt";

			debug(ts.str().c_str());
		}
	}
#endif

	/*if ((g->tIter >=0) && (g->tIter <2 )) {
	stringstream ts ;
	string debdir = "DEB\\";
	ts << inputdir << debdir << g->tIter << "_" << g->tFarmId << "_" << g->tFarmName << "_" << g->tInd << ".txt" ;

		  debug(ts.str().c_str());
		  g->tInd++;
	}
	//*/

	if (g->SDEBUG1 || g->SDEBUG2) {
		stringstream sstr;
		sstr << g->tInd;
		string str = sstr.str();
		if (g->SDEBUG1)
			debug("debug-r" + str + ".txt");
		if (g->SDEBUG2)
			debug("debug-n" + str + ".txt");
	}

	bool deb = false;
	
	if (deb)
        debug("ldebug2.txt");
    return objval;
} 

#ifdef GNU_SOLVER
#define MAX_NUM 20000
void RegLpInfo::glp_solve(){
	glp_prob *glp;
	glp_smcp cparm;
	glp_init_smcp(&cparm);
	cparm.msg_lev=GLP_MSG_OFF;
	cparm.tm_lim=60000; //milliseconds

	int ia[1+MAX_NUM], ja[1+MAX_NUM];  // ? globale constant 
	double ar[1+MAX_NUM]; //array of the non-zero coefficients 

	glp=glp_create_prob();
	glp_set_prob_name(glp,"Agent");
	glp_set_obj_dir(glp,GLP_MAX);

	glp_add_rows(glp, numrows);

    for (int i=1;i<numrows+1;i++) {
		switch (sense[i-1]) {
		case 'L': 
			 glp_set_row_bnds(glp, i, GLP_UP, 0.0, rhs[i-1]); //auxiliary variables (rows) upper limits
			break;
		case 'G': 
			 glp_set_row_bnds(glp, i, GLP_LO, rhs[i-1], 0.0); //auxiliary variables (rows) lower limits
			break;
		case 'E': 
			 glp_set_row_bnds(glp, i, GLP_FX, rhs[i-1], rhs[i-1]); //auxiliary variables (rows) fixed value
			break;
		default: cout << "sense not one of L, E, or G \n";
		}
    }

    // adding columns (variables) with bounds and obj coefficients:
    glp_add_cols(glp, numcols);
    for (int i=1;i<numcols+1;i++) {
        glp_set_obj_coef(glp, i, obj[i-1]);
        if (ub[i-1]==INFBOUND) {
            glp_set_col_bnds(glp, i, GLP_LO, lb[i-1], 0.0);
        } else {
            if (ub[i-1]==lb[i-1])
                glp_set_col_bnds(glp, i, GLP_FX, lb[i-1], ub[i-1]);
            else
                glp_set_col_bnds(glp, i, GLP_DB, lb[i-1], ub[i-1]);
        }
    }

    // creating the three arrays needed for loading matrix: rows, column, value
    // zero coefficients are not allowed
    int matrix_counter=1;
    int nz_counter=1;
    for (int j=1;j<numcols+1;j++) {
        for (int i=1;i<numrows+1;i++) {
            if (mat_val[matrix_counter-1] != 0) {
                ia[nz_counter] = i;
                ja[nz_counter] = j;
                ar[nz_counter] = mat_val[matrix_counter-1];
                nz_counter++;
                if (nz_counter>=MAX_NUM) throw 1; // exeption: more non-zeros than declared
            }
            matrix_counter++;
        }
    }

	// loading the constrain coefficient matrix..
    glp_load_matrix(glp, nz_counter-1, ia, ja, ar);

/*	// solving the problem with only continuous variables..(no pre-solver for mip)
    int st = glp_simplex(glp, &cparm);

	if (st) {  //st==0 optimal
		cout << "error during solving lp:" << st << "\n";
		debug("gdebug1.txt");
	}
//*/
	// setting integer parameters
    for (int i=0;i<numcols;i++) {
        if (ctype[i] == 'I') {
            glp_set_col_kind(glp, i+1, GLP_IV);
        }
    }
	
	//control parameters for mip solver
	glp_iocp iparm;
	glp_init_iocp(&iparm);
	iparm.msg_lev=GLP_MSG_OFF;
	iparm.presolve= GLP_ON;

	iparm.br_tech=GLP_BR_FFV;
	iparm.bt_tech=GLP_BT_BFS;
	
	iparm.pp_tech = GLP_PP_ROOT;
	iparm.fp_heur = GLP_ON;

	iparm.gmi_cuts=GLP_ON;
	iparm.mir_cuts=GLP_OFF;// ON;
	iparm.cov_cuts= GLP_ON;
	iparm.clq_cuts=GLP_ON;
	//*/
	//iparm.tol_int=1e-2;
	iparm.tm_lim = 60000; //milliseconds

	// solving mip
    int statt=glp_intopt(glp, &iparm);
	if (statt) {
		cout << "\tError while solving mip !" << endl;// << statt << "\n";
	}

	stat = glp_mip_status(glp);
	objval = glp_mip_obj_val(glp);
/*	if (stat!=GLP_OPT) 
		debug("gdebug2.txt");
//*/

	// retrive activity levels...
    for (int i=0; i<numcols; i++) {
        double temp= glp_mip_col_val(glp, i+1);
        x[i]=temp;
    }

	glp_delete_prob(glp);
	return ;
}

#undef MAX_NUM
#endif

RegLinkObject* RegLpInfo::mklink(onelink& lk, int dn, int dk) {
    int vk;
    string skString= lk.linktype;
    std::transform(skString.begin(), skString.end(), skString.begin(),
               (int(*)(int)) std::toupper);
    if (skString=="MARKET") {
		int sn = marketId[lk.numbertype];
        string vkString= lk.valuetype;
        std::transform(vkString.begin(), vkString.end(), vkString.begin(),
               (int(*)(int)) std::toupper);
        if (vkString=="C")
            vk=0;
        if (vkString=="P")
            vk=1;
        if (vkString=="PE")
            vk=2;
        if (vkString=="GM")
            vk=3;

		double f = lk.factor;
        RegLinkMarketObject *link = new RegLinkMarketObject(dn,dk,sn,vk,f);
        market_links.push_back(link);
        return link;
    }
    if (skString=="INVEST") {
        int sn;
        if (dk == 1) //capacity links
           sn = atoi(lk.numbertype.c_str());
        else {
			sn = investId[lk.numbertype];
		}
		string vkString= lk.valuetype;
        std::transform(vkString.begin(), vkString.end(), vkString.begin(),
               (int(*)(int)) std::toupper);
        if (vkString=="CAP")
            vk=0;
        if (vkString=="LE")
            vk=1;
        if (vkString=="BE")
            vk=2;
        if (vkString=="AC")
            vk=3;
        if (vkString=="NORMCAP")
            vk=4;
        double f = lk.factor;
        RegLinkInvestObject *link =  new RegLinkInvestObject(dn,dk,sn,vk,f);
        invest_links.push_back(link);
        return link;
    }
    if (skString=="REFERENCE") {
        string vkString= lk.valuetype;
        std::transform(vkString.begin(), vkString.end(), vkString.begin(),
               (int(*)(int)) std::toupper);
        if (vkString=="LIQUIDITY")
            vk=0;
        if (vkString=="MILK")
            vk=1;
        if (vkString=="LAND")
            vk=2;
        if (vkString=="LABOUR")
            vk=3;
        if (vkString=="FINANCIALRULE")
            vk=4;
        if (vkString=="ARABLELAND")
            vk=5;
        if (vkString=="GRASSLAND")
            vk=6;
        if (vkString=="INCOMEPAY")
            vk=7;
        if (vkString=="UNMODINCOMEPAY")
            vk=8;
        if (vkString=="TRANCH1WIDTH")
            vk=9;
        if (vkString=="TRANCH2WIDTH")
            vk=10;
        if (vkString=="TRANCH3WIDTH")
            vk=11;
        if (vkString=="TRANCH4WIDTH")
            vk=12;
        if (vkString=="TRANCH5WIDTH")
            vk=13;
        if (vkString=="TRANCH1DEG")
            vk=14;
        if (vkString=="TRANCH2DEG")
            vk=15;
        if (vkString=="TRANCH3DEG")
            vk=16;
        if (vkString=="TRANCH4DEG")
            vk=17;
        if (vkString=="TRANCH5DEG")
            vk=18;

        //labour substitution
        if (vkString=="LABOUR_SUBSTITUTION")
            vk=19;

		//LUcapacity for re-invest
		if (vkString == "REINVESTLUCAP")
			vk = 20;

		//young farmer
		if (vkString == "HAPAYYOUNGFARMER")
			vk = 21;
		if (vkString == "MAXPAYYOUNGFARMER")
			vk = 22;
		if (vkString == "YOUNGFARMERYEARS")
			vk = 23;

        double f = lk.factor;
        RegLinkReferenceObject *link =   new RegLinkReferenceObject(dn,dk,vk,f);
        reference_links.push_back(link);
        if (vk==7 || vk==8)
            incomepay_links.push_back(link);
        return link;
    }
    if (skString=="NUMBER") {
        double v = atof(lk.numbertype.c_str());
        RegLinkNumberObject *link =  new RegLinkNumberObject(dn,dk,v);
        number_links.push_back(link);
        return link;
    }
    if (skString=="LAND") {
        string vkString= lk.valuetype;
        int i=0;
        for (i=0;i<g->NO_OF_SOIL_TYPES;i++)  {
            if (vkString==g->NAMES_OF_SOIL_TYPES[i])
                break;
        }
        double f = lk.factor;
        RegLinkLandObject *link = new RegLinkLandObject(dn,dk,i,f);
        land_links.push_back(link);
        return link;
    }

  if (g->HAS_SOILSERVICE) {
	if (skString=="YIELD") {
		string vkString= lk.valuetype;
		std::transform(vkString.begin(), vkString.end(), vkString.begin(),
               (int(*)(int)) std::toupper);
        int sn = marketId[lk.numbertype];

		if (vkString=="SY")
            vk=0;
        if (vkString=="P")
            vk=1;
        if (vkString=="K")
            vk=2;
        if (vkString=="PE")
            vk=3;
		if (vkString=="ENV")
            vk=4;
		if (vkString=="SN")
			vk=5;
		
        double f = lk.factor;
        RegLinkYieldObject *link = new RegLinkYieldObject(dn,dk,sn,vk,f);
        yield_links.push_back(link);
        return link;
    }
  }
    return NULL;
}

void RegLpInfo::stdMatLink() {
    int dk = 0; //matrix link
    onelink lk;
    for (int i=0; i<2; i++) {   //0 liquidity 1 financing_rule
        int r = i;
        /*
		map<string,int>::iterator it= marketind.begin();
        int k= marketind.size();
        for (int j = 0 ; j< k; j++) {
            int c =  colindex[(*it).first];
            if (linkmat.find(pair<int,int>(r,c))!=linkmat.end()) {
                it++; continue;
            }
            lk.linktype="MARKET";
            lk.numbertype= (*it).first;
            lk.valuetype="C";
            lk.factor= matlinkdata.mark_fac[(*it).first];

            mat_links.push_back(mklink(lk, numrows*c+r,dk));
            it++;
        }
		//*/
        int sz = matrixdata.colnames.size();
        int hli = colindex["HIREDLAB"];
        int ofli = colindex["OFFFARMLAB"];
        for (int j = 0 ; j< sz ; j++) {
            // must be an invest
            if  (investId.find(matrixdata.colnames[j])==investId.end()){
                continue;
            }
            int c =  j;
            // voher noch nicht gelinkt
            if (linkmat.find(pair<int,int>(r,c))!=linkmat.end()) continue;
            lk.linktype="INVEST";
            lk.numbertype= matrixdata.colnames[j];
            if (i==0) lk.valuetype="BE";
            else lk.valuetype="LE";
            lk.factor= 1;
            if ( ( hli==j )   || (ofli==j))
                lk.factor = 0;

            mat_links.push_back(mklink(lk, numrows*c+r,dk));
        }
    }
    return;
}

void RegLpInfo::stdObjLink() {
    int dk=2 ; //obj link
    int dn;
    onelink lk;
    int sz = matrixdata.colnames.size();
    for (int i = 0; i < sz; i++) {
        if (linkobj.find(i)!=linkobj.end()) continue;
        if (marketId.find(matrixdata.colnames[i])!=marketId.end()) {
            lk.linktype="MARKET";
            lk.numbertype=matrixdata.colnames[i];
            lk.valuetype="GM";
            lk.factor=1;
        }
        else if (investId.find(matrixdata.colnames[i])!=investId.end()) {
            lk.linktype="INVEST";
            lk.numbertype=matrixdata.colnames[i];
            lk.valuetype="AC";
            lk.factor=-1;
        }
        else {
            cout << "UNKNOWN COL FOR OBJ FUNC LINKS" << endl;
            exit(2);
        }
        dn= i;
        obj_links.push_back(mklink(lk,dn,dk));
    }
    return;
}

RegLinkObject*
RegLpInfo::readLink(int dn,int dk) {
    onelink lk;
    int r, c;
    switch (dk) {
    case 0:     //matrixlink
                r = rowindex[matlinkdata.matlinks[dn].row];
                c = colindex[matlinkdata.matlinks[dn].col];
                lk.linktype=matlinkdata.matlinks[dn].linktype;
                lk.numbertype=matlinkdata.matlinks[dn].col;// numbertype;
                lk.valuetype=matlinkdata.matlinks[dn].valuetype;
                lk.factor=matlinkdata.matlinks[dn].factor;
                dn = c*numrows + r;
                break;
    case 1:     // capacitylink rhs
                lk = caplinkdata.caplinks[dn];
                break;
    case 2:     // obj. func. link
                lk= objlinkdata.objlinks[dn];
				//lk.numbertype=lk.name;
                dn = colindex[lk.name];
                break;
    default: ;
    }

    return mklink(lk,dn,dk);
}


RegLpInfo*
RegLpInfo::create() {
    return new RegLpInfo();
}
RegLpInfo*
RegLpInfo::clone() {
    RegLpInfo* cl=new RegLpInfo(*this);
    cl->flat_copy=true;
    return cl;
}

RegLpInfo*
RegLpInfo::clone(RegGlobalsInfo* G) {
    RegLpInfo* n=create();
    (*n).g=G;
    //Um die Limks zu kopieren ist elementweises kopieren noetig
    for (unsigned i=0;i<invest_links.size();i++) {
        RegLinkInvestObject *link=new RegLinkInvestObject(*invest_links[i]);
        (*n).invest_links.push_back(link);
        switch (link->getDestKind()) {
        case 0:
            (*n).mat_links.push_back(link);
            break;
        case 1:
            (*n).cap_links.push_back(link);
            break;
        case 2:
            (*n).obj_links.push_back(link);
            break;
        default:
            break;
        }
    }
    for (unsigned i=0;i<market_links.size();i++) {
        RegLinkMarketObject *link=new RegLinkMarketObject(*market_links[i]);
        (*n).market_links.push_back(link);
        switch (link->getDestKind()) {
        case 0:
            (*n).mat_links.push_back(link);
            break;
        case 1:
            (*n).cap_links.push_back(link);
            break;
        case 2:
            (*n).obj_links.push_back(link);
            break;
        default:
            break;
        }
    }

	//soil service yield links 
	for (unsigned i=0;i<yield_links.size();i++) {
        RegLinkYieldObject *link=new RegLinkYieldObject(*yield_links[i]);
        (*n).yield_links.push_back(link);
        switch (link->getDestKind()) {
        case 0:
            (*n).mat_links.push_back(link);
            break;
        case 1:
            (*n).cap_links.push_back(link);
            break;
        case 2:
            (*n).obj_links.push_back(link);
            break;
        default:
            break;
        }
    }
    for (unsigned i=0;i<land_links.size();i++) {
        RegLinkLandObject *link=new RegLinkLandObject(*land_links[i]);
        (*n).land_links.push_back(link);
        switch (link->getDestKind()) {
        case 0:
            (*n).mat_links.push_back(link);
            break;
        case 1:
            (*n).cap_links.push_back(link);
            break;
        case 2:
            (*n).obj_links.push_back(link);
            break;
        default:
            break;
        }
    }
    for (unsigned i=0;i<reference_links.size();i++) {
        RegLinkReferenceObject *link=new RegLinkReferenceObject(*reference_links[i]);
        (*n).reference_links.push_back(link);
        switch (link->getDestKind()) {
        case 0:
            (*n).mat_links.push_back(link);
            break;
        case 1:
            (*n).cap_links.push_back(link);
            break;
        case 2:
            (*n).obj_links.push_back(link);
            break;
        default:
            break;
        }
        if (link->getSourceNumber()==7 || link->getSourceNumber()==8)
            (*n).incomepay_links.push_back(link);
    }
    for (unsigned i=0;i<number_links.size();i++) {
        RegLinkNumberObject *link=new RegLinkNumberObject(*number_links[i]);
        (*n).number_links.push_back(link);
        switch (link->getDestKind()) {
        case 0:
            (*n).mat_links.push_back(link);
            break;
        case 1:
            (*n).cap_links.push_back(link);
            break;
        case 2:
            (*n).obj_links.push_back(link);
            break;
        default:
            break;
        }
    }
    (*n).numcols=numcols;
    (*n).numrows=numrows;
    (*n).prodcols=prodcols;
    (*n).mat_val.resize(numcols*numrows);
    (*n).nzspace=nzspace;
    for (int i = 0; i < numcols*numrows; i++)
        (*n).mat_val[i] = mat_val[i];
    (*n).rhs.resize(numrows);         // capacity values
    (*n).obj.resize(numcols);         // array with objective function coefficients
    (*n).sense.resize(numrows);         // array containing the sense of each constraint
    (*n).lb.resize(numcols);          // array containing the lower bound on each variable
    (*n).ub.resize(numcols);          // array containing the upper bounds each variable
    (*n).x.resize(numcols);           // array that contains the optimal values of the
    // primal variables
    (*n).ctype.resize(numcols);         // array indicating the type of variables in the
    // array indicating the type of variables in the
    for (int i = 0; i < numrows; i++) {
        (*n).sense[i]= sense[i];
    }
    // activities after prodcols are integer activities
    for (int i = 0; i < numcols; i++) {
        (*n).ctype[i] = ctype[i];
    }
    // set Bounds
    for (int i = 0; i < numcols; i++) {
        (*n).lb[i] = lb[i];
        (*n).ub[i] = ub[i];
    }
    // set to min/max problem
    (*n).objsen=objsen;
    //debug("aftercopy.txt");
    return n;
}

void
RegLpInfo::initLinks(RegFarmInfo*f,
                     RegInvestList* il,
                     RegProductList* pl,
                     RegLabourInfo *lab,
                     double* liq,
                     double* lnd,
                     double* milk,
                     double* finrule,
                     double* incpay,
                     double* unmodincpay,
                     double* tranch1width,
                     double* tranch2width,
                     double* tranch3width,
                     double* tranch4width,
                     double* tranch5width,
                     double* tranch1deg,
                     double* tranch2deg,
                     double* tranch3deg,
                     double* tranch4deg,
                     double* tranch5deg) {
	farm = f;

    for (unsigned i=0;i<invest_links.size();i++) {
        switch (invest_links[i]->getDestKind()) {
        case 0:
            invest_links[i]->init(il,&(*mat_val.begin()));
            break;
        case 1:
            invest_links[i]->init(il,&(*rhs.begin()));
            break;
        case 2:
            invest_links[i]->init(il,&(*obj.begin()));
            break;
        default:
            break;
        }
    }
    for (unsigned i=0;i<market_links.size();i++) {
        switch (market_links[i]->getDestKind()) {
        case 0:
            market_links[i]->init(pl,&(*mat_val.begin()));
            break;
        case 1:
            market_links[i]->init(pl,&(*rhs.begin()));
            break;
        case 2:
            market_links[i]->init(pl,&(*obj.begin()));
            break;
        default:
            break;
        }
    }
    for (unsigned i=0;i<land_links.size();i++) {
        switch (land_links[i]->getDestKind()) {
        case 0:
            land_links[i]->init(f,&(*mat_val.begin()));
            break;
        case 1:
            land_links[i]->init(f,&(*rhs.begin()));
            break;
        case 2:
            land_links[i]->init(f,&(*obj.begin()));
            break;
        default:
            break;
        }
    }

	//soil service
	for (unsigned i=0;i<yield_links.size();i++) {
        switch (yield_links[i]->getDestKind()) {
        case 0:
            yield_links[i]->init(f,&(*mat_val.begin()));
            break;
        case 1:
            yield_links[i]->init(f,&(*rhs.begin()));
            break;
        case 2:
            yield_links[i]->init(f,&(*obj.begin()));
            break;
        default:
            break;
        }
    }

    for (unsigned i=0;i<reference_links.size();i++) {
        double* source;
        switch (reference_links[i]->getSourceNumber()) {
        case 0:
            source = liq;
            break;
        case 1:
            source = milk;
            break;
        case 2:
            source = lnd;
            break;
        case 3:
            source = &lab->labour_capacity;
            break;
        case 4:
            source = finrule;
            break;
        case 7:
            source = incpay;
            break;
        case 8:
            source = unmodincpay;
            break;
        case 9:
            source = tranch1width;
            break;
        case 10:
            source = tranch2width;
            break;
        case 11:
            source = tranch3width;
            break;
        case 12:
            source = tranch4width;
            break;
        case 13:
            source = tranch5width;
            break;
        case 14:
            source = tranch1deg;
            break;
        case 15:
            source = tranch2deg;
            break;
        case 16:
            source = tranch3deg;
            break;
        case 17:
            source = tranch4deg;
            break;
        case 18:
            source = tranch5deg;
            break;
       case 19:
            source = &il->labSubstitution;
            break ;
	   case 20:
		   source = &farm->getRefReinvestLUcap();
		   break;
	   case 21:
		   source = &g->YF_priceHa;
		   break;
	   case 22:
		   source = &g->YF_maxPay;
		   break;
	   case 23:
		   source = &farm->refYoungFarmerYears();
		   break;
        default:
            break;
        }

        switch (reference_links[i]->getDestKind()) {
        case 0:
            reference_links[i]->init(source,&(*mat_val.begin()));
            break;
        case 1:
            reference_links[i]->init(source,&(*rhs.begin()));
            break;
        case 2:
            reference_links[i]->init(source,&(*obj.begin()));
            break;
        default:
            break;
        }
    }
    for (unsigned i=0;i<number_links.size();i++) {
        switch (number_links[i]->getDestKind()) {
        case 0:
            number_links[i]->init(&(*mat_val.begin()));
            break;
        case 1:
            number_links[i]->init(&(*rhs.begin()));
            break;
        case 2:
            number_links[i]->init(&(*obj.begin()));
            break;
        default:
            break;
        }
    }
}

double
RegLpInfo::LpWithPrice(RegProductList* PList, vector<int >& ninv, int maxofffarmlu) {
    bool prod = false;
    if (PList->setUsePriceExpectation(false)) { // hier war der Fehler
        for (unsigned i=0;i<market_links.size();i++)
            market_links[i]->trigger();
    }
    return Lp(PList,ninv,prod,maxofffarmlu);
}
double // LP for investment and other LPs
RegLpInfo::LpWithPriceExpectation(RegProductList* PList, vector<int >& ninv, int maxofffarmlu) {
    bool prod = false;
    if (PList->setUsePriceExpectation(true)) { // hier war der Fehler
        for (unsigned i=0;i<market_links.size();i++)
            market_links[i]->trigger();
    }
    return Lp(PList,ninv, prod, maxofffarmlu);
}
double  // LP for production
RegLpInfo::LpProdPriceExpectation(RegProductList* PList, vector<int >& ninv, int maxofffarmlu) {
    bool prod = true;
    if (PList->setUsePriceExpectation(true)) { // hier war der Fehler
        for (unsigned i=0;i<market_links.size();i++)
            market_links[i]->trigger();
    }
    return Lp(PList,ninv, prod,maxofffarmlu);
}

bool
RegLpInfo::changeMatrix(int nel ,int* indexRow, int* indexCol, double* val) {
    for (int i=0;i<nel;i++) {
        int index=numrows*indexCol[i]+indexRow[i];
        if (mat_val[index]==0 && val[i]!=0)
            nzspace--;
        if (mat_val[index]!=0 && val[i]==0)
            nzspace++;
        mat_val[index]=val[i];
    }
    return true;
}

void
RegLpInfo::setCellValue(int c,int r,double val) {

    changeMatrix(1,&r,&c,&val);
}

void
RegLpInfo::setSenseLessEqual(int row) {
    if (row>=0 && row < numrows) {
        sense[row]='L';
    }
}
void
RegLpInfo::setSenseEqual(int row) {
    if (row>=0 && row < numrows) {
        sense[row]='E';
    }
}
void
RegLpInfo::setSenseGreaterEqual(int row) {
    if (row>=0 && row < numrows) {
        sense[row]='G';
    }
}    // Change bound
void
RegLpInfo::setUBoundZero(int col) {
    if (col>=0 && col<numcols) {
        ub[col]= 0;
    }
}
void
RegLpInfo::setUBoundInf(int col) {
    if (col>=0 && col<numcols) {
        ub[col]= +1E30;
    }
}

void RegLpInfo::setUBound(int col, int val){
    if (col<numcols) {
        ub[col] = val;
	}
}

void
RegLpInfo::setLBoundZero(int col) {
	if (col >= 0 && col<numcols) {
		lb[col] = 0;
	}
}

void RegLpInfo::setLBound(int col, int val) {
	if (col >= 0 && col<numcols) {
		lb[col] = val;
	}
}

void
RegLpInfo::backup() {
    obj_backup=clone();
}
void
RegLpInfo::restore() {
    RegLpInfo* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
}

#ifndef FRONTMIPISINSTALLED

double
RegLpInfo::LpGlpk(RegProductList* PList, vector<int >& ninv, bool prod, int maxofffarmlu ) {
    LPX *lpglpk;
    // ia[] -> rows; ja[]->columns;
    int ia[1+10000], ja[1+10000];  // nzspace must be lower than 10000
    double ar[1+10000]; //array of the coefficients
    double Z; //  objective variable


    lpglpk = lpx_create_prob();
    lpx_set_prob_name(lpglpk, "farmer");
    lpx_set_obj_dir(lpglpk, LPX_MAX);
    lpx_set_int_parm(lpglpk,LPX_K_MSGLEV,0);
    // adding rows and placing bounds:
    lpx_add_rows(lpglpk, numrows);
    for (int i=1;i<numrows+1;i++) {
        lpx_set_row_bnds(lpglpk, i, LPX_UP, 0.0, rhs[i-1]); //setting the constrain upper limits
    }

    // adding columns, placing bounds and obj coefficients:
    lpx_add_cols(lpglpk, numcols);
    for (int i=1;i<numcols+1;i++) {
        lpx_set_obj_coef(lpglpk, i, obj[i-1]);
        if (ub[i-1]==INFBOUND) {
            lpx_set_col_bnds(lpglpk, i, LPX_LO, lb[i-1], 0.0);
        } else {
            if (ub[i-1]==lb[i-1])
                lpx_set_col_bnds(lpglpk, i, LPX_FX, lb[i-1], ub[i-1]);
            else
                lpx_set_col_bnds(lpglpk, i, LPX_DB, lb[i-1], ub[i-1]);
        }
    }

    // creating the three arrays needed for lpx_load_matrix(): rows, column, value
    // zero coefficients are not allowed
    int matrix_counter=1;
    int nz_counter=1;
    for (int j=1;j<numcols+1;j++) {
        for (int i=1;i<numrows+1;i++) {
            if (mat_val[matrix_counter-1] != 0) {
                ia[nz_counter] = i;
                ja[nz_counter] = j;
                ar[nz_counter] = mat_val[matrix_counter-1];
                nz_counter++;
                if (nz_counter>=10000) throw 1; // exeption made if the matrix has more non-zero coefficients than the one declared
            }
            matrix_counter++;
        }
    }

// loading the constrain coefficient matrix..
    lpx_load_matrix(lpglpk, nz_counter-1, ia, ja, ar);
// solving the problem with only continuous variables..
    lpx_simplex(lpglpk);

// setting the problem as integer..
    lpx_set_class(lpglpk, LPX_MIP);

// setting integer parameters
    for (int i=0;i<numcols;i++) {
        if (ctype[i] == 'I') {
            lpx_set_col_kind(lpglpk, i+1, LPX_IV);
        }
    }

// solving the problem with the integer variables
    int statt=lpx_integer(lpglpk);

    Z = lpx_mip_obj_val(lpglpk);
// retrive activity levels...
    for (int i=0; i<numcols; i++) {
        double temp= lpx_mip_col_val(lpglpk, i+1);
        x[i]=temp;
    }
    objval=Z;
    lpx_delete_prob(lpglpk);
    return Z;

}
#endif


double
RegLpInfo::globalAllocation(list<RegFarmInfo*>& farms,RegRegionInfo* region, int iteration) {
    vector<int> free_plots;
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++)
        free_plots.push_back(region->getFreeLandPlotsOfType(i));
    int total_free_plots=0;
    for (unsigned int i=0;i<free_plots.size();i++)
        total_free_plots+=free_plots[i];

    int test=0;
    int non_zero_inc=0;
    for (unsigned int i=0;i<region->plots.size();i++) {
        if (region->plots[i]->getState()==0) {
            test++;
                if (region->plots[i]->getPaymentEntitlement()!=0) {
                non_zero_inc++;
                }
            }
    }
    //int no_farms;
    int non_zero=0;
    list<RegFarmInfo*>::iterator iter;
    int numrows=0;
    int numcols=0;
    for (iter=farms.begin();iter!=farms.end();iter++) {
        //(*iter)->getObjective();
        (*iter)->updateLpValues();
        for (unsigned int j=0;j<(*iter)->lp->mat_val.size();j++) {
            if ((*iter)->lp->mat_val[j]!=0)
                non_zero++;
        }
        numrows+=(*iter)->lp->numrows;
        numcols+=(*iter)->lp->numcols;
    }
    numcols+=farms.size()*total_free_plots;
    numrows+=total_free_plots;
    non_zero+=farms.size()*total_free_plots*2;
    non_zero+=non_zero_inc*farms.size();
    int* ia=new int[numcols];
    int* ja=new int[non_zero];

    double* ar=new double[non_zero]; //array of the coefficients
    double Z=-1; //  objective variable
    double *rhs=new double[numrows];
    int *rhs_sense=new int[numrows];
    double *rc=new double[numrows];
    double *ub=new double[numcols];
    double *lb=new double[numcols];
    double *obj=new double[numcols];
    double *x=new double[numcols];
    double *x_2=new double[numcols];
    int* ctype=new int[numcols];
    int c=0;
    for (iter=farms.begin();iter!=farms.end();iter++) {
        for (int j=0;j<(*iter)->lp->numrows;j++) {
            rhs[c]=(*iter)->lp->rhs[j]; //setting the constrain upper limits
            rhs_sense[c]=0;            // 0 for <=
            if((*iter)->lp->sense[j]=='L')  //setting the constrain upper limits
            rhs_sense[c]=0;            // 0 for <=
            if((*iter)->lp->sense[j]=='E')  //setting the constrain upper limits
            rhs_sense[c]=1;            // 0 for <=
            c++;
        }
    }
    for (unsigned int i=0;i<free_plots.size();i++) {
        for (int j=0;j<free_plots[i];j++) {

            rhs[c]=1;//0;//1; //setting the constrain upper limits
            rhs_sense[c]=0 ;  // 1 for =
            c++;
        }
    }
    // adding columns, placing bounds and obj coefficients:
    c=0;
    for (iter=farms.begin();iter!=farms.end();iter++) {
        for (int j=0;j<(*iter)->lp->numcols;j++) {
            if (j>=(*iter)->lp->prodcols)
                ub[c]=1E30;
            else
                ub[c]=(*iter)->lp->ub[j];

            c++;
        }
    }
    for (unsigned int i=0;i<total_free_plots*farms.size();i++) {
        ub[c]=+1E30;
        c++;
    }
// setting integer parameters
    c=0;
    for (iter=farms.begin();iter!=farms.end();iter++) {
        for (int j=0;j<(*iter)->lp->numcols;j++) {
            if((*iter)->lp->ctype[j]=='C')  //setting the constrain upper limits
            ctype[c]=0;            // 0 for <=
            if((*iter)->lp->ctype[j]=='I')  //setting the constrain upper limits
            ctype[c]=1;            // 0 for <=
            c++;
        }
    }

    for (int i=farms.size()*(*(farms.begin()))->lp->numcols;i<numcols;i++) {
        ctype[c]=2;  //§$%  2
        c++;
    }
#define ARROW 3
#define GRROW 4
    // creating the three arrays needed for lpx_load_matrix(): rows, column, value
    // zero coefficients are not allowed
    c=0;
    int col=0;
    int k=0;
    for (iter=farms.begin(),k=0;iter!=farms.end();iter++,k++) {
        string name=(*iter)->getFarmName() ;
        bool print=false;
        if(print) {
        (*iter)->getObjective();
        }

        int mc=0;
        for (int i=0;i<(*iter)->lp->numcols;i++) {
            ia[col]=c;
            for (int j=0;j<(*iter)->lp->numrows;j++) {
                if ((*iter)->lp->mat_val[mc]!=0) {
                    ja[c]=j+k*(*iter)->lp->numrows;
                    ar[c]=(*iter)->lp->mat_val[mc];
                    c++;
                }
                mc++;
            }
            col++;
        }
    }
    #define INCPAY 25
    int plotrow=(*(farms.begin()))->lp->numrows*farms.size();
    int sos_plotrows=plotrow;
    int plotcol=(*(farms.begin()))->lp->numcols*farms.size();
    int plll=0;
    for (unsigned int j=0;j<region->plots.size();j++) {

        int baserow=0;
        RegPlotInfo* pl=region->plots[j];
        if (pl->getState()==0) {
            plll++;
            if (pl->getSoilType()==0) baserow=ARROW;
            else baserow=GRROW;
            for (unsigned int i=0;i<farms.size();i++) {
                ia[col]=c;
                col++;
                ja[c]=baserow+i*(*(farms.begin()))->lp->numrows;
                ;
                ar[c]=-g->PLOT_SIZE;
                c++;
                if (pl->getPaymentEntitlement()!=0) {
                        ja[c]=INCPAY+i*(*(farms.begin()))->lp->numrows;
                        ar[c]=-pl->getPaymentEntitlement();
                        c++;
                }
                ja[c]=plotrow;
                ar[c]=1; //1
                c++;
                plotcol++;
            }
            plotrow++;
        }
    }

    // rows convert to
    vector< vector <double> > row_val;
    vector< vector <int> > col_ind;
    row_val.resize(numrows);
    col_ind.resize(numrows);
    for(int i=0;i<numcols;i++) {
        int begin=ia[i];
        int end;
        if(i<(numcols-1))
          end=ia[i+1];
        else
          end=non_zero;
        for(int j=begin;j<end;j++) {
          row_val[ja[j]].push_back(ar[j]);
          col_ind[ja[j]].push_back(i);
        }
    }

    c=0;
    for (iter=farms.begin();iter!=farms.end();iter++) {
        for (int j=0;j<(*iter)->lp->numcols;j++) {
            obj[c]=(*iter)->lp->obj[j];
            c++;
        }

    }
    c=farms.size()*(*(farms.begin()))->lp->numcols;
    for (unsigned int j=0;j<region->plots.size();j++) {
        if (region->plots[j]->getState()==0) {
            for (iter=farms.begin();iter!=farms.end();iter++) {
                double tk=region->plots[j]->calculateDistanceCosts((*iter)->farm_plot);
                obj[c]= -tk;
                c++;
            }
        }
    }
      ofstream out;
  stringstream file_o;
  stringstream file_i;
  file_o<<"test"<<iteration<<".lp";
  file_i<<"test"<<iteration<<".prt";
     out.open(file_o.str().c_str());
     out << "maximize\nobj:";
     for(int i=0;i<numcols;i++) {
        if(obj[i]!=0)
           out << printVar(obj[i],i).c_str();
        if(i%5==0) out <<"\n";
     }
     out << "\nsubject to\n";
     int soscount=0;
     bool SOS=false;
     for(int i=0;i<numrows;i++) {
       out << "r" << i << ": ";
       for(unsigned int j=0;j<row_val[i].size();j++) {
         out << printVar(row_val[i][j],col_ind[i][j]).c_str();
         if(j%5==0) out <<"\n";
       }
       if(SOS && i>=sos_plotrows) {
          out << " = S" << soscount <<"\n";
          soscount++;
        } else {
         if(rhs_sense[i]==0) out << " <= ";
         if(rhs_sense[i]==1) out << " = ";
         out << rhs[i] <<"\n";
        }
     }
     out << "bounds\n";
     for(int i=0;i<numcols;i++) {
        if(ub[i]!=1E30) {
         out << "x" << i << " <= " << ub[i] << "\n";
        }
     }
     out << "generals\n";
     for(int i=0;i<numcols;i++) {
        if(ctype[i]==1) {
         out << "x" << i << "\n";
        }
     }
     out << "binaries\n";
     for(int i=0;i<numcols;i++) {
        if(ctype[i]==2) {
         out << "x" << i << "\n";
        }
     }
    out << "end\n";
     out.close();

bool XPRESSMP=true;
if(XPRESSMP) {
if(iteration>5) {
string p1="optimizer " + file_i.str() + " @c:\\xpressmp\\param.txt";
system(p1.c_str());
}
ifstream in;
in.open(file_i.str().c_str(),ios::in);
string temp;
char* end;
for(int i=0;i<numcols;i++) {
        x[i]=0;
        x_2[i]=0;
}

while(!in.eof()) {
        in >> temp;
        if(temp=="C") {
                string s_num,s_col,s_exp,s_val;
               in>>s_num;
                in>>s_col;
                in>>s_exp;
                in>>s_val;
                s_col.erase(0,1);
                int col=static_cast<int>(strtod(s_col.c_str(),&end));
                double val=strtod(s_val.c_str(),&end);
                if (val==HUGE_VAL) {
                        int e=0;
                        e++;
                        e++;
                }

                x[col]=val;
//                if(col<alloc_cols)
//                  x_2[col]=val;
//                else
//                  x_2[col]=(int)(val+0.5);

         //       in >> x[ccol];
         //       ccol++;
        }

    /*
        if(temp=="\"C\",") {
                 double val;
                in>> val;
                x[ccol]=val;
                ccol++;

         //       in >> x[ccol];
        } */
}

in.close();
double sum=0;
double sum2=0;
for(int i=0;i<numcols;i++) {
        sum+=x[i]*obj[i];
        sum2+=x_2[i]*obj[i];

}
sum=sum;
}
// Define values for bundels.
    vector <vector <int> > allocation;
    vector<int> total_alloc;
    vector<double> tc;
    vector<double> value;
    vector<double> value_obj;

    allocation.resize(farms.size());
    tc.resize(farms.size());
    value.resize(farms.size());
    value_obj.resize(farms.size());
    total_alloc.resize(farms.size());
    for (unsigned int i=0;i<farms.size();i++) {
        tc[i]=0;
        value[i]=0;
        value_obj[i]=0;
        total_alloc[i]=0;
        allocation[i].resize(g->NO_OF_SOIL_TYPES);
        for (int j=0;j<g->NO_OF_SOIL_TYPES;j++) {
            allocation[i][j]=0;
        }
    }
    int count=0;
    int countf=0;
    for (iter=farms.begin();iter!=farms.end();iter++) {
        for (int j=0;j<(*iter)->lp->numcols;j++) {
         value_obj[countf]+=obj[count]*x[count];
         count++;
        }
        value_obj[countf]-=(*iter)->getObjective();
        countf++;
    }

    c=farms.size()*(*(farms.begin()))->lp->numcols;
    int r=farms.size()*(*(farms.begin()))->lp->numrows;
    for (unsigned int j=0;j<region->plots.size();j++) {
        if (region->plots[j]->getState()==0) {
            for (unsigned int i=0;i<farms.size();i++) {
                int test=(int)(x[c]+0.5);
                if (test==1) {
                    tc[i]+=obj[c];
                    allocation[i][region->plots[j]->getSoilType()]+=1;
                    total_alloc[i]+=1;
                }
                c++;
                r++;
            }
        }
    }

    double* x_val=new double[numcols];
    double* x_val_obj=new double[numcols];
    for(int i=0;i<numcols;i++) {
                 x_val[i]=0;
                 x_val_obj[i]=0;
    }
    c=farms.size()*(*(farms.begin()))->lp->numcols;
    r=farms.size()*(*(farms.begin()))->lp->numrows;
    for (unsigned int j=0;j<region->plots.size();j++) {
        if (region->plots[j]->getState()==0) {
            int f=0;
            for (iter=farms.begin();iter!=farms.end();iter++) {
                int test=(int)(x[c]+0.5);
                if (test==1) {
                    value[f]=(((*iter)->getValueOfPlots(allocation[f])/(double)total_alloc[f])+ obj[c])*g->RENT_ADJUST_COEFFICIENT;
                    x_val[c]=value[f];
                    x_val_obj[c]=((value_obj[f]/(double)total_alloc[f])+ obj[c])*g->RENT_ADJUST_COEFFICIENT;;
   
                     (*iter)->setRentedPlot(region->plots[j],x_val_obj[c],0);
                } else {
                  x_val[c]=0;
                  x_val_obj[c]=0;
                }
                c++;
                r++;
                f++;
            }
        }
    }
    stringstream file;
    file <<g->OUTPUTFILE<<free_plots[0]<<free_plots[1]<<".dat";
    ofstream out2;
    out2.open(file.str().c_str());
    out2 << numcols-farms.size()*(*(farms.begin()))->lp->numcols << "\n";
    for (int i=farms.size()*(*(farms.begin()))->lp->numcols;i<numcols;i++) {
        out2 << (int)(x[i]+0.5) << "\t"<< x_val[i] << "\t"<< x_val_obj[i]<< "\n";
    }
    out2.close();

    delete ia;
    delete ja;
    delete ar;
    delete rhs;
    delete ub;
    delete lb;
    delete rc;
   delete x;
   delete x_val;
   delete x_val_obj;
    delete obj;
    delete ctype;
    return Z;
}

void
RegLpInfo::globalAllocationFromFile(list<RegFarmInfo*>& farms,RegRegionInfo* region, string file) {
    int n;
     list<RegFarmInfo*>::iterator iter;
    ifstream in;
    in.open(file.c_str(),ios::in);
    in >> n;
    vector<int> x;
    vector<double> x_val;
    x.resize(n);
    x_val.resize(n);
    for (int i=0;i<n;i++) {
      in >> x[i];
    }
      for (int i=0;i<n;i++) {
      in >> x_val[i];
    }

//DCX
in.close();

  int testc=0;
    for (unsigned int j=0;j<region->plots.size();j++) {
        if (region->plots[j]->getState()==0) {
            for (iter=farms.begin();iter!=farms.end();iter++) {
              testc++;
              }
        }
    }

    int c=0;
    for (unsigned int j=0;j<region->plots.size();j++) {
        if (region->plots[j]->getState()==0) {
            for (iter=farms.begin();iter!=farms.end();iter++) {
                int test=(int)(x[c]+0.5);
                if (test==1) {
                    (*iter)->setRentedPlot(region->plots[j],x_val[c],0);
                }
                c++;
            }
        }
    }
}
