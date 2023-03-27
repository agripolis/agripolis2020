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

// RegFarm.cpp
#include "RegFarm.h"
#include "RegPlot.h"
#include <iomanip>
#include <map>
#include "random.h"

using namespace std;
const double EPS = 1E-7;

//Soil service
void RegFarmInfo::initCarbons(vector<double> mean, vector<double> var) { //var --std_dev
	list<RegPlotInfo*>::iterator iter=PlotList.begin();
	int nsoil=g->NO_OF_SOIL_TYPES;

	for (int i = 0; i<nsoil; ++i) {
		avCarbons[i]=mean[i];
		varCarbons[i]=var[i];
	}

	vector< vector <RegPlotInfo*> > pplots;
	pplots.resize(nsoil);
	while (iter != PlotList.end()){
		pplots[(*iter)->getSoilType()].push_back(*iter);
		++iter;
	}

	for (int i = 0; i<nsoil; ++i) {
		nPlots[i]= pplots[i].size(); 
	}

	for (int i = 0; i<nsoil; ++i) {
		for (unsigned j=0; j<pplots[i].size(); ++j) {
			double x =  rand_normal(mean[i],var[i]);
			while ( (x< g->CARBON_MIN )||(x>g->CARBON_MAX))
				x = rand_normal(mean[i],var[i]);
			pplots[i][j]->setCarbon(x); // *g->PLOT_SIZE); //x is c-value per ha ?
		}
	}

	calAvCarbons();
	return;
}

void RegFarmInfo::calAvCarbons(){
	int sz=g->NO_OF_SOIL_TYPES;
	vector<double> ns;
	vector<double> sum;
	vector<double> sum2;
	ns.resize(sz); 
	sum.resize(sz);
	sum2.resize(sz);

	list<RegPlotInfo*>::iterator iter=PlotList.begin();
	while (iter != PlotList.end()){
		sum[(*iter)->getSoilType()]+=(*iter)->getCarbon() ;
		sum2[(*iter)->getSoilType()]+=((*iter)->getCarbon())*((*iter)->getCarbon());
		ns[(*iter)->getSoilType()]++; //=g->PLOT_SIZE;
		++iter;
	}
	for (int i=0; i< sz; ++i){
		if(ns[i]>0) {
			avCarbons[i]=sum[i]/ns[i]; //land_input_of_type[i];  ???
			varCarbons[i]=sum2[i]/ns[i]-avCarbons[i]*avCarbons[i]; //land_input_of_type[i]
		}
	}
	return;
}

void RegFarmInfo::calDeltaCarbons(){
	int sz=g->NO_OF_SOIL_TYPES;
	vector<double> delta;
	for (int i=0; i<sz; ++i)
		delta.push_back(0);
	
	int psz=(*product_cat).size();
	for (int i=0; i<psz; ++i) {
		bool hasSoilserv= (*product_cat)[i].hasSoilService();
		if (!hasSoilserv) continue;
		double g=(*product_cat)[i].getGamma();
		double p = FarmProductList->getUnitsProducedOfNumber(i);
		int soiltype = (*product_cat)[i].getProdSoilType();
		delta[soiltype]+=g*p;
	}

	for (int i=0 ; i<sz; ++i)
		if (land_input_of_type[i] > 0) 
			deltCarbons[i] = delta[i] * avCarbons[i] /land_input_of_type[i];
		else deltCarbons[i]=0;
	return;
}

//landallocation, disinvest
void RegFarmInfo::updateCarbons(RegPlotInfo plot, bool neu){
	double delta = plot.getCarbon();
	delta = neu ? delta :-delta;
	int soiltype = plot.getSoilType();
	
	double sum = nPlots[soiltype]*avCarbons[soiltype] + delta;
	nPlots[soiltype]+= neu ? 1: -1;

	avCarbons[soiltype]=sum*g->PLOT_SIZE/land_input_of_type[soiltype]; //nPlots[soiltype];
	return;
}

//after deltaCarbons are updated
void RegFarmInfo::updateCarbons(){
	list<RegPlotInfo*>::iterator iter=PlotList.begin();
	
	while (iter != PlotList.end()){
		(*iter)->setCarbon((*iter)->getCarbon() + deltCarbons[(*iter)->getSoilType()]);
		++iter;
	}

	calAvCarbons();
	return;
}

RegProductList* RegFarmInfo::getProductList(){
	return FarmProductList;
}

void RegFarmInfo::setAvCarbons(vector<double> avc){
	avCarbons= avc;
	return;
}

void RegFarmInfo::setVarCarbons(vector<double> vc){
	varCarbons= vc;
	return;
}

vector<double> RegFarmInfo::getAvCarbons(){
	return avCarbons;
}

vector<double> RegFarmInfo::getVarCarbons(){
	return varCarbons;
}

//!!! here land_input_of_type is already updated
void RegFarmInfo::increaseCarbonOfType(int soiltype, int anz){
	avCarbons[soiltype]+= (g->regCarbons[soiltype]-avCarbons[soiltype])*anz* g->PLOT_SIZE/land_input_of_type[soiltype]; 
	return;
}

//!!! here land_input_of_type is already updated
void RegFarmInfo::decreaseCarbonOfType(int soiltype, int anz){
	if (land_input_of_type[soiltype] >0 ) 
		avCarbons[soiltype]+= (avCarbons[soiltype]-g->regCarbons[soiltype])*anz* g->PLOT_SIZE/land_input_of_type[soiltype]; 
	else 
		;//avCarbons[soiltype]=0;
	return;
}
//---------------------------------------------------------------------------
//  RegFarm constructors/destructors
//---------------------------------------------------------------------------

RegFarmInfo::RegFarmInfo(RegRegionInfo *reg,
                         RegGlobalsInfo* G,
                         vector<RegProductInfo> &PCat,               // reference to market's product objects
                         vector<RegInvestObjectInfo >& ICat, // InvestCatalog
                         RegLpInfo* lporig,
                         short int pop,
                         int number,
                         int fc,
                         string farmname,
                         int farmerwerbsform)
        :region(reg),g(G),invest_cat(&ICat),product_cat(&PCat) {
	
	restrict_invest = false;
	allow_invest = false;
	reinvestLUcap = 1e+30;

	GenChange_demograph = 0;
	YoungFarmer_years = 0;
	youngfarmerPay = 0;
	youngfarmerPaid = false;
	youngfarmerMinSize = false;

//soil service
	for (int i=0; i<g->NO_OF_SOIL_TYPES; ++i) {
		avCarbons.push_back(0);
		varCarbons.push_back(0);
		deltCarbons.push_back(0);
		nPlots.push_back(0);
	}
	
    ////////////
    /// GENERAL
    ////////////
    sunk_costs_labor=0;
    obj_backup=NULL;
    flat_copy=false;
    labour = new RegLabourInfo(g);
    display_modulation=0;
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        land_capacity_estimation_of_type.push_back(0);
        premium_estimation_of_type.push_back(0);
        land_input_of_type.push_back(0);
        rented_land_of_type.push_back(0);
        initial_owned_land_of_type.push_back(0);

        initial_rented_land_of_type.push_back(0);
        initial_rent_of_type.push_back(0);
        sp_estimation.push_back(0);
        cache_sp_of_type.push_back(0);
        cache_premium_of_type.push_back(0);
        cache_actual_of_type.push_back(false);
        lp_result_with_plotsn_new_plots_of_type.push_back(0);
        lp_result_with_new_plot_of_type.push_back(0);
        delta_profit_of_type.push_back(0);
        wanted_plot_of_type.push_back(RegPlotInformationInfo());

    }
    actual=false;

    assets=0;
    land_assets = 0;
    land_input=0;
    farm_name=farmname;
   
    farm_colour = -1;
    invest_get_older = false;
    farm_class = fc;
    old_farm_class = farm_class;
    farm_class_change = false;
    farm_type = pop;
    farm_id = number;
    farm_age = 0;
    full_time = true;
    legal_type = farmerwerbsform;
    switch_erwerbsform = -1;
    if (farmerwerbsform==1)
        full_time = true;
    else
        full_time = false;

    farm_colour = 0;// RGB(R,GR,B);

    // lp variables
    lp_result=0;
    first_time =true;

    // capital variables
    financing_rule =0;
    bonus = 0;
    lt_borrowed_capital_old = 0;
    lt_borrowed_capital = 0;
    lt_interest_costs_supported =0;
    st_borrowed_capital = 0;
    capital_input = 0;
    closed = 0;
    depreciation = 0;
    sc_relevant = 1;
    farm_distance_costs = 0;
    farm_rent_exp = 0;
    farm_tac=0;
    farm_repayment = 0;
    farm_income = 0;
    opp_own_land = 0;
    total_income = 0;
    total_maintenance = 0;
    fix_costs = 0;
    interest_costs = 0;
    lt_interest_costs = 0;
    profit = 0;
    rent_offer = 0;
    st_interest_costs = 0;
    st_interest_received = 0;
    value_added = 0;
    annuity = 0;
    gm = 0;
    gm_products = 0;
    gm_agriculture = 0;
    overheads = 0;
    add_st_capital = 0;
    revenue = 0;
    assets_at_production_wo_land = 0;
    premium  = 0;
    average_premium = 0;
    income_payment_farm = 0;
    modulated_income_payment = 0;
    reference_income_payment_farm = 0;
    reference_income_payment_farm_old = 0;
    number_new_investments_wo_labour = 0;
    ecchange = 0;

    // labour
    farm_hired_labour_fix_pay = 0;
    farm_hired_labour_var_pay = 0;
    farm_fix_labour_income = 0;
    farm_var_labour_income = 0;
    farm_factor_remuneration_fix = 0;
    farm_factor_remuneration_var = 0;
    land_remuneration = 0;

    //////////////////////////////////
    /// FARMPRODUCTLIST FARMINVESTLIST
    //////////////////////////////////
    // Generate Random Number between lower_border and upper_border
    double lower_border=g->LOWER_BORDER;
    double upper_border=g->UPPER_BORDER;

	if (g->USE_TRIANGULAR_DISTRIBUTED_MANAGEMENT_FACTOR) {
		management_coefficient = g->triangular("MGMTCOEFF", lower_border, lower_border + (upper_border - lower_border) / 2, upper_border);
    } else {
        //management_coefficient = lower_border + (upper_border-lower_border)*((double)randlong()/mtRandMax);//RAND_MAX);
		double r;
		string name = "MGMTCOEFF";
		
		if (g->ManagerDistribType != DISTRIB_TYPE::NORMAL) {
			r = g->getRandomReal(name, g->uni_real_distrib_mgmtCoeff);
			management_coefficient = lower_border + (upper_border - lower_border)*r;
		}
		else {
			r = g->getRandomNormal(name, g->normal_distr);
			while (r < lower_border || r > upper_border)
				r = g->getRandomNormal(name, g->normal_distr);
			management_coefficient = r;
		}
		//cout << mean<<"\t"<<stddev<<"\t"<< r << endl;
		
    }
    FarmInvestList = new RegInvestList(g,ICat);
    FarmProductList = new RegProductList(g,PCat);

    /////////////////////
    // GET READY FOR LP
    /////////////////////
    lp = lporig->clone(g);
    lp->initLinks(this,
                  FarmInvestList,
                  FarmProductList,
                  labour,
                  &liquidity,
                  &land_input,
                  &milk_quota,
                  &financing_rule,
                  &modulated_income_payment,
                  &income_payment_farm,
                  &(g->TRANCH_1_WIDTH),
                  &(g->TRANCH_2_WIDTH),
                  &(g->TRANCH_3_WIDTH),
                  &(g->TRANCH_4_WIDTH),
                  &(g->TRANCH_5_WIDTH),
                  &(g->TRANCH_1_DEG),
                  &(g->TRANCH_2_DEG),
                  &(g->TRANCH_3_DEG),
                  &(g->TRANCH_4_DEG),
                  &(g->TRANCH_5_DEG)
);

	/////////////
    // PRODUCTION
    /////////////

    inum_vector.resize((*invest_cat).size());

    PlotList.clear();
  /*  // place farm plot in list
    region->setFarmsteadPlot(farm_plot,this);
    if(g->WEIGHTED_PLOT_SEARCH)
	   farm_plot->initFreePlots(this);
    occupyPlot(farm_plot);
   //*/
	for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        cache_land_input_of_type.push_back(0);
    }
}

// LOCATION OF FARM
void RegFarmInfo::setFarmStead() {
	double max=0;
    int i_max=0;
    for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
      if(initial_owned_land_of_type[i]>max) {
        i_max=i;
        max=initial_owned_land_of_type[i];
      }
    }
    farm_plot = region->getRandomFreePlotOfType(i_max); 
	
	// place farm plot in list
    region->setFarmsteadPlot(farm_plot,this);
   // if(g->WEIGHTED_PLOT_SEARCH)
   //   farm_plot->initFreePlots(this);
    occupyPlot(farm_plot);
	return;
}

