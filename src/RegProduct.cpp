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

// RegProduct.cpp
//---------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include "RegProduct.h"

//---------------------------------------------------------------------------
// RegProduct methods
//---------------------------------------------------------------------------
//soil service
void RegProductInfo::setSoilservice(double aa, double bb, double cc, double dd,
			double ee, double ff, double cplat, double gam, int soil, bool hasSoil, bool dyn, 
	       double pp, double kk, double pest, double energy){
	a=aa;
	b=bb;
	c=cc;
	d=dd;
	e=ee; 
	f=ff;
	c_plat=cplat;
	gamma=gam;
	soiltype=soil;
	hasSoilservice = hasSoil;
	dynSoilservice = dyn;
	
	//neu Gss
	p=pp; k=kk; pesticide=pest; energyvar= energy;
	setSScoeffs();

	return;
}

void RegProductInfo::setSoilservice(bool hasSoil){
	hasSoilservice = hasSoil;
	dynSoilservice = hasSoil;
	return;
}

void RegProductInfo::setSScoeffs(){
	ssCoeffs["a"]=a;
	ssCoeffs["b"]=b;
	ssCoeffs["c"]=c;
	ssCoeffs["d"]=d;
	ssCoeffs["e"]=e;
	ssCoeffs["f"]=f;
	ssCoeffs["c_plat"]=c_plat;
	ssCoeffs["p"]=p;
	ssCoeffs["k"]=k;
	ssCoeffs["pesticide"]=pesticide;
	ssCoeffs["energyvar"]=energyvar;
}

void
RegProductInfo::setAttrib(int cn, int pt, int pg, string n,string sn, string c) {
    catalog_number = cn;
    product_type = pt;
    product_group = pg;
    name = n;
    stdName = sn;
    cl=c;
    reference_premium=0;
    reference_premium_calc_time=0;
	setRefPremPercent(0);
}

void
RegProductInfo::setEnvAttrib( double N_h, double P2O5_h, double K2O_h,
                              double fung_h, double herb_h, double insect_h,
                              double wusage_h, double SLossCoeff_h) {
    N_usage = N_h;
    P2O5_usage = P2O5_h;
    K2O_usage = K2O_h;
    Fungicides_usage = fung_h;
    Herbicides_usage = herb_h;
    Insecticides_usage = insect_h;
    Water_usage = wusage_h;
    SLossCoeff = SLossCoeff_h;
}


string
RegProductInfo::debug() {
    stringstream r;
    r << "Name:\t" << name << "\t" <<
                   "Catalog Number:\t" << catalog_number << "\t" <<
                   "Type:\t" << product_type << "\t" <<
                   "Price:\t" << price << "\t" <<
                   "Price Expectation:\t" << price_expectation << "\t" <<
                   "Original Costs:\t" << var_cost << "\t";
    return r.str();
}

//======================================RegProductList =============================
/* 
    Products with dynamic yield are represented with two names, one for varCost and one for  price.
	These names have to meet the naming following naming convention to guarantee the correct calculation
	of the N value:
	      name for varCost: productname_XXX
		  name for price:   productname_SELL
	       
//*/

// these product names should be better as input data
const string varcostProductN = "N_FERT_BUY"; //in market with negative price value
const string varcostProductP = "P_FERT_BUY"; 
const string varcostProductK = "K_FERT_BUY"; 
const string varcostProductPe = "PESTICIDES_BUY"; 
const string varcostProductEvar = "ENERGY_var_BUY"; 

double RegProductList::getNoptOfNumber(int i, RegFarmInfo* s){
	double w, p;
	double wp, wk, wpe, we;
	string name, name_stam, xname;
	for (unsigned t=0; t<(*products).size(); ++t){
		name.clear(); name_stam.clear(); xname.clear();
		name = (*products)[i].getName();
		name_stam = name.substr(0,name.length()-4);
		xname = (*products)[t].getName();
		if (xname.find(name_stam.c_str())!=string::npos && xname.find("SELL")!=string::npos ){ 
			p=(*products)[t].getPrice();
		}
		
		if ((*products)[t].getName()==varcostProductN) {
			w=(-1)*(*products)[t].getPrice(); 
		}

		if ((*products)[t].getName()==varcostProductP) {
			wp=(-1)*(*products)[t].getPrice(); 
		}
		if ((*products)[t].getName()==varcostProductK) {
			wk=(-1)*(*products)[t].getPrice(); 
		}
		if ((*products)[t].getName()==varcostProductPe) {
			wpe=(-1)*(*products)[t].getPrice(); 
		}
		if ((*products)[t].getName()==varcostProductEvar) {
			we=(-1)*(*products)[t].getPrice(); 
		}

	}

	RegProductInfo prod = (*products)[i];
	double b=prod.ssCoeffs["b"];
	double c=prod.ssCoeffs["c"];
	double f= prod.ssCoeffs["f"];
	double energy = prod.ssCoeffs["energyvar"];
	double wall = w + prod.ssCoeffs["p"]*wp + prod.ssCoeffs["pesticide"]*wpe + 
					prod.ssCoeffs["k"]*wk;
	
	int soiltype= (*products)[i].getProdSoilType();
	double carbon = s ->getAvCarbons()[soiltype];

	double c_plat = prod.ssCoeffs["c_plat"];
	if (carbon > c_plat) 
		carbon = c_plat;

	double res = 0.5*(wall/(p*g->tech_develop_abs-energy*we)
				-(b+carbon*f))/c;
	return res>0?res:0;
}


