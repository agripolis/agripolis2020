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

// RegManager.cpp
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <cassert>

#include "RegManager.h"
#include "RegLpD.h"
#include "RegStructure.h"
#include "RegFarm.h"
#include "RegPlot.h"
#include "RegRL.h"

#include <iterator>
#include <regex>

#include "textinput.h"
#include "random.h"

using namespace std;
map<int,int> manageCoeffMap;
map<int, int> betaMap;

long int mtRandMin = 0 ;
long int mtRandMax= 0x7FFFFFFFUL ;

 struct farmsdata farmsdata  ;
 map <string, farminvestdata > farmsIinvest;

 struct globdata globdata;
 struct transdata transdata  ;
 struct matrixdata matrixdata  ;
 struct matrixdata_n matrixdata_n  ;
 struct matrixdata_new matrixdata_new  ;

 struct objlinkdata objlinkdata   ;
 struct caplinkdata caplinkdata   ;
 struct matlinkdata   matlinkdata   ;

 struct envmarketdata  envmarketdata    ;
 struct investdata investdata     ;
 struct marketdata marketdata    ;
 struct envdata  envdata       ;

 vector <oneyield> yielddata;

 //RL
 extern bool newIteration;
 //extern vector<int> newRentedplots;
 extern vector<double> recentRents; //farm
 extern vector<double> avRecentRents; //region

//soil service 
void RegManagerInfo::UpdateSoilserviceP(){
	if (g->HAS_SOILSERVICE) {
		list<RegFarmInfo* >::iterator farms_iter;
		for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end(); farms_iter++) {
 				(*farms_iter)->calDeltaCarbons();
				(*farms_iter)->updateCarbons();
		}
	}
	return;
}

void RegManagerInfo::UpdateSoilserviceLA(){
	if (g->HAS_SOILSERVICE) {
		list<RegFarmInfo* >::iterator farms_iter;
		for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end(); farms_iter++) {
				for (int i=0; i<g->NO_OF_SOIL_TYPES; ++i){
					if ((*farms_iter)->getNewRentedLandOfType(i) !=0)  {
						(*farms_iter)->calAvCarbons();
						break;
					}
				}
		}
	}
	return;
}

void RegManagerInfo::output_av_rents() {
	int n = g->NO_OF_SOIL_TYPES;
	for (int i = 0; i < n; ++i) {
		cout << Region->getAvRentOfType(i) << "\t";
	}
	cout << "\n";
}

RegManagerInfo::RegManagerInfo(RegGlobalsInfo* G)
        : g(G) {
    obj_backup=NULL;
    flat_copy= false;
    name="0";

	nfarms_restrict_invest = 0;

    // thread priority
    iteration = 0;
    evaluator= new Evaluator("policy_switching");
    randInit(g->SEED);
}

RegManagerInfo*
RegManagerInfo::clone(string name) {
    RegManagerInfo *n=create();
    (*n).name=name;
    (*n).iteration=iteration;
    (*n).g=g->clone();
    (*n).Sector=new RegSectorResultsInfo(*Sector,(*n).g);
    if (g->CALC_LEGAL_TYPES) {
        for (unsigned int i=0;i<g->LEGAL_TYPES.size();i++) {
            (*n).sector_type.push_back(new RegSectorResultsInfo(*(sector_type[i]),(*n).g));
        }
    }

    (*n).Env=new RegEnvInfo(*Env,(*n).g);
    (*n).Region=new RegRegionInfo(*Region,(*n).g);
	(*n).Data=new RegDataInfo((*n).g, (*n).Market, (*n).Region);
    (*n).Market=new RegMarketInfo(*Market,(*n).g);
    (*n).Mip= Mip->clone((*n).g);
    (*n).Policyoutput=new OutputControl(*Policyoutput,(*n).g);
    (*n).evaluator=new Evaluator(*evaluator);
    for (unsigned int i=0;i<InvestCatalog.size();i++) {
        (*n).InvestCatalog.push_back(InvestCatalog[i]);
    }
    (*n).FarmList.clear();
    (*n).RemovedFarmList.clear();
    list<RegFarmInfo* >::const_iterator farms;
    for (farms = FarmList.begin();
            farms != FarmList.end();
            farms++) {
        RegFarmInfo* tmp=(*farms)->clone((*n).g,(*n).Region,(*n).Market->getProductCat(),(*n).InvestCatalog);
        (*n).FarmList.push_back(tmp);
    }
    for (farms = RemovedFarmList.begin();
            farms != RemovedFarmList.end();
            farms++) {
        RegFarmInfo* tmp=(*farms)->clone((*n).g,(*n).Region,(*n).Market->getProductCat(),(*n).InvestCatalog);
        (*n).RemovedFarmList.push_back(tmp);
    }
    return n;
}

RegManagerInfo::~RegManagerInfo() {
    list<RegFarmInfo* >::iterator farms;
    if (!flat_copy) {
        for (farms = FarmList.begin();
                farms != FarmList.end();
                farms++) {
            delete (*farms);
            (*farms) = NULL;
        }

        for (farms = RemovedFarmList.begin();
                farms != RemovedFarmList.end();
                farms++) {
            delete (*farms);
            (*farms) = NULL;
        }
        delete Env;
        delete Market;
        delete Region;
        delete Sector;
        delete Data;
        delete Policyoutput;
        delete evaluator;
        delete Mip;
    }
    if (g->CALC_LEGAL_TYPES) {
        for (unsigned int i=0;i<g->LEGAL_TYPES.size();i++) {
            delete sector_type[i];
        }
    }
    RemovedFarmList.clear();
    FarmList.clear();
    if (obj_backup) delete obj_backup;

}
//---------------------------------------------------------------------------
//      INITIALISATION OF SIMULATION
//---------------------------------------------------------------------------

void RegManagerInfo::init() {
	debug = false;
    f=t=n=0;
    Policyoutput = new OutputControl(g);

	readfiles(g->INPUTFILEdir.c_str(), g->HAS_SOILSERVICE);
	randInit(g->SEED);
 
	// create market and pass globals
    Market = new RegMarketInfo(g);

	initGlobals(true);
	initCommandlineOptions();
	initGlobals(false);
    if (g->RL_training) {
        g->FARMOUTPUT = 0;
        g->SECTOROUTPUT = 0;
        g->INIT_OUTPUT = false;
    }

	Sector = new RegSectorResultsInfo(g,0);
    if (g->CALC_LEGAL_TYPES) {
        for (unsigned int i=0;i<g->LEGAL_TYPES.size();i++) {
            sector_type.push_back(new RegSectorResultsInfo(g,g->LEGAL_TYPES[i]));
        }
    }

    // create Region an pass graphics handle
   	initRandomDistribs();
    Region = new RegRegionInfo(g);
    initRegion();

    // create output files and pass globals
    Data = new RegDataInfo(g, Market, Region);

    Env=new RegEnvInfo(g);
    initEnv();
    initMarket();
    initEnv2();
    initGlobals2();

    // investmen objects create themselves and are stored in InvestCatalog vector
    initInvestmentCatalog();
    initMatrix0();
    initMatrix();

    setPremiumColRow();
	
	initPopulations(); 
	//outputManageCoeffDistrib();
    //if (g->Rent_Variation)
    //  outputMap(betaMap);

    initOutput();
	
	readPolicyChanges0();
	setIncreasePrices();

	//Modulation data also for the very first iteration
	//setModulationData();

    setLpChangesFromPoliySettingsNaming();

	Region->calcMaxRents();

	//testLivestockInvRand();
    if (g->Rent_Variation) {
        Data->initPrintPlots(FarmList);
        Data->printFarmSteads(FarmList);
    }

    cout << "Initialized " << endl;
}
void RegManagerInfo::testLivestockInvRand() {
	
	for (int t = 100; t < 1000000; t*=10) {
		int num = 0;
		for (int x = 0; x < t; ++x) {
			if (g->getRandomReal("LIVESTOCK_INV", g->uni_real_distrib_livestock_inv) < 0.1)
				++num;
		}
		printf("%d\t of \t%d\n", num, t);
	}

}

void RegManagerInfo::initRandomDistribs() {
	g->uni_int_distrib_contractLengthInit.param(uniform_int_distribution<>::param_type
		{ 1, g->MAX_CONTRACT_LENGTH - 1 });
	g->uni_int_distrib_contractLength.param(uniform_int_distribution<>::param_type
		{ g->MIN_CONTRACT_LENGTH, g->MAX_CONTRACT_LENGTH - 1 });
	g->uni_int_distrib_freePlot_initLand.param(uniform_int_distribution<>::param_type
		{ 0,g->NO_ROWS*g->NO_COLS - 1 });
	g->uni_int_distrib_freePlot_rentPlot.param(uniform_int_distribution<>::param_type
		{ 0,g->NO_ROWS*g->NO_COLS - 1 });
	g->uni_real_distrib_livestock_inv.param(uniform_real_distribution<>::param_type{0, 1});
}

void
RegManagerInfo::initGlobals(bool read=true) {
    if (read) g->initGlobalsRead();
	else g->initGlobals();
}

void RegManagerInfo::initCommandlineOptions() {
	map<int*, int> ops = g->options;
	for (auto item:ops) 	{
		g->setOption(item.first, item.second);
	}
}

void RegManagerInfo::setIncreasePrices(){
	int ind = evaluator->indOfVariable("V_HIRED_LABOUR_H_price_change");
	if (ind >= 0)
		g->IncPriceHiredLab= evaluator->getVariable(ind);//("V_HIRED_LABOUR_H_price_change");
	ind = evaluator->indOfVariable("V_OFF_FARM_LAB_price_change");
	if (ind >= 0)
		g->IncPriceOffFarmLab = evaluator->getVariable(ind);//"V_OFF_FARM_LAB_price_change");
};


void
RegManagerInfo::initGlobals2() {
  // nummber für globals nachhole
    map <string,int> prodId;
    map <string, int> invId;

    for (unsigned int i=0; i< marketdata.products.size(); i++){
        prodId[marketdata.products[i].name]=i;
        string aname = marketdata.products[i].stdName;
		if (!aname.compare("-")) continue;
		else {
            //GETQUOTA etc can be appended with _SWEDEN _JOENKEPING for  priceFunction
			if (aname.find("GETQUOTA")!=string::npos) 
                g->stdNameIndexs["GETQUOTA"]=i;
			else if (aname.find("LETQUOTA")!=string::npos)
                g->stdNameIndexs["LETQUOTA"]=i;
			else if (aname.find("SELLCALF_SUCK")!=string::npos)
                g->stdNameIndexs["SELLCALF_SUCK"]=i;
			else if (aname.find("BUYCALF_SUCK")!=string::npos)
                g->stdNameIndexs["BUYCALF_SUCK"]=i;
			else
				g->stdNameIndexs[aname]=i;
		}
		//	g->stdNameIndexs[aname]=i;
    }
  
    for (unsigned int i=0; i< investdata.invests.size(); i++)
        invId[investdata.invests[i].name]=i;

	g->FIXED_OFFFARM_LAB =  invId[investdata.FixOffFarmLabName];//g->sFIXED_OFFFARM_LAB];
	g->FIXED_HIRED_LAB =  invId[investdata.FixHiredLabName];//g->sFIXED_HIRED_LAB];

	g->uni_int_distrib_farmAge.param(std::uniform_int_distribution<>::param_type{ 0,g->GENERATION_CHANGE });
}

void
RegManagerInfo::initEnv() {
    if (g->ENV_MODELING)
        Env->initEnv();
}

void RegManagerInfo::initEnv2(){
 // nummer  für environmentals nachhole
  if (g->ENV_MODELING) {
     vector<string> sacts;
     int sz = Env->sassociated_activities.size();
     for (int i = 0; i < sz; i++){
        sacts =  Env->sassociated_activities[i];
        int sz2 = sacts.size();
        vector<int> acts;

        for (int j = 0; j<sz2; j++){
            acts.push_back(Market->product_id[sacts[j]]);
        }
        Env->associated_activities.push_back(acts);
     }
  }
}

// initialise investment catalog
void
RegManagerInfo::initInvestmentCatalog() {
    RegInvestObjectInfo readInvest(g);
    // init investment catalog
    // read in values and store in list
    InvestCatalog=readInvest.initInvestmentCatalog();
    g->NUMBER_OF_INVESTS = InvestCatalog.size();
    for (int i=0;i<g->NUMBER_OF_INVESTS;i++) {
        if (InvestCatalog[i].getInvestType()==g->OFFFARMLABTYPE) {
            g->setOffFarmLabour(InvestCatalog[i].getAcquisitionCosts()*
                                ((double)g->MAX_H_LU/InvestCatalog[i].getLabourSubstitution()));
        }
        InvestCatalog[i].setBcInterest(g->INTEREST_RATE);
        InvestCatalog[i].setInterestReduction(0);

		string nm = InvestCatalog[i].getName();
		for (auto x : g->Livestock_Invs) {
			if (nm.rfind(x, 0) == 0) {
				g->AllRestrictInvs[nm] = 0;
			}
		}
    }
    // update values of investments which depend on interest rate
    updateInterestDependentValues();
}

static string trim(const std::string& str, const std::string& whitespace = " ")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}


