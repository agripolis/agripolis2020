/*************************************************************************
* This file is part of AgriPoliS
*
* AgriPoliS: An Agricultural Policy Simulator
*
* Copyright (c)  Alfons Balmann, Kathrin Happe, Konrad Kellermann et al.
* (cf. AUTHORS.md) at Leibniz Institute of Agricultural Development in 
* Transition Economies
*
* SPDX-License-Identifier: CC-BY-NC-ND-4.0
**************************************************************************/

//  functions for reading text input files, without policy setting file

#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <iterator>
#include <vector>
#include "textinput.h"
#include "RegGlobals.h"

string inputdir;
bool hasCarbon;
extern RegGlobalsInfo* gg;
static void tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = ": \t=;")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find next "delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);
    int n= 0;
    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        n++;

        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
    if (n<2) tokens.push_back("=");
}

//static
 void glob(string fname=GLOBFILE) {
    ifstream ins;
    stringstream gfile;
	gfile << inputdir << fname;  // GLOBFILE;
    ins.open(gfile.str().c_str(),ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << gfile.str() << "\n";
	   exit(2);
    }
    string s, s2, sback;
    string::iterator it;


    while (!ins.eof()){
        getline(ins,s);
        sback = s;

		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;

		transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(),
              (int(*)(int)) std::toupper);
        
		if (tokens[0]=="NAMES_OF_SOIL_TYPES") {
		   for (unsigned int k=1; k< tokens.size(); ++k) {
                globdata.namesOfSoilTypes.push_back(tokens[k]);
           }
        }else {
		//	if (gg->Scenario_globs.count(tokens[0]))
		//		globdata.globs.insert(pair<string, string>(tokens[0], gg->Scenario_globs[tokens[0]]));
        //   else 
			globdata.globs.insert(pair<string, string>(tokens[0], tokens[1]));
		}
    }
    ins.close();
	for (auto x : gg->Scenario_globs) {
		globdata.globs[x.first] = x.second;
	}
    return;
}

void demograph() {
	 glob(DEMOGRAPH_FILE);
 }

void youngfarmer() {
	glob(YOUNGFARMER_FILE);
}

//static
 void trans() {
    ifstream ins;
    stringstream ifile;
    ifile<< inputdir <<TRANSLFILE;
    ins.open(ifile.str().c_str(),ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ifile.str() << "\n";
	   exit(2);
    }

    string s, s2;
    string::iterator it;

    while (!ins.eof()){
        getline(ins,s);

		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;

       // std::transform(s.begin(), s.end(), s.begin(),
         //      (int(*)(int)) std::toupper);

        istringstream iss(s);
        string t1, t2;
        iss >> t1;
		std::transform(t1.begin(), t1.end(), t1.begin(),
			      (int(*)(int)) std::toupper);

        iss >> t2;
        transdata.trans.insert(pair<string, string>(t1,t2));
    }
    ins.close();
    return;
}