RegFarmInfo* RegFarmInfo::clone() {
    RegFarmInfo* f=new RegFarmInfo(*this);
    f->flat_copy=true;
    return f;
}

RegFarmInfo* RegFarmInfo::create() {
    return new RegFarmInfo();
}
void RegFarmInfo::assign() {
    *((RegFarmInfo*)this)=*((RegFarmInfo*)obj_backup);
};

RegFarmInfo* RegFarmInfo::clone(RegGlobalsInfo* G,RegRegionInfo * reg,
                                vector<RegProductInfo> &PCat,
                                vector<RegInvestObjectInfo >& ICat) {
    RegFarmInfo* n=create();
    (*n).g=G;
    (*n).sunk_costs_labor=sunk_costs_labor;
    (*n).display_modulation=display_modulation;
    (*n).tacs=tacs;
    (*n).product_cat=&PCat;
    (*n).invest_cat=&ICat;
    (*n).region=reg;
    (*n).closed=closed;
    (*n).old_farm_class=old_farm_class;
    (*n).farm_class=farm_class;
    (*n).farm_class_change=farm_class_change;
    (*n).farm_type=farm_type;
    (*n).farm_id=farm_id;
    (*n).farm_colour=farm_colour;
    (*n).farm_age=farm_age;
    (*n).full_time=full_time;
    (*n).switch_erwerbsform=switch_erwerbsform;
    (*n).legal_type=legal_type;
    (*n).number_of_plots=number_of_plots;
    (*n).milk_quota=milk_quota;
    (*n).land_input=land_input;
    (*n).cache_land_input_of_type=cache_land_input_of_type;
    (*n).land_input_of_type=land_input_of_type;
    (*n).rented_land_of_type=rented_land_of_type;
    (*n).first_time=first_time;
    (*n).actual=actual;
    (*n).sp_estimation=sp_estimation;
    (*n).lp_result=lp_result;
    (*n).last_rented_plot=last_rented_plot;
    (*n).lp_result_with_plotsn_new_plots_of_type=lp_result_with_plotsn_new_plots_of_type;
    (*n).lp_result_with_new_plot_of_type=lp_result_with_new_plot_of_type;
    (*n).delta_profit_of_type=delta_profit_of_type;
    (*n).wanted_plot_of_type=wanted_plot_of_type;
    for (unsigned int i=0;i<wanted_plot_of_type.size();i++) {
        if (wanted_plot_of_type[i].plot!=NULL)
            (*n).wanted_plot_of_type[i].plot=(*n).region->plots[wanted_plot_of_type[i].plot->getId()];
        else
            (*n).wanted_plot_of_type[i].plot=NULL;
    }
    (*n).initial_owned_land_of_type=initial_owned_land_of_type;
    (*n).initial_rented_land_of_type=initial_rented_land_of_type;
    (*n).initial_rent_of_type=initial_rent_of_type;
    (*n).initial_land=initial_land;
    (*n).farm_rent_exp=farm_rent_exp;
    (*n).farm_tac=farm_tac;
    (*n).farm_distance_costs=farm_distance_costs;
    (*n).rent_offer=rent_offer;
    (*n).opp_own_land=opp_own_land;
    (*n).land_capacity_estimation_of_type=land_capacity_estimation_of_type;
    (*n).premium_estimation_of_type=premium_estimation_of_type;
    (*n).cache_sp_of_type=cache_sp_of_type;
    (*n).cache_premium_of_type=cache_premium_of_type;
    (*n).cache_actual_of_type=cache_actual_of_type;
    (*n).financing_rule=financing_rule;
    (*n).assets=assets;
    (*n).assets_at_production_wo_land=assets_at_production_wo_land;
    (*n).land_assets=land_assets;
    (*n).lt_borrowed_capital=lt_borrowed_capital;
    (*n).st_borrowed_capital=st_borrowed_capital;
    (*n).lt_borrowed_capital_old=lt_borrowed_capital_old;
    (*n).capital_input=capital_input;
    (*n).depreciation=depreciation;
    (*n).equity_capital=equity_capital;
    (*n).economic_profit=economic_profit;
    (*n).liquidity=liquidity;
    (*n).value_added=value_added;
    (*n).annuity=annuity;
    (*n).withdrawal=withdrawal;
    (*n).profit=profit;
    (*n).farm_income=farm_income;
    (*n).total_income=total_income;
    (*n).overheads=overheads;
    (*n).revenue=revenue;
    (*n).gm_products=gm_products;
    (*n).gm=gm;
    (*n).gm_agriculture=gm_agriculture;
    (*n).add_st_capital=add_st_capital;
    (*n).premium=premium;
    (*n).average_premium=average_premium;
    (*n).income_payment_farm=income_payment_farm;
    (*n).modulated_income_payment=modulated_income_payment;
    (*n).reference_income_payment_farm=reference_income_payment_farm;
    (*n).reference_income_payment_farm_old=reference_income_payment_farm_old;
    (*n).ecchange=ecchange;
    (*n).farm_hired_labour_fix_pay=farm_hired_labour_fix_pay;
    (*n).farm_hired_labour_var_pay=farm_hired_labour_var_pay;
    (*n).farm_fix_labour_income=farm_fix_labour_income;
    (*n).farm_var_labour_income=farm_var_labour_income;
    (*n).farm_factor_remuneration_fix=farm_factor_remuneration_fix;
    (*n).farm_factor_remuneration_var=farm_factor_remuneration_var;
    (*n).land_remuneration=land_remuneration;
    (*n).farm_repayment=farm_repayment;
    (*n).fix_costs=fix_costs;
    (*n).interest_costs=interest_costs;
    (*n).lt_interest_costs=lt_interest_costs;
    (*n).lt_interest_costs_supported=lt_interest_costs_supported;
    (*n).st_interest_costs=st_interest_costs;
    (*n).st_interest_received=st_interest_received;
    (*n).total_maintenance=total_maintenance;
    (*n).reference_premium=reference_premium;
    (*n).number_new_investments_wo_labour=number_new_investments_wo_labour;
    (*n).rel_invest_age=rel_invest_age;
    (*n).bonus=bonus;
    (*n).sc_relevant=sc_relevant;
    (*n).invest_get_older=invest_get_older;
    (*n).management_coefficient=management_coefficient;
    (*n).rent_offer_old=rent_offer_old;
    (*n).inum_vector=inum_vector;

    (*n).PlotList.clear();
    list<RegPlotInfo* >::const_iterator plrh;
    for (plrh = PlotList.begin();
            plrh != PlotList.end();
            plrh++) {
        (*n).PlotList.push_back((*n).region->plots[(*plrh)->getId()]);
    }
    (*n).FarmInvestList=new RegInvestList(*FarmInvestList,(*n).g,(*((*n).invest_cat)));
    (*n).FarmProductList=new RegProductList(*FarmProductList,(*n).g,(*((*n).product_cat)));
    if (farm_plot!=NULL)
        (*n).farm_plot=(*n).region->plots[farm_plot->getId()];
    else
        (*n).farm_plot=NULL;
    if (wanted_plot!=NULL)
        (*n).wanted_plot=(*n).region->plots[wanted_plot->getId()];
    else
        (*n).wanted_plot=NULL;
    (*n).labour=new RegLabourInfo(*labour,(*n).g);
    (*n).lp=lp->clone((*n).g);
    (*n).lp->initLinks(n,
                       (*n).FarmInvestList,
                       (*n).FarmProductList,
                       (*n).labour,
                       &((*n).liquidity),
                       &((*n).land_input),
                       &((*n).milk_quota),
                       &((*n).financing_rule),
                       &((*n).modulated_income_payment),
                       &((*n).income_payment_farm),
                       &((*n).g->TRANCH_1_WIDTH),
                       &((*n).g->TRANCH_2_WIDTH),
                       &((*n).g->TRANCH_3_WIDTH),
                       &((*n).g->TRANCH_4_WIDTH),
                       &((*n).g->TRANCH_5_WIDTH),
                       &((*n).g->TRANCH_1_DEG),
                       &((*n).g->TRANCH_2_DEG),
                       &((*n).g->TRANCH_3_DEG),
                       &((*n).g->TRANCH_4_DEG),
                       &((*n).g->TRANCH_5_DEG)
);
    (*n).updateLpValues();
    (*n).contiguous_plots=contiguous_plots;
    return n;
}

RegFarmInfo::~RegFarmInfo() {
    farm_plot = NULL;
    if (!flat_copy) {
        delete lp;
        delete labour;
        delete FarmInvestList;
        delete FarmProductList;
    }
    if (obj_backup) delete obj_backup;
}


/////////////////////////
// INITIALISATION METHODS
/////////////////////////

void
RegFarmInfo::setInitialFamLu(double m) {
    labour->adjustFamLu(m,management_coefficient);
}

void
RegFarmInfo::updateCosts() {
    FarmProductList->updateCosts(management_coefficient);
}

double RegFarmInfo::getRandomNormalRange(string nm, std::normal_distribution<>& distr, 
	double low, double high) {
	double r = g->getRandomNormal(nm, distr);
	while (r <low || r >high) {
		//cout << r << endl;
		r = g->getRandomNormal(nm, distr);
	}
	return r;
}

void
RegFarmInfo::setAsynchronousFarmAge() {
    int ran;
	int lb, ub;
	lb = (legal_type==2)? g->CF_InitAge_min: g->FF_InitAge_min;
	ub = (legal_type==2)? g->CF_InitAge_max: g->FF_InitAge_max;
    double eqinterest = (*product_cat)[1].getPrice();
    // int ran = (int)(((double)rand()/RAND_MAX)*g->GENERATION_CHANGE);
	if (g->ManagerDemographics || g->YoungFarmer) {
		double r;
		if (legal_type == 2) {
			r = getRandomNormalRange("DEMOGCF", g->CF_age_normal_distr,lb,ub);
		}
		else {
			r = getRandomNormalRange("DEMOGFF", g->FF_age_normal_distr, lb, ub);
		}
		ran = (int)(r + 0.5);
	}else {
		if (g->USE_TRIANGULAR_DISTRIBUTED_FARM_AGE) {
			ran = static_cast<int>(g->triangular("FARMAGE", 1, 12, g->GENERATION_CHANGE));
		}
		else {
			int r;
			string name = "FARMAGE";
			r = g->getRandomInt(name, g->uni_int_distrib_farmAge);

			ran = r;// randlong() % g->GENERATION_CHANGE;
			//cout <<"fage: "<< ran << endl;
		}
	}

    setFarmAge(ran);
	if (g->ManagerDemographics|| g->YoungFarmer) {
		if (legal_type <= 1) g->farmAgeDists[1][ran]++;
		else g->farmAgeDists[2][ran]++;
		if (g->YoungFarmer && ran <= g->YF_startpayMaxAge) {
			updateYoungFarmer();
		}
	}

    // initial assets
    assets += land_assets;

    FarmInvestList->setAsynchronousInvestAge(farm_age,&assets,&liquidity,&lt_borrowed_capital,eqinterest,rel_invest_age);
    labour->setLabourCapacity(labour->getFamilyLabour()
                              +FarmInvestList->getLabourSubstitution() // lab effect of invests inkl fix labour
                             );

    calculateLiquidity();
    calculateFinancingRule();
    updateLpValues();
}

double& RegFarmInfo::refYoungFarmerYears() {
	return YoungFarmer_years;
}

double
RegFarmInfo::getInitialOwnedLand() const {
    double sum=0;
    for (int j=0;j<g->NO_OF_SOIL_TYPES ;j++) {
        sum += initial_owned_land_of_type[j];
    }
    return sum;
}

int RegFarmInfo::getRandomContractLength() {
	string name = "CONTRACTLENGTHINIT";

	return g->getRandomInt(name, g->uni_int_distrib_contractLengthInit);
}