double getFactor(string str){
    string tstr = trim(str," ()");
    double d = stod(tstr); 
    
    return d;
}

vector<double> getFactorList(string str) {
    vector<double> vec;
	string tstr = trim(str, " {}");
	regex sep("[, ()]+");
	auto it = sregex_token_iterator(tstr.begin(),tstr.end(), sep,-1);
	sregex_token_iterator tend;
    for(;it!=tend;++it) {
		string astr = (*it);
		if (astr.length()==0) continue;
		vec.push_back(getFactor(astr));
	}
    return vec;
}

vector<string> getVarNames(map<string,int> m, string str){
  vector<string> avec;
  string str1, str2;
  smatch sm;
  if (!regex_search(str,sm,regex("=>"))) {
	  cout << "=> not found in the name range.\n";
	  exit(5);
  }
  str1 = trim(sm.prefix(),"{ ");
  str2 = trim (sm.suffix(), "} ");
      
  if (m.find(str1)!=m.end() && m.find(str2)!=m.end()) {
       for (int i=m[str1]; i<=m[str2]; ++i) {
			avec.push_back(matrixdata.colnames[i]);
	    }
  }
  return move(avec);
}

pair<string,double> getTerm(string str, string name){
    auto pos1 = str.find_first_of('.', 0); 
    string fname = str.substr(0,pos1);
    string cname = str.substr(pos1+1, str.size()-1-pos1);
    double val;
    if (!fname.compare("market")) {
		auto prod = marketdata.products;
        auto ap = prod[0];
        for (auto &p: prod) {
            if (!name.compare(p.name)) {
                ap = p;
                break;
			}
		}
        if (!cname.compare("labour")) {
			val = ap.labour;
		}else if (!cname.compare("initPrem")) {
			val = ap.initprem;
		}else {
            cout << "not implemented yet.\n";
            exit(5);
		}
	}else if (!fname.compare("investments")) {
		auto inv = investdata.invests;
        auto ai = inv[0];
		for (auto &v:inv){
			if (!name.compare(v.name)) {
                ai = v;
                break;
	    	}
		}
        if (!cname.compare("labSub")){
            val = ai.labsub;
		}else if (!cname.compare("capacity")){
			val = ai.capacity;
		}else {
            cout << "not yet implemented. \n";
            exit(5);
		}
	}
	else {
		cout << "not implemented yet. \n";
        exit(5);
	}
    return make_pair(name,val);
}

vector<pair<string,double>> getTerms(string text, string namerange=""){
    vector<pair<string,double>> terms;
    
    auto pos1 = text.find_first_of('.', 0); 
    auto pos2 = text.find_first_of('.', pos1+1);
    string fname = text.substr(0,pos1);
    string cname = text.substr(pos1+1, pos2-pos1-1);

	bool totake = false;
    bool tocheck = false;

	if (namerange.length()==0) {
		totake = true;
	}

	string nam1, nam2;
	smatch sm;
	if (!totake){
	  tocheck = true;
	  if (regex_search(namerange, sm, regex("=>"))) {
		nam1 = trim(sm.prefix(),"{ ");
		nam2 = trim(sm.suffix(), "} ");
	  }else {
		cout << "not a valid range of variables. \n";
		exit (5);
	  }
	}

	bool over = false;
    string astr;
    double aval;
    if (!fname.compare("market")) {
        //norange
        for (auto &x: marketdata.products) {
			if (over) break;
			if (!totake) {
				if ((x.name).compare(nam1)) continue;
				else totake  = true;
			}
			if (totake) {
              astr = x.name;
              if (!cname.compare("labour")) {
                  aval = x.labour;
			  }else if (!cname.compare("initPrem")){
				  aval = x.initprem;
			  }else {
                  cout << "not implemented yet. \n";
                  exit(5);
			  }
			  if (tocheck) {
		      if (!((x.name).compare(nam2))) 
				  over = true;
			  }
			}
			terms.push_back(make_pair(astr,aval));
		}
	}else if(!fname.compare("investments")) {
        
		for (auto &x: investdata.invests){
			if(over) break;
			if (!totake) {
				if (!((x.name).compare(nam1))) {
					totake = true;
				}
			}
			if (totake) {
              astr = x.name;
              if (!cname.compare("labSub")){
				 aval = x.labsub;
			  }else if (!cname.compare("capacity")){
                  aval = x.capacity;
			  }else {
                  cout << "not yet implemented.\n";
                  exit(5);
			  }
			  if (tocheck) {
			    if (!((x.name).compare(nam2))) {
				  over = true;
			    }
			  }
			  terms.push_back(make_pair(astr,aval));
			}
		}
	}else {
        cout << " I dont know where to find the data from " << fname << endl;
        exit(5);
	}

    return move(terms);
}

// initialise MIP matrix
void RegManagerInfo::initMatrix0(){
    int ind=0;
    map<string,int> colInd;
	for (auto &x:matrixdata_n.VarNames0) {
        if (!trim(x).compare("market.name._all_")) {
			for( auto ap: marketdata.products){
				matrixdata.colnames.push_back(ap.name);
                colInd[ap.name]=ind++;
			}
		}else if(!trim(x).compare("investments.name._all_")) {
			for (auto ai: investdata.invests){
				matrixdata.colnames.push_back(ai.name);
                colInd[ai.name]=ind++;
			}
		}else {
			matrixdata.colnames.push_back(trim(x));
            colInd[trim(x)]=ind++;
		}
	}

    set<string> intSet;
	for (auto &ii: matrixdata_n.IntVars0) {
        if (ii.find("=>")!=std::string::npos) {
            smatch sm;
            regex_search(ii, sm, regex("=>"));
			string nam1=trim(sm.prefix(),"{ ");
			string nam2=trim(sm.suffix(),"} ");
            if (colInd.find(nam1)!=colInd.end() && colInd.find(nam2)!=colInd.end()) {
                for (int i=colInd[nam1]; i<=colInd[nam2]; ++i) {
                    intSet.insert(matrixdata.colnames[i]);
				}
			}else {
                 cout << nam1 << " or " << nam2 << "not found as a variable name\n";
                 exit(5);
			}
		}else {
            intSet.insert(trim(ii));
		}
	}
    for (auto &x: matrixdata.colnames) {
        if ( intSet.find(x)!=intSet.end()) {
			matrixdata.isInt.push_back(1);
		}
		else
			matrixdata.isInt.push_back(0);
	}

    //rownames and mat
    //int ri = 0;
	for (auto &x: matrixdata_n.Restricts0) {
		string xname=trim(x.name);
		if (xname.length() == 0) {
			cout << "Error : no restriction name ?\n";
			exit(5);
		}
		string uname;
		uname.resize(xname.size());
		transform(xname.begin(), xname.end(),uname.begin(), (int (*)(int))std::toupper);
		matrixdata.rownames.push_back(uname);
        
        vector<double> avec;
		avec.resize(colInd.size(),0);
		smatch sm;
		string str1,str2,str3;
		vector<string> nams;
		vector<double> vals;
		vector<pair<string,double>> terms;
		for (auto &x2: x.terms0) {
			if ((trim(x2).length()==0)) continue;
            if (x2.find("matrixLinks")!=string::npos) continue;
            if (x2.find("_defaultLinks_")!=string::npos) continue;
            
			double val;
            vector<pair<string,double>> tms;
            vector<string> ns;
			regex stern("\\*");
		    int cnt = std::distance(std::sregex_iterator(x2.begin(),x2.end(),stern),std::sregex_iterator());
	        switch (cnt) {
			case 0:
				if (x2.find(".")!=string::npos ){
                    tms = getTerms(x2);
                    for (auto tt: tms){
						//matrixdata.mat[ri][colInd[tt.first]] = tt.second;
						avec[colInd[tt.first]] = tt.second;
					}
		        }else if (x2.find("=>")!=string::npos) {
                    ns = getVarNames(colInd, x2);
                    for (auto tt: ns){
                        avec[colInd[tt]] = 1;
						//matrixdata.mat[ri][colInd[tt]] = 1;
					}
				}else {
					avec[colInd[x2]] = 1;
					//matrixdata.mat[ri][colInd[x2]] = 1;
				}
                break;
			case 1:
				regex_search(x2,sm,regex("\\*"));
				str1=trim(sm.prefix());
				str2=trim(sm.suffix());
				
				if (str1.find("market.")!=string::npos || str1.find("investments.")!=string::npos) {
					if (str2.find("=>")!=string::npos) {
						terms = getTerms(str1,str2);
						for (auto tt:terms){
							avec[colInd[tt.first]] = tt.second;
							//matrixdata.mat[ri][colInd[tt.first]]=tt.second;
						}
					}else {
						avec[colInd[str2]] = (getTerm(str1,str2)).second;
						//matrixdata.mat[ri][colInd[str2]] = (getTerm(str1,str2)).second;
					}
				}else if (str1.find("{")!=string::npos) {
					vals = getFactorList(str1);
					nams = getVarNames(colInd,str2);
					int sz = vals.size();
					for (int i=0;i<sz;++i){
						avec[colInd[nams[i]]] = vals[i];
						//matrixdata.mat[ri][colInd[nams[i]]] = vals[i];
					}
				}else {
					val = getFactor(str1);
					if (str2.find("=>")!=string::npos) {
						nams = getVarNames(colInd, str2);
						for (auto nn:nams){
							avec[colInd[nn]] = val;
							//matrixdata.mat[ri][colInd[nn]]=val;
						}
					}else if (str2.find(".")!=string::npos) {
						terms = getTerms(str2);
						for (auto &tt:terms){
							avec[colInd[tt.first]] = val*tt.second;
							//matrixdata.mat[ri][colInd[tt.first]] = val*tt.second;
						}
					}else {  //one Variable
						avec[colInd[str2]] = val;
						//matrixdata.mat[ri][colInd[str2]] = val;
					}
				}
                break;
			case 2:
				regex_search(x2,sm,regex("(.*)\\*(.*)\\*(.*)"));
				str1=trim(sm[1]);
				str2=trim(sm[2]);
				str3=trim(sm[3]);
				
				val = getFactor(str1);
				if (str2.find("{")!=string::npos) {
					vals = getFactorList(str2);
					nams = getVarNames(colInd,str3);
					int sz = vals.size();
					for (int i=0;i<sz;++i){
						avec[colInd[nams[i]]] = val*vals[i];
						//matrixdata.mat[ri][colInd[nams[i]]] = val * vals[i];
					}
				}else if (str3.find("{")!=string::npos){
					terms = getTerms(str2,str3);
					for (auto &tt:terms){
						avec[colInd[tt.first]] = val*tt.second;
						//matrixdata.mat[ri][colInd[tt.first]] = val*tt.second;
					}
				}else { //one varialble name
					avec[colInd[str3]] = val*(getTerm(str2,str3)).second;
					//matrixdata.mat[ri][colInd[str3]] = val*(getTerm(str2,str3)).second;
				}
				break;
			default: ;
			}
		}
		matrixdata.mat.push_back(avec);
		//++ri;
	}
}

void
RegManagerInfo::initMatrix() {
    Mip=createMatrix();
    Mip->setupMatrix(g);
}
RegLpInfo*
RegManagerInfo::createMatrix() {
    return new RegLpInfo();
}
// initialise market
void
RegManagerInfo::initMarket() {
    Market->createMarket(Env);
}

void
RegManagerInfo::setPremiumColRow() {
    //set premium row and col
    int row;
    map <string, int> colindex;

    string premName=g->premiumName;
    vector <string> rownames= matrixdata.rownames;
    vector <string> colnames= matrixdata.colnames;

    for (unsigned int j=0; j<colnames.size();j++)
        colindex[colnames[j]]=j;

    int sz= rownames.size();
    int i;
    for (i=0; i<sz; i++)
        if (rownames[i]==premName) break;

    if (i >= sz ) {
        cout << "PREMIUM ROW NOT FOUND" << endl;
        exit(2);
    }
    row= i;

    vector <RegProductInfo> prodcat;
    prodcat = Market->getProductCat();

    int psz= prodcat.size();
    for (i = 0 ; i< psz; i++) {
        RegProductInfo* p= &(Market->getProductCat()[i]);
        if ((*p).getPremiumLegitimation()){
            (*p).setPremiumCol(colindex[prodcat[i].getName()]);
            (*p).setPremiumRow(row);
        }
    }
}

// INITIALISE REGION AND PLOTS
void
RegManagerInfo::initRegion() {
    Region->initialisation();
}
// INITIALISE DATA OUTPUT
void
RegManagerInfo::initOutput() {
    if(g->INIT_OUTPUT) {
        Data->initialisation(InvestCatalog,Market->getProductCat(),Env);
        if (g->ENV_MODELING)
           Env->initEnvOutput(Market);
		//soil service output
		if (g->HAS_SOILSERVICE)
			Data->initSoilservice(FarmList);
    }
}

double RegManagerInfo::get_beta() {
    return g->triangular("RENTVARIATION", g->Beta_min, (g->Beta_max + g->Beta_min )/2, g->Beta_max);
}

