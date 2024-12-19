/*************************************************************************
* This file is part of AgriPoliS
*
* AgriPoliS: An Agricultural Policy Simulator
*
* Copyright (c) 2024, Alfons Balmann, Kathrin Happe, Konrad Kellermann et al.
* (cf. AUTHORS.md) at Leibniz Institute of Agricultural Development in 
* Transition Economies
*
* SPDX-License-Identifier: MIT
**************************************************************************/

// RegGlobals.cpp
//---------------------------------------------------------------------------
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include <algorithm>

#include "RegGlobals.h"
#include "textinput.h"
#include "random.h"


#define TCHAR char
#define _T(x) (x)

const string RegGlobalsInfo::UsageString=
    "USAGE: program [options] dirOfSzenarios  [ szenario [repeatNum] ] \n";

RegGlobalsInfo::RegGlobalsInfo() {
	Livestock_Inv_farmsPercent = 0;
	RestrictInvestments = false;

	ManagerDemographics = false;
	YoungFarmer = false;
	farmAgeDists.resize(3);

	tPhase = SimPhase::INIT;
	DebMip = false;
	uPhase = SimPhase::ALL;
	uFarmId = -1;
	uIter = -1;
	uFarmName = "ALL";
	
	PRODGROUPLAB = -1;
	PRODTYPE = 4;
	HIREDLABTYPE = 0;
	OFFFARMLABTYPE = 1;
	VAROFFARMLABTYPE = 3;
	VARHIREDLABTYPE = 2;
	ST_BOR_INTERESTTYPE = 0;
	ST_EC_INTERESTTYPE = 1;

	V = 0;
	SCENARIO = 0;
    NUMBER_OF_INVESTTYPES= 0;

	tech_develop_abs=1;   //

    STATUS=0;
    LEGAL_TYPES.push_back(1);
    LEGAL_TYPES.push_back(3);
    NAMES_OF_LEGAL_TYPES.push_back("IF");
    NAMES_OF_LEGAL_TYPES.push_back("CF");
    SEED=0;
    INITIALISATION=true;
    obj_backup=NULL;
    REGIONAL_DECOUPLING=0;
    FULLY_DECOUPLING=0;
    FARMSPECIFIC_DECOUPLING=0;
    REGIONAL_DECOUPLING_SWITCH=0;
    FULLY_DECOUPLING_SWITCH=0;
    FARMSPECIFIC_DECOUPLING_SWITCH=0;
    WITHDRAWFACTOR = 0.7;
    REGION_OVERSIZE=1.15;
    AVERAGE_OFFER_BUFFER_SIZE=5;
    MIN_CONTRACT_LENGTH=9;                        //Standard: 9, CZ=5, Saxony=12
    MAX_CONTRACT_LENGTH=18;                       //Standard: 18, Saxony=24
    SWEDEN=false;                                 //Standard: false, Sweden: true (calf and milk quota market)
    // flase means the settings for Västerbotton are used. THis is only relevant if Sweden is true.
    JOENKEPING=false;//true;
    ENV_MODELING=false;//true;                           //Standard: false, CZ, Västerbotten, Jönköping: true
    SOIL_TYPE_VARIATION=false;                    //true only for regions with various soil qualities of arable and/or grassland, like OPR.
                                                  //If true, then shares of soil types will be varried among the farms
    ADJUST_PAID_RENT=false;
    OLD_LAND_RENTING_PROCESS=true;
    OLD_LAND_RELEASING_PROCESS=false;             //Standard: false, Brittany: true
    FIRSTPRICE=true;
    SECONDPRICE_REGION=false;
    USE_TRIANGULAR_DISTRIBUTED_MANAGEMENT_FACTOR=false;
    USE_TRIANGULAR_DISTRIBUTED_FARM_AGE=false;
    FAST_PLOT_SEARCH=false;
    WEIGHTED_PLOT_SEARCH=true;                        //standard: true, OPR false farm areas are initialised as a circle around the farm; they search for the plot with the lowest costs, however this only matters when transaction costs are considered
    WEIGHTED_PLOT_SEARCH_VALUE=50;
    RELEASE_PLOTS_BEFORE_EXPECTATION_FORMATION=true;
    USE_VARIABLE_PRICE_CHANGE=true;
    AGE_DEPENDENT=false;//true;                          //Standard: false, CZ, Lithuania, Slovakia: true
	NO_SUCCESSOR_BY_RANDOM = false;

    ASSOCIATE_ACTIVITIES=false;
    CALCULATE_CONTIGUOUS_PLOTS=false;
    USE_HISTORICAL_CONTIGUOUS_PLOTS=false;
    LP_MOD=true;//false;                                 //Standard: false, true is necessary for Saxony, Hohenlohe, Brittany and CZ because here we have different input files to calculate the modulation within the LP-model
    USE_TC_FRAMEWORK=false;//true;

    REGION_OVERSIZE=1.15;
    REGION_NON_AG_LAND=0;
    OUTPUTFILE="../outputfiles/";
    POLICYFILE="";
    PRINT_REMOVED_FARMS=true;
    TRANCH_1_DEG=-1;
    TRANCH_2_DEG=-1;
    TRANCH_3_DEG=-1;
    TRANCH_4_DEG=-1;
    TRANCH_5_DEG=-1;
    CALC_LEGAL_TYPES=true;                      //Attention, it is only differentiated between IF=1 and CF=3. In the input-files of Hohenlohe, Saxony, OPR and Brittany more legal types are defined. In Brittany and Saxony CF=2. To changes this search above for "Legal_types"
    GLOBAL_STRATEGY=0;
    GLOBAL_OPTIMUM_EVERY_PERIOD=false;
    FIX_PRICES=false;
    MIN_WITHDRAWAL=false;
    SET_FREE_PLOTS=false;
    CALCULATE_EXPECTED_RENTAL_PRICE=true;
    for(int i=0;i<10;i++)
      FREE_PLOTS_OF_TYPE.push_back(0);
    PRINT_SEC_RES=true;
    PRINT_SEC_PRICE=true;
    PRINT_SEC_EXP_PRICE=false;
    PRINT_SEC_COSTS=false;
    PRINT_SEC_COND=false;
    PRINT_FARM_RES=true;
    PRINT_FARM_INV=true;
    PRINT_FARM_PROD=true;
    PRINT_FARM_COSTS=false;
    PRINT_CONT_PLOTS=false;
    PRINT_TAC=false;
    PRINT_VA=false;
    PRINT_POLICY=false;
    INIT_OUTPUT=true;

	tInd = 0;
	tInd_land = 0;
	tInd_future = 0;
	NASG = false;
	NASG_startPeriod = 0;

	SDEBUG1 = false;
	SDEBUG2 = false;
	initRandEngines();
}