double RegProductList::getPOfNumber(int i){
	return (*products)[i].ssCoeffs["p"];
}

double RegProductList::getKOfNumber(int i){
	return (*products)[i].ssCoeffs["k"];
}

double RegProductList::getPesticideOfNumber(int i){
	return (*products)[i].ssCoeffs["pesticide"];
}

double RegProductList::getEnergyvarOfNumber(int i){
	return (*products)[i].ssCoeffs["energyvar"];
}

double RegProductList::getYieldOfNumber(int i, RegFarmInfo* s){
	double N = getNoptOfNumber(i,s);
	int soiltype= (*products)[i].getProdSoilType();
	map <string, double> ssCoeffs = (*products)[i].ssCoeffs;
	
	double cplat = (*products)[i].ssCoeffs["c_plat"];
	double carbon = s ->getAvCarbons()[soiltype];

	if (carbon > cplat) carbon = cplat;

	double y = (ssCoeffs["a"]+ssCoeffs["b"]*N + ssCoeffs["c"]*N*N+ssCoeffs["d"]*carbon
		    +ssCoeffs["e"]*carbon*carbon + ssCoeffs["f"]*N*carbon);

	return y*g->tech_develop_abs;
}

RegProductList::RegProductList(RegGlobalsInfo* G,vector<RegProductInfo>& P)
        :g(G),products(&P) {
    obj_backup=NULL;
    var_costs.resize((*products).size());
    var_costs_old.resize((*products).size());
    var_costs_original.resize((*products).size());
    var_costs_standard.resize((*products).size());
    units_produced.resize((*products).size());
    units_produced_old.resize((*products).size());
    for (unsigned int i=0;i < (*products).size();i++) {
        var_costs[i] = (*products)[i].getVarCost();
        var_costs_old[i] = var_costs[i];
        var_costs_original[i] = var_costs[i];
        var_costs_standard[i] = var_costs[i];
        units_produced[i]=0;
        units_produced_old[i]=0;
        if ((*products)[i].getPremiumLegitimation()) fixed_reference_production.push_back(0);
    }
    use_price_expectation=true;
    units_produced_for_prem_calc.resize((*products).size());
}

RegProductList::RegProductList(RegProductList& rh,RegGlobalsInfo* G,vector<RegProductInfo>& P)
        :g(G),products(&P) {
    obj_backup=NULL;
    var_costs=rh.var_costs;
    var_costs_original=rh.var_costs_original;
    var_costs_standard=rh.var_costs_standard;
    var_costs_old=rh.var_costs_old;
    units_produced=rh.units_produced;
    units_produced_old=rh.units_produced_old;
    use_price_expectation=rh.use_price_expectation;
    units_produced_for_prem_calc=rh.units_produced_for_prem_calc;
    fixed_reference_production=rh.fixed_reference_production;
}

double
RegProductList::getUnitsProducedOfType(int t) {
    double sum=0;
    for(unsigned int i=0;i<(*products).size();i++) {
      if((*products)[i].getProductType()==t)
        sum+=units_produced[i];
    }
    return sum;
}

double
RegProductList::getUnitsProducedOfGroup(int t) {
    double sum=0;
    for(unsigned int i=0;i<(*products).size();i++) {
      if((*products)[i].getProductGroup()==0)
        sum+=units_produced[i];
    }
    return sum;
}

void
RegProductList::debug(string filename) {
    ofstream out;
    out.open(filename.c_str(),ios::trunc);
    out << "Product List\n";
    for (unsigned int i=0;i<(*products).size();i++) {
        out << (*products)[i].debug().c_str()
        << "Farm Specific Costs:\t" << var_costs[i] << "\t"
        << "Var. Costs:\t" << var_costs[i] << "\t"
        << "Units Prodeced:\t" << units_produced[i] << "\n";
    }
    out.close();
}