// INITIALISE FARM POPULATION(S)
void
RegManagerInfo::initPopulations() {
     // READ INVESTMENT DATA FOR EACH FARM

    vector<double> initial_land_input = farmsdata.land_inputs;
    vector<vector<double> > owned_land_input;
    vector<vector<double> > rented_land_input;
    vector<vector<double> > initial_rent;
	
	owned_land_input.resize(g->NO_OF_SOIL_TYPES);
    rented_land_input.resize(g->NO_OF_SOIL_TYPES);
    initial_rent.resize(g->NO_OF_SOIL_TYPES);

    for (int j=0; j< g->NO_OF_SOIL_TYPES; j++) {
        vector<double> oli =   farmsdata.alllands[g->NAMES_OF_SOIL_TYPES[j]].owned_land;
        vector<double> rli =   farmsdata.alllands[g->NAMES_OF_SOIL_TYPES[j]].rented_land;
        vector<double> ir =    farmsdata.alllands[g->NAMES_OF_SOIL_TYPES[j]].initial_rent_price;

        owned_land_input[j]=  oli;
        rented_land_input[j]= rli;
        initial_rent[j]= ir;
    }
   
    vector<double>  milk_quota = farmsdata.milk_quotas;
    vector<double>  family_labour = farmsdata.fam_lab_units;
    vector<double>  off_farm_labour = farmsdata.off_fam_labs;
    vector<double>  equity_capital = farmsdata.equity_capitals;
    vector<double>  land_assets = farmsdata.land_assets;
    vector<double>  rel_invest_age = farmsdata.rel_invest_ages;

    vector< vector<int> > cap_vec;
    vector< vector<int> > noi_vec;
    vector< vector<int> > cat_num_vec;
    cap_vec.resize(g->number_of_farmtypes);
    noi_vec.resize(g->number_of_farmtypes);
    cat_num_vec.resize(g->number_of_farmtypes);
    for (int i=0;i<g->number_of_farmtypes;i++) {
        cap_vec[i].resize(g->NUMBER_OF_INVESTS);
        cat_num_vec[i].resize(g->NUMBER_OF_INVESTS);
        noi_vec[i].resize(g->NUMBER_OF_INVESTS);
    }

    //initial investments
    for (int i = 0;i < g->number_of_farmtypes; i++) {
        int index=0;
        unsigned dind=0;
        for (int j=0;j<g->NUMBER_OF_INVESTS;j++) {
            if (dind < farmsIinvest[g->sheetnames[i]].initinvests.size() ) {
                string named =  farmsIinvest[g->sheetnames[i]].initinvests[dind].name;
                string namej= InvestCatalog[j].getName();

                if ( named == namej ) {
                    cap_vec[i][index] = farmsIinvest[g->sheetnames[i]].initinvests[dind].capacity;
                    noi_vec[i][index] =  farmsIinvest[g->sheetnames[i]].initinvests[dind].quant;
                    cat_num_vec[i][index]= j;
                    dind++;
                    index++;
                    continue;
                }
            }
            cap_vec[i][index] = 0;
            noi_vec[i][index] =  0;
            cat_num_vec[i][index]= j;

           index++;
        }
    }

    // create pointer to farm
    RegFarmInfo* newFarm;
    int newnumber = 0;
    int new_farm_id;
    int farmclass;
    int farmef;
    string farmname;

	double sum_milk_quota = 0;

    for (int i = 0;i < g->number_of_farmtypes; i++) {
        farmclass = g->farm_class[i];
        farmname  = g->sheetnames[i];
        farmef    = g->ef[i];
        for (int j = 0;j<g->number_of_each_type[i];j++) {
            new_farm_id = 10000 * iteration + newnumber;

			g->tFarmId = new_farm_id;

            // create farm and initialise
            newFarm = createFarm(Region,
                                 g,
                                 Market->getProductCat(),
                                 InvestCatalog, // list pass by reference
                                 Mip,
                                 i,
                                 new_farm_id,
                                 farmclass,
                                 farmname,
                                 farmef);
            // set values read from file
            if (g->RL) {
                if (new_farm_id == g->RLfarmID)
                    RLfarm = newFarm;
            }

                newFarm->setInitialLand(initial_land_input[i]);
            for (int k=0;k<g->NO_OF_SOIL_TYPES;k++) {
                newFarm->setInitialOwnedLandOfType(owned_land_input[k][i],k);
                newFarm->setInitialRentedLandOfType(rented_land_input[k][i],k);
                newFarm->setInitialRentOfType(initial_rent[k][i],k);
            }

	     	newFarm->setFarmStead();

            newFarm->setInitialFamLu(family_labour[i]+off_farm_labour[i]);
            newFarm->setMilkQuota(milk_quota[i]);
			
			sum_milk_quota += milk_quota[i];

            newFarm->setEquityCapital(equity_capital[i]);
            newFarm->setLandAssets(land_assets[i]);
            newFarm->setRelInvestAge(rel_invest_age[i]);

            for (int k=0;k<g->NUMBER_OF_INVESTS;k++) {
                newFarm->addInvestments(cat_num_vec[i][k],noi_vec[i][k],cap_vec[i][k]);
            }
		            // set asynchronous age of farms and of their assets
            newFarm->setAsynchronousFarmAge();
            // store farms in Farm list
            FarmList.push_back(newFarm);

			manageCoeffMap[int(newFarm->getManagementCoefficient()*100 + 0.5)]++;
            newnumber++;

			if (g->RestrictInvestments) {
				double x = g->getRandomReal("LIVESTOCK_INV", g->uni_real_distrib_livestock_inv);
				if (x * 100 > g->Livestock_Inv_farmsPercent) {
					newFarm->setAllowInvest(false);
					//cout << "##FarmId: " << newFarm->getFarmId() << endl;
				}
				else {
					newFarm->setAllowInvest(true);
					//cout << "farmId0: " << newFarm->getFarmId() << endl;
				}
			}

            if (g->Rent_Variation) {
                double r = get_beta();
                betaMap[int(r * 100 + 0.5)]++;
                newFarm->set_beta(r);
            }

            if (g->RL) {
                newFarm->set_act_beta(g->RENT_ADJUST_COEFFICIENT);
            }
        }
    }

	g->REGION_MILK_QUOTA = sum_milk_quota;

    /////////////////////////////
    // ALLOCATION OF INITIAL LAND
    /////////////////////////////
    Region->initPlotSearch();
    list<RegFarmInfo* >::iterator farms_iter;
    bool ready=false;

//srand(g->SEED) for randoming soil types, all other things being equal;
//here srand(g->SEED) after the while loop to get the same random ST at each simulation;
//if srand(g->SEED) before the loop, you get always different random ST distribution for each simulation;

    if (g->SOIL_TYPE_VARIATION) {
        randInit(g->SEED);
        //srand(g->SEED);
        }
	
    while (!ready) {
        ready=true;
        for (farms_iter = FarmList.begin();
                farms_iter != FarmList.end();
                farms_iter++) {
            if (!(*farms_iter)->allokateInitialLand())
                ready = false;
        }
    }

    // finish up setup of region (create black plots)
    Region->finish();
    Region->initPlotSearch();
    g->INITIALISATION=false;
 
	//init carbon
	if (g->HAS_SOILSERVICE) {
	   for (farms_iter = FarmList.begin();
               farms_iter != FarmList.end();
               farms_iter++) {
			vector <double> c_mean;
			vector <double> c_var;
			for (int i=0;i<g->NO_OF_SOIL_TYPES;++i) {
				c_mean.push_back(farmsdata.alllands[g->NAMES_OF_SOIL_TYPES[i]].carbon_mean[(*farms_iter)->getFarmType()]);
				c_var.push_back(farmsdata.alllands[g->NAMES_OF_SOIL_TYPES[i]].carbon_std_dev[(*farms_iter)->getFarmType()]);
			}
			(*farms_iter)->initCarbons(c_mean, c_var);
       }
	}
}

void RegManagerInfo::outputManageCoeffDistrib() {
	for (auto x : manageCoeffMap) {
		cout << x.first << "\t" << x.second << endl;
	}
}

void RegManagerInfo::outputMap(std::map<int,int> &amap){
    for (auto x : amap) {
        cout << x.first << "\t" << x.second << endl;
    }
}

void RegManagerInfo::outputFarmAgeDists() {
	for (auto x : g->farmAgeDists) {
		for (auto t : x) {
			cout << t.first << "\t" << t.second << endl;
		}
		cout << endl;
	}
}

//---------------------------------------------------------------------------
//         SIMULATION
//---------------------------------------------------------------------------

void
RegManagerInfo::simulate() {
    init();

    while (iteration < g->RUNS) {
		cout << "Iteration : " << iteration << "\t ( Number of Farms:  " << getNoOfFarms()<< " )"<< endl;

		//TEST
		g->tIter = iteration;
		g->tPhase = SimPhase::BETWEEN;
        step();
    }
	//outputFarmAgeDists();
}

void
RegManagerInfo::stepwhile() {
    while (iteration < g->RUNS) {
        step();
    }
}
void RegManagerInfo::step() {
	setIncreasePrices();

	if(g->GLOBAL_OPTIMUM_EVERY_PERIOD) {
        RegManagerInfo* tmp1=this->clone("tmp1");
        RegManagerInfo* tmp2=this->clone("tmp2");
        RegGlobalsInfo* tmpg1=tmp1->getGlobals();
        RegGlobalsInfo* tmpg2=tmp2->getGlobals();
        tmpg1->GLOBAL_OPTIMUM_EVERY_PERIOD=false;
        tmpg1->GLOBAL_STRATEGY=1;
        tmpg2->GLOBAL_OPTIMUM_EVERY_PERIOD=false;
        tmpg2->GLOBAL_STRATEGY=2;
        tmp1->step();
        tmp2->step();
    }
    PreparationForPeriod();

    // adjust costs by management coefficient
    if (iteration == 0)
        CostAdjustment();
    readPolicyChanges();
 //   Region->calculateAverageRent();
 //   Region->calculateAverageNewRent();
	if (g->CALCULATE_CONTIGUOUS_PLOTS) Region->countContiguousPlots();
    g->WERTS1=RentStatistics();
    g->WERTS=-g->WERTS1;
    LandAllocation();
	if (g->YoungFarmer)
		updateYoungFarmerLand();
	if (g->NASG && iteration >= g->NASG_startPeriod - 1) 
		updateNASG();
	//Region->outputMaxRents();
	UpdateSoilserviceLA();
	Region->calculateAverageRent();
    Region->calculateAverageNewRent();
    
	if(iteration==0)
        Region->setNewRentFirstPeriod();
    f=static_cast<int>(Region->getExpAvNewRentOfType(1)/Region->getAvRentOfType(1));
    g->WERTS2=RentStatistics();
    g->WERTS+=g->WERTS2;
    InvestmentDecision();
    Production();
	UpdateSoilserviceP();  
    UpdateMarket();
    FarmPeriodResults();
    RemovedFarmPeriodResults();
    SectorResults();
    FarmOutput();
    CapacityEstimationForBidding();
    setPolicyChanges();
    Disinvest();
	FutureOfFarms();
	SectorResultsAfterDisinvest();
    ResetPeriodLabour();
    SectorOutput();
    EnvSpeciesCalc();
	RemoveFarms();
    ProcessMessages();
}

void RegManagerInfo::updateYoungFarmerLand() {
	for (auto t : FarmList) {
		t->updateYoungFarmerLand();
		t->updateYoungFarmer();
	 }
}

void
RegManagerInfo::CapacityEstimationForBidding() {
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        (*farms_iter)->calculateEstimationForBidding();
    }
}

void
RegManagerInfo::ResetPeriodLabour() // actually part of FutureOfFarms
{
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        (*farms_iter)->resetFarmVariables();
    }
}

void
RegManagerInfo::RegionalPerHaPayment() {
    double total_direct_payment = 0;
    double total_ha = 0;
    double average_ha_payment = 0;
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        total_direct_payment += (*farms_iter)->getAveragePremium();
        total_ha += (*farms_iter)->getLandInput();
    }
    if (total_ha != 0)
        average_ha_payment = total_direct_payment/total_ha;
    else
        average_ha_payment = 0;
    Region->setHaPaymentPerPlot(average_ha_payment*g->PLOT_SIZE);
}

void
RegManagerInfo::PreparationForPeriod() {
	if (iteration>0) 
		g->tech_develop_abs *= 1+ g->TECH_DEVELOP;
    if(g->SET_FREE_PLOTS) {
      Region->setIdlePlotsDead();
      for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        Region->setDeadPlotsToType(g->FREE_PLOTS_OF_TYPE[i],i);
      }
    }
    list<RegFarmInfo* >::iterator farms_iter;
    // reset sector output values for new iteration
    Sector->resetSector();
    if (g->CALC_LEGAL_TYPES) {
        for (unsigned int i=0;i<g->LEGAL_TYPES.size();i++) {
            sector_type[i]->resetSector();
        }
    }
    // if (g->WEIGHTED_PLOT_SEARCH)
    // Region->setUpdate();
    // compute new MIP values
    updateInterestDependentValues();

    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
		//cout<< (*farms_iter)->getFarmName() << " " << (*farms_iter)->farm_plot->getSoilName() << "\n"; 
        (*farms_iter)->newRentingProcess(iteration);
        (*farms_iter)->updateLpValues();
    }
    //  if (g->WEIGHTED_PLOT_SEARCH)
    //  Region->resetUpdate();

	//soil service
	if (g->HAS_SOILSERVICE) {
		for (int i=0; i<g->NO_OF_SOIL_TYPES; ++i) {
			double sum=0;
			double area=0;
			for (farms_iter=FarmList.begin(); farms_iter!=FarmList.end(); farms_iter++) {
				double a=(*farms_iter)->getLandInputOfType(i);
				sum+=(*farms_iter)->getAvCarbons()[i]*a;
				area+=a;
			}
			if (area!=0) g->regCarbons[i]=sum/area;
			else g->regCarbons[i]=0; //??
		}
	}
} 