bool 
RegFarmInfo::getInitialRentedPlot(double rent,int type) {
	// int col0, row0, col1, row1;
	if (g->INITIALISATION) {
		wanted_plot = farm_plot->findMostPreferablePlotOfType(type, this).plot;
		//	col0 = wanted_plot->getCol();
		//	row0 = wanted_plot->getRow();
	}
	else {
		wanted_plot = farm_plot->findMostPreferablePlotOfType(type).plot;
		//	col1 = wanted_plot->getCol(); 
		//	row1 = wanted_plot->getRow();
	}
	//	if (this->farm_id==0 && (col0!=col1 ||row0!=row1))
	//	cout << farm_plot->getCol() << ", " << farm_plot->getRow() << ": \t "
	//   	 << col0<<", "<<row0<< "  !=  " << col1 << ", "<<row1<<endl;

	// wanted_plot = farm_plot->findFreePlotOfType(type);
    // only enter if wanted_plot is not NULL
    // but it is free
    if (wanted_plot != NULL) {
        setRentedPlot(wanted_plot,rent,0);

        if (g->Rent_Variation)
            wanted_plot->setSecondOffer(rent);

		int cl = getRandomContractLength();
		// 1 + randlong() % (g->MAX_CONTRACT_LENGTH - 1);
		
        wanted_plot->setContractLength(cl);
        wanted_plot->setNewleyRented(false);
        wanted_plot = NULL;
        return true;
    } else {
        return false;
    }
}
bool
RegFarmInfo::getInitialOwnedPlot(int type) {
	if (g->INITIALISATION)
		wanted_plot = farm_plot->findMostPreferablePlotOfType(type, this).plot;
	else
		wanted_plot = farm_plot->findMostPreferablePlotOfType(type).plot;
//   wanted_plot = farm_plot->findFreePlotOfType(type);
    // only enter if wanted_plot is not NULL
    // but it is free
    if (wanted_plot != NULL) {
        setOwnedPlot(wanted_plot);
        wanted_plot = NULL;
        return true;
    } else {
        return false;
    }
}
bool
RegFarmInfo::allokateInitialLand() {
    bool enough=true;
    // as long as the total farm land is not reached, new plots are added
    //Wenn noch nicht genug Flaeche vorhanden ist kommt neue hinzu

    if (g->SOIL_TYPE_VARIATION) {

    vector<double> land_input_of_group;
    vector<double> initial_owned_land_of_group;
    vector<double> initial_rented_land_of_group;
    for(int i=0;i<2;i++) {
        land_input_of_group.push_back(0);
        initial_owned_land_of_group.push_back(0);
        initial_rented_land_of_group.push_back(0);
    }
    land_input_of_group[0]=land_input_of_type[0]+land_input_of_type[1]+land_input_of_type[2];
    land_input_of_group[1]=land_input_of_type[3]+land_input_of_type[4];
    initial_owned_land_of_group[0]=initial_owned_land_of_type[0]+initial_owned_land_of_type[1]+initial_owned_land_of_type[2];
    initial_owned_land_of_group[1]=initial_owned_land_of_type[3]+initial_owned_land_of_type[4];
    initial_rented_land_of_group[0]=initial_rented_land_of_type[0]+initial_rented_land_of_type[1]+initial_rented_land_of_type[2];
    initial_rented_land_of_group[1]=initial_rented_land_of_type[3]+initial_rented_land_of_type[4];
    for (int i=0;i<2;i++) {
        if (land_input_of_group[i]<initial_owned_land_of_group[i]+initial_rented_land_of_group[i]) {
            if (land_input_of_group[i]<initial_owned_land_of_group[i]) {
                int t;
                if(i==0)
                  t=randlong()%3;
                else
                  t=(randlong()%2)+3;
                getInitialOwnedPlot(t);
                enough=false;
            } else {
                int t;
                if(i==0)
                  t=randlong()%3;
                else
                  t=(randlong()%2)+3;
                getInitialRentedPlot(initial_rent_of_type[t]*g->PLOT_SIZE,t);
                enough=false;
            }
        }
    }
    return enough;
    } else {
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        if (land_input_of_type[i]<initial_owned_land_of_type[i]+initial_rented_land_of_type[i]) {
            if (land_input_of_type[i]<initial_owned_land_of_type[i]) {
                getInitialOwnedPlot(i);
                enough=false;
            } else {
                getInitialRentedPlot(initial_rent_of_type[i]*g->PLOT_SIZE,i);
                enough=false;
            }
        }
    }
    return enough;
    }
}

void
RegFarmInfo::setInitialLand(double l) {
    setInitialLand(&l);
    initial_land=l;
}
void
RegFarmInfo::setInitialOwnedLandOfType(double l,int type) {
    setInitialLand(&l);
    initial_owned_land_of_type[type]=l;
}
void
RegFarmInfo::setInitialRentedLandOfType(double l,int type) {
    setInitialLand(&l);
    initial_rented_land_of_type[type]=l;
}
void RegFarmInfo::setInitialLand(double* var)   // in ha
{
    // in case the number is odd the number is rounded to the next
    // lower plot size
    int n=(int)(*var/g->PLOT_SIZE);
    *var = (double)n*g->PLOT_SIZE;
}
/////////////
// CLOSE FARM
/////////////

void
RegFarmInfo::closeDown() {
    wanted_plot = NULL;
    // ---- FREE ALL PLOTS --------
    list<RegPlotInfo* >::iterator plot=PlotList.begin();
    list<RegPlotInfo*> owned;
    while (plot != PlotList.end()) {
          if(        ((*plot)->getState()== 2) || ((*plot)->getState()== 3)                   ) {
            owned.push_back(*plot);
        }
        plot=releasePlot(plot);
    }
    PlotList=owned;
    number_of_plots=0;
}

//////////////////
// RENTING PROCESS
//////////////////

void
RegFarmInfo::newRentingProcess(int period) {
    if (g->WEIGHTED_PLOT_SEARCH)
        farm_plot->initFreePlots(this);
    if (g->FAST_PLOT_SEARCH)
        farm_plot->resetPlotPointer();
    actual=false;
    first_time = true;
    if (period > 0) {
        // TF wirksam zu Beginn von neuer Periode
        // adjustVarCosts();
        labour->setLabourCapacity(labour->getFamilyLabour()
                                  +FarmInvestList->getLabourSubstitution() // lab effect of invests inkl fix labour
                                 );
        lp->updateCapacities();
    }
    list<RegPlotInfo* >::iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        (*plot_iter)->setNewleyRented(false);
    }
}

void
RegFarmInfo::calculateEstimationForBidding() {
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        land_capacity_estimation_of_type[i]=0;
        premium_estimation_of_type[i]=0;
    }
    list<RegPlotInfo* >::iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        land_capacity_estimation_of_type[(*plot_iter)->getSoilType()]+=1;
        premium_estimation_of_type[(*plot_iter)->getSoilType()]+=(*plot_iter)->getPaymentEntitlement();
    }
}

void RegFarmInfo::set_beta(double b) {
    rent_beta = b;
}

double RegFarmInfo::get_beta() const {
    return rent_beta;
}

void
RegFarmInfo::demandForLand(RegPlotInfo* p) {
    if (!actual || p->getPaymentEntitlement()!=cache_premium_of_type[p->getSoilType()]) {
	
    lp_result=doLpWithPriceExpectation();
	for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        double pe = p->getPaymentEntitlement();
        incDirectPayment(pe);
        increaseLandCapacityOfType(i,1);
		
        double lp_with_new= doLpWithPriceExpectation();
		
        decDirectPayment(pe);
        decreaseLandCapacityOfType(i,1);
        cache_sp_of_type[i]=lp_with_new-lp_result;
        cache_premium_of_type[i]=p->getPaymentEntitlement();
    }
        actual=true   ;

    }
	
    // If in the l  ast period there was a land allocation then the
    // computed value of lp_result for the
    // respective type is kept in this period
    double one=cache_sp_of_type[p->getSoilType()];
    double max_offer=one;//max(one,est);
    max_offer-=p->calculateDistanceCosts(farm_plot);

    double factor = 0;
    if (g->Rent_Variation) factor = rent_beta;
    else factor = g->RENT_ADJUST_COEFFICIENT;
    p->identifyPlotsSameStateAndFarm(farm_id);
    rent_offer = max_offer*factor;
    if (rent_offer < 0)
        rent_offer = 0;
}

//////////////////////
// OLD RENTING PROCESS
//////////////////////
void
RegFarmInfo::calculateShadowPriceForLandOfType(int type,double premium_entitlement) {
    incDirectPayment(premium_entitlement);
    increaseLandCapacityOfType(type,1);
	if (g->HAS_SOILSERVICE) increaseCarbonOfType(type,1);
    lp_result_with_new_plot_of_type[type] = doLpWithPriceExpectation();
    decDirectPayment(premium_entitlement);
    decreaseLandCapacityOfType(type,1);
    incDirectPayment(premium_entitlement*g->PLOTSN);
    increaseLandCapacityOfType(type,g->PLOTSN);
	if (g->HAS_SOILSERVICE) increaseCarbonOfType(type,g->PLOTSN-1);
    lp_result_with_plotsn_new_plots_of_type[type] = doLpWithPriceExpectation();
    decDirectPayment(premium_entitlement*g->PLOTSN);
    decreaseLandCapacityOfType(type,g->PLOTSN);
	if (g->HAS_SOILSERVICE) decreaseCarbonOfType(type,g->PLOTSN);
    delta_profit_of_type[type] = max(lp_result_with_new_plot_of_type[type]-lp_result,
                                     (lp_result_with_plotsn_new_plots_of_type[type]-lp_result)/(double)g->PLOTSN);
}
void
RegFarmInfo::demandForLandOfType(int type,int count) {
    // the first time 3 LPs have to be computed
    if (!actual) {
        actual=true;
        lp->updatePaymentEntitlement();
        lp_result=doLpWithPriceExpectation();

        for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
            wanted_plot_of_type[i]=farm_plot->findMostPreferablePlotOfType(i);

            if (wanted_plot_of_type[i].plot!=0) {
                calculateShadowPriceForLandOfType(i,wanted_plot_of_type[i].pe);
            } else {
                lp_result_with_new_plot_of_type[i] = 0;
                lp_result_with_plotsn_new_plots_of_type[i] = 0;
                delta_profit_of_type[i]=0;
            }
        }
    }
//DCX
    double x = delta_profit_of_type[type];
    if (((x>=0) && (x<EPS)) || ((x<0) && (x>-EPS))) {
   // if (delta_profit_of_type[type]==0)  {
        adjusted_rent_offer =unadjusted_rent_offer=        rent_offer=0;
        return;
    } else {
        if (wanted_plot_of_type[type].plot==NULL) {
            adjusted_rent_offer =  unadjusted_rent_offer=        rent_offer=0;
            return;
        } else {
            if (wanted_plot_of_type[type].plot->getState()!=0) {
                double pe_old=wanted_plot_of_type[type].pe;
                wanted_plot_of_type[type]=farm_plot->findMostPreferablePlotOfType(type);

                if (wanted_plot_of_type[type].plot==NULL) {
                    adjusted_rent_offer =  unadjusted_rent_offer=        rent_offer=0;
                    return;
                } else {
                    double pe_new=wanted_plot_of_type[type].pe;
                    if (pe_old!=pe_new) {
                        calculateShadowPriceForLandOfType(type,pe_new);
                    }
                }

            }
        }
    }
   
    double factor = 0;
    if (g->Rent_Variation) factor = rent_beta;
    else factor = g->RENT_ADJUST_COEFFICIENT;

    unadjusted_rent_offer=adjusted_rent_offer=rent_offer = factor* (delta_profit_of_type[type] - wanted_plot_of_type[type].costs());
    if (rent_offer < 0)
        rent_offer = 0;
    if (unadjusted_rent_offer < 0)
        unadjusted_rent_offer = 0;
    if (adjusted_rent_offer < 0)
        adjusted_rent_offer = 0;
}
double RegFarmInfo::getRentOffer(RegPlotInfo* plot) {
    int type=plot->getSoilType();
    RegPlotInformationInfo pi=farm_plot->getValue(plot,this);
    if (wanted_plot_of_type[type].plot==NULL) {
        lp->updatePaymentEntitlement();
        lp_result=doLpWithPriceExpectation();
        calculateShadowPriceForLandOfType(type,pi.pe);
    } else {
        if (pi.pe!=wanted_plot_of_type[type].pe) {
            lp->updatePaymentEntitlement();
            lp_result=doLpWithPriceExpectation();
            calculateShadowPriceForLandOfType(type,pi.pe);
        }
    }
    
    double factor = 0;
    if (g->Rent_Variation) factor = rent_beta;
    else factor = g->RENT_ADJUST_COEFFICIENT;

    //return g->RENT_ADJUST_COEFFICIENT* (delta_profit_of_type[type] - pi.costs());
    return factor * (delta_profit_of_type[type] - pi.costs());
}


/////////////////////
// INVESTMENT PROCESS
/////////////////////