static string usageString = RegGlobalsInfo::UsageString;
// show the usage of this program
void ShowUsage() {
	cout << usageString;
}

// define the ID values to indentify the option
enum { OPT_HELP = 1000 };

// declare a table of CSimpleOpt::SOption structures. See the SimpleOpt.h header
// for details of each entry in this structure. In summary they are:
//  1. ID for this option. This will be returned from OptionId() during processing.
//     It may be anything >= 0 and may contain duplicates.
//  2. Option as it should be written on the command line
//  3. Type of the option. See the header file for details of all possible types.
//     The SO_REQ_SEP type means an argument is required and must be supplied
//     separately, e.g. "-f FILE"
//  4. The last entry must be SO_END_OF_OPTIONS.
//
CSimpleOpt::SOption g_rgOptions[] = {
		{ 40,  ("--TEILER"),     SO_REQ_SEP},
		//{ 50,  ("--REGION_OVERSIZE"),     SO_REQ_SEP },
		//{ 60,  ("--REGION_NON_AG_LAND"),     SO_REQ_SEP },
		{ 70,  ("--RUNS"),     SO_REQ_SEP},
		{ OPT_HELP, "--help", SO_NONE},
		{ OPT_HELP, "-help", SO_NONE },
		{ OPT_HELP, "-h", SO_NONE },
		{ OPT_HELP, "-?", SO_NONE},
                                                                           
        SO_END_OF_OPTIONS                       // END
};