//static
void farms() {
    ifstream ins;
    stringstream ifile;
    ifile<< inputdir <<FARMSDATAFILE;
    ins.open(ifile.str().c_str(),ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ifile.str() << "\n";
	   exit(2);
    }

    int noft;

    string s, s2;
    string::iterator it;

    while (!ins.eof()){
        getline(ins,s);

		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;

        istringstream is(s);
        is >> s2;
        string t, t1, t2;
        std::transform(s2.begin(), s2.end(), s2.begin(),
               (int(*)(int)) std::toupper);

        if (s2.compare("NUMOFFARMS")==0) {
             is >> t1;
             noft=atoi(t1.c_str());
             farmsdata.numOfFarms= noft;
        }else if (s2.find("NAME")!=s2.npos){
             while ( is >> t2 ){
                    farmsdata.names.push_back(t2);
             }
        }
        else if   (s2.find("FARM_TYP")!=s2.npos){
             while ( is >> t2 ) {
                  farmsdata.farm_types.push_back(atoi(t2.c_str()));
             }
        }
        else if   (s2.find("FORM_OF_ORG")!=s2.npos){
              while ( is >> t2 ) {
                    farmsdata.formsOrgs.push_back(atoi(t2.c_str()));
                  }
              }
        else if   (s2.find("WEIGHTING_F")!=s2.npos){
              while ( is >> t2 ) {
                     farmsdata.weightFacs.push_back(atoi(t2.c_str()));
              }
        }
        else if   (s2.find("LAND_INP")!=s2.npos){
              while (is >> t2 ) {
                    farmsdata.land_inputs.push_back(atof(t2.c_str()));
              }
        }
        else if   (s2.find("MILK_QU")!=s2.npos){
                  while ( is >> t2 ){
                      farmsdata.milk_quotas.push_back(atof(t2.c_str()));
                  }
              }

        else if   (s2.compare("FAM_LABOUR_UNITS")==0){
                  while ( is >> t2 ){
                      farmsdata.fam_lab_units.push_back(atof(t2.c_str()));
                  }
              }
        else if   (s2.find("OFF_FAM")!=s2.npos){
                  while ( is >> t2 ){
                      farmsdata.off_fam_labs.push_back(atof(t2.c_str()));
                  }
              }
        else if   (s2.find("EQUITY_CAP")!=s2.npos){
                  while ( is >> t2 ){
                      farmsdata.equity_capitals.push_back(atof(t2.c_str()));
                  }
              }
        else if   (s2.find("LAND_ASS")!=s2.npos){
                  double d;
                  while ( is >> d ){
                      farmsdata.land_assets.push_back(d);//atof(t2.c_str()));
                  }
              }
        else if   (s2.find("REL_INVEST_A")!=s2.npos){
                  while ( is >> t2 ){
                      if (t2.compare("")==0) continue;
                      farmsdata.rel_invest_ages.push_back(atof(t2.c_str()));
                  }
              }

        else  {
                 int sz = globdata.namesOfSoilTypes.size();
                 size_t found=s.npos;
                 string ts;
                 for (int i =0 ; i<sz; i++) {
                      ts =  globdata.namesOfSoilTypes[i] ;
                      string tt = ts;
                      //std::transform(tt.begin(), tt.end(), tt.begin(),
                        //    (int(*)(int)) std::toupper);
                      found=s.find(tt);
                      if (found != s.npos) break;
                 }
				 if (found==s.npos) continue;
                 lands landsvecs;
                 vector <double> ol;
                 vector <double> rl;
                 vector <double> irp;

				 vector <double> c_mean;
				 vector <double> c_var;

                 if (found!=s.npos) found = 1;

      int j = hasCarbon? 5 : 3;
	  
      while ((j>0 ) && (found==1)) {
         getline(ins,s);
         if (s.compare("")==0) continue;
         std::transform(s.begin(), s.end(), s.begin(),
               (int(*)(int)) std::toupper);
         istringstream iss(s);
         iss >> s2;
         if (s2[0]=='#') continue;

               if  (s2.find("OWNED_L")!=s2.npos){
                   j--;
                   while ( iss >> t2) {
                       ol.push_back(atof(t2.c_str()));
                   }
                }else if ( s2.find("RENTED_L")!=s2.npos){
                   j--;
                   while ( iss >> t2 ) {
                       rl.push_back(atof(t2.c_str()));
                   }
                }else if ( s2.find("INITIAL_R")!=s2.npos){
                   j--;
                   while ( iss >> t2 ) {
                       irp.push_back(atof(t2.c_str()));
                   }
				}else if ( s2.find("CARBON_MEAN")!=s2.npos){
                   j--;
                   while ( iss >> t2 ) {
                       c_mean.push_back(atof(t2.c_str()));
                   }
				}else if ( s2.find("CARBON_STD_DEV")!=s2.npos){
                   j--;
                   while ( iss >> t2 ) {
                       c_var.push_back(atof(t2.c_str()));
                   }
                } else {
                    throw 1;
                }
      }
      landsvecs.owned_land=ol;
      landsvecs.rented_land=rl;
      landsvecs.initial_rent_price=irp;

	  if (hasCarbon) {
			landsvecs.carbon_mean = c_mean;
			landsvecs.carbon_std_dev = c_var;
	  }	
	  
      farmsdata.alllands.insert(pair<string, lands>(ts,landsvecs));

		} // else
    }
    ins.close();

    return;
}

