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

#ifndef RegGlobalsH
#define RegGlobalsH
//#include <tchar.h>
#include "math.h"
#include "SimpleOpt.h"
#include <vector>
#include <set>
#define PLUSINF 1E30
#define MINEPS 0.0000000001
#include <map>
#include <string>
#include <random>
#include <variant>

enum class SimPhase { INIT, LAND, INVEST, PRODUCT, FUTURE, BETWEEN,ALL };
enum class DISTRIB_TYPE {UNIFORM, NORMAL};

//static int rndcounter=0;
using namespace std;
class RegGlobalsInfo {
public:
    //rent-variation
    bool Rent_Variation = false;
    double Beta_min = 0.35;
    double Beta_max = 0.8;

	//emsland
	bool RestrictInvestments;
	double Livestock_Inv_farmsPercent;
	set<string> Livestock_Invs;
	map<string, int> AllRestrictInvs;
	
	//notations FF: family farm, CF: corporate farms
	//	GC: generation change, prob: probability
	bool ManagerDemographics;
	bool YoungFarmer;
	double FF_initAge_mean, FF_initAge_dev;
	double CF_initAge_mean, CF_initAge_dev;
	int FF_InitAge_min, FF_InitAge_max;
	int CF_InitAge_min, CF_InitAge_max;
	int GC_age;
	int GC_age_max;
	int GC_newAge_min, GC_newAge_max;
	double GC_newAge_mean, GC_newAge_dev;
	double FF_GC_prob;
	double CF_GC_prob;
	double FF_prod_decrease;
	void initDemograph();

	void initYoungFarmer();
	double YF_startpayMaxAge;
	double YF_priceHa;
	double YF_minHa;
	double YF_maxHa;
	int YF_payYears;
	double YF_maxPay;

	//Economicsize exception by GC
	std::uniform_real_distribution<> ECON_SIZE_CLASS_uni_distr;
	std::minstd_rand0 minstd0_ESC;
    int	ESC_exclusion;
	double ESC_exclusion_prob;

	std::normal_distribution<> FF_age_normal_distr, CF_age_normal_distr,GC_newage_normal_distr;
	std::uniform_real_distribution<> GC_FF_uni_distr, GC_CF_uni_distr;
	std::minstd_rand0 minstd0_cf, minstd0_ff;
	
	vector<map<int, int>> farmAgeDists;
		
	bool DebMip;
	int tIter, uIter;
    int tFarmId, uFarmId;
    string tFarmName, uFarmName;
    int tInd;
	int tInd_land, tInd_future;
	SimPhase tPhase, uPhase;
	map<tuple<int, int, SimPhase>, int> mapMIP;
		
 // NASG
	bool NASG;
	int NASG_startPeriod;
	double NASG_maxRentAv;
	double NASG_maxShareUAA;
	double NASG_maxSizeFactor;
	vector<double> MaxRents;
	bool SDEBUG1, SDEBUG2;

	static const size_t RCOUNT = 13; //# of extra random number generators
	string rand_gen_names[RCOUNT] = { "mgmtCoeff","farmAge","closeFarm", "investAge",
		"contractLengthInit","contractLength",
		"freePlot_rentPlot", "freePlot_initLand",
		"demogFF", "demogCF", "demogNewage", "livestock_inv", "rentVariation"};// , "randPlotType_preparePeriod"


	enum R_ENGINES { MINSTD_RAND0, MINSTD_RAND, MT19937, MT19937_64, KNUTH_B };
	
	map <string,int> RAND_SEEDS;
	void setSeeds();
	map<string,R_ENGINES> RAND_ENGINES;
	void initRandEngines();
	void setRandGens();
	R_ENGINES TO_RAND_ENGINE(string);
	
	int getRandomInt(string, std::uniform_int_distribution<>&);
	double getRandomReal(string, std::uniform_real_distribution<>&);