void
RegFarmInfo::invest(int ordernumber, int quantity, bool test) {
    double acqcosts;
    while (quantity != 0) {
        acqcosts = (*invest_cat)[ordernumber].getAcquisitionCosts();
        // add invest object "objectordered" from invest catalog
        if (!test) { // real investment
            FarmInvestList->add((*invest_cat)[ordernumber]);
            number_new_investments_wo_labour
            = FarmInvestList->getNumberOfNewInvestmentsWithoutLabour();
        }
        if (test) { // for planning calculation for next period
            FarmInvestList->addTest((*invest_cat)[ordernumber]);
        }
        // adjust capital constraints
        if ((*invest_cat)[ordernumber].getAcquisitionCosts() > 0)  {
            //        liquidity -= acqcosts * g->SHARE_SELF_FINANCE;
            lt_borrowed_capital += acqcosts * (1 - g->SHARE_SELF_FINANCE);
            assets += acqcosts;
        }
        quantity--;
    }
    // compute effect on capacities
    labour->setLabourCapacity(labour->getFamilyLabour()
                              +FarmInvestList->getLabourSubstitution()
                             );// lab effect of invests inkl fix labour
//   lp->updateCapacities();  // var labour is added here
}

void
RegFarmInfo::adjustVarCosts() {
    double factor = 0;
    // a factor by which var. costs of a product produced with an
    // investment object are reduced are read in. This factor is higher for
    // larger objects and lower for smaller objects. If there is more than
    // one investment object of a group (a group is e.g. a pig sty, dairy stable, etc.
    // then the factors are added and the actual factor is the average of
    // the individual factor's number.
    for (int i=0;i<g->INVEST_GROUPS;i++) {
        // cost factor for machinery depends on number of neighbours and
        // total land
        if (i==0) {
            factor = getCostFactorMachinery(i);
        } else {
            factor = FarmInvestList->getCostFactor(i);
        }
        if (factor>0)
            FarmProductList->changeVarCosts(factor, i);
    }
    updateLpValues();
}

double
RegFarmInfo::doLpInvest() {
    FarmInvestList->setBackNewleyInvestedVector();
    double objective = lp->LpWithPriceExpectation(FarmProductList,
                       inum_vector,
                       static_cast<int>(labour->getFamilyLabour()));
    // determine the catalog number of investment and the number of investments
    int iobject, num; // catalog number of investment object, index of inum_vector
    // num i number of investments in object iobject
    int size = inum_vector.size();
    bool test =  false;
    bool invested = false;
    for (iobject = 0; iobject < size; iobject ++) {
        num = inum_vector[iobject];
        if (num != 0) {
            invest(iobject, num, test);
            invested = true;
        }
    }
    
    if (invested)
        adjustVarCosts();
    calculateLiquidity();
    calculateFinancingRule();
    lp->updateCapacities();
    FarmInvestList->getLSWithoutLabour1();

    return objective;
}

//////////
// RESULTS
//////////

double
RegFarmInfo::getUnitsProducedOfGroup(int t) const{
    if (farm_class == 5) {
        return getInitialOwnedLand();
    } else     {
        return FarmProductList->getUnitsProducedOfGroup(t);
    }
}
double
RegFarmInfo::getTotalLU(string cl) const {
    return FarmProductList->getTotalLU(cl);
}
double
RegFarmInfo::getTotalLU() const {
    return FarmProductList->getTotalLU();
}
void
RegFarmInfo::periodResults(int period) {
    double depr = 0;
    double oppcostslabourhour = 0;
    double ec_interest =  FarmProductList->getPriceOfType(g->ST_EC_INTERESTTYPE);

    // determine opp costs of labour = fix off farm labour
    oppcostslabourhour = (- FarmInvestList->getAcquisitionCostsOfNumber(g->FIXED_OFFFARM_LAB)) /
                         - FarmInvestList->getLabourSubstitutionOfNumber(g->FIXED_OFFFARM_LAB);
    // farm ages one year
    farm_age++;
    // relevance of sunk costs in depreciation
    sc_relevant = true;  // false: machinery and buildings are not written off
    //        costs of machinery and buildings are sunk
    // true:  depreciation is calculated here
    //        costs of machinery and buildings are not sunk
    invest_get_older = true;       // getolder = 0: investment object does NOT age

    //Caution: some Values passed by Reference!!!
    depreciation = depr = FarmInvestList->depreciateCapital(
                              &farm_repayment,
                              &farm_fix_labour_income,
                              &value_added,
                              &annuity,
                              sc_relevant,
                              invest_get_older,
                              ec_interest
                          );
    // getolder = 1: investment object ages by 1 each year
    lt_interest_costs = FarmInvestList->getOwnBcInterest();
    if(g->LP_MOD) {
	modulated_income_payment=getUnitsOfProduct(g->stdNameIndexs["TOTAL_PREM_MODULATED"]);
    display_modulation=0;
    if((int)(getUnitsOfProduct(g->stdNameIndexs["TRANCH_1"])+0.5)>0)
      display_modulation=1;
    if((int)(getUnitsOfProduct(g->stdNameIndexs["TRANCH_2"])+0.5)>0)
      display_modulation=2;
    if((int)(getUnitsOfProduct(g->stdNameIndexs["TRANCH_3"])+0.5)>0)
      display_modulation=3;
    if((int)(getUnitsOfProduct(g->stdNameIndexs["TRANCH_4"])+0.5)>0)
      display_modulation=4;
    if((int)(getUnitsOfProduct(g->stdNameIndexs["TRANCH_5"])+0.5)>0)
      display_modulation=5;
}
    // INCOME
    gm = 0;
    profit = 0;

    // INTEREST
    land_remuneration = 0;
    interest_costs = 0;
    // caution: at this point, the investment has aged already by one
    // year
    // "-" because interest is negative in RegMarket
    st_interest_costs = (- FarmProductList->getReturnOfType(g->ST_BOR_INTERESTTYPE));
    st_interest_received = FarmProductList->getReturnOfType(g->ST_EC_INTERESTTYPE);
    interest_costs = (st_interest_costs + lt_interest_costs);

    // LABOUR
    // fix hired
    farm_hired_labour_fix_pay= FarmInvestList->getInvestmentsOfCatalogNumber(g->FIXED_HIRED_LAB)
                               * FarmInvestList->getAcquisitionCostsOfNumber(g->FIXED_HIRED_LAB);
    labour->addFixOnfarmLabour(FarmInvestList->getInvestmentsOfCatalogNumber(g->FIXED_HIRED_LAB)
                               * FarmInvestList->getLabourSubstitutionOfNumber(g->FIXED_HIRED_LAB));
    // var hired
    farm_hired_labour_var_pay = (-FarmProductList->getReturnOfType(g->VARHIREDLABTYPE));
    labour->addVarOnfarmLabour(FarmProductList->getUnitsProducedOfType(g->VARHIREDLABTYPE));
    // fix off
    farm_fix_labour_income = FarmInvestList->getInvestmentsOfCatalogNumber(g->FIXED_OFFFARM_LAB)
                             * (-FarmInvestList->getAcquisitionCostsOfNumber(g->FIXED_OFFFARM_LAB));
    labour->addFixOfffarmLabour(FarmInvestList->getInvestmentsOfCatalogNumber(g->FIXED_OFFFARM_LAB)
                                *(-FarmInvestList->getLabourSubstitutionOfNumber(g->FIXED_OFFFARM_LAB)));
    // negative because of sign of lab Sub.
    // var off
    farm_var_labour_income = FarmProductList->getReturnOfType(g->VAROFFARMLABTYPE);
    labour->addVarOfffarmLabour(FarmProductList->getUnitsProducedOfType(g->VAROFFARMLABTYPE));
    // ad labour substitution
    //  labour->addLabourSubstitution(FarmInvestList->getLSWithoutLabour());

    // total income/expenditure

    // PROFIT
    bonus = 0;
    gm_products = FarmProductList->getGrossMarginOfType(g->PRODTYPE);
	if (g->YoungFarmer)
		gm_products += getYoungFarmerPay();

    total_maintenance =  FarmInvestList->getTotalMaintenance();

    overheads = g->OVERHEADS * gm_products;
    fix_costs = depr;

    // ordentliches Ergebnis
    // nach Buchfuehrungsdaten 9000.xls

    // income_payment goes into profit calculation as long as farms are
    // still producing

    // Verzinsung des Umlaufvermoegens wurde bei der DB-Berechnung nicht beruecksichtigt
    // d.h. es muss auch nicht zum Gewinn dazu gezaehlt werden

    profit = gm_products
             + st_interest_received // Verzinsung restliches Umlaufvermoegen
             - total_maintenance
             - fix_costs
             - overheads
             - farm_rent_exp
             - farm_tac
             - farm_distance_costs
             - lt_interest_costs
             + (- st_interest_costs
                - farm_hired_labour_var_pay)
             ;

    gm_agriculture = gm_products
                     - st_interest_costs
                     - farm_hired_labour_var_pay;

    /////////////////////////////////////////
    // Stability and Liquidity considerations
    /////////////////////////////////////////

    // total income to get equity capital change
    total_income = profit + farm_fix_labour_income + farm_var_labour_income;

    bool old_erwerbsform = full_time;
    double labourunits = labour->getLabourInputHours()/g->MAX_H_LU;

    if (labourunits >= 1.5)
        full_time = true;
    if ((labourunits > 0.75) && (labourunits < 1.5)) {
        if ((profit >= 0.5 * total_income)) {
            full_time = true;
        } else {
            full_time = false;
        }
    }
    if (labourunits <= 0.75) {
        full_time = false;
    }
    // change Erwerbsform
    if (full_time != old_erwerbsform) {
        if ((old_erwerbsform==true)&&(full_time==false))
            // change from full-time to part-time
            switch_erwerbsform = 1;
        if ((old_erwerbsform==false)&&(full_time==true))
            // change from part_time to full-time
            switch_erwerbsform = 0;
    } else {
        //
        switch_erwerbsform = -1;
    }
	
    // WTIHDRAWAL OF CAPITAL
    withdrawal = withdrawCapital(total_income);
    // assumption: all of offfarmlabour income which is not spent is
    // added to equity capital
    ecchange =  total_income - withdrawal;

    // equity_capital increase
    equity_capital += ecchange;
    assets_at_production_wo_land = assets - land_assets;
    assets -= depr;

    st_borrowed_capital= FarmProductList->getUnitsProducedOfNumber(g->ST_BOR_INTERESTTYPE);
    capital_input = equity_capital + lt_borrowed_capital;

    ////////////////
    // Farm analysis
    ////////////////

    // farm income (Betriebseinkommen)
    // insurance is not included because it is around 1000 DM
    // the FARM INCOME is the income which pays for
    // all factors used on the farm
    // Das BETRIEBSEINKOMMEN ist der Betrag, der zur
    // Entlohung aller im Unternehmen eingesetzten Faktoren
    // zur Verfuegung steht HLBS 14 (1996), S. 53
    // value added
    farm_income = value_added = profit
                                + farm_rent_exp
                                + farm_tac
                                + st_interest_costs
                                + lt_interest_costs
                                + farm_hired_labour_var_pay
                                + farm_hired_labour_fix_pay
                   - st_interest_received   //E-Mail from Alfons on 29.10.2020
                                ;

    // labour yield 
    // = profit - interest on average bound equity capital

    double famOfffarmLabourHours = FarmInvestList->getInvestmentsOfCatalogNumber(g->FIXED_OFFFARM_LAB) * g->MAX_H_LU
                                   +  FarmProductList->getUnitsProducedOfType(g->VAROFFARMLABTYPE);

    double unpayedlabourhours = labour->getFamilyLabour() - famOfffarmLabourHours;
    double unpayedlabour = 0;

    if (unpayedlabourhours > 0)
        unpayedlabour = (double) unpayedlabourhours * oppcostslabourhour;

    double equityinterest = 0;
    double ecwithoutland = 0;
    // Remuneration  (land, labour)
    opp_own_land=0;
//    if (rented !=0) {
        opp_own_land=0;
        for (int j=0;j<g->NO_OF_SOIL_TYPES;j++) {
            opp_own_land += initial_owned_land_of_type[j]*region->getAvRentOfType(j);
        }
//    }
    ecwithoutland = equity_capital - land_assets;
    // average bound equity capital without land (HLBS, S.53)
    equityinterest = ((ecwithoutland - ecchange + ecwithoutland)/2 )*ec_interest
                     + opp_own_land  ;
    // land remuneration
    land_remuneration = profit - unpayedlabour - equityinterest ;
    // labour remuneration
    farm_factor_remuneration_var = farm_var_labour_income;
    farm_factor_remuneration_fix = farm_fix_labour_income;

    // NETTORENTABILITAET  HLBS, 14 , S.53,54
    // net profitability
    economic_profit = 0;
    if ((unpayedlabour + equityinterest) !=0)
        economic_profit = profit/(unpayedlabour + equityinterest + opp_own_land);

    // repay long term borrowed capital
    lt_borrowed_capital_old = lt_borrowed_capital -= farm_repayment;
    if (lt_borrowed_capital < 0)
        lt_borrowed_capital = 0;

    // calculating farm_class
    old_farm_class = farm_class;
    farm_class=FarmProductList->calculateFarmClass();
    if (farm_class!=old_farm_class)
        farm_class_change = true;
    else {
        farm_class_change = false;
    }
    calculateLiquidity();
    calculateFinancingRule();
    if (g->CALCULATE_CONTIGUOUS_PLOTS) countContiguousPlots();
}
void
RegFarmInfo::periodResultsForRemovedFarms() {
    double depr = 0;             // depreciation
    double oppcostslabourhour = 0;
    double ec_interest =  FarmProductList->getPriceOfType(g->ST_EC_INTERESTTYPE);

    //new farm type for quitting farms
    farm_class = 5;

    labour->setLabourInputHours(0);

    // determine opp costs of labour = fix off farm labour
    oppcostslabourhour = (- FarmInvestList->getAcquisitionCostsOfNumber(g->FIXED_OFFFARM_LAB)) /
                         - FarmInvestList->getLabourSubstitutionOfNumber(g->FIXED_OFFFARM_LAB);
    double quotarent = - milk_quota * (*product_cat)[g->stdNameIndexs["GETQUOTA"]].getPriceExpectation();
    // farm ages one year
    farm_age++;
    interest_costs = 0;

    sc_relevant = true;  // false: no relevance for decision
    //        case with sunk costs
    // true: relevance for decision
    //        case without sunk costs
    invest_get_older = true;       // getolder = 0: investment object does NOT age
    // getolder = 1: investment object ages by 1 each year

    // Interest: The amortisation for the investments is considered in RegInvestList::depreciateCapital.
    // Therefore the interest must be calculated beforehand. 

    //Caution: some Values passed by Reference
    depreciation = depr = FarmInvestList->depreciateCapital(
                              &farm_repayment,
                              &farm_fix_labour_income,
                              &value_added,
                              &annuity,
                              sc_relevant,
                              invest_get_older,
                              ec_interest);

    lt_interest_costs = FarmInvestList->getOwnBcInterest();
    lt_interest_costs_supported = FarmInvestList->getPoliticalBcInterest();

    // INCOME
    profit = 0;

    // INTEREST
    interest_costs =lt_interest_costs;//+lt_interest_costs_supported;

    // LABOUR
    if (closed==2){
        farm_fix_labour_income = (double)labour->getFamilyLabour()*oppcostslabourhour*1.25;
        }
        else {
        farm_fix_labour_income = (double)labour->getFamilyLabour()*oppcostslabourhour;}

    // PROFIT
    gm_products =0;
    fix_costs = 0;

    list<RegPlotInfo* >::iterator plot_iter;
    double rent=0;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        rent += (*plot_iter)->getRentPaid();
    }
    // good results according to accounting data 9000.xls
    profit = - lt_interest_costs
             + quotarent
             + rent
             + st_interest_received
             - depr;

    if (g->FULLY_DECOUPLING) {
        profit+=modulated_income_payment;
    }
    st_interest_received =liquidity*ec_interest;

    // total income to get equity capital change
    total_income = profit
                   + farm_fix_labour_income
                   + farm_var_labour_income
                   ;

    // WTIHDRAWAL OF CAPITAL
    withdrawal = withdrawCapital(total_income);
    // assumption: all of offfarmlabour income which is not spent is
    // added to equity capital
    // equity_capital increase
    ecchange = total_income - withdrawal;
    equity_capital += ecchange;
    assets -= depr;

    calculateLiquidity();

    //  liquidity+=ecchange;

    lt_borrowed_capital_old = lt_borrowed_capital -= farm_repayment;
    if (lt_borrowed_capital < 0)
        lt_borrowed_capital = 0;
    FarmInvestList->removeInvestment();
}