static map <string, int> initInvestCols;
//static
void nfarms() {
    int noft = farmsdata.names.size();
    string name, namefile;
 for (int i = 0; i< noft; i++) {
    ifstream ins;
    stringstream ss;

    name = farmsdata.names[i];
    ss << inputdir << FARMDIR << name << TXT;
    ins.open(ss.str().c_str() ,ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ss.str() << "\n";
	   exit(2);
    }
    string s, sback, str1, str2;
    farminvestdata fid;
    vector <oneInvest> invs;
    while (!ins.eof()){
        getline(ins,s);
        sback = s;

		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;

        istringstream is(s);
        is >> str1;
        std::transform(str1.begin(), str1.end(), str1.begin(),
               (int(*)(int)) std::toupper);
        if (str1.find("FARMNAME")!=str1.npos) {
            is >> str2;  // Name
            if (str2.compare(name)!=0)
                printf("WARNING: %s != %s \n", name.c_str(), str2.c_str());
            continue;
        }

        if (str1[0]=='!') {
            int i=0;
            while ( is >> str1) {
                std::transform(str1.begin(), str1.end(), str1.begin(),
                       (int(*)(int)) std::toupper);
                if (!(str1.compare("INVESTITION") && str1.compare("ANZAHL") && str1.compare("KAPAZITAET")))
                    initInvestCols[str1]=i;
                i++;
            }
            continue;
        }

        int j=0;
        string nam;
        int anz=0;
        int cap=0;

        istringstream iss(sback);
        while ( iss >> str1) {
             fid.initinvests.clear();

             if (initInvestCols["INVESTITION"]==j)
                    nam = str1;
             else if ( initInvestCols["ANZAHL"]==j)
                    anz = atoi(str1.c_str());
             else if ( initInvestCols["KAPAZITAET"] == j )
                    cap = atoi (str1.c_str());
             j++;
        }
        invs.push_back(oneInvest(nam, anz, cap));
    }
    fid.initinvests= invs;
    farmsIinvest[name]=fid;
    ins.close();
  }
  return;
}

//static
void readenv() {
    int zust=0;
    ifstream ins;
    stringstream ifile;
    ifile<< inputdir <<ENVIRONFILE;
    ins.open(ifile.str().c_str(),ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ifile.str() << "\n";
	   exit(2);
    }

    string s, s2, sback;
    int grp=0;

    vector <string> snames;
    int snum;
    group g;
    biohab bh;

  while (!ins.eof()){
        getline(ins,s);
        sback = s;

		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;

        std::transform(s.begin(), s.end(), s.begin(),
               (int(*)(int)) std::toupper);
        istringstream is(s);
        is >> s2;

        istringstream iss(sback);
        string t1, t2,tt;
        act akt;
      switch (zust) {
      case 0:
          if (s2.compare("Z_COEFFICIENT")==0) {
             double d ;
             is >> d;
             envdata.zcoef=d;
             continue;
           }
           else if (s2.compare("BIOHABITATS")==0) {
             zust=1;
             continue;
           }
           else if (s2.compare("ASSOCIATED")==0) {
              continue;
           }
           else if (s2.compare("GROUP")==0) {
                zust=2;
                continue;
           }
           else cout << "WARNING: Error while reading " << ENVIRONFILE << "\n";

           break;
        case 1:
              iss >> t1;
              tt = t1;
              std::transform(tt.begin(), tt.end(), tt.begin(),
               (int(*)(int)) std::toupper);
              if (tt.compare("ASSOCIATED")==0) {
                    zust = 0;
                    continue;
              }
              iss >> t2;
              bh.name= t1;
              bh.value= atof(t2.c_str());
              envdata.biohabs.push_back(bh);
              break;
        case 2:
              iss >> t1;
              tt = t1;
              std::transform(tt.begin(), tt.end(), tt.begin(),
               (int(*)(int)) std::toupper);
              if (tt.compare("SOILS")==0) {
                 zust=3;
                 snum=0;
                 snames.clear();
                 while ( iss >> t2) {
                    //if (t2.compare(":")==0) continue;
                    g.soilNames.push_back(t2);
                    snum++;
                 }
                 g.numSoils=snum;

               }
               else cout << "WARNING: SOILS ? in  "<< ENVIRONFILE << "\n";
               break;
          case 3 :
              iss >> t1;
              tt = t1;
              std::transform(tt.begin(), tt.end(), tt.begin(),
               (int(*)(int)) std::toupper);
              if (tt.compare("GROUP")==0) {
                    zust = 2;
                    envdata.groups.push_back(g);
                    g.acts.clear();
                    g.soilNames.clear();
                    grp++;
                    continue;
              }
              iss >> t2;
              akt.name= t1;
              akt.q = atoi(t2.c_str());
              g.acts.push_back(akt);
              break;
          default:    ;
        }
    }
    envdata.groups.push_back(g);
    envdata.numGroups=++grp;
    ins.close();

    return;
}