    std::uniform_real_distribution<> uni_real_distrib_rentVar;
	std::uniform_real_distribution<> uni_real_distrib_mgmtCoeff;
	std::uniform_int_distribution<> uni_int_distrib_farmAge;
	std::uniform_real_distribution<> uni_real_distrib_closeFarm;
	std::uniform_int_distribution<> uni_int_distrib_investAge;
	std::uniform_int_distribution<> uni_int_distrib_contractLengthInit;
	std::uniform_int_distribution<> uni_int_distrib_contractLength;
	std::uniform_int_distribution<> uni_int_distrib_freePlot_rentPlot;
	std::uniform_int_distribution<> uni_int_distrib_freePlot_initLand;

	std::uniform_real_distribution<> uni_real_distrib_livestock_inv;

	DISTRIB_TYPE ManagerDistribType;
	double ManagerMean;
	double ManagerDev;
	std::normal_distribution<> normal_distr;
	double getRandomNormal(string, std::normal_distribution<>&);

	using var_RAND_GEN = variant<std::mt19937, std::mt19937_64,
		std::minstd_rand, std::minstd_rand0, std::knuth_b>;
	map<string, var_RAND_GEN> RAND_GENs;
	double triangular(string, double, double, double);

	static const string UsageString;

	//New for Soil service
    bool HAS_SOILSERVICE; 
	double TECH_DEVELOP;  
	double CARBON_MIN;
	double CARBON_MAX;
	double tech_develop_abs;
	vector<double> regCarbons;   //average for the region

    bool PRINT_SEC_RES;
    bool PRINT_SEC_PRICE;
    bool PRINT_SEC_EXP_PRICE;
    bool PRINT_SEC_COSTS;
    bool PRINT_SEC_COND;
    bool PRINT_FARM_RES;
    bool PRINT_FARM_INV;
    bool PRINT_FARM_PROD;
    bool PRINT_FARM_COSTS;
    bool PRINT_CONT_PLOTS;
    bool PRINT_TAC;
    bool PRINT_VA;
    bool PRINT_POLICY;

    bool INIT_OUTPUT;
    bool CALCULATE_EXPECTED_RENTAL_PRICE;
        int STATUS;
    double WERTS;
    double WERTS1;
    double WERTS2;
    virtual RegGlobalsInfo* create();
    virtual RegGlobalsInfo* clone();
    bool LP_CHANGED;
    bool INITIALISATION;
    virtual void backup();
    virtual void restore();
    bool FIXED_BONUS;
    double FIXED_BONUS_VALUE;
    bool VARIABLE_BONUS;
    double VARIABLE_BONUS_VALUE;
    bool PREV_OWNER_BONUS;
    bool LEGAL_TYPE_BONUS;
    bool ADJUST_PAID_RENT;
    bool SWEDEN;
    bool JOENKEPING;
    bool USE_TC_FRAMEWORK;
    bool ASSOCIATE_ACTIVITIES;
    bool USE_VARIABLE_PRICE_CHANGE;
    bool AGE_DEPENDENT;
    bool NO_SUCCESSOR_BY_RANDOM;
    bool ENV_MODELING;
    bool SOIL_TYPE_VARIATION;
    bool WEIGHTED_PLOT_SEARCH;
    double WEIGHTED_PLOT_SEARCH_VALUE;
    bool FIX_PRICES;
    bool MIN_WITHDRAWAL;
    bool USE_TRIANGULAR_DISTRIBUTED_MANAGEMENT_FACTOR;
    bool USE_TRIANGULAR_DISTRIBUTED_FARM_AGE;
    bool USE_HISTORICAL_CONTIGUOUS_PLOTS;
    bool RELEASE_PLOTS_BEFORE_EXPECTATION_FORMATION;
    vector<int> LEGAL_TYPES;
    bool CALC_LEGAL_TYPES;
    vector<string> NAMES_OF_LEGAL_TYPES;
    bool SET_FREE_PLOTS;
    vector<int> FREE_PLOTS_OF_TYPE;
    bool LP_MOD;
	
    string INPUTFILEdir; 

    string INPUTFILE;
    string OUTPUTFILE;
    string POLICYFILE;

	string SCENARIOFILE;
	string Scenario;
	string Scenario_Description;
	map <string, string> Scenario_globs;
	map <string, string> Scenario_options;
	string TimeStart, TimeStop;

