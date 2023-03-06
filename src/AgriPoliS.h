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

// Reading options from options.txt
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <iterator>
#include <vector>
#include <string>

#include "RegGlobals.h"

const string OPTFILE = "options.txt";

RegGlobalsInfo* gg;

static map<string,string> optionsdata;
static string optiondir;

static void tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " \t=;")
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

static string trim(const std::string& str, const std::string& whitespace = " ")
{
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}
static SimPhase phaseFromStr(string name) {
   std::transform(name.begin(), name.end(), name.begin(),
		(int(*)(int)) std::toupper);
   SimPhase result = SimPhase::INIT;
   if (name.compare("BETWEEN") == 0)
	   result = SimPhase::BETWEEN;
   if (name.compare("INVEST") == 0)
	   result = SimPhase::INVEST;
   if (name.compare("LAND") == 0)
	   result = SimPhase::LAND;
   if (name.compare("PRODUCT") == 0)
	   result = SimPhase::PRODUCT;
   if (name.compare("FUTURE") == 0)
	   result = SimPhase::FUTURE;
   if (name.compare("ALL") == 0)
	   result = SimPhase::ALL;
   return result;
}

bool deb_mip() {
	bool result = false;
	string fname = "debug_mip.txt";
	string fullfn = optiondir + "\\"+ fname;
	
	ifstream ins(fullfn.c_str());
	if (ins.good()) {
		result = ins.good();
		//ins.open(fullfn.c_str(), ios::in);
		if (!ins.is_open()) {
			cerr << "Error while opening: " << fname << "\n";
			exit(2);
		}
		string line;
		string prop;
		vector <string> tokens;
		while (!ins.eof()) {
			getline(ins, line);

			if (line.compare("") == 0) continue;
			if (line[0] == '#') continue;
			tokens.clear();
			tokenize(line, tokens, ":\t ");

			if (tokens[1].compare("=")==0) continue;
			prop = tokens[0];
			std::transform(prop.begin(), prop.end(), prop.begin(),
				(int(*)(int)) std::toupper);

			if (prop.compare("FARM_ID") == 0)
				gg->uFarmId = atoi(tokens[1].c_str());
			else if (prop.compare("FARM_NAME") == 0)
				gg->uFarmName = tokens[1];
			else if (prop.compare("ITERATION") == 0)
				gg->uIter = atoi(tokens[1].c_str());
			else if (prop.compare("PROCESS") == 0)
				gg->uPhase = phaseFromStr(tokens[1]);
		}
		ins.close();
	}
	return result;
}

void readScenario() {
	ifstream ins;
	stringstream gfile;
	gfile << optiondir + "\\" << gg->SCENARIOFILE;
	ins.open(gfile.str().c_str(), ios::in);
	if (!ins.is_open()) {
		cerr << "Error while opening: " << gfile.str() << "\n";
		exit(2);
	}
	
	string s, s2, sback;
	vector <string> tokens;
	int state = 0;
	while (!ins.eof()) {
		getline(ins, s);
		sback = s;
		if (s.compare("") == 0) continue;
		if (s[0] == '#') continue;
		tokens.clear();
		tokenize(s, tokens,":\t" );

		if (tokens.size() == 1) continue;
		s2 = trim(tokens[0]);
		std::transform(s2.begin(), s2.end(), s2.begin(),
			(int(*)(int)) std::toupper);
		switch (state) {
		case 0:
			if (s2.compare("SCENARIO") == 0) {
				gg->Scenario = trim(tokens[1]);
			}
			else if (s2.compare("DESCRIPTION") == 0) {
				gg->Scenario_Description = trim(tokens[1]);
			}
			else if (s2.compare("POLICY_FILE") == 0) {
				gg->POLICYFILE = trim(tokens[1]);
			}
			else if (s2.compare("[GLOBALS]") == 0){
				state = 1;
			}
			else if (s2.compare("[OPTIONS]") == 0) {
				state = 2;
			}
			break;
		case 1:
			if (s2.compare("[OPTIONS]") == 0) {
				state = 2;
				break;
			}
			if (s2.compare("LIVESTOCK_INVS") == 0) {
				vector<string> tokens1;
				tokenize(tokens[1], tokens1, " ");
				int sz = tokens1.size();
				for (int i = 0; i < sz; ++i) {
					if (tokens1[i].compare("=") != 0) {
						gg->Livestock_Invs.insert(tokens1[i]);
					}
				}
				/*
				if (gg->Livestock_Invs.size() == 0)
					gg->Scenario_globs.insert(pair("RESTRICTINVESTMENTS", "false"));
				else
					gg->Scenario_globs.insert(pair("RESTRICTINVESTMENTS", "true"));
					//*/
			}else
				gg->Scenario_globs.insert(pair<string, string>(s2, trim(tokens[1])));
			break;
		case 2:
			if (s2.compare("[GLOBALS]") == 0) {
				state = 1;
				break;
			}
			gg->Scenario_options.insert(pair<string, string>(s2, trim(tokens[1])));
			break;
		default:;
		}
	}
	ins.close();
	return;
}