RegGlobalsInfo::~RegGlobalsInfo() {
    if (obj_backup) delete obj_backup;
}

void
RegGlobalsInfo::readFromCommandLine() {
	CSimpleOpt args(ARGC,ARGV, g_rgOptions,SO_O_ICASE);
    while (args.Next()) {
        if (args.LastError() != SO_SUCCESS) {
            const TCHAR * pszError = &(_T("Unknown error")[0]);
            switch (args.LastError()) {
            case SO_OPT_INVALID:
                pszError = _T("Unrecognized option");
                break;
            case SO_OPT_MULTIPLE:
                pszError = _T("Option matched multiple strings");
                break;
            case SO_ARG_INVALID:
                pszError = _T("Option does not accept argument");
                break;
            case SO_ARG_INVALID_TYPE:
                pszError = _T("Invalid argument format");
                break;
            case SO_ARG_MISSING:
                pszError = _T("Required argument is missing");
                break;
            case SO_SUCCESS:
                pszError = NULL;
            }
			cout << pszError << ": " << args.OptionText() << " (use --help to get command line help)\n";
            continue;
        }

        if (args.OptionId() == OPT_HELP) {
            ShowUsage();
            exit(0);
        }

        switch (args.OptionId()) {
		case 40:
			options[&TEILER] = atoi(args.OptionArg());
			//this->TEILER = atoi(args.OptionArg());
			break;
			/*
        case 50:
            this->REGION_OVERSIZE=atof(args.OptionArg());
            break;
        case 60:
            this->REGION_NON_AG_LAND=atof(args.OptionArg());
            break;
			//*/
        case 70:
			options[&RUNS] = atoi(args.OptionArg());
            //this->RUNS=atoi(args.OptionArg());
            break;
              
        default:
            break;
        }
        //cout << args.OptionText() << ": "<<args.OptionArg()<<"\n";
    }
	int nf = args.FileCount();
	commandlineFILES.clear();
	for (int i = 0; i < nf; ++i) {
		commandlineFILES.push_back(args.File(i));
	}
}

static string Upper(string str) {
	//cout << str;
	transform(str.begin(), str.end(), str.begin(), ::toupper);
	//cout << str << endl;
	return str;
}

void RegGlobalsInfo::initRandEngines() {
	for (int i = 0; i < RCOUNT; ++i) {
		RAND_ENGINES[Upper(rand_gen_names[i])] = MINSTD_RAND0;
	}
}

RegGlobalsInfo::R_ENGINES RegGlobalsInfo::TO_RAND_ENGINE(string str) {
	//cout << str << endl;
	str = Upper(str);
		if (!str.compare("MINSTD_RAND0"))
			return MINSTD_RAND0;
		else if (!str.compare("MINSTD_RAND"))
			return MINSTD_RAND;
		else if (!str.compare("MT19937"))
			return MT19937;
		else if (!str.compare("MT19937_64"))
			return MT19937_64;
		else if (!str.compare("KNUTH_B"))
			return KNUTH_B;
		else 
			return MINSTD_RAND0;
}

double RegGlobalsInfo::getRandomReal(string name, uniform_real_distribution<>& distr) {
	switch (RAND_ENGINES[name]) {
	case 0: return distr(std::get<minstd_rand0>(RAND_GENs[name])); break;
	case 1: return distr(std::get<minstd_rand>(RAND_GENs[name])); break;
	case 2: return distr(std::get<mt19937>(RAND_GENs[name])); break;
	case 3: return distr(std::get<mt19937_64>(RAND_GENs[name])); break;
	case 4: return distr(std::get<knuth_b>(RAND_GENs[name])); break;
	default: return distr(std::get<minstd_rand0>(RAND_GENs[name]));
	}
}