static map <string, int> marketCols;
//static
void readmarket() {
	vector<string> names{ "ST_BORROW_CAP","EC_INTEREST" ,"V_HIRED_LABOUR", "V_OFF_FARM_LAB"};
	for (auto &nam : names) {
		oneproduct prod;
		int ind;
		float lab;
		if (nam == "ST_BORROW_CAP") {
			ind = gg->ST_BOR_INTERESTTYPE;
			lab = 0;
		}
		else if (nam == "EC_INTEREST") {
			ind = gg->ST_EC_INTERESTTYPE;
			lab = 0;
		}
		else if (nam == "V_HIRED_LABOUR") {
			ind = gg->VARHIREDLABTYPE;
			lab = -1;
		}
		else if (nam == "V_OFF_FARM_LAB") {
			ind = gg->VAROFFARMLABTYPE;
			lab = 1;
		}
		if (globdata.globs[nam] != "") {
			prod = oneproduct(ind, nam,
				atof(globdata.globs[nam].c_str()), lab);
			marketdata.products.push_back(prod);
		}
	}
	ifstream ins;

    stringstream ifile;
    ifile<< inputdir <<MARKETFILE;
    ins.open(ifile.str().c_str(),ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ifile.str() << "\n";
	   exit(2);
    }

    string s, sback, str1, str2;
    while (!ins.eof()){
        getline(ins,s);
        sback= s;

		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;

        std::transform(s.begin(), s.end(), s.begin(),
               (int(*)(int)) std::toupper);
        istringstream is(s);
        is >> str1;

        if (str1.find("PREMIUMNAME")!=str1.npos) {
            is >> str2;  // Name
            if (str2.compare("")==0)
                cout << "WARNING: NO PREMIUM NAME !" << endl;
            else marketdata.premiumName=str2;
            continue;
        }


        if (str1[0]=='!') {
            int i=0;
            while ( is >> str1) {
                if (!(str1.compare("TYPE") && str1.compare("GROUP") && str1.compare("NAME") && str1.compare("STDNAME") 
                    &&str1.compare("PRICE") && str1.compare("VARCOST") && str1.compare("LABOUR") && str1.compare("PRICEFLEX")
                    &&str1.compare("CHANGERATE") && str1.compare("PRODUCEDBYFARMTYPE") && str1.compare("LUPERPLACE")
                    &&str1.compare("PREMIUM") && str1.compare("INITPREM") && str1.compare("PRICESUPP")
                    &&str1.compare("REFPREM") && str1.compare("XYEARS")  ))
                        marketCols[str1]=i;
                i++;
            }
            continue;
        }

        int j=0;
        oneproduct prod;

        istringstream iss(sback);
        while ( iss >> str1) {
             if (marketCols["TYPE"]==j)
                    prod.type = atoi(str1.c_str());
             else if ( marketCols["GROUP"]==j)
                    prod.group = atoi(str1.c_str());
             else if ( marketCols["NAME"] == j )
                    prod.name = str1;
             else if ( marketCols["STDNAME"] == j )
                    prod.stdName = str1;
             else if ( marketCols["PRICE"] == j )
                    prod.price = atof (str1.c_str());
             else if ( marketCols["VARCOST"]==j)
                    prod.varcost = atof(str1.c_str());
			 else if ( marketCols["LABOUR"] == j)
                   prod.labour = atof(str1.c_str());
             else if ( marketCols["PRICEFLEX"] == j )
                    prod.priceflex = atof (str1.c_str());
             else if ( marketCols["CHANGERATE"]==j)
                    prod.changerate = atof(str1.c_str());
             else if ( marketCols["PRODUCEDBYFARMTYPE"]==j)
                    prod.prodbytype = str1;

             else if ( marketCols["LUPERPLACE"] == j )
                    prod.luperplace = atof (str1.c_str());
             else if ( marketCols["PREMIUM"]==j)  {
                    std::transform(str1.begin(), str1.end(), str1.begin(),
                       (int(*)(int)) std::toupper);
                    prod.premium = str1.compare("TRUE")==0 ? true: false;
             }
             else if ( marketCols["INITPREM"] == j )
                    prod.initprem = atof (str1.c_str());
             else if ( marketCols["PRICESUPP"]==j)
                    prod.pricesupp = str1.compare("TRUE")==0 ? true: false;
             else if ( marketCols["REFPREM"] == j )
                    prod.refprem = atoi (str1.c_str());
             else if ( marketCols["XYEARS"]==j)
                    prod.xyears = atoi(str1.c_str());
             j++;
        }
        marketdata.products.push_back(prod);
  }
  ins.close();

  return;
}

