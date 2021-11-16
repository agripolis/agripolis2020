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

//---------------------------------------------------------------------------
#ifndef RegProductH
#define RegProductH
#include <list>
#include <map>
#include <fstream>
#include <queue>
#include "RegGlobals.h"
#include "RegFarm.h"

/** RegProductInfo class.
    This class defines the attributes of a product
    @author Kathrin Happe, Alfons Balmann, Konrad Kellermann
    @version June 2001
*/

class RegProductInfo {
private:
	//soil service
	double a, b, c, d; //Yield coefficients 
	double e, f, c_plat; //yield coeff new
	double p, k, pesticide, energyvar; 
	double N;  //optimale Nitrogen value
	double gamma; //carbon change 
	int soiltype; 
	bool  hasSoilservice;
	bool dynSoilservice;
	
    /// number of product in a catalog
    int catalog_number;
    /** product type
        \begin{enumerate}
           \item \textbf{0}: short term borrowed capital
           \item \textbf{1}: equity capital interest
           \item \textbf{2}: variable hired labour pay per hour
           \item \textbf{3}: var off farm labour income per hour
           \item \textbf{4}: products
        \end{enumerate}
    */
    int product_type;
    /// price of product (used during production)
    double   price;
    /// expected price of product (used in farm closing decision)
    double   price_expectation;
    /// variable costs of product
    double var_cost;
    ///labour needed
    double labour;
    /// product group
    int product_group;
    /// product name
    string name;

    string stdName;

    /// farm class
    string cl;
    ///Livestock unit
    double Lives_unit;
    bool premium_legitimation;
    int premium_col;
    int premium_row;
    double premium;
	double policy_premium;
	double price_change;
    bool price_support;
    double reference_premium;
	double refPremPercent;
    double reference_premium_calc_time;

    //Environmental variables 20060426
    double N_usage;
    double P2O5_usage;
    double K2O_usage;
    double Fungicides_usage;
    double Herbicides_usage;
    double Insecticides_usage;
    double Water_usage;
    double SLossCoeff;

public:
	//soil service
	void setSoilservice(double, double, double, double, double, double, double, double, int, bool, bool, double, double, double, double); //initialize
	void setSoilservice(bool hasSoil);
	double getGamma(){return gamma;};
	bool hasSoilService(){return hasSoilservice;}
	bool hasDynSoilservice(){return dynSoilservice; }
	int getProdSoilType(){return soiltype;}
	map<string, double> ssCoeffs;
	void setSScoeffs();
	
    // For Premium
    void setPremiumCol(int pc) {
        premium_col=pc;
    }
    int getPremiumCol() const {
        return premium_col;
    }
    void setPremiumRow(int pr) {
        premium_row=pr;
    }
    int getPremiumRow() const {
        return premium_row;
    }

	void setPolicyPremium(double p) {
        policy_premium=p;
    }
    double getPolicyPremium() const {
        return policy_premium;
    }

	void setPriceChange(double p) {
        price_change=p;
    }
    double getPriceChange() const {
        return price_change;
    }

    void setPremium(double p) {
        premium=p;
    }
    double getPremium() const {
        return premium;
    }
    void setPremiumLegitimation(bool pl) {
        premium_legitimation=pl;
    }
    bool getPremiumLegitimation() const {
        return premium_legitimation;
    }
    void setPriceSupport(bool ps) {
        price_support=ps;
    }
    bool getPriceSupport() const {
        return price_support;
    }
    /// constructor
    RegProductInfo() {};

    void setReferencePremium(int ref_pre) {
        reference_premium=ref_pre;
    }
    double getReferencePremium() const {
        return reference_premium;
    }

	void setRefPremPercent(double ref_perc) {
        refPremPercent=ref_perc;
    }
    double getRefPremPercent() const {
        return refPremPercent;
    }

    void setReferencePremiumCalcTime(int ref_pre) {
        reference_premium_calc_time=ref_pre;
    }
    double getReferencePremiumCalcTime() const {
        return reference_premium_calc_time;
    }
    void    setCatalogNumber(int cn) {
        catalog_number = cn;
    }
    int     getCatalogNumber() const {
        return catalog_number;
    }
    void    setPrice(double p) {
        price = p;
    }
    double     getPrice() const {
        return price;
    }
    void    setPriceExpectation(double p) {
        price_expectation=p;
    }
    double     getPriceExpectation() const {
        return price_expectation;
    }
    void    setVarCost(double vc) {
        var_cost = vc;
    }
    double     getVarCost() const {
        return var_cost;
    }
     void    setLabour(double vc) {
		 labour = vc;
    }
    double     getLabour() const {
        return labour;
    }
    void    setProductType(int p) {
        product_type = p;
    }
    int     getProductType() const {
        return product_type;
    }
    void    setProductGroup(int pg) {
        product_group = pg;
    }
    int        getProductGroup() const {
        return product_group;
    }
    string getName() const{
        return name;
    }
    void setName(string n) {
        name = n;
    }
    string getStdName() const{
        return stdName;
    }
    void setStdName(string n) {
        stdName = n;
    }

    string getClass() const {
        return cl;
    }
    void setClass(string n) {
        cl = n;
    }
    void    setLU(double lun) {
        Lives_unit = lun;
    }
    double     getLU() const {
        return Lives_unit;
    }

    /** Set variables with file input values
        @param cn catalog number of product
        @param pt product type
        @param n product name
        @param sn stdName
        @param c farm class
    */
    void setAttrib(int cn,int pt,int pg,string n, string sn, string c);
   
    // return descriptions 
    string debug();

    //env 20060426