int RegGlobalsInfo::getRandomInt(string name, uniform_int_distribution<>& distr) {
	switch (RAND_ENGINES[name]) {
	case 0: return distr(std::get<minstd_rand0>(RAND_GENs[name])); break;
	case 1: return distr(std::get<minstd_rand>(RAND_GENs[name])); break;
	case 2: return distr(std::get<mt19937>(RAND_GENs[name])); break;
	case 3: return distr(std::get<mt19937_64>(RAND_GENs[name])); break;
	case 4: return distr(std::get<knuth_b>(RAND_GENs[name])); break;
	default: return distr(std::get<minstd_rand0>(RAND_GENs[name]));
	}
}

//distr should be initialized
double RegGlobalsInfo::getRandomNormal(string name, normal_distribution<>& distr) {
	switch (RAND_ENGINES[name]) {
	case 0: return distr(std::get<minstd_rand0>(RAND_GENs[name])); break;
	case 1: return distr(std::get<minstd_rand>(RAND_GENs[name])); break;
	case 2: return distr(std::get<mt19937>(RAND_GENs[name])); break;
	case 3: return distr(std::get<mt19937_64>(RAND_GENs[name])); break;
	case 4: return distr(std::get<knuth_b>(RAND_GENs[name])); break;
	default: return distr(std::get<minstd_rand0>(RAND_GENs[name]));
	}
}

void RegGlobalsInfo::setRandGens() {
	string name0="RAND_GEN_";
	string name;
	string nm;
	for (int i = 0; i < RCOUNT; ++i) {
		nm = Upper(rand_gen_names[i]);
		name = name0 + nm;
		//cout <<name << ": "<< globdata.globs[name] << endl;
		if (globdata.globs.count(name) > 0)
			RAND_ENGINES[nm] = TO_RAND_ENGINE(globdata.globs[name]);
		//cout << RAND_ENGINES[name] << endl;
		int sd;
		std::mt19937 mt;
		std::mt19937_64 mt64;
		std::minstd_rand0 minstd0;
		std::minstd_rand minstd;
		std::knuth_b knuthb;
		if (RAND_SEEDS.count(nm) > 0) {
			sd = RAND_SEEDS[nm];
			mt.seed(sd);
			mt64.seed(sd);
			minstd0.seed(sd);
			minstd.seed(sd);
			knuthb.seed(sd);
		}
		var_RAND_GEN rand_gen;

		switch (RAND_ENGINES[nm]) {
		case MINSTD_RAND0: rand_gen = minstd0; break;
		case MINSTD_RAND: rand_gen = minstd; break;
		case MT19937: rand_gen = mt; break;
		case MT19937_64: rand_gen = mt64; break;
		case KNUTH_B: rand_gen = knuthb; break;
		default: rand_gen = minstd0;
		}
		
		RAND_GENs[nm]=rand_gen;
  }
}

void RegGlobalsInfo::setSeeds() {
	string name0 = "SEED_";
	string name;
	string nm;
	for (int i = 0; i < RCOUNT; ++i) {
		nm = Upper(rand_gen_names[i]);
		name = name0 + nm;
		if (globdata.globs.count(name) > 0)
			RAND_SEEDS[nm] = atoi(globdata.globs[name].c_str());
	}
}

void RegGlobalsInfo::initYoungFarmer() {
	YF_startpayMaxAge = atof(globdata.globs["YF_STARTPAYMAXAGE"].c_str());
	YF_priceHa = - atof(globdata.globs["YF_PRICEHA"].c_str());
	YF_maxHa = atof(globdata.globs["YF_MAXHA"].c_str());
	YF_maxPay = YF_maxHa * YF_priceHa;
	YF_minHa = atof(globdata.globs["YF_MINHA"].c_str());
	YF_payYears = atoi(globdata.globs["YF_PAYYEARS"].c_str());
}