void
RegManagerInfo::CostAdjustment() {
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        // multiply individual costs on farm by management factor
        (*farms_iter)->updateCosts();
        (*farms_iter)->updateLpValues();
    }
}

void RegManagerInfo::updateNASG() {
	if (debug){
		cout << "NASG:\n";
		Region->outputMaxRents();
	}
	NASG_maxRentOfTypes.clear();
	Region->calculateAverageRent();
	Region->calculateAverageNewRent();
	int ntype= g->NO_OF_SOIL_TYPES;
	for (auto i = 0; i < ntype; ++i) {
		NASG_maxRentOfTypes.push_back(g->PLOT_SIZE * Region->getAvRentOfType(i) * (1 + g->NASG_maxRentAv));
		if (debug)
			cout << Region->getAvRentOfType(i) << endl;
	}

	NASG_UAA = 0;
	int nfarms = FarmList.size();
	for (auto f: FarmList) {
		NASG_UAA += f->getLandInput();
	}
	if (debug)
		cout << NASG_UAA << "\t"<<nfarms<<"\t";

	if (nfarms)
		NASG_avArea = NASG_UAA / nfarms;
	else
		NASG_avArea = 0;

	if (debug)
		cout << NASG_avArea << "\n";
}

double RegManagerInfo::NASG_RentOffer(double offer, RegFarmInfo* pfarm, int type) {
	bool debug1 = false;
	double res = offer;
	double area = pfarm->getLandInput();
	if (area > NASG_UAA*g->NASG_maxShareUAA) {
		res = 0;
		if (debug1)
			cout << " high share ! :  " << area << "\n";
	}
	if (area > NASG_avArea*g->NASG_maxSizeFactor) {
		res = 0;
		if (debug1)
			cout << " too large! : " << area << "\n";
	}

	if (offer > NASG_maxRentOfTypes[type])
		res = NASG_maxRentOfTypes[type];
	if (debug1) {
		if (res != offer)
			cout << "capped: " << res << "\t";
		else cout << "--ok--: ";
		cout << offer << endl;
	}
	return res;
}

void RegManagerInfo::calcMaxRents() {
	Region->calcMaxRents();
}

static vector<double> calcAvNewRents(RegManagerInfo* m) {
    vector<double> res;
    vector<double> res0;
    RegRegionInfo* reg = m->getRegion();
    int n = m->getGlobals()->NO_OF_SOIL_TYPES;
    res.resize(n);
    res0.resize(n);
    //newRentedplots.resize(n);

    for (auto i = 0; i < n; ++i) {
        res[i] = reg->getAvNewRentOfType(i);
        //newRentedplots[i] = reg->getNewlyRentedPlotsOfType(i);
        if (m->getIteration() == 0) {
            res0[i] = reg->getAvRentOfType(i);
            if (res[i] == 0)
                res[i] = res0[i];
        }
        else if (res[i] == 0) {
            res[i] = avRecentRents[i];
        }
    }
    return res;
}

static vector<double> calcRecentRents(RegFarmInfo* f, RegManagerInfo* m) {
    vector<double> res;
    vector<double> res0;
    int n = m->getGlobals()->NO_OF_SOIL_TYPES;
    res.resize(n);
    res0.resize(n);
    for (auto i = 0; i < n; ++i) {
        res[i] = f->getAvNewRentOfType(i);
        if (m->getIteration() == 0) {
            res0[i] = f->getAvRentOfType(i);
            if (res[i] == 0)
                res[i] = res0[i];
        }
        else if (res[i] == 0) {
            res[i] = recentRents[i];
        }
    }
    return res;
}

void
RegManagerInfo::LandAllocation() {
    if (g->RL) {
        newIteration = true;
    }

	g->tPhase = SimPhase::LAND;
	//cout << "vor landallocation: " << Region->free_plots.size() << endl;
//        if (iteration==0) {
//          printShadowPrices(100);   }
    if (g->GLOBAL_STRATEGY==1) {
        if (iteration>0) {
            if (FarmList.size()>0) {
                 string part2=g->OUTPUTFILE+ string("103.dat");
//                (*(FarmList.begin()))->lp->globalAllocationFromFile(FarmList,Region,part2);
                (*(FarmList.begin()))->lp->globalAllocation(FarmList,Region,iteration);
            }
        }
    }
    if(g->GLOBAL_STRATEGY==0) {
        if (g->USE_TC_FRAMEWORK) {
            released_plots=released_plots_IF=released_plots_CF=rented_plots_CF=rented_plots_IF=0;
            ;
            CF_to_CF=0;
            CF_to_IF=0;
            IF_to_CF=0;

            stay_CF=0;
            stay_IF=0;

            paid_tacs=0;
            total_tacs=0;
            stay_at_prev_owner=0;
            stay_at_prev_owner_because_of_tacs=0;

        }

        int bc=0;
        if (g->OLD_LAND_RENTING_PROCESS) {
            vector<int> count_rented_plots_of_type;

            vector<double> max_offer_of_type;
            vector<double> av_offer_of_type;
            vector<double> ratio_of_type;
            vector<double> secondprice_region;
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                count_rented_plots_of_type.push_back(0);
                max_offer_of_type.push_back(0);
                av_offer_of_type.push_back(0);
                ratio_of_type.push_back(0);
                secondprice_region.push_back(1E30);
            }
            bool stop;
            if (iteration > 0) {
                stop= true;
                bidcount=0;
                do { // as long as offers are positive
                    stop=true;
                    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                        if (Region->getFreeLandPlotsOfType(i)>0) {
                            max_offer_of_type[i]=rentOnePlot(count_rented_plots_of_type,i);
                            if(max_offer_of_type[i]<secondprice_region[i])
                              secondprice_region[i]=max_offer_of_type[i];
//                        pa << bc << "\t" << max_offer_of_type[i] << "\n";
                            bc++;
                        } else {
                            max_offer_of_type[i]=0;
                        }
                        av_offer_of_type[i]+= max_offer_of_type[i];
                    }
                    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                        if (max_offer_of_type[i]>0) stop = false;
                    }
                    bidcount++;
                } while (!stop);
//            pa.close();
                for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                    if (count_rented_plots_of_type[i]>0) {
                        av_offer_of_type[i]/=(double)count_rented_plots_of_type[i];
                        ratio_of_type[i]=(double)count_rented_plots_of_type[i]/(double)Region->getNumberOfLandPlotsOfType(i);
                    }
                }
                if(g->SECONDPRICE_REGION) {
                  list<RegFarmInfo* >::iterator farms_iter;
                        for (farms_iter = FarmList.begin();
                                farms_iter != FarmList.end();
                                farms_iter++) {
							
                            (*farms_iter)->setSecondPrice(secondprice_region);
                        }
                }

                ///////////////////
                // ADJUST RENT PAID
                ///////////////////
                // Here, the rent of already rented plots is adjusted according
                // to the ratio of newly rented plots of type to the total
                // number of plots of type
                // If there is no averageOffer then it is not possible to update
                // the rents.
                if (g->ADJUST_PAID_RENT) {
                    stop=false;
                    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                        if (count_rented_plots_of_type[i]>0) stop = false;
                    }
                    list<RegFarmInfo* >::iterator farms_iter;
                    if (!stop) {
                        for (farms_iter = FarmList.begin();
                                farms_iter != FarmList.end();
                                farms_iter++) {
                            if (iteration > 3)
                            (*farms_iter)->adjustPaidRent(av_offer_of_type,
                                                          ratio_of_type);
                        }
                    }
                }
            }
        } else {
            ////////////////////
            //  LAND ALLOCATION
            ////////////////////
            list<RegFarmInfo* >::iterator farms_iter;
            double sum=0;
            double sum_dist=0;
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                sum+=(*farms_iter)->getObjective();
                sum_dist+=(*farms_iter)->getFarmDistanceCosts();
            }
            sum-=sum_dist;

            vector<int> count_rented_plots_of_type;
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                count_rented_plots_of_type.push_back(0);
            }
            if (iteration > 0) {
                int pl= Region->getNumberOfFreePlots()*2;
                for (int i=0;i<pl;i++) {
                    rentOnePlot(count_rented_plots_of_type,-1);
                }
            } // end if(iteration > 0)

        }
        if ((g->USE_TC_FRAMEWORK && g->PRINT_TAC)) {
            string o=g->OUTPUTFILE+"tac.dat";
            if (iteration==0 && g->INIT_OUTPUT==true) {
                ofstream out;
                out.open(o.c_str(), ios::trunc);
                out <<"SCENARIO\tDESIGN_POINT\tRANDOM\tITERATION\tPAID_TACS\tTOTAL_TACS\tPAID_TACS_IN_REGION\tTOTAL_TACS_IN_REGION\tSTAY_AT_PREV_OWNER_BECAUSE_OF_TACS\tSTAY_AT_PREV_OWNER\tRELEASED_PLOTS\tRELEASED_PLOTS_CF\tRELEASED_PLOTS_IF\tRENTED_PLOTS_CF\tRENTED_PLOTS_IF\tCF_to_CF\tCF_toIF\tIF_to_CF\tIF_toIF\tstay_CF\tstay_IF\n";
                out.close();
            }

            ofstream out;
            out.open(o.c_str(), ios::app);
            out << g->SCENARIO << "\t" << g->DESIGN_POINT << "\t"<< g->RANDOM << "\t"<< iteration <<  "\t"  << paid_tacs <<  "\t"  << total_tacs <<  "\t" << Region->calcPaidTacs() <<  "\t" << Region->calcTacs() <<  "\t" << stay_at_prev_owner_because_of_tacs <<  "\t" <<  stay_at_prev_owner <<  "\t"  <<released_plots<<  "\t"  << released_plots_CF<<  "\t"  << released_plots_IF<<  "\t"  << rented_plots_CF<<  "\t"  << rented_plots_IF <<  "\t"  << CF_to_CF<< "\t" <<CF_to_IF<< "\t" <<IF_to_CF<< "\t" <<IF_to_IF<< "\t" <<stay_CF<< "\t" <<stay_IF<< "\n";
            out.close();                                                                                                                            

         Region->setTacs();
        }
    }
	//cout << "after Landallocation: " << Region->free_plots.size() << endl;

    if (g->Rent_Variation) {
        Data->printPlots(iteration);
    }

    //recent rents
    if (g->RL) {
        Region->calculateAverageRent();
        Region->calculateAverageNewRent();
        recentRents = calcRecentRents(RLfarm, this);
        avRecentRents = calcAvNewRents(this);
    }

	g->tPhase = SimPhase::BETWEEN;
}

double    RegManagerInfo::RentStatistics() {

//    if (iteration > 0) {
//}
    if(g->SECTOROUTPUT) {
    double sum=0;
    double sum_dist=0;
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        (*farms_iter)->updateLpValues();
#ifndef NDEBUG1
        g->tFarmName=(*farms_iter)->getFarmName();
        g->tFarmId= (*farms_iter)->getFarmId();
#endif
	//cout << g->tFarmId<< endl;
        sum+=    (*farms_iter)->getObjective();
        sum_dist+=(*farms_iter)->getFarmDistanceCosts();
    }
    if(g->PRINT_VA) {
    ofstream out;
    out.open("../outputfiles/wertsagrip.dat", ios::app);
    out << sum << "\t" << sum_dist << "\t" << sum-sum_dist << "\n";
    out.close();
    }
    return sum-sum_dist;
    } else {
      return 0;
    }
}

void
RegManagerInfo::InvestmentDecision() {
	///////////////////////
    //  INVESTMENT DECISION beginning of period
    ///////////////////////
	g->tPhase = SimPhase::INVEST;

    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
#ifndef NDEBUG1
        g->tFarmName=(*farms_iter)->getFarmName();
        g->tFarmId= (*farms_iter)->getFarmId();
#endif

        (*farms_iter)->doLpInvest();
    }
	g->tPhase = SimPhase::BETWEEN;
}
double
RegManagerInfo::Production() {
	g->tPhase = SimPhase::PRODUCT;
    double sum=0;
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
#ifndef NDEBUG1
        g->tFarmName=(*farms_iter)->getFarmName();
        g->tFarmId= (*farms_iter)->getFarmId();
#endif
        sum+=(*farms_iter)->doProductionLp();
		if (g->YoungFarmer)
			(*farms_iter)->saveYoungFarmerPay();
    }
	g->tPhase = SimPhase::BETWEEN;
    return sum;
}
void
RegManagerInfo::UpdateMarket() {
    /////////
    // MARKET
    /////////
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        Sector->setTotalProduction(*farms_iter);
        Sector->setTotalLandInput(*farms_iter);
    }
    Market->priceFunction(*Sector, evaluator,iteration);
}