void readoptions() {
    ifstream ins;
    stringstream gfile;
    gfile << optiondir <<OPTFILE;
    ins.open(gfile.str().c_str(),ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << gfile.str() << "\n";
	   exit(2);
    }
    string s, s2, sback;

    vector <string> tokens;

    while (!ins.eof()){
        getline(ins,s);
        sback = s;
        if (s.compare("")==0) continue;
        if (s[0]=='#') continue;
        tokens.clear();
        tokenize(s, tokens);

		if (tokens.size()==1) continue;
        s2=tokens[0];
        std::transform(s2.begin(), s2.end(), s2.begin(),
               (int(*)(int)) std::toupper);

		//if (gg->Scenario_options.count(s2))
		//	optionsdata.insert(pair<string, string>(s2, gg->Scenario_options[tokens[0]]));
		//else 
            optionsdata.insert(pair<string, string>(s2,tokens[1]));
    }
	for (auto x : gg->Scenario_options) {
		//if (optionsdata.count(x.first)>0)
		   optionsdata[x.first] = x.second;
	}
	ins.close();

    return;
}

DISTRIB_TYPE make_distribType(string str) {
	DISTRIB_TYPE distype = DISTRIB_TYPE::UNIFORM;
	transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {return toupper(c); });
	if (str.compare("NORMAL") == 0)
		distype = DISTRIB_TYPE::NORMAL;
	return distype;
}