void RegGlobalsInfo::initDemograph() {
	FF_initAge_mean = atof(globdata.globs["FF_INITAGE_MEAN"].c_str());
	FF_initAge_dev= atof(globdata.globs["FF_INITAGE_DEV"].c_str());
	CF_initAge_mean = atof(globdata.globs["CF_INITAGE_MEAN"].c_str());
	CF_initAge_dev = atof(globdata.globs["CF_INITAGE_DEV"].c_str());
	FF_InitAge_min = atoi(globdata.globs["FF_INITAGE_MIN"].c_str());
	FF_InitAge_max = atoi(globdata.globs["FF_INITAGE_MAX"].c_str());
	CF_InitAge_min = atoi(globdata.globs["CF_INITAGE_MIN"].c_str());
	CF_InitAge_max = atoi(globdata.globs["CF_INITAGE_MAX"].c_str());
	GC_age= atoi(globdata.globs["GC_AGE"].c_str());
	GC_age_max = atoi(globdata.globs["GC_AGE_MAX"].c_str());
	GC_newAge_min = atoi(globdata.globs["GC_NEWAGE_MIN"].c_str());
	GC_newAge_max = atoi(globdata.globs["GC_NEWAGE_MAX"].c_str());
	GC_newAge_mean = atof(globdata.globs["GC_NEWAGE_MEAN"].c_str());
	GC_newAge_dev = atof(globdata.globs["GC_NEWAGE_DEV"].c_str());
	FF_GC_prob = atof(globdata.globs["FF_GC_PROB"].c_str());
	CF_GC_prob = atof(globdata.globs["CF_GC_PROB"].c_str());
	FF_prod_decrease = atof(globdata.globs["FF_PROD_DECREASE"].c_str());

	if (globdata.globs.count("ESC_EXCLUSION"))
	   ESC_exclusion = atoi(globdata.globs["ESC_EXCLUSION"].c_str());
	else ESC_exclusion = 7; // ESC maximal 6
	if (globdata.globs.count("ESC_EXCLUSION_PROB"))
		ESC_exclusion_prob = atof(globdata.globs["ESC_EXCLUSION_PROB"].c_str());
	else ESC_exclusion_prob = 1;
}