static map <string, int> envCols;
//static
void readenvmarket() {
    ifstream ins;

    stringstream ifile;
    ifile<< inputdir <<ENV_MARKETFILE;
    ins.open(ifile.str().c_str(),ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ifile.str() << "\n";
	   exit(2);
    }

    string s, str1, str2, sback;
    while (!ins.eof()){
        getline(ins,s);
        sback = s;

		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;

        std::transform(s.begin(), s.end(), s.begin(),
               (int(*)(int)) std::toupper);
        istringstream is(s);
        is >> str1;

        if (str1[0]=='!') {
            int i=0;
            while ( is >> str1) {
                if (!(str1.compare("PROD_NAME") && str1.compare("N") && str1.compare("P2O5")
                    &&str1.compare("K2O") && str1.compare("FUNGICIDE") && str1.compare("HERBICIDE")
                    &&str1.compare("INSECTICIDE") && str1.compare("WATER")
                    &&str1.compare("SOIL_LOSS") && str1.compare("BIOHABITAT") ))
                        envCols[str1]=i;
                i++;
            }
            continue;
        }

        int j=0;
        envproduct prod;

        istringstream iss(sback);
        while ( iss >> str1) {
             if (envCols["PROD_NAME"]==j)
                    prod.name = str1;
             else if ( envCols["N"]==j)
                    prod.n = atof(str1.c_str());
             else if ( envCols["P2O5"] == j )
                    prod.p2o5 = atof(str1.c_str());

             else if ( envCols["K2O"] == j )
                    prod.k2o = atof (str1.c_str());
             else if ( envCols["FUNGICIDE"]==j)
                    prod.fungicide = atof(str1.c_str());
             else if ( envCols["HERBICIDE"] == j )
                    prod.herbicide = atof (str1.c_str());
             else if ( envCols["INSECTICIDE"]==j)
                    prod.insecticide = atof(str1.c_str());

             else if ( envCols["WATER"] == j )
                    prod.water = atof (str1.c_str());
             else if ( envCols["SOIL_LOSS"] == j )
                    prod.soil_loss = atof (str1.c_str());
              else if ( envCols["BIOHABITAT"] == j )
                    prod.biohabitat = atoi (str1.c_str());

             j++;
        }
        envmarketdata.envproducts.push_back(prod);
  }
  ins.close();

  return;
}

static map <string, int> yieldCols;
//static
void readyield() {
    ifstream ins;

    stringstream ifile;
    ifile<< inputdir <<YIELDFILE;
    ins.open(ifile.str().c_str(),ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ifile.str() << "\n";
	   exit(2);
    }

    string s, sback, str1, str2;
    while (!ins.eof()){
        getline(ins,s);
        sback= s;

		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;

        std::transform(s.begin(), s.end(), s.begin(),
               (int(*)(int)) std::toupper);
        istringstream is(s);
        is >> str1;

        if (str1[0]=='!') {
            int i=0;
            while ( is >> str1) {
                if (!(str1.compare("PRODUCTNAME") && str1.compare("A") && str1.compare("B")
                    &&str1.compare("C") && str1.compare("D") 
					&& str1.compare("E") && str1.compare("F") && str1.compare("C_PLAT") 
					&& str1.compare("GAMMA")
                    &&str1.compare("SOIL_TYPE") && str1.compare("DYNAM")
					&&str1.compare("P") && str1.compare("K") && str1.compare("PESTICIDE")
                    &&str1.compare("ENERGY_VAR")
					))
                        yieldCols[str1]=i;
                i++;
            }
            continue;
        }

        int j=0;
        oneyield yield;

        istringstream iss(sback);
        while ( iss >> str1) {
             if ( yieldCols["PRODUCTNAME"] == j )
                    yield.name = str1;

             else if ( yieldCols["A"] == j )
                    yield.a = atof (str1.c_str());
             else if ( yieldCols["B"]==j)
                    yield.b = atof(str1.c_str());
             else if ( yieldCols["C"] == j )
                    yield.c = atof (str1.c_str());
             else if ( yieldCols["D"]==j)
                    yield.d = atof(str1.c_str());
			 else if ( yieldCols["E"]==j)
                    yield.e = atof(str1.c_str());
			 else if ( yieldCols["F"]==j)
                    yield.f = atof(str1.c_str());
			 else if ( yieldCols["C_PLAT"]==j)
                    yield.c_plat = atof(str1.c_str());
             else if ( yieldCols["SOIL_TYPE"]==j)
                    yield.soiltype = str1;
             else if ( yieldCols["GAMMA"] == j )
                    yield.gamma = atof (str1.c_str());
			 else if ( yieldCols["DYNAM"] == j )
					yield.dyn = (str1.compare("true")==0 || str1.compare("TRUE")==0) ? true: false;
			 else if ( yieldCols["P"] == j )
                    yield.p = atof (str1.c_str());
             else if ( yieldCols["K"]==j)
                    yield.k = atof(str1.c_str());
             else if ( yieldCols["PESTICIDE"] == j )
                    yield.pesticide = atof (str1.c_str());
             else if ( yieldCols["ENERGY_VAR"]==j)
                    yield.energyvar = atof(str1.c_str());
           
             j++;
        }
        yielddata.push_back(yield);
  }
  ins.close();

  return;
}