void
RegManagerInfo::FarmPeriodResults() {
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {

        (*farms_iter)->periodResults(iteration);
        if (g->RL && (*farms_iter) == RLfarm && iteration>0) {
            send_val((*farms_iter)->getEquityCapital());
        }
    }
    if (g->ASSOCIATE_ACTIVITIES && g->ENV_MODELING)
        Env->associateActivities(FarmList);
}
void
RegManagerInfo::RemovedFarmPeriodResults() {
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = RemovedFarmList.begin();
            farms_iter != RemovedFarmList.end();
            farms_iter++) {
        (*farms_iter)->periodResultsForRemovedFarms();
        if (g->RL && (*farms_iter) == RLfarm && iteration>0) {
            send_val((*farms_iter)->getEquityCapital());
        }
    }
}
    void
RegManagerInfo::increaseLandCapacityOfType(int farm,int type,int no_of_plots) {
f=farm;t=type;n=no_of_plots;
}

    void
RegManagerInfo::increaseLandCapacityOfTypel(int farm,int type,int no_of_plots) {
    list<RegFarmInfo* >::iterator farms_iter;
    int c=0;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        if(c==farm)
        (*farms_iter)->increaseLandCapacityOfType(type,no_of_plots);
        c++;
    }
}

void
RegManagerInfo::FarmOutput() {
    ////////////////////
    /// FARM DATA OUTPUT
    ////////////////////
        list<RegFarmInfo* >::iterator farms_iter;
        if (g->FARMOUTPUT) {
            Data->openFarmOutput();
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                if (g->CALCULATE_CONTIGUOUS_PLOTS) {
                if(g->PRINT_CONT_PLOTS)
                    Data->printContiguousPlotsOutput( (*farms_iter),iteration);
                }
                Data->cacheFarmResults((*farms_iter),
                                       InvestCatalog,
                                       Market->getProductCat(),
                                       iteration);
                if(g->PRINT_FARM_INV)
                Data->printFarmInvestment((*farms_iter),
                                          InvestCatalog,
                                          iteration);
                if(g->PRINT_FARM_PROD)
                Data->printFarmProduction((*farms_iter),
                                          Market->getProductCat(),
                                          iteration);
                if(g->PRINT_FARM_COSTS)
                Data->printFarmVarCosts((*farms_iter),
                                          Market->getProductCat(),
                                          iteration);
                if (g->ENV_MODELING) {
                    Data->printEnvDataUsage((*farms_iter),
                                            Market->getProductCat(),
                                            iteration);
                }
            }
            if (g->PRINT_REMOVED_FARMS) {
                for (farms_iter = RemovedFarmList.begin();
                        farms_iter != RemovedFarmList.end();
                        farms_iter++) {
                    Data->cacheFarmResults((*farms_iter),
                                           InvestCatalog,
                                           Market->getProductCat(),
                                           iteration);
                }
            }
            Data->closeFarmOutput();
        }
}
void
RegManagerInfo::SectorResults() {
    /////////////////////
    // SECTOR DATA OUTPUT
    /////////////////////

    Sector->periodResultsSector(InvestCatalog, *Region, FarmList, iteration);
    if (g->CALC_LEGAL_TYPES) {
        for (unsigned int i=0;i<g->LEGAL_TYPES.size();i++) {
            sector_type[i]->periodResultsSector(InvestCatalog, *Region, FarmList, iteration);
        }
    }
}
void
RegManagerInfo::SectorResultsAfterDisinvest() {
    /////////////////////
    // SECTOR DATA OUTPUT
    /////////////////////
    Sector->periodResultsSectorAfterDisinvest(InvestCatalog,FarmList);
    if (g->CALC_LEGAL_TYPES) {
        for (unsigned int i=0;i<g->LEGAL_TYPES.size();i++) {
            sector_type[i]->periodResultsSectorAfterDisinvest(InvestCatalog,FarmList);
        }
    }
}

void RegManagerInfo::outputRestrictedInvs(RegFarmInfo* farm) {
	map<string, int> rinvs = farm->getRestrictedInvests();
	cout << "--FarmId: " << farm->getFarmId() << "\t";
	for (auto x : rinvs) {
		cout << x.first << "\t" << x.second << "\t";
	}
	cout << endl;
}

void
RegManagerInfo::Disinvest() {
    list<RegFarmInfo* >::iterator farms_iter;
	nfarms_restrict_invest = 0;

    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
#ifndef NDEBUG1
        g->tFarmName=(*farms_iter)->getFarmName();
        g->tFarmId= (*farms_iter)->getFarmId();
#endif            
        (*farms_iter)->updateLpValues();
        (*farms_iter)->disInvest();

		if (g->RestrictInvestments) {
			(*farms_iter)->setRestrictInv();
			
			if ((*farms_iter)->restrictInvestments()) {
				(*farms_iter)->setReinvestLUcap();
				++nfarms_restrict_invest;
				//outputRestrictedInvs(*farms_iter);
			}
			else {
				double x = g->getRandomReal("LIVESTOCK_INV", g->uni_real_distrib_livestock_inv);
				//printf("%d\t %f\n", (*farms_iter)->getFarmId(), x);
				if (x * 100 > g->Livestock_Inv_farmsPercent) {
					(*farms_iter)->setAllowInvest(false);
					//cout << "++ FarmId : " << (*farms_iter)->getFarmId()<< endl;
				}
				else {
					(*farms_iter)->setAllowInvest(true);
					//cout << "farmId: "<<(*farms_iter)->getFarmId() << endl;
				}
			}
		}

		if (g->HAS_SOILSERVICE) 
			(*farms_iter)->calAvCarbons();
       (*farms_iter)->updateLpValues();
    }
	//cout << g->tIter << "\t" << nfarms_restrict_invest << endl;
}

void
RegManagerInfo::FutureOfFarms() {
	g->tPhase = SimPhase::FUTURE;
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
#ifndef NDEBUG1
        g->tFarmName=(*farms_iter)->getFarmName();
        g->tFarmId= (*farms_iter)->getFarmId();
#endif 
        (*farms_iter)->futureOfFarm(iteration);

        if (g->RL && (*farms_iter) == RLfarm && iteration>0) {
            send_val((*farms_iter)->getClosed()+1);
        }
    }
	g->tPhase = SimPhase::BETWEEN;
}
void
RegManagerInfo::SectorOutput() {
    /////////////////////
    // SECTOR DATA OUTPUT
    /////////////////////
   if (g->CALC_LEGAL_TYPES) {
       Data->printLegalTypeResults(*Sector,sector_type,Market->getProductCat(),InvestCatalog, iteration);
   } 
    if (g->SECTOROUTPUT ) {
        if(g->PRINT_SEC_RES)
        Data->printSectorResults(*Sector,Market->getProductCat(), iteration);
        if(g->PRINT_SEC_COSTS)
        Data->printSectorVarCosts(*Sector,Market->getProductCat(), iteration);
        if(g->PRINT_SEC_PRICE)
        Data->printSectorPrices(*Sector,Market->getProductCat(), iteration);
        if(g->PRINT_SEC_EXP_PRICE)
        Data->printExpectedSectorPrices(*Sector,Market->getProductCat(), iteration);
        if(g->PRINT_SEC_COND)
        Data->printCondensedSectorOutput(*Sector, Market->getProductCat(),InvestCatalog,iteration);
        if (g->CALCULATE_CONTIGUOUS_PLOTS) {
            if(g->PRINT_CONT_PLOTS)
              Data->printRegionContiguousPlotsOutput(Region,iteration);
        }
    }
    if (g->FARMOUTPUT && g->PRINT_FARM_RES) {
        int c=0;
        Data->openFarmStandardOutput();
        list<RegFarmInfo* >::iterator farms_iter;
        for (farms_iter = FarmList.begin();
                farms_iter != FarmList.end();
                farms_iter++) {
            Data->printFarmResults((*farms_iter),
                                   InvestCatalog,
                                   Market->getProductCat(),
                                   iteration,c);
            c++;
        }
        if (g->PRINT_REMOVED_FARMS) {
            for (farms_iter = RemovedFarmList.begin();
                    farms_iter != RemovedFarmList.end();
                    farms_iter++) {
                Data->printFarmResults((*farms_iter),
                                       InvestCatalog,
                                       Market->getProductCat(),
                                       iteration,c);
                c++;
            }
        }
        Data->closeFarmStandardOutput();
    }
    if (g->ENV_MODELING) {
        Data->printSpeciesOut(Env, iteration);
    }

	if (g->HAS_SOILSERVICE) {
        Data->printSoilservice(FarmList, iteration);
    }
}

void
RegManagerInfo::EnvSpeciesCalc() {
    if (g->ENV_MODELING) {

        Env->resetHaProducedByHabitat();

        for (int i=0;i<Market->getNumProducts();i++) {
            double produced= Sector->getTotalUnitsProduced(i);
            Env->sumHaProducedByHabitat(i, produced);
        }
        if (iteration==0) {
            Env->calculateCCoefficients();
        } else {
            Env->calculateSpeciesByHabitat();
        }
    }
}

void
RegManagerInfo::RemoveFarms() {
    int size;
    size=FarmList.size();
    list<RegFarmInfo* >::iterator farms_iter;
    int df=0;

    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        if ((*farms_iter)->getClosed()) {
            RemovedFarmList.push_back((*farms_iter));
            (*farms_iter)= NULL;
            df++;
        }
    }
   
    list<RegFarmInfo* >::iterator removed =
        remove_if(FarmList.begin(), FarmList.end(), CheckIfClosed());
    FarmList.erase(removed, FarmList.end());
    //    removed =
    //	  remove_if(RemovedFarmList.begin(), RemovedFarmList.end(), CheckIfClosed());
    //    RemovedFarmList.erase(removed, RemovedFarmList.end());
    size-=FarmList.size();
}

void RegManagerInfo::ProcessMessages() {
    iteration++;
	g->tIter = iteration;
	g->tInd_land = 0;
	g->tInd_future = 0;
}

//---------------------------------------------------------------------------
//      UPDATE FUNCTIONS
//---------------------------------------------------------------------------