    int TC;
    int MT;
    int I;
    int P;
    int PC;
    int V;
    bool DEFAULT;
    int SEED;
    int SCENARIO;
    double QUOTA_PRICE;
    int GLOBAL_STRATEGY;
    bool GLOBAL_OPTIMUM_EVERY_PERIOD;
    bool FIRSTPRICE;
    bool SECONDPRICE_REGION;
    const void setNumberOfProducts(int n) {
        MAXPRODUCTS=n;
    }
    // GENERAL GLOBALS
    int number_of_farmtypes;
    vector<string> sheetnames;
    vector<int> number_of_each_type;
    vector<int> farm_class;
    vector<int> ef;
    int AVERAGE_OFFER_BUFFER_SIZE;
    double REGION_OVERSIZE;
    bool    CALCULATE_CONTIGUOUS_PLOTS;

    double REGION_NON_AG_LAND;
    double NON_AG_LANDINPUT;
    int NO_ROWS;
    int NO_COLS;
    int TRANSPORT_COSTS;
    
	int RUNS;
    int TEILER;
    
	vector<double> LAND_INPUT_OF_TYPE;
    int NO_OF_SOIL_TYPES;
    vector<string> NAMES_OF_SOIL_TYPES;
    bool OLD_LAND_RENTING_PROCESS;
    bool OLD_LAND_RELEASING_PROCESS;
    double RENT_ADJUST_COEFFICIENT;
    int RENT_ADJUST_COEFFICIENT_N;
    int TC_MACHINERY;
    double WITHDRAWFACTOR;
    int SIM_VERSION;
	
    //management coefficient
    double LOWER_BORDER;
    double UPPER_BORDER;
    // FARM GLOBALS
    double PLOT_SIZE;
    int GENERATION_CHANGE;
    // labour
    int MAX_H_LU;   // max hours per LU
    double OFF_FARM_LABOUR;
    // capital
    int    WD_FACTOR;      // variability factor for capital withdrawal
    double SHARE_SELF_FINANCE;
    double EQ_INTEREST;
    // not in dialog
    double INTEREST_RATE;
    double REGION_MILK_QUOTA;
    double OVERHEADS;

	// Display
    int APPEARANCE;
    int FARMOUTPUT;
    int SECTOROUTPUT;
    int VISION;
    
	// Constants
	int DESIGN_POINT;
    int RANDOM;
    int MAXPRODUCTS;           // max. number of products produced on farm
    int NUMBER_OF_INVESTS;
    int INVEST_GROUPS;
    //Files
    RegGlobalsInfo();
    const void setAppearance(int a) {
        APPEARANCE=a;
    }
    const void setEqInterest(double e) {
        EQ_INTEREST=e;
    }
    const void setInterest(double e) {
        INTEREST_RATE=e;
    }
    const void setOffFarmLabour(double e) {
        OFF_FARM_LABOUR=e;
    }
    void initGlobals();
	void initGlobalsRead();
    void readFromCommandLine();
    
    //DECOUPLING
    int MIN_CONTRACT_LENGTH;
    int MAX_CONTRACT_LENGTH;

    double LB_LOW_TRANCH;
    double UB_LOW_TRANCH;
    double LB_MIDDLE_TRANCH;
    double UB_MIDDLE_TRANCH;
    double LB_HIGH_TRANCH;
    double UB_HIGH_TRANCH;

    double DEG_LOW_TRANCH;
    double DEG_MIDDLE_TRANCH;
    double DEG_HIGH_TRANCH;

    double TRANCH_1_WIDTH;
    double TRANCH_2_WIDTH;
    double TRANCH_3_WIDTH;
    double TRANCH_4_WIDTH;
    double TRANCH_5_WIDTH;
    double TRANCH_1_DEG;
    double TRANCH_2_DEG;
    double TRANCH_3_DEG;
    double TRANCH_4_DEG;
    double TRANCH_5_DEG;
    int FIX_REFERENCE_PERIOD;

    int REGIONAL_DECOUPLING;
    int FULLY_DECOUPLING;
    int FARMSPECIFIC_DECOUPLING;
    int REGIONAL_DECOUPLING_SWITCH;
    int FULLY_DECOUPLING_SWITCH;
    int FARMSPECIFIC_DECOUPLING_SWITCH;
    string OUTPUT_FOLDER;