static map <string, int> investCols;
//static
void readinvest() {
	investdata.FixOffFarmLabName = "OFFFARMLAB";
	investdata.FixHiredLabName = "HIREDLAB";

	ifstream ins;

    stringstream ifile;
    ifile<< inputdir <<INVESTFILE;
    ins.open(ifile.str().c_str(),ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ifile.str() << "\n";
	   exit(2);
    }

    int nLabNames = 0;
    string s, str1, str2,sback;
    while (!ins.eof()){
        getline(ins,s);
        sback= s;

		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;
        
        /*if (nLabNames<2) {
           ++nLabNames;
		   if (!tokens[0].compare("FIXED_OFFFARM_LAB"))
			   investdata.FixOffFarmLabName = tokens[1];
		   else if (!tokens[0].compare("FIXED_HIRED_LAB"))
			   investdata.FixHiredLabName = tokens[1];
		   else {
               cerr << "Names of Fixed Labors \n";
               exit(3);
		   }
           continue;
		}
		//*/

        std::transform(s.begin(), s.end(), s.begin(),
               (int(*)(int)) std::toupper);
        istringstream is(s);
        is >> str1;
        if (str1[0]=='!') {
            int i=0;
            while ( is >> str1) {
                if (!(str1.compare("TYPE") && str1.compare("NAME")
                    &&str1.compare("COST") && str1.compare("LIFE") && str1.compare("LABSUB")
                    &&str1.compare("LANDSUB") && str1.compare("CAPACITY")
                    &&str1.compare("MAINTENANCE") && str1.compare("PRODGROUP")
                    &&str1.compare("TECHCHANGE") ))
                        investCols[str1]=i;
                i++;
            }
            continue;
        }

        int j=0;
        oneinvest inv;
        istringstream iss(sback);
        while ( iss >> str1) {
             if (investCols["TYPE"]==j)
                    inv.type = atoi(str1.c_str());

             else if ( investCols["NAME"] == j )
                    inv.name = str1;

             else if ( investCols["COST"] == j ){
                //read in as int due to compatibility with the old version
			        double t = atof(str1.c_str());
                    t= t>0? t+0.5: t-0.5;
                    inv.cost = int (t);
                }
             else if ( investCols["LIFE"]==j){
				//read in as int due to compatibility with the old version
                    double t = atof(str1.c_str());
                    t= t>0? t+0.5: t-0.5;
                    inv.life = int(t);
                }
             else if ( investCols["LABSUB"] == j ) {
                //Christoph: landsub should be read in as int
                    double t = atof(str1.c_str());
                    t =  t>0 ? t+0.5: t-0.5;
                    inv.labsub = int (t);
             }else if ( investCols["LANDSUB"]==j)  {
                    double t = atof(str1.c_str());
                    t =  t>0 ? t+0.5: t-0.5;
                    inv.landsub = int(t);
                 }
             else if ( investCols["CAPACITY"]==j){
				 //read in as int due to compatibility with the old version
                    double t = atof(str1.c_str());
                    t= t>0? t+0.5: t-0.5;
                    inv.capacity = int(t);
                }
             else if ( investCols["MAINTENANCE"] == j )
                    inv.maintenance = atof (str1.c_str());
             else if ( investCols["PRODGROUP"]==j)
                    inv.group = atoi(str1.c_str());
             else if ( investCols["TECHCHANGE"] == j )
                    inv.techchange = atof (str1.c_str());

             j++;
        }
        investdata.invests.push_back(inv);
  }
  ins.close();

  vector<string> names{ "HIREDLAB","OFFFARMLAB" };
  double labsub = stof(globdata.globs["LABOUR_SUB"]);
  int ind;
  double ls;
  for (auto &nm : names) {
	  if (nm == "HIREDLAB") {
		  ind = gg->HIREDLABTYPE;
		  ls = labsub;
	  }
	  else if (nm == "OFFFARMLAB") {
		  ind = gg->OFFFARMLABTYPE;
		  ls = -labsub;
	  }
	  oneinvest inv = oneinvest(ind, nm, stof(globdata.globs[nm]), ls);
	  investdata.invests.push_back(inv);
  }

  return;
}