void
RegManagerInfo::updateInterestDependentValues() {
    vector<RegInvestObjectInfo >::iterator invest;

    double acqcosts;
    double maintenance;
    int lifetime;
    int type;

    double aec;       // bound equity capital on average
    double aac;       // average costs
    double le;        // liquidity effect

    // interest on borrowed and equity capital
    double bc_interest = g->INTEREST_RATE;
    vector<RegProductInfo>& prod_cat = Market->getProductCat();
    double ec_interest = prod_cat[g->ST_EC_INTERESTTYPE].getPrice();
    for (invest = InvestCatalog.begin();
            invest != InvestCatalog.end();
            invest++) {
        acqcosts = invest->getAcquisitionCosts();
        lifetime = invest->getEconomicLife();
        maintenance = invest->getMaintenanceCosts();
        type = invest->getInvestType();

        switch (type) {
        case 0:  // hired fix labour
            if (iteration > 0) {
                aac = acqcosts * g->IncPriceHiredLab;//INCREASEPRICE;
                invest->setAcquisitionCosts(aac);
            } else {
                aac = acqcosts;
            }
            break;
        case 1:  // fix off-farm labour
            if (iteration > 0) {
                aac = acqcosts * g->IncPriceOffFarmLab;//INCREASEPRICE;
                invest->setAcquisitionCosts(aac);
            } else {
                aac = acqcosts;
            }
            break;
        default:
            aac =  g->SHARE_SELF_FINANCE*  ( acqcosts / lifetime)
                   +  (1 - g->SHARE_SELF_FINANCE) * acqcosts * capitalReturnFactor(bc_interest, lifetime)
                   + maintenance;
            break;
        }

        invest->setAverageCost(aac);

        // average annual costs of investment (gross margin, obj. function)
        // includes maintenance costs
        // approximative Kalkulation

        //    aac = acqcosts * (g->SHARE_SELF_FINANCE / lifetime)
        //       + ( (acqcosts + maintenance*(1/capitalReturnFactor(interest,lifetime)))
        //               * (1 - g->SHARE_SELF_FINANCE)
        //               * capitalReturnFactor(interest, lifetime)); //capital costs

        // liquidity effect of investment (equity capital)
        le = (acqcosts)* g->SHARE_SELF_FINANCE;
        invest->setLiqEffect(le);
        // average bound equity (capital for calculatory interest)
        // FK-Anspruch
        aec = g->SHARE_SELF_FINANCE * (acqcosts) * averageReturn(ec_interest, lifetime);
        invest->setBoundEquityCapital(aec);
    }
}
// type 0: arableLand, type 1:grassLand
double
RegManagerInfo::rentOnePlot(vector<int>& count_rented_plots_of_type, int type) {
    //double second_offer;
    if (g->OLD_LAND_RENTING_PROCESS) {
        // maximum offer at a point in time
        double offer;
        double maxoffer;
        double true_second_offer=0;
        // list of farms with equal offer
        list<RegFarmInfo* >  equalbidder;
        list<RegFarmInfo* >::iterator farms_iter;
        list<RegFarmInfo* >::iterator equalbidder_iter;
        list<RegFarmInfo* >::iterator  prev_owner;

        RegFarmInfo* maxbidder;

        // search for the highest offer
        for (farms_iter = FarmList.begin(), maxoffer = 0, maxbidder = NULL, equalbidder.clear();
                farms_iter != FarmList.end();
                farms_iter++) {
            // determine individual rent offer for plot
            // here,  0: arable, 1: grassland
#ifndef NDEBUG1
        g->tFarmName=(*farms_iter)->getFarmName();
        g->tFarmId= (*farms_iter)->getFarmId();
#endif
        
        if (g->RL && (*farms_iter) == RLfarm && newIteration) {
            RLdata rlData = getRLdata((*farms_iter), this);
            //output(rlData, this, "RL.dat");
            sendRLdata(rlData);
            (*farms_iter)->set_act_beta(recv_val());
        }
            (*farms_iter)->demandForLandOfType(type,bidcount);
            offer=(*farms_iter)-> getRentOffer();
			
			double offer1;
			if (g->NASG && iteration>=g->NASG_startPeriod) {
				offer1 = NASG_RentOffer(offer, (*farms_iter), type);
				if (debug)
					cout << offer << " - " << offer1 << "\t"<< type <<endl;
				offer = offer1;
			}

            if (offer>0) {

				//if (offer/(fabs(maxoffer-offer)) < 1E-5 ) {
                //if ((maxoffer) == offer) {
                //    equalbidder.push_back(*farms_iter);
                //} else {
                    // if current offer is higher than maxoffer
                    if ((maxoffer) < offer) {
                        // set maxbiddr to highest bidding farm
                        maxbidder = *farms_iter;
                        // if there is a higher offer, the list of equal bidders
                        // is senseless and is cleared
                        equalbidder.clear();
                        // the equalbidder list is filled
                        // with new offers
                        equalbidder.push_back(maxbidder);
                        maxoffer = offer;
                    }
                //}// end else
            }
        }// end for

        // allocate land to highest bidder
//        if (g->WEIGHTED_PLOT_SEARCH)
//        Region->resetUpdate();
        if (maxoffer > 0) {
            for (equalbidder_iter = equalbidder.begin();
                    equalbidder_iter != equalbidder.end();
                    equalbidder_iter++) {
                // allocate land to highest bidder
                // offercount is incremented by the farm
                RegPlotInfo* pl=(*equalbidder_iter)->getWantedPlotOfType(type);
                if (pl->getState() == 0) {
                    count_rented_plots_of_type[type]+=1;
                }
                maxoffer=(*equalbidder_iter)->getRentOffer();
                if(!g->FIRSTPRICE) {
                  for (farms_iter = FarmList.begin();
                        farms_iter != FarmList.end();
                        farms_iter++) {
                        if ((*farms_iter)->getFarmId()!=(*equalbidder_iter)->getFarmId()) {
                          double tmp_offer=(*farms_iter)->getRentOffer(pl);
                          if (tmp_offer>true_second_offer)
                            true_second_offer=tmp_offer;
                        }
                  }
                }
                if (g->FIRSTPRICE) {

                    offer=(*equalbidder_iter)->getRentOffer();
                } else {

                    offer=true_second_offer;
                }
                double f_tac=0;
                double tac=0;
                if (g->USE_TC_FRAMEWORK)  {
                    f_tac=(*equalbidder_iter)->getFarmTacsOfWantedPlotOfType(pl->getSoilType());
                    tac=(*equalbidder_iter)->getTacsOfWantedPlotOfType(pl->getSoilType());
                }
				
				Region->updateMaxRents(offer,pl->getSoilType());

                (*equalbidder_iter)->setRentedPlot(pl,offer,f_tac);
                if (g->Rent_Variation) {
                    pl->setSecondOffer(true_second_offer);
                }

                if (g->USE_TC_FRAMEWORK) {
                    paid_tacs+=f_tac;
                    total_tacs+=tac;
                    if (pl->getPreviouslyRentedByAgent()==pl->getRentedByAgent())
                        stay_at_prev_owner+=g->PLOT_SIZE;
                    if ( pl->getPreviouslyRentedByAgent()==pl->getRentedByAgent() && ((maxoffer-true_second_offer)<tac))
                        stay_at_prev_owner_because_of_tacs+=g->PLOT_SIZE;
                    released_plots+=g->PLOT_SIZE;

                    if (pl->getPreviouslyRentedByLegalType()==pl->getRentedByLegalType() && pl->getRentedByLegalType()==3 && pl->getPreviouslyRentedByAgent()!=pl->getRentedByAgent())
                        CF_to_CF+=g->PLOT_SIZE;
                    if (pl->getPreviouslyRentedByLegalType()==pl->getRentedByLegalType() && pl->getRentedByLegalType()==1 && pl->getPreviouslyRentedByAgent()!=pl->getRentedByAgent())
                        IF_to_IF+=g->PLOT_SIZE;
                    if (pl->getRentedByLegalType()==3 && pl->getPreviouslyRentedByAgent()==pl->getRentedByAgent())
                        stay_CF+=g->PLOT_SIZE;
                    if (pl->getRentedByLegalType()==1 && pl->getPreviouslyRentedByAgent()==pl->getRentedByAgent())
                        stay_IF+=g->PLOT_SIZE;
                    if (pl->getPreviouslyRentedByLegalType()!=pl->getRentedByLegalType() && pl->getRentedByLegalType()==3)
                        IF_to_CF+=g->PLOT_SIZE;
                    if (pl->getPreviouslyRentedByLegalType()!=pl->getRentedByLegalType() && pl->getRentedByLegalType()==1)
                        CF_to_IF+=g->PLOT_SIZE;
                    if (pl->getPreviouslyRentedByLegalType()==3)
                        released_plots_CF+=g->PLOT_SIZE;
                    else
                        released_plots_IF+=g->PLOT_SIZE;
                    if (pl->getRentedByLegalType()==3)
                        rented_plots_CF+=g->PLOT_SIZE;
                    else
                        rented_plots_IF+=g->PLOT_SIZE;
                }

                //Mark plots which have to be updated;
                // pl->identifyContiguousPlot(true,false,false,-1,true);
            }
        }

        return maxoffer;
    } else {
        // maximum offer at a point in time
        double max_offer=0;
        double second_offer=0;
        // list of farms with equal offer
        list<RegFarmInfo* >  equalbidder;
        list<RegFarmInfo* >::iterator farms_iter;
        list<RegFarmInfo* >::iterator equalbidder_iter;

        RegFarmInfo* maxbidder;
        // search for the highest offer
        RegPlotInfo* p=Region->getRandomFreePlot();
        if (p==NULL) return 0;
        else {
            for (farms_iter = FarmList.begin(), max_offer = 0, maxbidder = NULL, equalbidder.clear();
                    farms_iter != FarmList.end();
                    farms_iter++) {

#ifndef NDEBUG1
				g->tFarmName = (*farms_iter)->getFarmName();
				g->tFarmId = (*farms_iter)->getFarmId();
#endif
                if (g->RL && (*farms_iter)==RLfarm && newIteration) {
                    RLdata rlData = getRLdata((*farms_iter), this);
                    //output(rlData, this, "RL.dat");
                    sendRLdata(rlData);
                    (*farms_iter)->set_act_beta(recv_val());
                }

                (*farms_iter)->demandForLand(p);

				double offer = ((*farms_iter)->getRentOffer());
				double offer1;
				if (g->NASG && iteration>=g->NASG_startPeriod) {
					offer1 = NASG_RentOffer(offer, *farms_iter, p->getSoilType());
					if (debug)
						cout << offer << " - " << offer1 <<"\t" << p->getSoilType()<< endl;
					offer = offer1;
				}

                if (offer>0) {  //(*farms_iter)->getRentOffer()>0) {
                    if ((max_offer) == offer ) {//((*farms_iter)->getRentOffer())) {
                        equalbidder.push_back(*farms_iter);
                    } else {
                        // if current offer is higher than maxoffer
                        if ((max_offer) < offer ) { //((*farms_iter)->getRentOffer())) {
                            // set maxbidder to highest bidding farm
                            maxbidder = *farms_iter;
                            // if there is a higher offer, the list of equal bidders
                            // is senseless and is cleared
                            equalbidder.clear();
                            // the equalbidder list is filled
                            // with new offers
                            equalbidder.push_back(maxbidder);
                            second_offer=max_offer;
							max_offer = offer; // maxbidder->getRentOffer();
                        }
                    }// end else
                }
            }// end for
            // allocate land to highest bidder
            if (max_offer > 0) {
                int w =randlong()%equalbidder.size();
				
                equalbidder_iter = equalbidder.begin();
                for (int i=0;i<w;i++)
                    equalbidder_iter++;
                // Second Price Auction!!!
                if (g->FIRSTPRICE) {
					//cout << "Max Offer: " << max_offer << "\t"<< g->NAMES_OF_SOIL_TYPES[p->getSoilType()]<< endl;
					Region->updateMaxRents(max_offer, p->getSoilType());
                    (*equalbidder_iter)->setRentedPlot(p,max_offer,0);
                } else {
                    (*equalbidder_iter)->setRentedPlot(p,second_offer,0);
                }

                if (g->Rent_Variation) {
                    //if (max_offer - second_offer > 0)
                    //    cout << "pl: " << p->getId() << endl;
                    p->setSecondOffer(second_offer);
                }

                count_rented_plots_of_type[p->getSoilType()]++;
            }
         return second_offer;
        }
    }
}

void
RegManagerInfo::readPolicyChanges0() {
    evaluator->clearFunctionBase();
    string e=Policyoutput->getPolicySettingsSep(0);
	current_policy=e;
	int pos = e.find("others:");
	evaluator->addFunctionBase(e.substr(0,pos));
    evaluator->evaluate();
	initPolicy();
}

void
RegManagerInfo::readPolicyChanges() {
    if (g->PRINT_POLICY) {
        Data->openPolicyOutput();

        if (iteration==0)  {
            Data->initPolicyOutput();
        }
    }
    evaluator->clearFunctionBase();
   /*if (iteration != 0) {
        vector<double> val = Data->getSectorValues();
        vector<string> n = Data->getSectorNames();
        for (unsigned int i=0;i<n.size();i++) {
            evaluator->setVariable(n[i],val[i]);
        }
    }
	//*/
    string e=Policyoutput->getPolicySettingsSep(iteration+1);
	current_policy=e;
	size_t pos = e.find("others:");
	evaluator->addFunctionBase(e.substr(0,pos));
    //evaluator->addFunctionBase(e);
    evaluator->evaluate();
}
void
RegManagerInfo::setPolicyChanges() {
    setPremium();
    setDecoupling();
    setLpChangesFromPoliySettingsNaming();
    if (g->PRINT_POLICY) {
        Data->printPolicyOutput("\n");
        Data->closePolicyOutput();
    } 
}

void
RegManagerInfo::setPremium() {
    vector<int> cols=Market->getColsOfPremiumProducts();
    vector<int> rows=Market->getRowsOfPremiumProducts();
    vector<double> premium;
	//vector<int> ninds;
	vector<int> inds = Market->getCatNumberOfPremiumProducts();
    vector<string> names=Market->getNamesOfPremiumProducts();
    for (unsigned int i=0;i<names.size();i++) {
		int ind = evaluator->indOfVariable(names[i]+"_premium");
		if (ind >=0) {
			//ninds.push_back(i);
			double val = evaluator->getVariable(ind);
			//cout << names[i]<<"_premium: \t"<< val<<endl;
			Market->getProductCat()[inds[i]].setPolicyPremium(val);
			premium.push_back(val);//evaluator->getVariable(ind));//names[i]+"_premium"));
		}
		else {
			premium.push_back(Market->getProductCat()[inds[i]].getPolicyPremium());
		}
		//cout << " "<<premium[i];
		//*/
    }
    //cout << endl;
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        for (unsigned int i=0;i<premium.size();i++) {
            (*farms_iter)->lp->setCellValue(cols[i],rows[i],-premium[i]);
        }
    }

	//all products
    if (g->SECTOROUTPUT) {
        for (unsigned int i=0;i<names.size();++i) { //premium.size();i++)  {
            stringstream s;
			s<<names[i]<<"_premium="<<Market->getProductCat()[i].getPolicyPremium() << ";\t";//premium[i]<<";\t";
            Data->printPolicyOutput(s.str());
        }
    }
}
void
RegManagerInfo::calculateExpectedRentalPriceChange() {
    backup();
    g->backup();
    f=static_cast<int>(Region->getExpAvNewRentOfType(0)/Region->getAvRentOfType(0));
    setRegionalDecoupling();
    setFullyDecoupling();
    setFarmspecificDecoupling();
    setLpChangesFromPoliySettingsNaming();
    f=static_cast<int>(Region->getExpAvNewRentOfType(0)/Region->getAvRentOfType(0));

        setLpChangesFromPoliySettingsNaming();
    g->FARMOUTPUT=false;
    g->SECTOROUTPUT=false;
    g->ENV_MODELING=false;
    CapacityEstimationForBidding();
    Disinvest();
    Region->calculateAverageRent();
    Region->calculateAverageNewRent();
    FutureOfFarms();
    SectorResultsAfterDisinvest();
    ResetPeriodLabour();
    SectorOutput();
    EnvSpeciesCalc();
    RemoveFarms();
    PreparationForPeriod();
    // adjust costs by management coefficient
    if (iteration == 0)    readPolicyChanges();
    if (g->CALCULATE_CONTIGUOUS_PLOTS) Region->countContiguousPlots();
    g->WERTS1=RentStatistics();
    g->WERTS=-g->WERTS1;
    LandAllocation();
    Region->calculateAverageRent();
    Region->calculateAverageNewRent();
    vector<double> exp;
    vector<double> exp_new;
    for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
      exp.push_back(Region->getAvRentOfType(i));
      exp_new.push_back(Region->getAvNewRentOfType(i));
    }
    restore();
    g->restore();
    for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
      Region->setExpAvRentOfType(i,exp[i]);
      Region->setExpAvNewRentOfType(i,exp_new[i]);
    }
}
void
RegManagerInfo::setDecoupling() {
    calculateReferencePaymentPerFarm();
    if(g->CALCULATE_EXPECTED_RENTAL_PRICE) {
      if(setModulationData())
          calculateExpectedRentalPriceChange();
    } else {
      setModulationData();
    }
    setRegionalDecoupling();
    setFullyDecoupling();
    setFarmspecificDecoupling();
}