void RegFarmInfo::saveYoungFarmerPay() {
	youngfarmerPay = lp->getValOfIndex(lp->getColIndex("PAY_YOUNG_FARMER"));
}

double RegFarmInfo::getYoungFarmerPay() const {
	return youngfarmerPay;
}

void RegFarmInfo::updateYoungFarmerLand() {
	if (land_input < g->YF_minHa) {
		//YoungFarmer_years = 0;
		youngfarmerMinSize = false;
	}
	else youngfarmerMinSize = true;
}

void RegFarmInfo::updateYoungFarmer() {
	if (YoungFarmer_years > 0)
		--YoungFarmer_years;
	else {
		if (!youngfarmerPaid && farm_age < g->YF_startpayMaxAge && youngfarmerMinSize) {
			YoungFarmer_years = g->YF_payYears;
			youngfarmerPaid = true;
		}
	}
}

void RegFarmInfo::changeGeneration() {
	GenChange_demograph = 1;
	farm_age = (int)(getRandomNormalRange("DEMOGNEWAGE", g->GC_newage_normal_distr,
		g->GC_newAge_min, g->GC_newAge_max) + 0.5);
	g->farmAgeDists[0][farm_age]++;
}

void
RegFarmInfo::futureOfFarm( int period) {
    double oppcostslabourhour = 0;
    bool agedependent = g->AGE_DEPENDENT;
    bool no_successor_by_random = g->NO_SUCCESSOR_BY_RANDOM;
    double oppcosts = 0;
    double ex_total_income = 0;
    double quotarent = 0;
    
	// hoc = high oppportunity costs
	int hoc = farm_age % g->GENERATION_CHANGE;
	
	bool incManagerCoeff = false; 

	if (g->ManagerDemographics || g->YoungFarmer) {
		int esc = getEconomicSizeClass();
		g->GENERATION_CHANGE = g->GC_age;

		if (farm_age >= g->GC_age && GenChange_demograph == 0 ) {
			if (esc >= g->ESC_exclusion) {
				if (g->ECON_SIZE_CLASS_uni_distr(g->minstd0_ESC) < g->ESC_exclusion_prob) {
					changeGeneration();
				}
			}
			else if (farm_age == g->GC_age) {
				if (legal_type == 2) {
					if (g->GC_CF_uni_distr(g->minstd0_cf) < g->CF_GC_prob) {
						changeGeneration();
					}
				}
				else {
					if (g->GC_FF_uni_distr(g->minstd0_ff) < g->FF_GC_prob) {
						changeGeneration();
					}
				}
			}
		}

		//if (g->YoungFarmer)
		//	updateYoungFarmer();

		if (farm_age >= g->GC_age ) {
			if (farm_age >= g->GC_age_max) {
				closed = 10;
				closeDown();
			}
			else {
				incManagerCoeff = true;
			}
		}
	
		int age; // = farm_age - g->GENERATION_CHANGE;
		if (farm_age >= g->GENERATION_CHANGE) {
			age = farm_age - g->GENERATION_CHANGE;
		}
		else {
			age = farm_age;
		}

		// determine opp costs of labour
		oppcostslabourhour = (-FarmInvestList->getAcquisitionCostsOfNumber(g->FIXED_OFFFARM_LAB)) /
			-FarmInvestList->getLabourSubstitutionOfNumber(g->FIXED_OFFFARM_LAB);

		// MAKE OPPORTUNITY COSTS DEPENDENT ON FARM AGE
		if (agedependent) {
			if (age <= 45) 
				oppcostslabourhour = oppcostslabourhour * 1;
			if (age>45 && age <= 50) 
				oppcostslabourhour = oppcostslabourhour * 0.75;
			if (age >50 && age <=60) 
				oppcostslabourhour = oppcostslabourhour * 0.5;
			if (age>60 && age <= g->GENERATION_CHANGE) 
				oppcostslabourhour = 0;
			if (age >g->GENERATION_CHANGE) 
				oppcostslabourhour = oppcostslabourhour * 0.5;
		}
	}
	else {
		int age; // = farm_age - g->GENERATION_CHANGE;
		if (farm_age >= g->GENERATION_CHANGE) {
			age = farm_age - g->GENERATION_CHANGE;
		}
		else {
			age = farm_age;
		}

		// determine opp costs of labour
		oppcostslabourhour = (-FarmInvestList->getAcquisitionCostsOfNumber(g->FIXED_OFFFARM_LAB)) /
			-FarmInvestList->getLabourSubstitutionOfNumber(g->FIXED_OFFFARM_LAB);
		// MAKE OPPORTUNITY COSTS DEPENDENT ON FARM AGE
		// except for
		//
		if (agedependent) {
			/*
			// non-linear
			oppcostsclabourhourgenerationchange = oppcostslabourhour;
			oppcostslabourhour = oppcostslabourhour
			  - 0.000019 * pow((ageoppcosts-1), 5)
			  + 0.00091 * pow((ageoppcosts-1), 4)
			  - 0.016 * pow((ageoppcosts-1), 3)
			  + 0.12 * pow((ageoppcosts-1), 2)
			  - 0.34 * (ageoppcosts-1);
			// linear
			// oppcostsclabourhourgenerationchange = oppcostslabourhour;
			// oppcostslabourhour = oppcostslabourhour - (oppcostslabourhour/25) * farm_age;
			*/
			
			// step-wise
			if ((age >= 0) && (age <= 10))
				oppcostslabourhour = oppcostslabourhour * 1;
			if ((age > 10) && (age <= 20))
				oppcostslabourhour = oppcostslabourhour * 0.5;
			if ((age > 20) && (age <= g->GENERATION_CHANGE))
				oppcostslabourhour = 0;
		}
	}
    // opportunity costs for milk quota
    quotarent = - milk_quota * (*product_cat)[g->stdNameIndexs["GETQUOTA"]].getPriceExpectation();

	// change of generation every x years
    // hoc = high oppportunity costs
    //int hoc = farm_age%g->GENERATION_CHANGE;
    
    if ((closed!=10)&& (legal_type==1) && (no_successor_by_random) && (hoc==0)) {
		/*int border = 100;
		int randomnumber = randlong() % border;
		if (randomnumber <= 25)
		*/
		double r;
		string name = "CLOSEFARM";
		r = g->getRandomReal(name, g->uni_real_distrib_closeFarm);
		
		//cout << r << endl;
		if (r<=0.25) {
                closed=4;
                closeDown();
        }
    }

    if ((financing_rule <= 0)  && (closed<4)) {
        closed=3;
        closeDown();
    } else { // farm stays alive
        // depreciate capital
        // (..., ..., 0, false ) no ageing anymore, happened at the beginning of thi
        // method
        // sc_relevant = false, ie. the costs for all buildings and machinery
        // are assumed to be sunk
        //  REASONING:
        //    expectedyield is calculated to decide on whether to continue
        //    or to stop farming
        //    depriation are fixed costs -> if they are considered sunk,
        //    they are not included into the decision, i.e. they are not
        //    calculated
        //
        //    This is different if depreciation is considered relevant
        //    for decision
        //
        // includes the value for the farmstead
        // unlike in periodResults() the gross margin can be equated from the
        // objective function value, since it is a planning calculation

        // if fixed labour is offered offfarm in this period, then this can be
        // expected to be the case in the next period, too.

        // update Liquidity and Financial Rule, they have changed because
        // assets were removed from the investList

        // before the planning calculation variable costs are adjusted to reflect
        // the technical change embodied in new investment objects that were
        // invested in in this period

        /// SET ONE OFF PAYMENT

        // only take the return value when it is greater than testyield
        double ex_lp_result = anticipateNewPeriod();

        // ex_total_income includes fix off farm labour and fix hired labour!
        // assumes that same plots, overheads, total maintenance, and distance costs
        // are the same than in this period
        // depreciation and long term interest in anticipate period
        vector<double> quotient;
        vector<double> quotient_new;
        for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
          if(region->getAvRentOfType(i)==0)
            quotient.push_back(1);
          else
            quotient.push_back(region->getExpAvRentOfType(i)/region->getAvRentOfType(i));
          if(period==0){
                quotient_new.push_back(1);
          }
          else{
                if(region->getAvNewRentOfType(i)==0)
                  quotient_new.push_back(1);
                else
                  quotient_new.push_back(region->getExpAvNewRentOfType(i)/region->getAvNewRentOfType(i));
          }
        }
        double adjusted_farm_rent_exp=0;
        double adjusted_opp_own_land=0;
        for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                if(countRentedPlotsOfType(i)>0) {
                double farm_average=getFarmRentExpOfType(i)/(double)(countRentedPlotsOfType(i)*g->PLOT_SIZE);
                double region_average=region->getAvRentOfType(i);
                double average=(farm_average+region_average)/2.;
           adjusted_farm_rent_exp+=countRentedPlotsOfType(i)*g->PLOT_SIZE* average*                      quotient[i];
           }
           adjusted_opp_own_land+=initial_owned_land_of_type[i]*region->getAvRentOfType(i)* quotient[i];
        }
        
        ex_total_income = ex_lp_result
                          - adjusted_farm_rent_exp
                          - farm_tac
                          - farm_distance_costs
                          ;
        oppcosts = (double)labour->getFamilyLabour() * oppcostslabourhour+ liquidity * g->INTEREST_RATE+ adjusted_opp_own_land+ quotarent;
        if (g->FULLY_DECOUPLING) {
                oppcosts+=modulated_income_payment;
        }
              
        // direct transfer payment
        ex_total_income += (g->BONUS);


        // high opportunity costs when generation change
        if (hoc == 0) {
            oppcosts += (double)labour->getFamilyLabour() * oppcostslabourhour * .25;
        }

        if (oppcosts > (ex_total_income)) {
            // close down the farms, set closed == true
            if ((hoc > 0) && (closed<4)) {
                closed=1;
            } else {
                if (closed<4)
                closed=2;
            }
            closeDown();
            // increase number of closed farms in sector
            if (hoc > 0) {

                sunk_costs_labor = ( (double)labour->getFamilyLabour()*(double) oppcostslabourhour * .25)
                                   /
                                   capitalReturnFactor(g->INTEREST_RATE, g->GENERATION_CHANGE - hoc
                                                      );
            } else {
                sunk_costs_labor=0;
            }
        }
    }
	if (!closed && incManagerCoeff) {
		management_coefficient *= (1 + g->FF_prod_decrease);
		if (management_coefficient > g->UPPER_BORDER)
			management_coefficient = g->UPPER_BORDER;
	}

	if (g->YoungFarmer && closed > 0) {
		YoungFarmer_years = 0;
		youngfarmerPay = 0;
	}

    calculateLiquidity();
    calculateFinancingRule();
    //  resetFarmVariables();
}
double
RegFarmInfo::anticipateNewPeriod() {
      updateLpValues();
    /** This method simulates the production and investment activities of the
        next period without renting activities. The expected profit taking
        account of lower costs associated with new investments is determined.
        If the expected profit shows to be lower th
    */

    FarmProductList->saveProductList();
    double ec_interest = FarmProductList->getPriceOfType(g->ST_EC_INTERESTTYPE);
    bool invested = false;
    // set effect of sunk costs
    sc_relevant = false;
    invest_get_older = false;
    // depreciation
    double depr = 0;
    double lt_interest = 0;
    // save values to be restored after planning calculation
    double lt_borrowed_capital_old = lt_borrowed_capital;
    double assets_old = assets;

    double objective_new = 0;
    double objective = doLpWithPriceExpectation();

    // temporarily invest in new objects
    for (unsigned int iobject = 0; iobject < inum_vector.size(); iobject ++) {
        int num = inum_vector[iobject];
        bool test = true;
        if (num != 0) {
            invested = true;
            invest(iobject, num, test); // does not include updateCapacities()
            // this is done after liquidity and financing rule
        }
    }
    // produce if no investment
    objective = doProductionLp();
    depr = FarmInvestList->depreciateCapital(&farm_repayment,
            &farm_fix_labour_income,
            &value_added,
            &annuity,
            sc_relevant,
            invest_get_older,
            ec_interest);
    if (sc_relevant)
        lt_interest = FarmInvestList->getOwnBcInterest();
    objective = objective
                - total_maintenance
                - overheads
                - depr
                - lt_interest;

    // production with new investment
    if (invested) {
        // save var. costs to be restored after planning calculation
        doLpWithPriceExpectation();
        FarmProductList->saveProductList();
        adjustVarCosts();  // includes updateLpValues()
        calculateLiquidity();
        calculateFinancingRule();
        // update all capacities here!!
        lp->updateCapacities();

        // production with new investment
        objective_new = doProductionLp();
        //      objective_new = doLpWithPriceExpectation();
        depr = FarmInvestList->depreciateCapital(&farm_repayment,
                &farm_fix_labour_income,
                &value_added,
                &annuity,
                sc_relevant,
                invest_get_older,
                ec_interest);
        if (sc_relevant)
            lt_interest = FarmInvestList->getOwnBcInterest();

        double ex_farm_fix_labour_income = FarmInvestList->getInvestmentsOfCatalogNumber(g->FIXED_OFFFARM_LAB)
                                           * (-FarmInvestList->getAcquisitionCostsOfNumber(g->FIXED_OFFFARM_LAB));
        double ex_farm_hired_labour_fix_pay= FarmInvestList->getInvestmentsOfCatalogNumber(g->FIXED_HIRED_LAB)
                                             * FarmInvestList->getAcquisitionCostsOfNumber(g->FIXED_HIRED_LAB);
        double ex_gm_products = FarmProductList->getGrossMarginOfType(g->PRODTYPE);
        double ex_total_maintenance = FarmInvestList->getTotalMaintenance();
        double ex_overheads = g->OVERHEADS * ex_gm_products;

        // adjust for labour investments
        objective_new = objective_new
                        + ex_farm_fix_labour_income
                        - ex_farm_hired_labour_fix_pay
                        - lt_interest
                        - depr
                        - ex_total_maintenance
                        - ex_overheads;

        // RESET

        // remove new investments with age = -1 from list
        FarmInvestList->removeInvestment();
        lt_borrowed_capital = lt_borrowed_capital_old;
        assets = assets_old;
        // compute effect on capacities
        labour->setLabourCapacity(labour->getFamilyLabour()
                                  +FarmInvestList->getLabourSubstitution()
                                 );// lab effect of invests inkl fix labour
        // reset costs to old costs
        // old var_costs are restored
        FarmProductList->restoreProductList();
        calculateLiquidity();
        calculateFinancingRule();
        // update Lp to equal old LP
        updateLpValues();
        depr = FarmInvestList->depreciateCapital(&farm_repayment,
                &farm_fix_labour_income,
                &value_added,
                &annuity,
                sc_relevant,
                invest_get_older,
                ec_interest);

        // test has to have same value as objective, then values were
        // correctly restored
        //      double test = doProductionLp();
    }
    FarmProductList->restoreProductList();
    if (objective_new < objective)
        return objective;
    else
        return objective_new;
}
// return false if deltayield>0 or there is no plot of this soil type 
// sonst true ;
bool
RegFarmInfo::disinvestPlotOfType(int type) {
    double derent = 0;      // rent saved by renting less plots
    double dedistance = 0;  // distance costs saved by renting less plots
    double deltayield = 0;
    double r, d;//, p;
    bool deleteplot = false;
    list<RegPlotInfo* >::iterator plot_iter;
    list<RegPlotInfo* >::iterator delplot;   // create new plot object

    // RELEASE PLOTS
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        r = (*plot_iter)->getRentPaid();
        d = (*plot_iter)->getDistanceCosts();
        // bool test;
        /// dedirectp hier beibehalten, da der Betriebe diese ja verlieren wuerde
        /// und sie auch darueber entscheiden, ob die Flaeche gehalten wird oder nicht!!!
        if ((derent + dedistance < r + d) && ((*plot_iter)->getState()==1 ) && type==(*plot_iter)->getSoilType()) {
            // Plot ist gepachte und entspricht dem von flip gegebenen typ
            delplot = plot_iter;
            deleteplot = true;
            derent = r;
            dedistance = d;
        }
    }
    if (deleteplot) {
        double pe = 0;
        pe = (*delplot)->getPaymentEntitlement();

        deltayield = doLpWithPriceExpectation();
        decreaseLandCapacityOfType(type,1);
        decDirectPayment(pe);
        double obj = doLpWithPriceExpectation();
        deltayield -= obj;
        increaseLandCapacityOfType(type,1);
        incDirectPayment(pe);
        deltayield = deltayield - (derent + dedistance) /*+ dedirectp*/;
        if (deltayield < 0) {
            // delete plot from PlotList
            releasePlot(delplot);
            return true;
        } else
            return false;
    } else
        return false;
}