//static
void  readmatrix() {
    ifstream ins;
    stringstream ss;
    int zust=0;
    int cols=0;
    int rows = 0;

    ss << inputdir << MIPDIR << MATRIXFILE;

    ins.open(ss.str().c_str(), ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ss.str() << "\n";
	   exit(2);
    }

    string s, str1;
    while (!ins.eof()){
		getline(ins,s);
		
		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;

        int i=0; int t=0; double d=0.;
        int err=0;
        vector<double> vd;

        string tt;
        switch (zust) {
            case 0:
                    i = 0;
					for (unsigned int k=0; k< tokens.size(); ++k){
						if (tokens[k][0]=='#') break;
	                    matrixdata.colnames.push_back(tokens[k]);
                        i++;
					}
                    cols = i;
                    zust=1;
                    break;
            case 1:
                    tt = tokens[0];
                    std::transform(tt.begin(), tt.end(), tt.begin(),
                        (int(*)(int)) std::toupper);

                    if (tt.compare("ISINTEGER")==0) {
                        i=0;
						for (unsigned int k=1; k< tokens.size(); ++k){
							t=atoi(tokens[k].c_str());
                            matrixdata.isInt.push_back(t);
                            i++;
                        }
                        if (i!=cols) err =1;
                    }
                    else err=2;
                    if (err ==1 ) cout << " ERROR: NUMBER OF COLUMNS" << endl;
                    if (err ==2 ) cout << " ERROR: isInteger Line ? " << endl;
                    if (err) exit(1);
                    zust = 2;
                    break;
            case 2:
                    i=0;
					str1=tokens[0];
                    std::transform(str1.begin(), str1.end(), str1.begin(),
                        (int(*)(int)) std::toupper);
                    matrixdata.rownames.push_back(str1);
					for (unsigned int k=1; k< tokens.size(); ++k){
						d=atof(tokens[k].c_str());
                        vd.push_back(d);
                        i++;
						if (i==cols) break;
                    }
                    matrixdata.mat.push_back(vd);
                    rows++;
            default:   ;
        }
    }
    ins.close();
   return;
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

//static
void  readmatrix_new() {
    ifstream ins;
    stringstream ss;
    int zust=0;
    ss << inputdir << MIPDIR << MATRIX_NEWFILE;

    ins.open(ss.str().c_str(), ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ss.str() << "\n";
	   exit(2);
    }

    string s, str1;
    
    while (!ins.eof()){
		getline(ins,s);
		
		vector <string> tokens;
        string tab="\t+;:";
		tokenize(s,tokens,tab);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;
		if (trim(tokens[0]).length()==0) continue;

        string tt;
        struct aRestrict0 arest;
        switch (zust) {
            case 0:
                    if (!(tokens[0].compare("_VARIABLES_"))) {
						zust = 1;
					    for (unsigned int k=1; k< tokens.size(); ++k){
							matrixdata_n.VarNames0.push_back(trim(tokens[k]));
                        }
					}else { 
                        cout << "_VARIABLES_ line missing\n";
                        exit(4);
					}
                    break;
            case 1:
                    if (!(tokens[0].compare("_INTEGERS_"))) {
						zust = 2;
					    for (unsigned int k=1; k< tokens.size(); ++k){
							matrixdata_n.IntVars0.push_back(trim(tokens[k]));
                        }
					}else { 
                        cout << "_INTEGERS_ line missing\n";
                        exit(4);
					}
                    break;
            case 2: //restrictions
                   	str1=tokens[0];
                    arest.name=str1;
					for (unsigned int k=1; k< tokens.size(); ++k){
						arest.terms0.push_back(trim(tokens[k]));
                    }
					matrixdata_n.Restricts0.push_back(arest);
            default:   ;
        }
    }
   ins.close();
   return;
}

//static
   void  readlink(istringstream& is, onelink& cl, int dk){ //dk==1 for capacity links
        string str1;
        double d;

        is >> str1;
        cl.name=str1;
        is >> str1;
        cl.linktype=str1;
        std::transform(str1.begin(), str1.end(), str1.begin(),
               (int(*)(int)) std::toupper);
        if (str1.compare("REFERENCE")==0) {
            is >> str1;
            cl.valuetype=str1;
            is >> str1;
            //matrix links reference type no factor ?!
            cl.factor=str1.compare("")==0? 0: atof(str1.c_str());
        }else if (str1.compare("LAND")==0) {
            is >> str1;
            cl.valuetype = str1;
            is >> d;
            cl.factor= d;
        }else if (str1.compare("NUMBER")==0) {
            is >> str1;
            cl.numbertype = str1; // valuetype = str1; Fehler !!
        }else if (str1.compare("MARKET")==0) {
			if (dk==1) {
				is >> str1;
				cl.numbertype = str1;
			}
            is >> str1;
            cl.valuetype = str1;
            is >> d;
            cl.factor= d;
		}else if (str1.compare("YIELD")==0) {
			is >> str1;
            cl.valuetype = str1;
            is >> d;
            cl.factor= d;
        }else if (str1.compare("INVEST")==0) {
			if (dk==1) {
				is >> str1;
				cl.numbertype = str1;
			}
            is >> str1;
            cl.valuetype = str1;
            is >> d;
            cl.factor= d;
        }
   return;
}

//static
void  readcaplinks() {
    ifstream ins;
    stringstream ss;
    int i=0;

    ss << inputdir << MIPDIR << CAPLFILE;

    ins.open(ss.str().c_str(), ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ss.str() << "\n";
	   exit(2);
    }

    string s;
    while (!ins.eof()){
        getline(ins,s);

		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;

        istringstream is(s);

        i++;
        onelink cl;
		int dk= 1;
        readlink(is, cl, dk);
        caplinkdata.caplinks.push_back(cl) ;
   }

    ins.close();

	return;
}

//static
void  readobjlinks() {
    ifstream ins;
    stringstream ss;
    int i=0;

    ss << inputdir << MIPDIR << OBJLFILE;

    ins.open(ss.str().c_str(), ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ss.str() << "\n";
	   exit(2);
    }

    string s;
    while (!ins.eof()){
        getline(ins,s);
 
		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;

        istringstream is(s);

        i++;
        onelink cl;
		int dk = 2;
        readlink(is, cl, dk);
        objlinkdata.objlinks.push_back(cl) ;
    }   
    ins.close();
    return;
}

//static
void  readmatrixlinks() {
    ifstream ins;
    stringstream ss;
    int i=0;
    int j=0;

    ss << inputdir << MIPDIR << MATLFILE;

    ins.open(ss.str().c_str(), ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << ss.str() << "\n";
	   exit(2);
    }

    string s,str1, sback;
    //double d;

    onelink cl;
    onematlink ml;

    while (!ins.eof()){
        getline(ins,s);
        sback = s;
 
		vector <string> tokens;
		tokenize(s,tokens);

        if (tokens.size()==1) continue;
        if (tokens[0][0]=='#') continue;

        std::transform(s.begin(), s.end(), s.begin(),
               (int(*)(int)) std::toupper);
        istringstream is(sback);

        i++;
        is >> str1;
        std::transform(str1.begin(), str1.end(), str1.begin(),
               (int(*)(int)) std::toupper);
        ml.row=str1;
		int dk=2;
        readlink(is, cl, dk);
        ml.col=cl.name;
        ml.linktype=cl.linktype;
        ml.valuetype=cl.valuetype;
        ml.factor=cl.factor;
		
		// liquidity/financing_rule links for market products
		if ((ml.row.find("LIQUIDITY")==string::npos)
				&&(ml.row.find("FINANCING_RULE")==string::npos)){
			matlinkdata.matlinks.push_back(ml) ;
		}else {
			string tstr= ml.row;
			if (tstr.find("LIQUIDITY")!=string::npos) {
				ml.row="LIQUIDITY";
				matlinkdata.matlinks.push_back(ml) ;
			}
			if (tstr.find("FINANCING_RULE")!=string::npos) {
				ml.row="FINANCING_RULE";
				matlinkdata.matlinks.push_back(ml) ;
			}
		}
  }
  ins.close();
  return;
}

//static
void readmip(){
    //readmatrix();
    readmatrix_new();
    readcaplinks();
    readobjlinks();
    readmatrixlinks();

 return ;
}

void readfiles(string idir, bool hasSoilservice){
    inputdir = idir;
	hasCarbon = hasSoilservice;

    glob();
	if (gg->ManagerDemographics)
		demograph();

	if (gg->YoungFarmer)
		youngfarmer();

    farms();
    nfarms();

	if (gg->ENV_MODELING)
		readenv();

    readmarket();

	if (gg->ENV_MODELING)
        readenvmarket();

	if (hasCarbon) readyield();
    readinvest();

    readmip();
    return;
}