    int PLOTSN;
    int NUMBER_OF_INVESTTYPES;
    double PIGLETS_PER_SOW;
    double PIGS_PER_PLACE;
    double MILKPROD;
    double ESU;
    
	//INCREASEPRICE differentiated 
	double IncPriceHiredLab;
	double IncPriceOffFarmLab;

    double BONUS;
    double REFINCOME;
    int PRODUCTGROUPS;
    int PRODGROUPLAB;
    int PRODTYPE;
    double CHANGEPERHA;
    double CHANGEUP    ;
    double CHANGEDOWN   ;
    double MILKUPPERLIMIT;
    double MILKLOWERLIMIT ;

    int OFFFARMLABTYPE;
	int HIREDLABTYPE;
    int VAROFFARMLABTYPE;
    int VARHIREDLABTYPE  ;
    int ST_BOR_INTERESTTYPE;
    int ST_EC_INTERESTTYPE;

    string premiumName;

    //2016
    map<string,int> stdNameIndexs;

    int   FIXED_OFFFARM_LAB;
    int   FIXED_HIRED_LAB;


    bool FAST_PLOT_SEARCH;
    bool PRINT_REMOVED_FARMS;
    void setNumberOfInvestType(int t) {
        NUMBER_OF_INVESTTYPES=t;
    };
    void setEnvModeling(bool n) {
        ENV_MODELING=n;
    };

	void setOption(int* name, int val) {
		*name = val;
	}

	vector<string> commandlineFILES;
	map<int*, int> options;
    virtual ~RegGlobalsInfo();
    int ARGC;
	char ** ARGV;
		
protected:
    RegGlobalsInfo* obj_backup;

};

//double triangular(double min, double ml, double max);
double capitalReturnFactor(double,int);
double averageReturn(double,int);

template<class A,class B>
void swap(A* a,B* b,int l,int r) {
    A tmp1=a[l];
    B tmp2=b[l];
    a[l]=a[r];
    b[l]=b[r];
    a[r]=tmp1;
    b[r]=tmp2;
}
template<class A,class B>
int partition( A* a,B* b, int low, int high ) {
    int left, right;
    A pivot_item;
    B pivot_item2;
    pivot_item = a[low];
    pivot_item2 = b[low];
    left = low;
    right = high;
    while ( left < right ) {
        //* Move left while item < pivot 
        while ( a[left] <= pivot_item && left<high) left++;
        //* Move right while item > pivot 
        while ( a[right] > pivot_item && right>low) right--;
        if ( left < right ) swap(a,b,left,right);
    }
    //* right is final position for the pivot 
    a[low] = a[right];
    a[right] = pivot_item;
    b[low] = b[right];
    b[right] = pivot_item2;
    return right;
}
template<class A,class B>
void quicksort(A* a,B* b, int low, int high ) {
    int pivot;
    //* Termination condition! 
    if ( high > low )   {
        pivot = partition( a, b ,low, high );
        quicksort( a, b, low, pivot-1 );
        quicksort( a, b, pivot+1, high );
    }
}

template<class A>
void swap(A* a,int l,int r) {
    A tmp1=a[l];
    a[l]=a[r];
    a[r]=tmp1;
}
template<class A>
int partition( A* a, int low, int high ) {
    int left, right;
    A pivot_item;
    pivot_item = a[low];
    left = low;
    right = high;
    while ( left < right ) {
        //* Move left while item < pivot 
        while ( a[left] <= pivot_item && left<high) left++;
        //* Move right while item > pivot 
        while ( a[right] > pivot_item && right>low) right--;
        if ( left < right ) swap(a,left,right);
    }
    //* right is final position for the pivot 
    a[low] = a[right];
    a[right] = pivot_item;
    return right;
}
template<class A>
void quicksort(A* a, int low, int high ) {
    int pivot;
    //* Termination condition! 
    if ( high > low )   {
        pivot = partition( a, low, high );
        quicksort( a, low, pivot-1 );
        quicksort( a, pivot+1, high );
    }
}
//*/
#endif