void setoptions(){
gg->RUNS=atoi(optionsdata["RUNS"].c_str());
gg->TEILER = atoi(optionsdata["TEILER"].c_str());

gg->Rent_Variation = optionsdata["RENT_VARIATION"].compare("true") == 0 ? true : false;

gg->RestrictInvestments=optionsdata["RESTRICTINVESTMENTS"].compare("true") == 0 ? true : false;

gg->NASG = optionsdata["NASG"].compare("true") == 0 ? true : false;

gg->ManagerDemographics = optionsdata["MANAGERDEMOGRAPHICS"].compare("true") == 0 ? true : false;
gg->YoungFarmer = optionsdata["YOUNGFARMER"].compare("true") == 0 ? true : false;

gg->DebMip = optionsdata["DEBUG_MIP"].compare("true") == 0 ? true : false;

gg->ManagerDistribType = make_distribType(optionsdata["MANAGERCOEFFDISTRIBUTION"]);

string tstr = optionsdata["INPUTFILEDIR"];

if ((tstr.compare("=")==0)|| tstr.compare("")==0) 
   gg->INPUTFILEdir= optiondir;
else if (tstr[tstr.length()-1]!='\\' ) gg->INPUTFILEdir = tstr+ "\\";
else gg->INPUTFILEdir = tstr;

string updir = gg->INPUTFILEdir;
size_t pos =updir.find_last_not_of('\\');
updir = updir.substr(0,pos);
pos = updir.find_last_of('\\');
updir = updir.substr(0,pos+1);

tstr = gg->POLICYFILE;
if (tstr.compare("")==0)
    gg->POLICYFILE = gg->INPUTFILEdir+"policy_settings.txt";
else
    gg->POLICYFILE = gg->INPUTFILEdir+tstr;

/*string extstr=gg->POLICYFILE;
pos = extstr.find_last_of('\\');

if ( pos != string::npos)
    extstr= extstr.substr(pos+1,string::npos);

pos = extstr.find("policy_settings");
size_t pos1 = extstr.find('.');

string ext;
if (pos==string::npos){
    if (pos1==string::npos) ext = extstr;
    else ext = extstr.substr(0,pos1);
}else {
    if (pos1==string::npos) ext = extstr.substr(pos+15);
    else ext =  extstr.substr(pos+15,pos1-pos-15) ;
}
//*/
string ext = string("") + "_" + gg->Scenario;

tstr = optionsdata["OUTPUTFILE"];
if ((tstr.compare("") == 0) || tstr.compare("=")==0) 
   gg->OUTPUTFILE = updir + "outputfiles"+ ext + "\\";
else if (tstr[tstr.length()-1]!='\\' ) gg->OUTPUTFILE = tstr+ "\\"  ;
else gg->OUTPUTFILE = tstr;

gg->FARMOUTPUT = optionsdata["FARMOUTPUT"].compare("true") == 0 ? true : false;
gg->SECTOROUTPUT = optionsdata["SECTOROUTPUT"].compare("true") == 0 ? true : false;

//---  Region dependent Globals ------------

//Standard: false, Sweden: true (calf and milk quota market)
gg->SWEDEN   =  (optionsdata["SWEDEN"]).compare("true")==0 ? true : false;

// flase means the settings for Västerbotton are used. THis is only relevant if Sweden is true.
gg->JOENKEPING   =  optionsdata["JOENKEPING"].compare("true")==0 ? true : false;

//Standard: false, CZ, Västerbotten, Jönköping: true
gg->ENV_MODELING = optionsdata["ENV_MODELING"].compare("true")==0 ? true : false;

//true only for regions with various soil qualities of arable and/or grassland, like OPR.
//If true, then shares of soil types will be varried among the farms
gg->SOIL_TYPE_VARIATION   = optionsdata["SOIL_TYPE_VARIATION"].compare("true")==0 ? true : false;

//Standard: false, Brittany: true
gg->OLD_LAND_RELEASING_PROCESS   = optionsdata["OLD_LAND_RELEASING_PROCESS"].compare("true")==0 ? true : false;

gg->OLD_LAND_RENTING_PROCESS = optionsdata["OLD_LAND_RENTING_PROCESS"].compare("true") == 0 ? true : false;

//standard: true, OPR false farm areas are initialised as a circle around the farm  they search
//for the plot with the lowest costs, however this only matters when transaction costs are considered
gg->WEIGHTED_PLOT_SEARCH   =   optionsdata["WEIGHTED_PLOT_SEARCH"].compare("true")==0 ? true : false;

//Standard: false, CZ, Lithuania, Slovakia: true
gg->AGE_DEPENDENT   =  optionsdata["AGE_DEPENDENT"].compare("true")==0 ? true : false;

//Standard: false, true is necessary for Saxony, Hohenlohe, Brittany and CZ because here we have
//different input files to calculate the modulation within the LP-model
gg->LP_MOD   =  optionsdata["LP_MOD"].compare("true")==0 ? true : false;

//Attention, it is only differentiated between IF   =   1 and CF   =   3. In the input-files of Hohenlohe, Saxony,
//OPR and Brittany more legal types are defined. In Brittany and Saxony CF   =   2. To changes this search above for "Legal_types"
gg->CALC_LEGAL_TYPES   =  optionsdata["CALC_LEGAL_TYPES"].compare("true")==0 ? true : false;

//===================================================
//New 04.04.2011 Soilservice
//====================================================
gg->HAS_SOILSERVICE=optionsdata["HAS_SOILSERVICE"].compare("true")==0 ? true : false;	
gg->TECH_DEVELOP = atof(optionsdata["TECH_DEVELOP"].c_str());
gg->CARBON_MIN = atof(optionsdata["CARBON_MIN"].c_str());
gg->CARBON_MAX = atof(optionsdata["CARBON_MAX"].c_str());

//====================================================
//New  15.03.2011
//====================================================
    gg->REGIONAL_DECOUPLING=atoi(optionsdata["REGIONAL_DECOUPLING"].c_str());
    gg->FULLY_DECOUPLING=atoi(optionsdata["FULLY_DECOUPLING"].c_str());
    gg->FARMSPECIFIC_DECOUPLING=atoi(optionsdata["FARMSPECIFIC_DECOUPLING"].c_str());
    gg->REGIONAL_DECOUPLING_SWITCH=atoi(optionsdata["REGIONAL_DECOUPLING_SWITCH"].c_str());
    gg->FULLY_DECOUPLING_SWITCH=atoi(optionsdata["FULLY_DECOUPLING_SWITCH"].c_str());
    gg->FARMSPECIFIC_DECOUPLING_SWITCH=atoi(optionsdata["FARMSPECIFIC_DECOUPLING_SWITCH"].c_str());
    //gg->WITHDRAWFACTOR = atof(optionsdata["WITHDRAWFACTOR"].c_str());
    gg->AVERAGE_OFFER_BUFFER_SIZE=atoi(optionsdata["AVERAGE_OFFER_BUFFER_SIZE"].c_str());

    gg->USE_TRIANGULAR_DISTRIBUTED_MANAGEMENT_FACTOR=optionsdata["USE_TRIANGULAR_DISTRIBUTED_MANAGEMENT_FACTOR"].compare("true")==0 ? true : false;
    gg->USE_TRIANGULAR_DISTRIBUTED_FARM_AGE=optionsdata["USE_TRIANGULAR_DISTRIBUTED_FARM_AGE"].compare("true")==0 ? true : false;

    gg->WEIGHTED_PLOT_SEARCH_VALUE=atoi(optionsdata["WEIGHTED_PLOT_SEARCH_VALUE"].c_str());

    gg->USE_VARIABLE_PRICE_CHANGE=optionsdata["USE_VARIABLE_PRICE_CHANGE"].compare("true")==0 ? true : false;

    gg->NO_SUCCESSOR_BY_RANDOM=optionsdata["NO_SUCCESSOR_BY_RANDOM"].compare("true")==0 ? true : false;
    gg->ASSOCIATE_ACTIVITIES=optionsdata["ASSOCIATE_ACTIVITIES"].compare("true")==0 ? true : false;
    gg->CALCULATE_CONTIGUOUS_PLOTS=optionsdata["CALCULATE_CONTIGUOUS_PLOTS"].compare("true")==0 ? true : false;
    gg->USE_HISTORICAL_CONTIGUOUS_PLOTS=optionsdata["USE_HISTORICAL_CONTIGUOUS_PLOTS"].compare("true")==0 ? true : false;

    gg->FIX_PRICES=optionsdata["FIX_PRICES"].compare("true")==0 ? true : false;

    /////////////////////////////////////

    gg->PRINT_SEC_RES=optionsdata["PRINT_SEC_RES"].compare("true")==0 ? true : false;
    gg->PRINT_SEC_PRICE=optionsdata["PRINT_SEC_PRICE"].compare("true")==0 ? true : false;
    gg->PRINT_SEC_EXP_PRICE=optionsdata["PRINT_SEC_EXP_PRICE"].compare("true")==0 ? true : false;
    gg->PRINT_SEC_COSTS=optionsdata["PRINT_SEC_COSTS"].compare("true")==0 ? true : false;
    gg->PRINT_SEC_COND=optionsdata["PRINT_SEC_COND"].compare("true")==0 ? true : false;
    gg->PRINT_FARM_RES=optionsdata["PRINT_FARM_RES"].compare("true")==0 ? true : false;
    gg->PRINT_FARM_INV=optionsdata["PRINT_FARM_INV"].compare("true")==0 ? true : false;
    gg->PRINT_FARM_PROD=optionsdata["PRINT_FARM_PROD"].compare("true")==0 ? true : false;
    gg->PRINT_FARM_COSTS=optionsdata["PRINT_FARM_COSTS"].compare("true")==0 ? true : false;
    gg->PRINT_CONT_PLOTS=optionsdata["PRINT_CONT_PLOTS"].compare("true")==0 ? true : false;

    gg->PRINT_TAC=optionsdata["PRINT_TAC"].compare("true")==0 ? true : false;
    gg->PRINT_VA=optionsdata["PRINT_VA"].compare("true")==0 ? true : false;
    gg->PRINT_POLICY=optionsdata["PRINT_POLICY"].compare("true")==0 ? true : false;

	//derived from replication
    //gg->INIT_OUTPUT=optionsdata["INIT_OUTPUT"].compare("true")==0 ?  true : false;
return;
}

void options(string idir){
	if (idir[idir.length()-1]!='\\' )
		optiondir = idir + '\\';
	else 
		optiondir= idir;
    readoptions();
    setoptions();
    return;
}
