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
#ifndef RegInvestH
#define RegInvestH
#include <list>
#include <fstream>

#include "RegGlobals.h"

using namespace std;

/** RegInvestObjectInfo class.
    RegInvestObjectInfo defines the characteristics of an
    investment object
    @author Kathrin Happe, Alfons Balmann, Konrad Kellermann
    @version June 2001
*/
class RegInvestObjectInfo {
private:
    RegGlobalsInfo* g;
    /// order number of investment object
    int      catalog_number;
    double   acquisition_costs;
    /// maximum lifetime of object
    int      economic_life;
    /// labour substitution in hours associcated with object
    double   labour_substitution;
    double   norm_labour_substitution;
    /// land substitution in ha associated with object
    double   land_substitution;
    /// capacity of object in  the respective unit (heads, ha, etc.)
    double   capacity;
    /// number of product that can be produced with this object
    int     affects_product_group;
    /// residual value of object after investment
    double   residual_value;
    /// equity capital share of residual value after investement
    double   residual_ec_share;
    
    int invest_type;
    /// age of object after investment
    int invest_age;
    double   average_cost;

    // INTEREST
    /// borrowed capital interest of investment
    double   bc_interest;
    double interest_reduction;
    double reduced_bc_interest;
    double residual_bc_share;

    /** part of investment financed with equity capital ->MIP (financing rule)
        ec_share of acquisition costs
    */
    double   liq_effect;
    /// average bound equity capital -> MIP (liquidity constraint)
    double   bound_equity_capital;
    /** maintenance costs
        buildings: 1%, machinery: 4%
    */
    double   maintenance_costs;
    /// name of investment object
    string name;
    /// size of investment
    double technical_change_effect;


public:
    /** Method that creates investment object, assign values supplied
        by input file and then stores the investment objects in the
        InvestCatalog vector.
        @return vector of invest objects
    */
    vector <RegInvestObjectInfo >  initInvestmentCatalog();

    /** assign values to variables
        @param cn catalog number in InvestCatalog
        @param ac acquisition costs
        @param el economic life of object
        @param ls labour substitution
        @param ls2 land substitution
        @param c capacity
        @param it invest type
        @param n name of investment object
        @param ac residual value  (initially = acquisition costs)
        @param mc maintenance costs
    */
    void    setAttrib(int cn,
                      double ac,
                      unsigned int el,
                      double ls,
                      double ls2,
                      double c,
                      double mc,
                      int pg,
                      unsigned int it,
                      double tc,
                      string n
                     );

    // attribute access functions
    void    setCatalogNumber(int cn) {
        catalog_number = cn;
    }
    int     getCatalogNumber() const {
        return catalog_number;
    }
    void    setAcquisitionCosts(double ac) {
        acquisition_costs = ac;
    }
    double   getAcquisitionCosts() const {
        return acquisition_costs;
    }
    void    setEconomicLife(unsigned int el) {
        economic_life = el;
    }
    int     getEconomicLife() const {
        return economic_life;
    }
    void    setLabourSubstitution(double ls) {
        labour_substitution = ls;
    }
    double   getLabourSubstitution() const {
        return labour_substitution;
    }
    void    setNormLabourSubstitution(double ls) {
        norm_labour_substitution = ls;
    }
    double   getNormLabourSubstitution() const {
        return norm_labour_substitution;
    }

    void    setLandSubstitution(double ls2) {
        land_substitution = ls2;
    }
    double   getLandSubstitution() const {
        return land_substitution;
    }
    void    setCapacity(double c) {
        capacity = c;
    }
    double   getCapacity() const {
        return capacity;
    }
    void    setInvestType (unsigned int it) {
        invest_type = it;
    }
    int     getInvestType() const {
        return invest_type;
    }
    void    incrementInvestAge();
    int     getInvestAge() const {
        return invest_age;
    }
    void    setInvestAge(int ia) {
        invest_age = ia;
    }
    double   getAverageCost() const {
        return average_cost;
    }
    void    setAverageCost(double ac) {
        average_cost = ac;
    }
    double   getLiqEffect() const {
        return liq_effect;
    }
    void    setLiqEffect(double le) {
        liq_effect = le;
    }
    double   getBoundEquityCapital() const {
        return bound_equity_capital;
    }
    void    setBoundEquityCapital(double bec) {
        bound_equity_capital = bec;
    }
    double   getResidualValue() const {
        return residual_value;
    }
    void    setResidualValue(double rv) {
        residual_value = rv;
    }
    double   getResidualEcShare() const {
        return residual_ec_share;
    }
    void    setResidualEcShare(double res) {
        residual_ec_share = res;
    }
    double   getMaintenanceCosts() const {
        return maintenance_costs;
    }
    void    setMaintenanceCosts(double amc) {
        maintenance_costs = amc;
    }
    int      getAffectsProductGroup() const {
        return affects_product_group;
    }
    void    setAffectsProductGroup(int apg) {
        affects_product_group = apg;
    }
    string getName() const{
        return name;
    }
    void setName(string n) {
        name = n;
    }
    double getTechnicalChangeEffect()  const{
        return technical_change_effect;
    }
    void setTechnicalChangeEffect(double tc) {
        technical_change_effect=tc;
    }

    //Interest
    void setBcInterest(double);
    void setInterestReduction(double);
    void setReducedBcInterest(double);
    double getBcInterest() {
        return bc_interest;
    }
    double getInterestReduction() {
        return interest_reduction;
    }
    double getReducedBcInterest() {
        return reduced_bc_interest;
    }
    void setResidualBcShare(double residual_bc_share) {
        this->residual_bc_share=residual_bc_share;
    }
    double getResidualBcShare() {
        return residual_bc_share;
    }
    double getOwnBcInterest() {
        return residual_bc_share*reduced_bc_interest;
    }
    double getPoliticalBcInterest() {
        return residual_bc_share*interest_reduction;
    }