void
RegGlobalsInfo::initGlobalsRead() {
	//RestrictInvestments = globdata.globs["RESTRICTINVESTMENTS"].compare("true") == 0 ? true : false;
	Livestock_Inv_farmsPercent = atof(globdata.globs["LIVESTOCK_INV_FARMSPERCENT"].c_str());
	
	//TEILER = atoi(globdata.globs["TEILER"].c_str());

	MIN_CONTRACT_LENGTH = atoi(globdata.globs["MIN_CONTRACT_LENGTH"].c_str());
	MAX_CONTRACT_LENGTH = atoi(globdata.globs["MAX_CONTRACT_LENGTH"].c_str());
	WITHDRAWFACTOR = atof(globdata.globs["WITHDRAWFACTOR"].c_str());

	REGION_OVERSIZE = atof(globdata.globs["OVERSIZE"].c_str());
	REGION_NON_AG_LAND = atof(globdata.globs["NON_AG_LAND"].c_str());

	PLOTSN = atoi(globdata.globs["PLOTSN"].c_str());
	PIGLETS_PER_SOW = atof(globdata.globs["PIGLETS_PER_SOW"].c_str());
	PIGS_PER_PLACE = atof(globdata.globs["PIGS_PER_PLACE"].c_str());
	MILKPROD = atof(globdata.globs["MILKPROD"].c_str());
	ESU = atof(globdata.globs["ESU"].c_str());

	BONUS = atof(globdata.globs["BONUS"].c_str());
	REFINCOME = atof(globdata.globs["REFINCOME"].c_str());
	//PRODUCTGROUPS = atoi(globdata.globs["PRODUCTGROUPS"].c_str());
	//PRODGROUPLAB = atoi(globdata.globs["PRODGROUPLAB"].c_str());
	//PRODTYPE = atoi(globdata.globs["PRODTYPE"].c_str());

	CHANGEPERHA = atof(globdata.globs["CHANGEPERHA"].c_str());
	CHANGEUP = atof(globdata.globs["CHANGEUP"].c_str());
	CHANGEDOWN = atof(globdata.globs["CHANGEDOWN"].c_str());

	MILKUPPERLIMIT = atof(globdata.globs["MILKUPPERLIMIT"].c_str());
	MILKLOWERLIMIT = atof(globdata.globs["MILKLOWERLIMIT"].c_str());

	//OFFFARMLABTYPE = atoi(globdata.globs["OFFFARMLABTYPE"].c_str());
	//VAROFFARMLABTYPE = atoi(globdata.globs["VAROFFFARMLABTYPE"].c_str());
	//VARHIREDLABTYPE = atoi(globdata.globs["VARHIREDLABTYPE"].c_str());
	//ST_BOR_INTERESTTYPE = atoi(globdata.globs["ST_BOR_INTERESTTYPE"].c_str());
	//ST_EC_INTERESTTYPE = atoi(globdata.globs["ST_EC_INTERESTTYPE"].c_str());

	PLOT_SIZE = atof(globdata.globs["PLOTSIZE"].c_str());
	MAX_H_LU = atoi(globdata.globs["LABOUR_HOURS_PER_UNIT"].c_str());
	INTEREST_RATE = atof(globdata.globs["INTEREST"].c_str());
	REGION_MILK_QUOTA = atof(globdata.globs["REGIONAL_MILK_QUOTA"].c_str());
	OVERHEADS = atof(globdata.globs["OVERHEADS"].c_str());
	LOWER_BORDER = atof(globdata.globs["LOWER_BORDER"].c_str());
	UPPER_BORDER = atof(globdata.globs["UPPER_BORDER"].c_str());

	ManagerMean = atof(globdata.globs["MANAGERMEAN"].c_str());
	ManagerDev= atof(globdata.globs["MANAGERDEV"].c_str());
	
	SHARE_SELF_FINANCE = atof(globdata.globs["SELF_FINANCE_SHARE"].c_str());
	TRANSPORT_COSTS = atoi(globdata.globs["TRANSPORT_COSTS"].c_str());
	WD_FACTOR = atoi(globdata.globs["CAPITAL_WITHDRAW_FACTOR"].c_str());
	GENERATION_CHANGE = atoi(globdata.globs["GENERATION_CHANGE"].c_str());
	INVEST_GROUPS = atoi(globdata.globs["INVEST_GROUPS"].c_str());
	RENT_ADJUST_COEFFICIENT_N = atoi(globdata.globs["RENT_ADJUST_NEIGHBOURS"].c_str());
	RENT_ADJUST_COEFFICIENT = atof(globdata.globs["RENT_ADJUST_FACTOR"].c_str());
	TC_MACHINERY = atoi(globdata.globs["TC_MACHINERY"].c_str());
	SIM_VERSION = atoi(globdata.globs["SIMVERSION"].c_str());

	if (LP_MOD) {
		TRANCH_1_WIDTH = atof(globdata.globs["TRANCH_1_WIDTH"].c_str());
		TRANCH_2_WIDTH = atof(globdata.globs["TRANCH_2_WIDTH"].c_str());
		TRANCH_3_WIDTH = atof(globdata.globs["TRANCH_3_WIDTH"].c_str());
		TRANCH_4_WIDTH = atof(globdata.globs["TRANCH_4_WIDTH"].c_str());
		TRANCH_5_WIDTH = atof(globdata.globs["TRANCH_5_WIDTH"].c_str());
	}
	else {
		LB_LOW_TRANCH = atof(globdata.globs["LOWER_BOUND_LOW_TRANCH"].c_str());
		UB_LOW_TRANCH = atof(globdata.globs["UPPER_BOUND_LOW_TRANCH"].c_str());
		LB_MIDDLE_TRANCH = atof(globdata.globs["LOWER_BOUND_MIDDLE_TRANCH"].c_str());
		UB_MIDDLE_TRANCH = atof(globdata.globs["UPPER_BOUND_MIDDLE_TRANCH"].c_str());
		LB_HIGH_TRANCH = atof(globdata.globs["LOWER_BOUND_HIGH_TRANCH"].c_str());
		UB_HIGH_TRANCH = atof(globdata.globs["UPPER_BOUND_HIGH_TRANCH"].c_str());
	}

	FIX_REFERENCE_PERIOD = atoi(globdata.globs["FIX_REFERENCE_PERIOD"].c_str());
	if (USE_TC_FRAMEWORK) {
		LEGAL_TYPE_BONUS = atoi(globdata.globs["LEGAL_TYPE_BONUS"].c_str()) ? true : false;
		PREV_OWNER_BONUS = atoi(globdata.globs["PREV_OWNER_BONUS"].c_str()) ? true : false;
		FIXED_BONUS = atoi(globdata.globs["FIXED_BONUS"].c_str()) ? true : false;
		if (FIXED_BONUS)
			FIXED_BONUS_VALUE = atoi(globdata.globs["FIXED_BONUS_VALUE"].c_str());
		VARIABLE_BONUS = atoi(globdata.globs["VARIABLE_BONUS"].c_str()) ? true : false;
		if (VARIABLE_BONUS)
			VARIABLE_BONUS_VALUE = atoi(globdata.globs["VARIABLE_BONUS_VALUE"].c_str());
		MIN_CONTRACT_LENGTH = atoi(globdata.globs["MIN_CONTRACT_LENGTH"].c_str());
		MAX_CONTRACT_LENGTH = atoi(globdata.globs["MAX_CONTRACT_LENGTH"].c_str());
	}
	NO_OF_SOIL_TYPES = atoi(globdata.globs["NUMBER_OF_SOIL_TYPES"].c_str());

	//NASG 2019
	NASG_startPeriod= atoi(globdata.globs["NASG_START_PERIOD"].c_str());
	NASG_maxRentAv = atof(globdata.globs["NASG_MAXRENTAV"].c_str());
	NASG_maxShareUAA = atof(globdata.globs["NASG_MAXSHAREUAA"].c_str());
	NASG_maxSizeFactor = atof(globdata.globs["NASG_MAXSIZEFACTOR"].c_str());
    
	setSeeds();
	setRandGens();
	if (ManagerDemographics||YoungFarmer)
		initDemograph();
	if (YoungFarmer) {
		initYoungFarmer();
	}
}