void
RegProductList::setUnitsProducedOfNumber(int n,double p) {
    units_produced[n]=p;
}

double
RegProductList::getReturnOfType(int t) {
    double ret=0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getProductType()==t) {

            ret+=units_produced[i]*getPriceOfNumber(i);
        }
    }
    return ret;
}
double
RegProductList::getReturnOfNumber(int n) {
    return (units_produced[n]*getPriceOfNumber(n));
}
double  // quantity
RegProductList::getUnitsProducedOfNumber(int n) {
    return units_produced[n];
}

double  // monetary
RegProductList::getGrossMarginOfType(int t) {
    double gm=0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getProductType()==t) {
            gm+=units_produced[i]*getGrossMarginOfNumber(i);
        }
    }
    return gm;
}
void
RegProductList::updateCosts(double mc) {
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getProductType()==4) {    
            var_costs[i]*=mc;
            var_costs_original[i]=var_costs[i];
            var_costs_old[i]=var_costs[i];
        }
    }
}
void
RegProductList::changeVarCosts(double f, int investgroup) {
    // change var costs of selected (*products)
    for (unsigned int i=0;i<(*products).size();i++) {
        int productgroup = (*products)[i].getProductGroup();
        if ((productgroup==investgroup)&&(productgroup!=-1)) { // all products
            if (var_costs[i]>0 && f!=0) {
                if (productgroup==0) { // crop products using machinery; take original values as basis
                    setVarCostsOfNumber(i,(var_costs_original[i]-var_costs_original[i]*f));
                } else { // all other products
                    setVarCostsOfNumber(i,(var_costs[i]-var_costs[i]*f));
                }
            }
        }
    }
}
void
RegProductList::saveProductList() {
    for (unsigned int i=0;i<(*products).size();i++) {
        var_costs_old[i] = var_costs[i];
        units_produced_old[i]=units_produced[i];
    }
}
void
RegProductList::restoreProductList() {
    for (unsigned int i=0;i<(*products).size();i++) {
        var_costs[i] = var_costs_old[i];
        units_produced[i]=units_produced_old[i];
    }
}
double
RegProductList::getVarCostsOfNumber(int n) {
    return var_costs[n];
}

void
RegProductList::setVarCostsOfNumber(int n, double v) {
    var_costs[n] = v;
}

double
RegProductList::getPriceOfNumber(int n) {
    return (*products)[n].getPrice();
}
double
RegProductList::getPriceExpectationOfNumber(int n) {
    return (*products)[n].getPriceExpectation();
}
double
RegProductList::getGrossMarginOfNumber(int n) {
    return (*products)[n].getPrice() - var_costs[n];
//    return products[n].getPriceExpectation() - var_costs[n];
}
double
RegProductList::getStandardGrossMarginOfGroup(int g) {
    double standardgm = 0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getProductGroup()==g) {
            standardgm+=units_produced[i]*getStandardGrossMarginOfNumber(i);
        }
    }
    return standardgm;
}
double
RegProductList::getStandardGrossMarginOfNumber(int i) {
    return  (*products)[i].getPrice() - var_costs_standard[i];
}
double
RegProductList::getGrossMarginOfGroup(int g) {
    double gm = 0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getProductGroup()==g) {
            gm+=units_produced[i]*getGrossMarginOfNumber(i);
        }
    }
    return gm;
}

double
RegProductList::getVarCostsOfType(int n) {
    return var_costs[n];
}
void
RegProductList::setVarCostsOfType(int n,double v) {
    var_costs[n] = v;
}
double
RegProductList::getPriceOfType(int n) {
    return (*products)[n].getPrice();
}
double
RegProductList::getPriceExpectationOfType(int n) {
    return (*products)[n].getPriceExpectation();
}
int
RegProductList::calculateFarmClass() {
    // retrieve the total production yield of a farm class
    double mf = getStandardGmOfFarmType("ARABLE");
    double ve = getStandardGmOfFarmType("PIG/POULTRY");
    double fb = getStandardGmOfFarmType("GRASSLAND");
    double ag = mf + ve + fb;
    // if more than 50% of total gross margin is from pig and poultryp production
    // then the farm class is pig and poultry
    double pp = ve/ag;
    double g = fb/ag;
    double a = mf/ag;
    double x=2;
    double y=3;
    double teiler = x/y;
    if (pp>teiler) return 1;  // pig/poultry
    if (g>teiler) return 2;   // grassland
    if (a>teiler) return 3;   // arable
    if ((pp<teiler)&&(g<teiler)&&(a<teiler))
        return 4;             // mixed
    return -1;
}
double
RegProductList::getGmOfFarmType(string cl) {
    double ret=0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getClass()==cl) {
            ret+=units_produced[i]*getGrossMarginOfNumber(i);
        }
    }
    return ret;
}
double
RegProductList::getStandardGmOfFarmType(string cl) {
    double ret=0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getClass()==cl) {
            ret+=units_produced[i]*getStandardGrossMarginOfNumber(i);
        }
    }
    return ret;
}
double
RegProductList::getRevenueOfClass(string cl) {
    double rev = 0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getClass()==cl) {
            rev+=units_produced[i]*getPriceOfNumber(i);
        }
    }
    return rev;
}
double
RegProductList::getGmOrGmExpectedOfNumber(int n) {
    if (use_price_expectation)
        return (*products)[n].getPriceExpectation() - var_costs[n];
    else
        return (*products)[n].getPrice() - var_costs[n];
}
bool
RegProductList::setUsePriceExpectation(bool set) {
    bool changed;
    if (use_price_expectation==set)
        changed=false;
    else
        changed=true;
    use_price_expectation=set;
    return changed;
}