double& RegFarmInfo::getRefReinvestLUcap() {
	return reinvestLUcap;
}

void RegFarmInfo::setReinvestLUcap() {
	reinvestLUcap = 0;
	double lu;
	double cap;
	if (restrictedInvests.size() > 0) {
		for (auto x : restrictedInvests) {
			lu = 0;
			cap = 0;
			for (auto p : (*invest_cat)) {
				string name = p.getName();
				if ((x.first).compare(name) == 0) {
					cap = p.getCapacity();
					int prod = p.getAffectsProductGroup();
					for (auto t : (*product_cat)) {
						if (t.getProductGroup() == prod) {
							lu = t.getLU();
							break;
						}
					}
					reinvestLUcap += x.second * lu * cap;
					break;
				}
			}
		}
	}
}

void RegFarmInfo::setRestrictInv() {
	if (restrictedInvests.size() > 0)
		restrict_invest = true;
	else
		restrict_invest = false;
}

bool RegFarmInfo::allowInvest() {
	return allow_invest;
}

void RegFarmInfo::setAllowInvest(bool b) {
	allow_invest = b;
	if (b)
		reinvestLUcap = 1E30;
	else
		reinvestLUcap = 0;
}

bool RegFarmInfo::restrictInvestments() {
	return restrict_invest;
}

map<string, int> RegFarmInfo::getRestrictedInvests() {
	return restrictedInvests;
}

static map<string, int> checkRestrictInvs(map<string, int> invs, set<string> rinvs) {
	map<string, int> res;
	for (auto x : invs) {
		for (auto r : rinvs) {
			if ((x.first).rfind(r, 0) == 0) {
				res.insert(x);
				break;
			}
		}
	}
	return res;
}

void
RegFarmInfo::disInvest() {
	if (g->RestrictInvestments) {
		FarmInvestList->resetRemovedInvs();
		FarmInvestList->removeInvestment();
		map<string, int> removed_invs = FarmInvestList->getRemovedInvests();
		restrictedInvests = checkRestrictInvs(removed_invs, g->Livestock_Invs);
		/*if (restrictedInvests.size() > 0)
			restrict_invest = true;
		else
			restrict_invest = false;
		//*/
	}else
		FarmInvestList->removeInvestment();

    labour->setLabourCapacity(labour->getFamilyLabour()
                              +FarmInvestList->getLabourSubstitution() // lab effect of invests inkl fix labour
                              );
    calculateLiquidity();
    calculateFinancingRule();
    lp->updateCapacities();
    FarmInvestList->getLSWithoutLabour1();
    list<RegPlotInfo* >::iterator plot_iter;
    if (g->OLD_LAND_RELEASING_PROCESS) {
        bool stop;
        do {
            stop=true;
            for (int t=0;t<g->NO_OF_SOIL_TYPES;t++) {
                bool test=disinvestPlotOfType(t);
                if (test==true) stop=false;
            }
        } while (!stop);
    } else {
        // RELEASE PLOTS
        int delcount=0;
        plot_iter = PlotList.begin();
        while (plot_iter != PlotList.end()) {
            if ((*plot_iter)->getState()==1) {
                (*plot_iter)->decreaseContractLength();
                int age=(*plot_iter)->getContractLength();
                if (age==0) {
                    plot_iter=releasePlot(plot_iter);
                    delcount++;
                } else {
                    plot_iter++;
                }
            } else {
                plot_iter++;
            }
        }
    }
}

//////////////////
// UPDATE ROUTINES
//////////////////
void
RegFarmInfo::resetFarmVariables() {
    labour->setBackLabour();
}

double
RegFarmInfo::withdrawCapital(double inc) {
    double minwithdrawal, rest;
    minwithdrawal = (double)(labour->getFamilyLabour()/(double)g->MAX_H_LU) * (double)g->WD_FACTOR;
    if(g->MIN_WITHDRAWAL) {
      return minwithdrawal;
    } else {
      rest = inc - minwithdrawal;
      return( ( (rest > 0) ? rest * g->WITHDRAWFACTOR : 0 ) + minwithdrawal );
    }
}

list<RegPlotInfo* >::iterator RegFarmInfo::releasePlot(list<RegPlotInfo* >::iterator p) {
    int type=(*p)->getSoilType();
    int state=(*p)->getState();
    farm_rent_exp -= (*p)->getRentPaid();
    farm_tac -= (*p)->getTacs();
    farm_distance_costs -= (*p)->getDistanceCosts();
    decDirectPayment((*p)->getPaymentEntitlement());
    land_input -= g->PLOT_SIZE;
    number_of_plots--;
    land_input_of_type[type]-= g->PLOT_SIZE;
    region->releasePlot((*p));
    list<RegPlotInfo* >::iterator ret;
    if (state==1) {
        rented_land_of_type[type]-=g->PLOT_SIZE;
        // RELEASE PLOT FROM PLOT LIST IN CASE ITS NOT OWNED
    }
    lp->updateLand();
    return PlotList.erase(p);
    ;
}
void
RegFarmInfo::occupyPlot(RegPlotInfo* p) {
    int type=p->getSoilType();
    land_input +=g->PLOT_SIZE;
    number_of_plots +=1;
    farm_distance_costs += p->getDistanceCosts();
    increaseLandCapacityOfType(type,1);
    incDirectPayment(p->getPaymentEntitlement());
    PlotList.push_back(p);
}
void
RegFarmInfo::setRentedPlot(RegPlotInfo* p, double rent, double tacs) {
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        cache_actual_of_type[i]=false;
    }
    actual=false;
    if (p->getState()==0) {

        region->setRentedPlot(p,this);
        occupyPlot(p);
        farm_rent_exp+=rent;
        farm_tac+=tacs;
        int type=p->getSoilType();
		p->setRentPaid(rent);
        p->setTacs(tacs);
        p->setNewleyRented(true);
        rented_land_of_type[type]+=g->PLOT_SIZE;
    }
}
void
RegFarmInfo::setSecondPrice(vector<double>& secondprice_region) {
    list<RegPlotInfo* >::iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
            if((*plot_iter)->getNewleyRented()) {
              int type=(*plot_iter)->getSoilType();
              double rent=(*plot_iter)->getRentPaid();
              (*plot_iter)->setRentPaid(secondprice_region[type]);
              farm_rent_exp-=rent;
              farm_rent_exp+=secondprice_region[type];
            }
    }
}