    void setEnvAttrib(double N_h, double P2O5_h, double K2O_h,
                      double fung_h, double herb_h, double insect_h,
                      double wusage_h, double SLossCoeff_h);
    double getN_usage() const {
        return N_usage;
    };
    double getP2O5_usage() const {
        return P2O5_usage;
    };
    double getK2O_usage() const {
        return K2O_usage;
    };
    double getFungicides_usage() const {
        return Fungicides_usage;
    };
    double getHerbicides_usage() const {
        return Herbicides_usage;
    };
    double getInsecticides_usage() const {
        return Insecticides_usage;
    };
    double getWater_usage()const  {
        return Water_usage;
    };
    double getSLossCoeff() const {
        return SLossCoeff;
    };

};

/** RegProductList class.
    The class defines a list of products.
    @author Kathrin Happe, Alfons Balmann, Konrad Kellermann
    @version June 2001
*/
class RegFarmInfo;
class RegProductList {
private:
    RegGlobalsInfo* g;
    /// vector of references to product objects
    vector<RegProductInfo>* products;
    /** vector of var costs
        The vector has the size of the products vector (ie. the total
        number of products).
    */
    vector<double> var_costs;
    /** vector of original var_costs after multiplication with management factor
    */
    vector<double> var_costs_original;
    /** vector of var_costs before multiplication with management factor
    */
    vector<double> var_costs_standard;
    /** vector of 'old'
           This vector is needed is needed to store the var_costs values
           while the expected var costs are used in the matrix
       */
    vector<double> var_costs_old;
    /** vector of units produced for each product
        The vector has the size of the products vector.
    */
    vector<double> units_produced;
    vector<double> units_produced_old;
    bool use_price_expectation;
    vector< list<double> > units_produced_for_prem_calc;
    vector<double> fixed_reference_production;
    RegProductList* obj_backup;
public:
	//soil service 
	double getNoptOfNumber(int, RegFarmInfo*);
	double getYieldOfNumber(int, RegFarmInfo*);

	double getKOfNumber(int);
	double getPOfNumber(int);
	double getPesticideOfNumber(int);
	double getEnergyvarOfNumber(int);

    /** constructor:
        @param G pointer to globals
        @param P reference to vector of products
    */
    RegProductList(RegGlobalsInfo* G,vector<RegProductInfo>& P);
    RegProductList(RegProductList& rh,RegGlobalsInfo* G,vector<RegProductInfo>& P);
    ~RegProductList();
    void backup();
    void restore();
    /// Adjustment of the activity levels of capital according to the liquidity effect of new investments
    void adjustActivityLevel(double leeffect);
    /// return revenue for product type t

    double getReturnOfType(int );
    /// return total revenue for product number n
    double getReturnOfNumber(int );
    /// return gross margin of product type t
    double getGrossMarginOfType(int );
    /// retur var costs of product type t
    double getVarCostsOfType(int);
    /// set var costs of product type
    void setVarCostsOfType(int, double);
    /// get price of product type
    double getPriceOfType(int);
    /// get price expectation of type
    double getPriceExpectationOfType(int);
    /// get units produced of product type (does NOT make sense for type 4!!!)
    double getUnitsProducedOfType(int);
    /// get units produced of product group (to calculate used land)
    double getUnitsProducedOfGroup(int);
    /// get revenue farm specific products
    double getRevenueOfClass(string);

    /// get var costs of product number (not type!!!)
    double getVarCostsOfNumber(int);
    void setVarCostsOfNumber(int, double);
    /// get price of product number
    double getPriceOfNumber(int);
    /// get price expectation of product number
    double getPriceExpectationOfNumber(int);
    /// get gross margin of product number
    double getGrossMarginOfNumber(int);
    /// get gross margin of product number
    double getTotalGrossMargin();
    // get LU of product number
    double getTotalLU(string cl);
    double getTotalLU();
    double getTotalGrossMarginExpectation();
    /// get units produced of product number
    double getUnitsProducedOfNumber(int);
    void setUnitsProducedOfNumber(int,double);
    /// get standard gross margin of product group
    double getStandardGrossMarginOfGroup(int);
    /// get standard gross margin of product group
    double getStandardGrossMarginOfNumber(int);
    /// get gross margin of groups
    double getGrossMarginOfGroup(int );

	/** classification of gross margins according to farm classes
        E.g. if more than 50% of the total gross margin is from dairy
        production, than the farm belongs to the class dairy.
        \begin{itemize}
           \item 1: pig and poultry
           \item 2: dairy
           \item 3: arable
           \item 4: mixed
        \end{itemize}
        For this to be possible an additional column was introduced in the
        market table which indicates the products that define a certain
        farm class.

        It would also be interesting to show the development of single
        enterprises, like e.g. dairy production.
        @return farm class number
    */
    int calculateFarmClass();

    /// function for updating costs of Type 4 with the management_coefficient
    void updateCosts(double);

    /// update var costs as result of new investments
    void changeVarCosts(double , int);

    /// for new investments expect lower costs
    void expectLowerCosts(double );

    /// reset var_costs to old values
    void restoreProductList();

    /// total gross margin of all farms in one farm class
    double getGmOfFarmType(string cl);

    /// total standard gross margin of all farms in one farm class
    double getStandardGmOfFarmType(string cl);


    /// get gross margin of product number either with price or price_exp
    /// depends on bool use_price_expectation
    double getGmOrGmExpectedOfNumber(int);
    /// Function to set if the above function should use price or
    /// priceexpectation
    bool setUsePriceExpectation(bool);
    /// save var costs to be restored
    void saveProductList();

    void calculateReferencePeriod();
    void fixReferencePeriod();
    vector <double> getReferencePeriodProduction() {
        return fixed_reference_production;
    }
    
    void debug(string);
};

//---------------------------------------------------------------------------
#endif