void
RegGlobalsInfo::initGlobals() {
tIter=-1;
tFarmId=-1;
tFarmName="---";
tInd=0;
if (ManagerDistribType == DISTRIB_TYPE::NORMAL) {
	normal_distr.param(std::normal_distribution<>::param_type(ManagerMean, ManagerDev));
}

	
if (ManagerDemographics || YoungFarmer) {
	FF_age_normal_distr.param(std::normal_distribution<>::param_type(FF_initAge_mean, FF_initAge_dev));
	CF_age_normal_distr.param(std::normal_distribution<>::param_type(CF_initAge_mean, CF_initAge_dev));
	GC_newage_normal_distr.param(std::normal_distribution<>::param_type(GC_newAge_mean,GC_newAge_dev));

	GC_CF_uni_distr.param(std::uniform_real_distribution<>::param_type(0, 1));
	GC_FF_uni_distr.param(std::uniform_real_distribution<>::param_type(0, 1));
	ECON_SIZE_CLASS_uni_distr.param(std::uniform_real_distribution<>::param_type(0, 1));
	}

    int noft= number_of_farmtypes = farmsdata.numOfFarms;
	for (int i=0; i<noft; i++){
		int t ;
       ef.push_back(t= farmsdata.formsOrgs[i]);
	}

    for (int i=0;i<noft;i++)
       farm_class.push_back(farmsdata.farm_types[i]);

    for (int i=0;i<noft;i++) {
        int e=farmsdata.weightFacs[i];
        number_of_each_type.push_back(e);
    }

    for (int i=0;i<noft;i++) {
        sheetnames.push_back(farmsdata.names[i]);
    }

    for (int i=0;i<noft;i++) {
        int n=number_of_each_type[i];
        int number = (int)((double)n/(int)TEILER+0.5);
        number_of_each_type[i]=number;
    }

    for (int i=0;i<NO_OF_SOIL_TYPES;i++) {
        string s = globdata.namesOfSoilTypes[i];
        NAMES_OF_SOIL_TYPES.push_back(s);
    }

    // Berechnen des Flaechenbedarfs der Region
    for (int i=0;i<NO_OF_SOIL_TYPES;i++) {
        LAND_INPUT_OF_TYPE.push_back(0);
    }

    for (int i=0;i<noft;i++) {
        for (int k=0;k<NO_OF_SOIL_TYPES;k++) {
            LAND_INPUT_OF_TYPE[k] +=(farmsdata.alllands[globdata.namesOfSoilTypes[k]].owned_land[i])
                            *number_of_each_type[i];
            LAND_INPUT_OF_TYPE[k] += (farmsdata.alllands[globdata.namesOfSoilTypes[k]].rented_land[i])
                            *number_of_each_type[i];
         }
    }
    double landinput=0;
    for (int i=0;i<NO_OF_SOIL_TYPES;i++) {
        landinput+=LAND_INPUT_OF_TYPE[i];
    }
    NON_AG_LANDINPUT=landinput*REGION_NON_AG_LAND;
    landinput+=NON_AG_LANDINPUT;
    landinput*=REGION_OVERSIZE;
    int cols = static_cast<int>( sqrt(landinput/PLOT_SIZE));
    while ((cols*cols*PLOT_SIZE) < landinput) {
        cols++;
    }
    NO_ROWS=cols;
    NO_COLS=cols;
    // total region
    VISION=NO_COLS/2+1;

	//soil service
	if (HAS_SOILSERVICE) regCarbons.resize(NO_OF_SOIL_TYPES);

	for (auto i = 0; i < NO_OF_SOIL_TYPES; ++i)
		MaxRents.push_back(0);
}    // annuityFactor