void
RegFarmInfo::setOwnedPlot(RegPlotInfo* p) {
    if (p->getState()==0) {
        region->setOwnedPlot(p,this);
        occupyPlot(p);
    }
}

//update yield --soil service
void
RegFarmInfo::updateYield() {
    lp->updateYield();
}


// update values of objective function
// and determine the liquidity effect of each possible investment
// to be passed on to the LP object
void
RegFarmInfo::updateLpValues() {
    if (lp->updateLpValues())g->LP_CHANGED=true;
}

void
RegFarmInfo::adjustPaidRent(vector<double> av_offer_of_type,
                            vector<double> ratio_of_type) {

    list<RegPlotInfo* >::iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        if ((*plot_iter)->getState()==1) {
            double av_offer=av_offer_of_type[(*plot_iter)->getSoilType()];
            double ratio=ratio_of_type[(*plot_iter)->getSoilType()];
            if ( av_offer>0) {
                double rent_offer = (*plot_iter)->getRentPaid();
                // newley rented is set for each plot which
                // is allocated new to a farm
                double old_rent=(*plot_iter)->getRentPaid();
                if ((*plot_iter)->getNewleyRented()) {
                    double new_rent = sqrt(rent_offer*av_offer);
                    (*plot_iter)->setRentPaid(new_rent);
                } else { // if plot was rented by farm before
                    double new_rent = pow(av_offer,ratio)
                                      * pow(rent_offer,1-ratio);
                    (*plot_iter)->setRentPaid(new_rent);
                }
                double diff=(*plot_iter)->getRentPaid()-old_rent;
                farm_rent_exp+=diff;
            }
        }
    }

}
double
RegFarmInfo::getLabourHa() const {
    return  (double) labour->getLabourInputHours()/(double)(g->MAX_H_LU*land_input);

}
void
RegFarmInfo::setDirectPayment(double payment) {
    income_payment_farm = payment;
	if(!g->LP_MOD)
    modulateIncomePayment();
    lp->updatePaymentEntitlement();
}
double
RegFarmInfo::getDirectPayment() const {
    return income_payment_farm;
}
void
RegFarmInfo::incDirectPayment(double payment) {
    if (payment>0) {
        income_payment_farm += payment;
	if(!g->LP_MOD)
        modulateIncomePayment();
        lp->updatePaymentEntitlement();
    }
}
void
RegFarmInfo::decDirectPayment(double payment) {
    if (payment>0) {
        income_payment_farm -= payment;
	if(!g->LP_MOD)
        modulateIncomePayment();
        lp->updatePaymentEntitlement();
    }
}
void
RegFarmInfo::increaseLandCapacityOfType(int type,int no_of_plots) {
    land_input_of_type[type]+=no_of_plots*g->PLOT_SIZE;
    lp->updateLand();
}
void
RegFarmInfo::decreaseLandCapacityOfType(int type,int no_of_plots) {
    land_input_of_type[type]-=no_of_plots*g->PLOT_SIZE;
    lp->updateLand();
}
void
RegFarmInfo::setNoOfPlotsOfType(int type,int no_of_plots) {
    land_input_of_type[type]=no_of_plots*g->PLOT_SIZE;
    lp->updateLand();
}
int
RegFarmInfo::getNoOfPlotsOfType(int type)const  {
    return static_cast<int>(land_input_of_type[type]/g->PLOT_SIZE);
}
void
RegFarmInfo::addInvestments(int cn, int q, int cap) {
    FarmInvestList->add(cn,q,cap);
}
void
RegFarmInfo::calculateFinancingRule() {
    // as long as farms only dispose of their owned land if their quit production
    // land_assets do not change
    financing_rule = equity_capital - 0.7*land_assets - 0.3*FarmInvestList->getTotalResEcShareWithoutLabour();
}
int
RegFarmInfo::countRentedPlots() {
    int count = 0;
    list<RegPlotInfo* >::iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        if ((*plot_iter)->getState()==1) { // state 1 = rented plot
            count++;
        }
    }
    return count;
}
int
RegFarmInfo::countNewRentedPlots() {
    int count = 0;
    list<RegPlotInfo* >::iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        if ((*plot_iter)->getState()==1 && (*plot_iter)->getNewleyRented()==true) { // state 1 = rented plot
            count++;
        }
    }
    return count;
}
double
RegFarmInfo::getNewRentedLandOfType(int type) const{
    double count = 0;
    list<RegPlotInfo* >::const_iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        if ((*plot_iter)->getSoilType()==type && (*plot_iter)->getState()==1 && (*plot_iter)->getNewleyRented()==true) { // state 1 = rented plot
            count+=g->PLOT_SIZE;
        }
    }
    return count;
}
int
RegFarmInfo::countNewRentedPlotsOfType(int type)  const{
    int count = 0;
    list<RegPlotInfo* >::const_iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        if ((*plot_iter)->getSoilType()==type && (*plot_iter)->getState()==1 && (*plot_iter)->getNewleyRented()==true) { // state 1 = rented plot
            count++;
        }
    }
    return count;
}
double
RegFarmInfo::getFarmNewRentExpenditure() {
    double count = 0;
    list<RegPlotInfo* >::iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        if ((*plot_iter)->getState()==1 && (*plot_iter)->getNewleyRented()==true) { // state 1 = rented plot
            count+=(*plot_iter)->getRentPaid();
        }
    }
    return count;
}

void
RegFarmInfo::calculateLiquidity() {
    //@  liquidity -= add_st_capital;
    add_st_capital = 0;
    // Liquiditaetsberechnung nach Alfons A_1203_neue-DK.xls
    liquidity = equity_capital - land_assets - FarmInvestList->getTotalResEcShareWithoutLabour();
    //@  if(liquidity < 0){
    //@    add_st_capital = (-1) * liquidity;
    //@    liquidity += add_st_capital;
    //@  }
} 
int
RegFarmInfo::getFarmClassChange() const {
    if (farm_class_change)
        return 1;
    else
        return 0;
}

/// wird nicht gebraucht
double
RegFarmInfo::getGmOfFarmType() const {
    // retrieve the total production yield of a farm class
    double mf = FarmProductList->getGmOfFarmType("ARABLE");
    double ve = FarmProductList->getGmOfFarmType("PIG/POULTRY");
    double fb = FarmProductList->getGmOfFarmType("GRASSLAND");
    double ag = mf + ve + fb;
    // if more than 50% of total gross margin is from pig and poultryp production
    // then the farm class is pig and poultry
    double pp = ve/ag;
    double g = fb/ag;
    double a = mf/ag;
    if (pp*100>66) return ve;  // pig/poultry
    if (g*100>66) return fb;   // grassland
    if (a*100>66) return mf;   // arable
    if ((pp*100<66)&&(g*100<66)&&(a*100<66))
        return ag;
    else return -1;        // mixed
}
double
RegFarmInfo::getRevenueOfFarmType() const  {
    // retrieve the total production yield of a farm class
    double mf = FarmProductList->getRevenueOfClass("ARABLE");
    double ve = FarmProductList->getRevenueOfClass("PIG/POULTRY");
    double fb = FarmProductList->getRevenueOfClass("GRASSLAND");
    double ag = mf+ve+fb;
    // if more than 50% of total gross margin is from pig and poultryp production
    // then the farm class is pig and poultry
    if (farm_class == 1) return ve;  // pig/poultry
    if (farm_class == 2) return fb;  // grassland
    if (farm_class == 3) return mf;  // arable
    if (farm_class == 4 ) return ag;  // mixed
    else return -1;

}
double
RegFarmInfo::getLabSub() const  {
    double ls = FarmInvestList->getLSWithoutLabour();
    return ls;
}
double
RegFarmInfo::doLpWithPrice() {
    int maxofffarmlu = (int) labour->getFamilyLabour()/(g->MAX_H_LU/2);
    double objective = lp->LpWithPrice(FarmProductList, inum_vector, maxofffarmlu);
    return objective;
}
double
RegFarmInfo::doLpWithPriceExpectation() {
    int maxofffarmlu = (int) labour->getFamilyLabour()/(g->MAX_H_LU/2);
    double objective = lp->LpWithPriceExpectation(FarmProductList,inum_vector,maxofffarmlu);
    return objective;
}
double
RegFarmInfo::doProductionLp() {
    int maxofffarmlu = (int) labour->getFamilyLabour()/(g->MAX_H_LU/2);
    double objective = lp->LpProdPriceExpectation(FarmProductList,inum_vector, maxofffarmlu);
    return objective;
}

double
RegFarmInfo::getCostFactorMachinery(int group) const  {
    // Cost effect of larger plot sizes depends on the average number of neighbours
    // per farm and the total land input

    double avneighbours = 0;
    double cfs;     // cost factor plot size
    double cfl;     // cost factor land
    double tcf;     // total cost factor

    if (g->TC_MACHINERY==0)
        return 0;

    bool newmachinery =  FarmInvestList->newMachinery(group);
    if (newmachinery) {
        avneighbours = getAverageNumberOfNeighbours();
        cfs = 1 - 0.45/(1+100/(10*avneighbours+1));
        cfl = 1-0.15/(1+100/land_input);
        tcf = cfs * cfl;
        return (1-tcf);
    } else {
        return 0;
    }
}
double
RegFarmInfo::getAverageNumberOfNeighbours() const {
    int size = PlotList.size();
    double avneighbours;
    int countneighbours = 0;
    list<RegPlotInfo* >::const_iterator plot_iter ;

    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        countneighbours += (*plot_iter)->identifyPlotsSameStateAndFarm(farm_id);
    }
    if (size>0)
        avneighbours = (double) countneighbours / (double) size;
    return avneighbours;
}

double
RegFarmInfo::getFarmRentExpOfType(int type) const{
    double rentexp = 0;
    // update farm_distance_costs and farm_rent_exp
    list<RegPlotInfo* >::const_iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        if ((*plot_iter)->getSoilType()==type) {
            rentexp += (*plot_iter)->getRentPaid();
        }
    }
    return rentexp;
}
double
RegFarmInfo::getFarmNewRentExpOfType(int type) const{
    double rentexp = 0;
    // update farm_distance_costs and farm_rent_exp
    list<RegPlotInfo* >::const_iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        if ((*plot_iter)->getSoilType()==type && (*plot_iter)->getNewleyRented()==true) {
            rentexp += (*plot_iter)->getRentPaid();
        }
    }
    return rentexp;
}
int
RegFarmInfo::countRentedPlotsOfType(int type) const{
    int count = 0;
    // update farm_distance_costs and farm_rent_exp
    list<RegPlotInfo* >::const_iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        if ((*plot_iter)->getSoilType()==type && (*plot_iter)->getState()==1) {
            count++;
        }
    }
    return count;
}
void RegFarmInfo::setDirectPaymentPerPlot(double payment) {
    list<RegPlotInfo* >::iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        (*plot_iter)->setPaymentEntitlement(payment);
    }
}