void RegManagerInfo::initPolicy(){
 vector<string> names=Market->getNamesOfPremiumProducts();
	vector<int> inds = Market->getCatNumberOfPremiumProducts();
    for (unsigned int i=0;i<names.size();i++) {
		int ind = evaluator->indOfVariable(names[i]+"_reference_premium_percent");
		if (ind >= 0) {
			double val = evaluator->getVariable(ind);
			Market->getProductCat()[inds[i]].setRefPremPercent(val);
		}
		else 
			Market->getProductCat()[inds[i]].setRefPremPercent(0); //standard
		//cout << names[i] << ": "<< reference_premium_percent[i]<<"\t";

		ind = evaluator->indOfVariable(names[i]+"_premium");
		if (ind >= 0) {
			double val = evaluator->getVariable(ind);
			Market->getProductCat()[inds[i]].setPolicyPremium(val);
		}
	}

	for (unsigned i=0;i<Market->getProductCat().size();++i) {
		int ind = evaluator->indOfVariable(Market->getProductCat()[i].getName()+"_price_change");
		if (ind >= 0) {
			double val = evaluator->getVariable(ind);
			Market->getProductCat()[i].setPriceChange(val);
		}
		else 
			Market->getProductCat()[i].setPriceChange(1); //standard
	}

	int	ind = evaluator->indOfVariable("fully_decoupling");
		if (ind >= 0) {
			double val = evaluator->getVariable(ind);
			g->FULLY_DECOUPLING=int(val);
		}
		else 
			g->FULLY_DECOUPLING=0; //standard
		
		ind = evaluator->indOfVariable("regional_decoupling");
		if (ind >= 0) {
			double val = evaluator->getVariable(ind);
			g->REGIONAL_DECOUPLING=(int)val;
		}
		else 
			g->REGIONAL_DECOUPLING=0; //standard

		ind = evaluator->indOfVariable("farmspecific_decoupling");
		if (ind >= 0) {
			double val = evaluator->getVariable(ind);
			g->FARMSPECIFIC_DECOUPLING=(int)val;
		}
		else 
			g->FARMSPECIFIC_DECOUPLING=0; //standard


	if(g->LP_MOD) {
		int ind;
		ind=evaluator->indOfVariable("tranch_1_deg");
		g->TRANCH_1_DEG=ind>=0? evaluator->getVariable(ind)-1: 0-1;
		ind=evaluator->indOfVariable("tranch_2_deg");
		g->TRANCH_2_DEG=ind>=0? evaluator->getVariable(ind)-1: 0-1;
		ind=evaluator->indOfVariable("tranch_3_deg");
		g->TRANCH_3_DEG=ind>=0? evaluator->getVariable(ind)-1: 0-1;
		ind=evaluator->indOfVariable("tranch_4_deg");
		g->TRANCH_4_DEG=ind>=0? evaluator->getVariable(ind)-1: 0-1;
		ind=evaluator->indOfVariable("tranch_5_deg");
		g->TRANCH_5_DEG=ind>=0? evaluator->getVariable(ind)-1: 0-1;
	} else {
		int ind;
		ind = evaluator->indOfVariable("degression_low_tranch");
		g->DEG_LOW_TRANCH=ind>=0? evaluator->getVariable(ind):0;

		ind = evaluator->indOfVariable("degression_middle_tranch");
		g->DEG_MIDDLE_TRANCH=(ind>=0)?evaluator->getVariable(ind):0;
		
		ind = evaluator->indOfVariable("degression_high_tranch");
		g->DEG_HIGH_TRANCH=ind>=0?evaluator->getVariable(ind):0;
    }
	//cout << "\n";
}

void
RegManagerInfo::calculateReferencePaymentPerFarm() {
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        (*farms_iter)->calculateReferencePeriod();
        if (g->FIX_REFERENCE_PERIOD==iteration) {
            (*farms_iter)->fixReferencePeriod();
        }
    }
    vector<double> reference_premium;
    for (unsigned int i=0;i<Market->getProductCat().size();i++) {
        if (Market->getProductCat()[i].getPremiumLegitimation()) {
            reference_premium.push_back(Market->getProductCat()[i].getReferencePremium());
        }
    }
    vector<string> names=Market->getNamesOfPremiumProducts();
	vector<int> inds = Market->getCatNumberOfPremiumProducts();
    vector<double> reference_premium_percent;
    for (unsigned int i=0;i<names.size();i++) {
		int ind = evaluator->indOfVariable(names[i]+"_reference_premium_percent");
		if (ind >= 0) {
			double val = evaluator->getVariable(ind);
			Market->getProductCat()[inds[i]].setRefPremPercent(val);
			reference_premium_percent.push_back(val);//evaluator->getVariable(ind));//names[i]+"_reference_premium_percent"));
		}
		else 
			reference_premium_percent.push_back(Market->getProductCat()[inds[i]].getRefPremPercent());
		//cout << names[i] << ": "<< reference_premium_percent[i]<<"\t";
    }
	//cout << "\n";

    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        double farm_payment=0;
        vector <double> reference_production=(*farms_iter)->getReferencePeriodProduction();
        for (unsigned int i=0;i<reference_production.size();i++) {
            farm_payment+=reference_production[i]*reference_premium_percent[i]*reference_premium[i];
        }
        if (iteration==0)
            (*farms_iter)->setOldReferenceDirectPayment(farm_payment);
        else
            (*farms_iter)->setOldReferenceDirectPayment((*farms_iter)->getOldReferenceDirectPayment());
        (*farms_iter)->setReferenceDirectPayment(farm_payment);
    }

}
void
RegManagerInfo::ModulateDirectPayments() {
    Region->modulateDirectPayments();
}
void
RegManagerInfo::setLpChangesFromPoliySettings() {
    //go through all variables and select those how affect the farm lp's
    // the following naming convention is used
    // mat_cell_x4y12=0.8;  //to set a cell in the matrix
    // rhs_row4=0;     0: less_equal 1: equal 2: grater equal
    // ub_col5=0;   0:0 1: +INF
    vector<int> mat_x;
    vector<int> mat_y;
    vector<double> mat_val;
    vector<int> rhs_row;
    vector<int> rhs_val;
    vector<int> ub_col;
    vector<int> ub_val;

    int no=evaluator->getNoVariables();
    for (int i=0;i<no;i++) {
        string var =evaluator->getVariableName(i);
        double val=evaluator->getVariable(i);
        int pos;
        if ((pos=var.find("mat_cell_x",0))!=std::string::npos) {
            var.replace(0,10,"");
            if ((pos=var.find("y",0))!=std::string::npos) {
                var.replace(pos,1," ");
                istringstream var_s(var);
                int x;
                var_s>>x;
                int y;
                var_s>>y;
                mat_x.push_back(x);
                mat_y.push_back(y);
                mat_val.push_back(val);
            }
        }
        if ((pos=var.find("rhs_row",0))!=std::string::npos) {
            var.replace(0,7,"");
            istringstream var_s(var);
            int row;
            var_s>>row;
            rhs_row.push_back(row);
            rhs_val.push_back(static_cast<int>(val));
        }
        if ((pos=var.find("ub_col",0))!=std::string::npos) {
            var.replace(0,6,"");
            istringstream var_s(var);
            int col;
            var_s>>col;
            ub_col.push_back(col);
            ub_val.push_back(static_cast<int>(val));
        }

    }
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        for (unsigned int i=0;i<mat_val.size();i++) {
            (*farms_iter)->lp->setCellValue(mat_x[i],mat_y[i],mat_val[i]); //set aside
        }
        for (unsigned int i=0;i<rhs_val.size();i++) {
            switch (rhs_val[i]) {
            case 0:
                (*farms_iter)->lp->setSenseLessEqual(rhs_row[i]);
                break;
            case 1:
                (*farms_iter)->lp->setSenseEqual(rhs_row[i]);
                break;
            case 2:
                (*farms_iter)->lp->setSenseGreaterEqual(rhs_row[i]);
                break;
            default:
                break;
            }
        }
        for (unsigned int i=0;i<ub_val.size();i++) {
            switch (ub_val[i]) {
            case 0:
                (*farms_iter)->lp->setUBoundZero(ub_col[i]);
                break;
            case 1:
                (*farms_iter)->lp->setUBoundInf(ub_col[i]);
                break;
            default:
                break;
            }
        }
        //  (*farms_iter)->updateLpValues();
    }
}

void RegManagerInfo::setLpChangesFromPoliySettingsNaming() {
    //go through all variables and select those how affect the farm lp's
    // the following naming convention is used
    // mat_cell:col, row=0.8;  //to set a cell in the matrix
    // rhs_row:rowname=LE;     LE: less_equal EQ: equal GE: grater equal
    // ub_col:colname=value;   value: 0 or INF
    vector<int> mat_x;
    vector<int> mat_y;
    vector<double> mat_val;
    vector<int> rhs_row;
    vector<int> rhs_val;
    vector<int> ub_col;
    vector<int> ub_val;

	map <string,int> rowindex, colindex;
	for (unsigned int i=0; i<matrixdata.rownames.size();++i) {
		rowindex[matrixdata.rownames[i]]=i;
	}
	for (unsigned int i=0; i<matrixdata.colnames.size();++i){
		colindex[matrixdata.colnames[i]]=i;
	}
	size_t pos0 = current_policy.find("others:");
	size_t pos1=pos0;
	size_t pos2,pos3,pos4;
	int x,y;
	double val;
	while (pos1 != string::npos) {
		pos1=current_policy.find("mat_cell",pos1);
		if (pos1==string::npos) break;
		pos1=current_policy.find(":",pos1);
		pos2=current_policy.find(",",pos1);
		pos3=current_policy.find("=",pos2);
		pos4=current_policy.find(";",pos3);
		string colname=trim(current_policy.substr(pos1+1,pos2-pos1-1));
		string rowname = trim(current_policy.substr(pos2+1,pos3-pos2-1));
		std::transform(rowname.begin(),rowname.end(),rowname.begin(), (int (*)(int)) std::toupper);
		std::transform(rowname.begin(),rowname.end(),rowname.begin(), (int (*)(int)) std::toupper);
		y = rowindex[rowname];//trim(current_policy.substr(pos1+1,pos2-pos1-1))];
		x = colindex[colname];//trim(current_policy.substr(pos2+1,pos3-pos2-1))];
		val = atof(trim(current_policy.substr(pos3+1,pos4-pos3-1)).c_str());

		mat_x.push_back(x);
        mat_y.push_back(y);
        mat_val.push_back(val);
		pos1=pos4;
	}

	pos1=pos0;
	while (pos1 != string::npos) {
		pos1=current_policy.find("rhs_row",pos1);
		if (pos1==string::npos) break;
		pos1=current_policy.find(":",pos1);
		pos2=current_policy.find("=",pos1);
		pos3=current_policy.find(";",pos2);
		string rowname=trim(current_policy.substr(pos1+1,pos2-pos1-1));
		std::transform(rowname.begin(),rowname.end(),rowname.begin(), (int (*)(int)) std::toupper);
		x = rowindex[rowname];
		string rhs = trim(current_policy.substr(pos2+1,pos3-pos2-1));

		if (rhs=="LE")
		  val = 0;
		else if (rhs=="GE") 
			val=2;
		else if (rhs=="EQ")
			val=1;
		else {
			cout << "LE ???\n";
			val=0;
		}
		
		rhs_row.push_back(x);
        rhs_val.push_back(static_cast<int>(val));
		pos1=pos3;
		//cout << "rhs: "<<x<<"=" <<val << "; ";
	}

	pos1=pos0;
	while (pos1 != string::npos) {
		pos1=current_policy.find("ub_col",pos1);
		if (pos1==string::npos) break;
		pos1=current_policy.find(":",pos1);
		pos2=current_policy.find("=",pos1);
		pos3=current_policy.find(";",pos2);
		string colname=trim(current_policy.substr(pos1+1,pos2-pos1-1));
		std::transform(colname.begin(),colname.end(),colname.begin(), (int (*)(int)) std::toupper);
		y = colindex[colname];//trim(current_policy.substr(pos1+1,pos2-pos1-1))];
		string bound = trim(current_policy.substr(pos2+1,pos3-pos2-1));
		if (bound=="INF")
			val=1;
		else 
			val = 0;// atof(bound.c_str());
		ub_col.push_back(y);
        ub_val.push_back(static_cast<int>(val));

		pos1=pos3;
	}

    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        for (unsigned int i=0;i<mat_val.size();i++) {
            (*farms_iter)->lp->setCellValue(mat_x[i],mat_y[i],mat_val[i]); //set aside
        }
        for (unsigned int i=0;i<rhs_val.size();i++) {
            switch (rhs_val[i]) {
            case 0:
                (*farms_iter)->lp->setSenseLessEqual(rhs_row[i]);
                break;
            case 1:
                (*farms_iter)->lp->setSenseEqual(rhs_row[i]);
                break;
            case 2:
                (*farms_iter)->lp->setSenseGreaterEqual(rhs_row[i]);
                break;
            default:
                break;
            }
        }
        for (unsigned int i=0;i<ub_val.size();i++) {
            switch (ub_val[i]) {
            case 0:
                (*farms_iter)->lp->setUBoundZero(ub_col[i]);
                break;
            case 1:
                (*farms_iter)->lp->setUBoundInf(ub_col[i]);
                break;
            default:
                break;
            }
        }
        //  (*farms_iter)->updateLpValues();
    }
}