///////////////////////////////////////////////////////////////////////////////
void
RegProductList::expectLowerCosts(double factor) {
    // expect lower var costs for all products affected by investment objects
    for (unsigned int i=0;i<(*products).size();i++) {
        var_costs_old[i] = var_costs[i];
        int g = (*products)[i].getProductGroup();
        if ((g!=-1) && (var_costs[i] > 0)) {
            if (var_costs[i]>0 && factor!=0) {
                if (g==0) { // for machinery take original values as basis
                    setVarCostsOfNumber(i,(var_costs_original[i]-var_costs_original[i]*factor));
                } else { // all other products
                    setVarCostsOfNumber(i,(var_costs[i]-var_costs[i]*factor));
                }
            }
        }
    }
}

void RegProductList::fixReferencePeriod() {
    int j=0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getPremiumLegitimation()) {
            double av_prod=0;
            double count=units_produced_for_prem_calc[i].size();
            list<double>::iterator iter;
            for (iter = units_produced_for_prem_calc[i].begin();
                    iter != units_produced_for_prem_calc[i].end();
                    iter++) {
                av_prod+=(*iter);
            }
            av_prod/=count;
            fixed_reference_production[j]=av_prod;
            j++;
        }
    }
}

void RegProductList::calculateReferencePeriod() {
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getPremiumLegitimation()) {
            units_produced_for_prem_calc[i].push_back(units_produced[i]);
            if (units_produced_for_prem_calc[i].size()>(*products)[i].getReferencePremiumCalcTime())
                units_produced_for_prem_calc[i].pop_front();
        }
    }
}
void
RegProductList::adjustActivityLevel(double leeffect)   {
    if (units_produced[g->ST_EC_INTERESTTYPE]>0)  {
        double diff=leeffect+units_produced[g->ST_EC_INTERESTTYPE];
        if (diff>0) {
            units_produced[g->ST_EC_INTERESTTYPE]=diff;
        } else {
            units_produced[g->ST_EC_INTERESTTYPE]=0;
            units_produced[g->ST_BOR_INTERESTTYPE]=fabs(diff);
        }
    } else {
        units_produced[g->ST_BOR_INTERESTTYPE]+=fabs(leeffect);
    }
}
double
RegProductList::getTotalGrossMargin() {
    double gm=0;
    for (unsigned int i=0;i< ((*products).size());i++) {
        gm+= ((*products)[i].getPrice() - var_costs[i])*units_produced[i];
    }
    return gm;
}

double
RegProductList::getTotalLU(string cl) {
    double lu=0;
    for (unsigned int i=0;i< ((*products).size());i++) {
        if ((*products)[i].getClass()==cl) {
            lu+= (*products)[i].getLU()*units_produced[i];
        }
    }
    return lu;
}
double
RegProductList::getTotalLU() {
    double lu=0;
    for (unsigned int i=0;i< ((*products).size());i++) {
        lu+= (*products)[i].getLU()*units_produced[i];
    }
    return lu;
} 
double
RegProductList::getTotalGrossMarginExpectation() {
    double gm=0;
    for (unsigned int i=0;i< ((*products).size());i++) {
        gm+= ((*products)[i].getPriceExpectation() - var_costs[i])*units_produced[i];
    }
    return gm;
}

void
RegProductList::backup() {
    obj_backup=new RegProductList(*this);
}
void
RegProductList::restore() {
    RegProductList* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
}
RegProductList::~RegProductList() {
    if (obj_backup) delete obj_backup;
}