void
RegFarmInfo::modulateIncomePayment() {
    if(income_payment_farm>0) {
    display_modulation=0;
    if (income_payment_farm>g->LB_LOW_TRANCH && income_payment_farm<=g->UB_LOW_TRANCH) {
        modulated_income_payment=(1-g->DEG_LOW_TRANCH)*(income_payment_farm);
        display_modulation=1;
        lp->updatePaymentEntitlement();
        return;
    } else {
        modulated_income_payment=(1-g->DEG_LOW_TRANCH)*g->UB_LOW_TRANCH;
        if (income_payment_farm>g->LB_MIDDLE_TRANCH && income_payment_farm<=g->UB_MIDDLE_TRANCH) {
            modulated_income_payment+=(1-g->DEG_MIDDLE_TRANCH)*(income_payment_farm-g->UB_LOW_TRANCH);
            display_modulation=2;
            lp->updatePaymentEntitlement();
            return;
        } else {
            modulated_income_payment+=(1-g->DEG_MIDDLE_TRANCH)*(g->UB_MIDDLE_TRANCH-g->UB_LOW_TRANCH);
            if (income_payment_farm>g->LB_HIGH_TRANCH && income_payment_farm<=g->UB_HIGH_TRANCH) {
                modulated_income_payment+=(1-g->DEG_HIGH_TRANCH)*(income_payment_farm-g->UB_MIDDLE_TRANCH);
                display_modulation=3;
                lp->updatePaymentEntitlement();
                return;
            } else {
                modulated_income_payment+=(1-g->DEG_HIGH_TRANCH)*(income_payment_farm-g->UB_MIDDLE_TRANCH);
                lp->updatePaymentEntitlement();
            }
        }
    } } else {
    modulated_income_payment=0;
    }
}
double
RegFarmInfo::getCapitalInputDEA() const {
    double capitalwithoutflowtingcapital = 0;
    double services = 0;
    double quota = 0;
    services = (-1)* FarmProductList->getGrossMarginOfNumber(g->stdNameIndexs["SERVICES"])*FarmProductList->getUnitsProducedOfNumber(g->stdNameIndexs["SERVICES"]);
    quota = (getUnitsOfProduct(g->stdNameIndexs["MILK"])*g->MILKPROD)* FarmProductList->getPriceOfNumber(g->stdNameIndexs["LETQUOTA"]);
    capitalwithoutflowtingcapital = services + quota + getAnnuity();
    return capitalwithoutflowtingcapital;
}
double
RegFarmInfo::getOutputRevenue() const {
    double revDEA = 0;
    double prodrev = 0;
    double revservices = 0;
    double revletquota = 0;
    double revgetquota = 0;

    prodrev = FarmProductList->getReturnOfType(g->PRODTYPE);
    revservices = FarmProductList->getReturnOfNumber(g->stdNameIndexs["SERVICES"]);
    revletquota = FarmProductList->getReturnOfNumber(g->stdNameIndexs["LETQUOTA"]);
    revgetquota = FarmProductList->getReturnOfNumber(g->stdNameIndexs["GETQUOTA"]);

    revDEA = prodrev - revservices - revgetquota - revletquota;
    return revDEA;
}
double
RegFarmInfo::getOutputGrossMarginDEA() const {
    double gmDEA = 0;
    double prodgm = 0;
    double gmservices = 0;
    double gmletquota = 0;
    double gmgetquota = 0;
    prodgm = FarmProductList->getGrossMarginOfType(g->PRODTYPE);
    gmservices = FarmProductList->getGrossMarginOfNumber(g->stdNameIndexs["SERVICES"])*FarmProductList->getUnitsProducedOfNumber(g->stdNameIndexs["SERVICES"]);
    gmletquota = FarmProductList->getGrossMarginOfNumber(g->stdNameIndexs["LETQUOTA"])*FarmProductList->getUnitsProducedOfNumber(g->stdNameIndexs["LETQUOTA"]);
    gmgetquota = FarmProductList->getGrossMarginOfNumber(g->stdNameIndexs["GETQUOTA"])*FarmProductList->getUnitsProducedOfNumber(g->stdNameIndexs["GETQUOTA"]);

    gmDEA = prodgm - gmservices - gmgetquota - gmletquota;
    return gmDEA;
}
double
RegFarmInfo::getStandardGrossMargin() const {
    double standardgm = 0;
    for (int i=0;i<g->PRODUCTGROUPS;i++) {
        standardgm+=FarmProductList->getStandardGrossMarginOfGroup(i);
    }
    return standardgm;
}
double
RegFarmInfo::getGrossMargin() const {
    double gm=0;
    for (int i=0;i<g->PRODUCTGROUPS;i++) {
        gm+=FarmProductList->getStandardGrossMarginOfGroup(i);
    }
    return gm;
}
double
RegFarmInfo:: getAvRentOfType(int type) const  {
    double rent = getFarmRentExpOfType(type);
    double c = countRentedPlotsOfType(type);
    if (c!=0)
        return (rent/c)/g->PLOT_SIZE;
    else
        return 0;
}
double
RegFarmInfo::getAvNewRentOfType(int type) const  {
    double rent = getFarmNewRentExpOfType(type);
    double c = countNewRentedPlotsOfType(type);
    if (c!=0)
        return (rent/c)/g->PLOT_SIZE;
    else
        return 0;
}
double
RegFarmInfo::getEconomicLandRent()  const {

    double av_ec_land_rent;
    if (farm_class != 5)
    {
    double oppcostslabourhour = (- FarmInvestList->getAcquisitionCostsOfNumber(g->FIXED_OFFFARM_LAB)) /
                                - FarmInvestList->getLabourSubstitutionOfNumber(g->FIXED_OFFFARM_LAB);

    double hhincome = (profit
                       + farm_rent_exp
                       + farm_tac
                       + lt_interest_costs
                       + st_interest_costs
                       + farm_hired_labour_fix_pay
                       + farm_hired_labour_var_pay
                       + farm_factor_remuneration_fix
                       + farm_factor_remuneration_var)
                      / land_input;
    double wages = ( farm_hired_labour_fix_pay
                     + farm_hired_labour_var_pay )
                   / land_input;
    double oppinc = (   (double)labour->getFamilyLabour()*1.25*oppcostslabourhour  )   /land_input;
    double bcinterest = (lt_interest_costs
                         + st_interest_costs) /land_input;
    double ecinterest = ((equity_capital-land_assets-ecchange)*g->EQ_INTEREST)
                        /land_input;
    double quota = ((FarmProductList->getUnitsProducedOfNumber(g->stdNameIndexs["MILK"])*g->MILKPROD
                     + FarmProductList->getUnitsProducedOfNumber(g->stdNameIndexs["LETQUOTA"])
                     - FarmProductList->getUnitsProducedOfNumber(g->stdNameIndexs["GETQUOTA"])
                    ) * g->QUOTA_PRICE)/land_input;

    av_ec_land_rent =  hhincome
                       - wages
                       - oppinc
                       - bcinterest
                       - ecinterest
                       - quota; }
    else {av_ec_land_rent = 0 ;}                           
    return av_ec_land_rent;
}
int
RegFarmInfo::getEconomicSizeClass() const  {
    double ecsize=getStandardGrossMargin()/g->ESU;
    int sizeclass;
    if (ecsize<4)
        sizeclass = 1;
    if (ecsize>=4 && ecsize<8)
        sizeclass = 2;
    if (ecsize>=8 && ecsize<16)
        sizeclass = 3;
    if (ecsize>=16 && ecsize<40)
        sizeclass = 4;
    if (ecsize>=40 && ecsize<100)
        sizeclass = 5;
    if (ecsize>=100)
        sizeclass = 6;
    return sizeclass;
}
int
RegFarmInfo::getFarmSizeClass() const  {
    double farmsize = getNumberOfPlots()*g->PLOT_SIZE ;
    int sizeclass;
    if (farmsize < 10)
        sizeclass = 1;
    if (farmsize>=10 && farmsize<50)
        sizeclass = 2;
    if (farmsize>=50 && farmsize<100)
        sizeclass = 3;
    if (farmsize>=100 && farmsize<200)
        sizeclass = 4;
    if (farmsize>=200 && farmsize<500)
        sizeclass = 5;
    if (farmsize>=500 && farmsize<1000)
        sizeclass = 6;
    if (farmsize>=1000 && farmsize<3000)
        sizeclass = 7;
    if (farmsize>=3000)
        sizeclass = 8;
    return sizeclass;
}

void
RegFarmInfo::cacheLandInput() {
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        cache_land_input_of_type[i]=land_input_of_type[i];
    }
    lp->updateLand();
}
void
RegFarmInfo::restoreLandInput() {
    //Recover land input
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        land_input_of_type[i]=cache_land_input_of_type[i];
    }
    lp->updateLand();
}
double
RegFarmInfo::getObjective(double land_input) {
    land_input_of_type[0]=land_input;
    lp->updateLand();
    double t2=doLpWithPriceExpectation();
    return t2;
}
double
RegFarmInfo::getObjective() {
    lp->updateLand();
    double t2=doLpWithPriceExpectation();
    return t2;
}
double
RegFarmInfo::getValueOfPlots(vector<int>& plots) {
    double val=-getObjective();
    cacheLandInput();
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        land_input_of_type[i]+=plots[i]*g->PLOT_SIZE;
    }
    val+=getObjective();
    restoreLandInput();
    return val;
}


double
RegFarmInfo::getAvSizeOfContiguousPlotOfType(int type) const {
    double size=0;
    for (unsigned int i=0;i<contiguous_plots[type].size();i++) {
        size+=contiguous_plots[type][i];
    }
    if (size==0)
        return 0;
    else
        return size/(double)contiguous_plots[type].size();
}

double
RegFarmInfo::getSizeOfContiguousPlotsOfType(int type) const  {
    double size=0;
    for (unsigned int i=0;i<contiguous_plots[type].size();i++) {
        size+=contiguous_plots[type][i];
    }
    return size;
}
double
RegFarmInfo::getSizeOfContiguousPlotOfType(int type,int i)  const {
    return contiguous_plots[type][i];
}
int RegFarmInfo::getContiguousPlotsOfType(int type) const{
    return contiguous_plots[type].size();
}

void RegFarmInfo::countContiguousPlots() {
    list<RegPlotInfo* >::iterator i;
    double r1=0;
    double r2=0;
    for (i=PlotList.begin();i!=PlotList.end();i++) {
        if ((*i)->getSoilType()==0)
            r1+=g->PLOT_SIZE;
        else
            r2+=g->PLOT_SIZE;
    }
    clearTagsForContiguousPlots();
    initVectorOfContiguousPlots();
    i=PlotList.begin();
    int c=0;
    do {
        while (/*(*i)->getTag()==true && */i!=PlotList.end()) {
            if ((*i)->getTag()==false) break;
            c++;
            i++;
        }
        if (i!=PlotList.end()) {
            if ((*i)->getTag()==false) {
                int nop=(*i)->countContiguousPlots(farm_id);
                int st=(*i)->getSoilType();
                contiguous_plots[st].push_back(nop*g->PLOT_SIZE);
            }
        }
    } while (i!=PlotList.end());
    clearTagsForContiguousPlots();
}
void RegFarmInfo::clearTagsForContiguousPlots() {
    list<RegPlotInfo* >::iterator plot_iter;
    for (plot_iter = PlotList.begin();
            plot_iter != PlotList.end();
            ++plot_iter) {
        (*plot_iter)->unTag();
    }
}
void RegFarmInfo::initVectorOfContiguousPlots() {
    contiguous_plots.clear();
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        vector<double> temp;
        contiguous_plots.push_back(temp);
    }
}

void
RegFarmInfo::backup() {
    obj_backup=clone();
    FarmProductList->backup();
    FarmInvestList->backup();
    lp->backup();
    labour->backup();
}
void
RegFarmInfo::restore() {
    RegFarmInfo* tmp=obj_backup;
    assign();
    obj_backup=tmp;
    FarmProductList->restore();
    FarmInvestList->restore();
    lp->restore();
    labour->restore();
}
void
RegFarmInfo::releaseRentedPlots() {
    list<RegPlotInfo* >::iterator plot_iter;
        // RELEASE PLOTS
        int delcount=0;
        plot_iter = PlotList.begin();
        while (plot_iter != PlotList.end()) {
            if ((*plot_iter)->getState()==1) {
                    plot_iter=releasePlot(plot_iter);
                    delcount++;
            } else {
                plot_iter++;
            }
        }
}

double
RegFarmInfo::getFarmTacsOfWantedPlotOfType(int type) const  {
    return wanted_plot_of_type[type].farm_tac;
}
double
RegFarmInfo::getTacsOfWantedPlotOfType(int type)  const {
        return wanted_plot_of_type[type].tac;
}
RegPlotInfo*
RegFarmInfo::getWantedPlotOfType(int type) const  {
        return wanted_plot_of_type[type].plot;
}
bool
RegFarmInfo::getPreviouslyRentedByAgent(RegPlotInfo* p)  const {
        return (p->getPreviouslyRentedByAgent()==farm_id);
}

vector <double> RegFarmInfo::getReferencePeriodProduction()  const	{
        return FarmProductList->getReferencePeriodProduction();
}

void RegFarmInfo::fixReferencePeriod(){
        FarmProductList->fixReferencePeriod();
}

void RegFarmInfo::calculateReferencePeriod(){
        FarmProductList->calculateReferencePeriod();
}

double RegFarmInfo::getUnitsOfProduct(int i) const{
        return FarmProductList->getUnitsProducedOfNumber(i);
}

double RegFarmInfo::getVarCostsOfProduct(int i) const {
        return FarmProductList->getVarCostsOfNumber(i);
}