    string debug();
 
    /// constructor
    RegInvestObjectInfo(RegGlobalsInfo* G);

    /// destructor
    ~RegInvestObjectInfo();
};

class RegInvestList {
private:
	RegGlobalsInfo* g;
    /// list of investment objects on a farm
    list<RegInvestObjectInfo > farm_invests;
    /// reference to vector of Investment objects (InvestCatalog)
    vector<RegInvestObjectInfo>* invest_cat;
    //    vector<int> inum;
    /// vector indicating that object was newley invest in
    vector<int> newley_invested;
    RegInvestList* obj_backup;

	map<string, int> removed_invs;

public:
	map<string, int> getRemovedInvests();
	void resetRemovedInvs();

	int getRandomInvestAge();
    double labSubstitution;
    /// destructor
    ~RegInvestList();

    /** constructor
        @param G pointer to globals
        @param I reference to InvestCatalog
    */
    RegInvestList(RegGlobalsInfo* G, vector<RegInvestObjectInfo>& I);
    RegInvestList(RegInvestList&,RegGlobalsInfo* G, vector<RegInvestObjectInfo>& I);
    RegInvestList() {
        obj_backup=NULL;
    };
    void backup();
    void restore();
    /** add investment object i to farm invest list.
        Method is called for all investment activity that takes place during
        runtime.
    */
    void add(RegInvestObjectInfo i);
    /** Method adds investment to InvestList for testing in the planning
        calculation. The investment has an invest age of -1 and is therefore
        removed immediately after the calculation.
    */
    void addTest(RegInvestObjectInfo i);
    /** Method is called when the initial investment vector of each farm is filled.
        It is called by newFarm->addInvestments(...).
        @param cat_num catalog number in InvestCatalog of investment object
        @param quantity number of investments of object
        @param cap capacity associated with object
    */
    void add(int cat_num, int quantity, int cap);

    /// increase invest_age by 1 year
    void getOlder();
    /// remove investments that have reached their lifetime from FarmInvestList
    void removeInvestment();

    /** @return total investment expenditure for all objects except labour
    */
    int getNewInvestmentExpenditure();
    /** @return quantity of investments of catalog number t
    */
    int getInvestmentsOfCatalogNumber(int t);
    double getAverageAgeOfInvestmentsOfCatalogNumber(int t);
    int getNumberOfNewInvestmentsWithoutLabour();
    int getNewInvestmentsOfCatalogNumber(int t) {
        return newley_invested[t];
    }
    /** @return total land substitution
    */
    double getLandSubstitution();
    /** @return total labour substitution
    */
    double getLabourSubstitution();
    /** @return labour substitution of machinery and buildings
    */
    double getLSWithoutLabour();
    double getLSWithoutLabour1();
    /** @return total maintenance
    */
    double getTotalMaintenance();
    /** @total total residual equity share
    */
    double getTotalResEcShare();
    double getTotalResEcShareWithoutLabour();

    /** depreciate all investment objects
        investment objects keep track of their borrowed capital interest
        rate. This was implemented because of investment interest subsidies.
        As a consequence, borrowed capital interest costs are now determined
        on a per investment basis and not on a per farm basis anymore.

        @param repay pointer to repayment of farm
        @return total depreciation of farm
    */
    double depreciateCapital(double* repay,
                             double* fixincome,
                             double* value_added,
                             double* annuity,
                             bool sunkcostrelevant,
                             bool older,
                             const double& equity_interest);
    double getCapacityOfType(int);
    // returns the capacity of type normalized to the norm_labour_subtitutuion
    double getNormalizedCapacityOfType(int);
    double   getAverageCostOfNumber(int n) {
        return (*invest_cat)[n].getAverageCost();
    }
    double   getLiqEffectOfNumber(int n) {
        return (*invest_cat)[n].getLiqEffect();
    }
    double   getBoundEquityCapitalOfNumber(int n) {
        return (*invest_cat)[n].getBoundEquityCapital();
    }
    double   getAcquisitionCostsOfNumber(int n) {
        return (*invest_cat)[n].getAcquisitionCosts();
    }
    double   getLabourSubstitutionOfNumber(int n) {
        return (*invest_cat)[n].getLabourSubstitution();
    }

    /** After the data was read into the programme, all farm investments
        have a uniform investage. Since we have no information of the
        age of assets in reality, the asset age has to be determined
        randomly for each investment object. Depending on the initial
        investage, the remaining capital to be repayed differs, too.
    */
    void setAsynchronousInvestAge(int farm_age, double *assets,
                                  double *liquidity, double *lt_borrowed_capital,
                                  const double& equity_interest, const double& rel_invest_age);

    /// resets all elements of the newly_invested vector to zero
    void setBackNewleyInvestedVector();

    /// get total borrowed capital interest cost of farm
    double getOwnBcInterest();
    /** in case of interest subsidies for selected investment objects
        determine the costs of the policy for a politician.
    */
    double getPoliticalBcInterest();


    double getCostFactor(int prodgroup);

    /// check whether machinery is new
    bool newMachinery(int prodgroup);
    /// returns the number of invest types
    int getNumberOfInvestTypes();

    // Debug Routine
    void debug(string);

    void debug1(string,double);
};

/**  CheckAge class
     This class checks whether an investment object has reached
     its liftime. If it is true, then it is deleted from the
     InvestList.
*/
class CheckAge {
public:
    bool operator ()(const RegInvestObjectInfo& i) {
        if ((i.getInvestAge() == 0) || (i.getInvestAge() == -1) )
            return true;
        else
            return false;
    }
};
//---------------------------------------------------------------------------
#endif