void
RegManagerInfo::setFullyDecoupling() {
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        if (g->FULLY_DECOUPLING_SWITCH) {
            if (g->FULLY_DECOUPLING) {
                (*farms_iter)->setDirectPayment((*farms_iter)->getReferenceDirectPayment()/*-50*land_input*/);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            } else {
                (*farms_iter)->setDirectPayment(0);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            }
        } else {
            if (g->FULLY_DECOUPLING) {
                (*farms_iter)->setDirectPayment((*farms_iter)->getReferenceDirectPayment()/*-50*land_input*/);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            }
        }
    }
}

void
RegManagerInfo::setRegionalDecoupling() {
    if (g->REGIONAL_DECOUPLING_SWITCH) {
        //Switch on regional decoupling
        if (g->REGIONAL_DECOUPLING) {
            double total_payment=0;
            double total_land_input=0;
            list<RegFarmInfo* >::iterator farms_iter;
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                total_payment+=(*farms_iter)->getReferenceDirectPayment();
                total_land_input+=(*farms_iter)->getLandInput();
            }
            total_land_input+=Region->getNumberOfFreePlots()*g->PLOT_SIZE;
            double payment_per_ha=total_payment/total_land_input;
            for (unsigned int i=0;i<Region->plots.size();i++) {
                Region->plots[i]->setPaymentEntitlement(payment_per_ha*g->PLOT_SIZE);
            }
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                double land_input =(*farms_iter)->getLandInput();
                (*farms_iter)->setDirectPayment(land_input*payment_per_ha);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            }
        } else {
            list<RegFarmInfo* >::iterator farms_iter;
            for (unsigned int i=0;i<Region->plots.size();i++) {
                Region->plots[i]->setPaymentEntitlement(0);
            }
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                (*farms_iter)->setDirectPayment(0);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            }
        }
    } else {
        if (g->REGIONAL_DECOUPLING) {
            //changing situation, later
        }
    }
}
void
RegManagerInfo::setFarmspecificDecoupling() {
    if (g->FARMSPECIFIC_DECOUPLING_SWITCH) {
        //Switch on regional decoupling
        if (g->FARMSPECIFIC_DECOUPLING) {
            list<RegFarmInfo* >::iterator farms_iter;
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                double payment =(*farms_iter)->getReferenceDirectPayment();
                double land_input =(*farms_iter)->getLandInput();
                double payment_per_ha=payment/land_input;

                (*farms_iter)->setDirectPaymentPerPlot(payment_per_ha*g->PLOT_SIZE);

                (*farms_iter)->setDirectPayment(payment);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            }
        } else {
            list<RegFarmInfo* >::iterator farms_iter;
            for (unsigned int i=0;i<Region->plots.size();i++) {
                Region->plots[i]->setPaymentEntitlement(0);
            }
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                (*farms_iter)->setDirectPayment(0);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            }
        }
    } else {
        if (g->FARMSPECIFIC_DECOUPLING) {
            //changing situation, later
        }
    }
}
bool
RegManagerInfo::setModulationData() {
bool exp_rent=false;
int ind = evaluator->indOfVariable("fully_decoupling");
if (ind >=0 ){
   if (g->FULLY_DECOUPLING==evaluator->getVariable("fully_decoupling")) {
        g->FULLY_DECOUPLING_SWITCH=0;
    } else {
        g->FULLY_DECOUPLING_SWITCH=1;
        g->FULLY_DECOUPLING=static_cast<int>(evaluator->getVariable("fully_decoupling"));
        if(g->FULLY_DECOUPLING) exp_rent=true;
    }
} else 
	g->FULLY_DECOUPLING_SWITCH=0;

ind =evaluator->indOfVariable("regional_decoupling");
if (ind>=0) {
    if (g->REGIONAL_DECOUPLING==evaluator->getVariable("regional_decoupling")) {
        g->REGIONAL_DECOUPLING_SWITCH=0;
    } else {
        g->REGIONAL_DECOUPLING_SWITCH=1;
        g->REGIONAL_DECOUPLING=static_cast<int>(evaluator->getVariable("regional_decoupling"));
        if(g->REGIONAL_DECOUPLING) exp_rent=true;
    }
}else
	g->REGIONAL_DECOUPLING_SWITCH=0;

ind=evaluator->indOfVariable("farmspecific_decoupling");
if (ind>=0) {
    if (g->FARMSPECIFIC_DECOUPLING==evaluator->getVariable("farmspecific_decoupling")) {
        g->FARMSPECIFIC_DECOUPLING_SWITCH=0;
    } else {
        g->FARMSPECIFIC_DECOUPLING_SWITCH=1;
        g->FARMSPECIFIC_DECOUPLING=static_cast<int>(evaluator->getVariable("farmspecific_decoupling"));
        if(g->FARMSPECIFIC_DECOUPLING) exp_rent=true;
    }
}else
	g->FARMSPECIFIC_DECOUPLING_SWITCH=0;

	if(g->LP_MOD) {
		int ind = evaluator->indOfVariable("tranch_1_deg");
		if (ind>=0) 
			g->TRANCH_1_DEG=evaluator->getVariable(ind)-1;
		ind = evaluator->indOfVariable("tranch_2_deg");
		if (ind>=0) 
			g->TRANCH_2_DEG=evaluator->getVariable(ind)-1;
		ind = evaluator->indOfVariable("tranch_3_deg");
		if (ind>=0) 
			g->TRANCH_3_DEG=evaluator->getVariable(ind)-1;
		ind = evaluator->indOfVariable("tranch_4_deg");
		if (ind>=0) 
			g->TRANCH_4_DEG=evaluator->getVariable(ind)-1;
		ind = evaluator->indOfVariable("tranch_5_deg");
		if (ind>=0) 
			g->TRANCH_5_DEG=evaluator->getVariable(ind)-1;

    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
         farms_iter != FarmList.end();
         farms_iter++) {
        (*farms_iter)->updateLpValues();
    }

} else {
	int ind =evaluator->indOfVariable("degression_low_tranch");
	if (ind>=0) 
		g->DEG_LOW_TRANCH= evaluator->getVariable(ind);
	ind =evaluator->indOfVariable("degression_middle_tranch");
	if (ind>=0) 
		g->DEG_MIDDLE_TRANCH= evaluator->getVariable(ind);
	ind =evaluator->indOfVariable("degression_high_tranch");
	if (ind>=0) 
		g->DEG_HIGH_TRANCH= evaluator->getVariable(ind);

    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
         farms_iter != FarmList.end();
         farms_iter++) {
        (*farms_iter)->modulateIncomePayment();
    }
}
    return exp_rent;
}

void
RegManagerInfo::backup() {
    obj_backup=clone();
    Sector->backup();
    if (g->ENV_MODELING)
        Env->backup();
    Region->backup();
    Market->backup();
    evaluator->backup();
    list<RegFarmInfo* >::const_iterator farms;
    for (farms = FarmList.begin();
            farms != FarmList.end();
            farms++) {
        (*farms)->backup();
    }
    for (farms = RemovedFarmList.begin();
            farms != RemovedFarmList.end();
            farms++) {
        (*farms)->backup();
    }
}
void RegManagerInfo::assign() {
    *((RegManagerInfo*)this)=*obj_backup;
}
;
void
RegManagerInfo::restore() {
    RegManagerInfo* tmp=obj_backup;
    assign();
    obj_backup=tmp;

    list<RegFarmInfo* >::const_iterator farms;
    for (farms = FarmList.begin();
            farms != FarmList.end();
            farms++) {
        (*farms)->restore();
    }
    for (farms = RemovedFarmList.begin();
            farms != RemovedFarmList.end();
            farms++) {
        (*farms)->restore();
    }
    Sector->restore();
    if (g->ENV_MODELING)
        Env->restore();
    Region->restore();
    Market->restore();
    evaluator->restore();
}
void
RegManagerInfo::printShadowPrices(int nop)  {
    ofstream out;
    string file=g->OUTPUTFILE + string("avshadow.dat");
    out.open(file.c_str(),ios::ate);
    ofstream out2;
    string file2=g->OUTPUTFILE + string("shadow.dat");
    out2.open(file2.c_str(),ios::ate);
    ofstream out3;
    string file3=g->OUTPUTFILE + string("dk.dat");
    out2.open(file2.c_str(),ios::ate);

    list<RegFarmInfo* >::const_iterator farms;

    for(int i=0;i<=g->NO_OF_SOIL_TYPES;i++) {
      for (farms = FarmList.begin();
           farms != FarmList.end();
           farms++) {
              out << (*farms)->getFarmId() << "\t";
              out2 << (*farms)->getFarmId() << "\t";
              out3 << (*farms)->getFarmId() << "\t";
      }
    }
    out << "\n";
    out2 << "\n";
    for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
      for (farms = FarmList.begin();
           farms != FarmList.end();
           farms++) {
              out << (*farms)->getLandInputOfType(i) << "\t";
              out2 << (*farms)->getLandInputOfType(i) << "\t";
              out3 << (*farms)->getLandInputOfType(i) << "\t";
      }
    }
    for (farms = FarmList.begin();
         farms != FarmList.end();
         farms++) {
         double sum=0;
        for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
            sum+=(*farms)->getLandInputOfType(i);
        }
        out << sum << "\t";
        out2 << sum << "\t";
        out3 << sum << "\t";

    }
    out << "\n";
    out2 << "\n";
    out3 << "\n";
    vector<int> soils;
    for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
      soils.push_back(0);
    }

    for(int i=0;i<nop;i++) {
      for(int k=0;k<g->NO_OF_SOIL_TYPES;k++) {
        for (farms = FarmList.begin();
             farms != FarmList.end();
            farms++) {
              for(int j=0;j<g->NO_OF_SOIL_TYPES;j++) {
                if(k==j)
                  soils[j]=i;
                else
                  soils[j]=0;
              }
              double val=(*farms)->getValueOfPlots(soils);
              out << ((i==0)? 0 :(val/(i*g->PLOT_SIZE))) << "\t";
              out2 << ( (i==0)? 0 :val) << "\t";

        }
      }
      for(int j=0;j<g->NO_OF_SOIL_TYPES;j++) {
        soils[j]=i/2;
      }
      for (farms = FarmList.begin();
           farms != FarmList.end();
           farms++) {
           if((i%2)==0) {
              double val=(*farms)->getValueOfPlots(soils);
              out << ((i==0)? 0 :(val/(i*g->PLOT_SIZE))) << "\t";
              out2 << ((i==0)? 0 :val) << "\t";
           } else {
             out <<  "\t";
             out2 <<  "\t";
           }
      }
      out << "\n";
      out2 << "\n";
    }
    out.close();
    out2.close();
    out3.close();
}

RegFarmInfo* RegManagerInfo::createFarm(RegRegionInfo * reg,
                                        RegGlobalsInfo* G,
                                        vector<RegProductInfo >& PCat,
                                        vector<RegInvestObjectInfo >& ICat ,
                                        RegLpInfo* lporig,
                                        short int pop,
                                        int number,
                                        int fc,
                                        string farmname,
                                        int farmerwerbsform) {
    return new RegFarmInfo(reg,G,PCat,ICat ,lporig,pop,number,fc,farmname,farmerwerbsform);
}

RegManagerInfo* RegManagerInfo::clone() {
    RegManagerInfo* m=new RegManagerInfo(*this);
    m->flat_copy=true;
    return m;
}

RegManagerInfo* RegManagerInfo::create() {
    return new RegManagerInfo();
}