double RegGlobalsInfo::triangular(string whichgen, double min, double ml, double max) {
    double r;
	//string name = "MGMTCOEFF";
	var_RAND_GEN& rgen = RAND_GENs[whichgen];
	if (!whichgen.compare("MGMTCOEFF")) {
		r = getRandomReal(whichgen, uni_real_distrib_mgmtCoeff);
	}
	else if (!whichgen.compare("FARMAGE")) {
		r = getRandomInt(whichgen, uni_int_distrib_farmAge);
	}
	else if (!whichgen.compare("RENTVARIATION")) {
		r = getRandomReal(whichgen, uni_real_distrib_rentVar);
	}

	//cout << "triang: " << r << endl;
	if (r <= (ml - min) / (max - min)) {
        return min + sqrt(r * (ml - min) * (max - min));
    } else {
        return max - sqrt((1 - r) * (max - ml) * (max - min));
    }
}
double capitalReturnFactor(double p, int t) {
    double q = 1 + p;
    double crf = 0;

    if (q <= 1) {
        crf = 1 / ((double) t);
    } else {
        crf = pow(q,t) * (q - 1) / ( pow(q,t) - 1);
    }
    return crf;
}
// average return

double averageReturn(double p, int t) {
    double q = 1 + p;
    double ar = 0;

    ar = ( ( pow(q,t) / ( pow(q,t)-1) ) - ( 1 / (t*(q-1)) ) );
    return ar;
}

void
RegGlobalsInfo::backup() {
    obj_backup=clone();
}
void
RegGlobalsInfo::restore() {
    RegGlobalsInfo* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
}
RegGlobalsInfo* RegGlobalsInfo::clone() {
    return new RegGlobalsInfo(*this);
}
RegGlobalsInfo* RegGlobalsInfo::create() {
    return new RegGlobalsInfo();
}
