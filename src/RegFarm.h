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

#ifndef RegFarmH
#define RegFarmH
#include <stdlib.h>

#include "RegMarket.h"
#include "RegLpD.h"
#include "RegProduct.h"
#include "RegInvest.h"
#include "RegGlobals.h"
#include "RegLabour.h"
#include "RegPlotInformation.h"
/** RegFarmInfo class.
    @short class defines the properties and actions of a single farm.
    @author Kathrin Happe, Alfons Balmann, Konrad Kellermann
    @version June 2001
*/

using namespace std;

class RegRegionInfo;
class RegPlotInfo;
class RegSectorResultsInfo;
class RegLpInfo;
class RegProductList;
class RegProductInfo;
class RegFarmInfo {
protected:
	void changeGeneration();

	double reinvestLUcap;
	bool restrict_invest;
	map<string, int> restrictedInvests;
	bool allow_invest;

	int GenChange_demograph;
	double YoungFarmer_years;
	double youngfarmerPay;
	bool youngfarmerPaid;
	bool youngfarmerMinSize;

	//soil service
	vector<double> avCarbons;  //mean
	vector<double> varCarbons; //variance
	vector<double> deltCarbons;
	vector<int> nPlots;

	int getRandomContractLength();
    // farmname
    string farm_name;
    // Globals
    RegGlobalsInfo* g;

    // ATTRIBUTES
    // farm closed or not
    // 0: not closed
    // 1: closed down because of opportunity costs
    // 2: closed down because of high opportunity costs, due to generation change
    // 3: closed down because of illiquidity
    int     closed;
    // farm class in previous period
    int      old_farm_class;
    // classification of farm
    int      farm_class;
    // indicates whether farm class has changed
    bool     farm_class_change;

    int      farm_type;
    int      farm_id;
    int      farm_colour;
    int      farm_age;
    bool     full_time;          // false(0) = pt, true(1) = ft
    int      switch_erwerbsform; // -1 = no change, 1 = ft->pt, 0 = pt->ft
    int 	   legal_type;

    // number of plots rented by farm
    int 	   number_of_plots;
    double   milk_quota;
    // input of land in production in ha
    double   land_input;
    // GA
    vector<double> cache_land_input_of_type;
    // antonello
    vector<double>   land_input_of_type;
    vector<double>   rented_land_of_type;
    // LP OPTIMISATION
    // lp is calculated the first time
    bool first_time;
	bool actual;
	vector<double> sp_estimation;
	double lp_result;

    /**   pointer to the plot rented last in previous period
          if last_rented_plot = NULL, then the farm has not received
          any plot in the last allocation round
    */
    RegPlotInfo* last_rented_plot;
    vector<double> lp_result_with_plotsn_new_plots_of_type;
    vector<double> lp_result_with_new_plot_of_type;
    vector<double> delta_profit_of_type;
    vector<RegPlotInformationInfo> wanted_plot_of_type;

    // RENT
    vector<double> initial_owned_land_of_type;
    vector<double> initial_rented_land_of_type;
    vector<double> initial_rent_of_type;
    double  initial_land;
    // rent expenditure for all rented plots
    double  farm_rent_exp;
    double  farm_tac;
    // total costs from farmstead to plots
    double  farm_distance_costs;
    // offer for wanted plot
    double  rent_offer;
    double adjusted_rent_offer;
    double unadjusted_rent_offer;
    double  opp_own_land;
    vector<int> land_capacity_estimation_of_type;
    vector<double> premium_estimation_of_type;
    vector<double> cache_sp_of_type;
    vector<double> cache_premium_of_type;
    vector<bool> cache_actual_of_type;

    // RESULTS, FINANCING and ASSETS
    double  financing_rule;
    double  assets;
    double  assets_at_production_wo_land;
    double  land_assets;
    double  lt_borrowed_capital;
    double  st_borrowed_capital;
    // borrowed capital of previous period
    double  lt_borrowed_capital_old;
    // input of capital in production
    double  capital_input;
    // depreciation of asset capital
    double  sunk_costs_labor;
    double  depreciation;
    double  equity_capital;
    double  economic_profit;
    double  liquidity;
    double  value_added;
    double  annuity;
    double  withdrawal;
    double  profit;
    double  farm_income;
    double  total_income;
    double  overheads;
    double  revenue;
    double  gm_products;
    double  gm;
    double  gm_agriculture;
    double  add_st_capital;
    double  premium;
    double  average_premium;
    double  income_payment_farm;
    double  modulated_income_payment;
    double  reference_income_payment_farm;
    double  reference_income_payment_farm_old;
    double  ecchange;

    // COSTS, REVENUES and EXPENDITURES
    double  farm_hired_labour_fix_pay;
    double  farm_hired_labour_var_pay;
    double  farm_fix_labour_income;
    double  farm_var_labour_income;
    double  farm_factor_remuneration_fix;
    double  farm_factor_remuneration_var;
    double  land_remuneration;
    double  farm_repayment;
    double  fix_costs;
    double  interest_costs;
    double  lt_interest_costs;
    double  lt_interest_costs_supported;
    double  st_interest_costs;
    double  st_interest_received;
    double  total_maintenance;

    //Decoupling
    // 0: no modulation  else Number of tranch.
    int display_modulation;
    double reference_premium;
    // OTHER
    int number_new_investments_wo_labour;
    // relative initial investment age (not relevant right now)
    double  rel_invest_age;
    // direct transfer payment
    double  bonus;
    // effect of sc on decision
    bool    sc_relevant;
    // indicates whether investments age or not
    bool    invest_get_older;
    // value that is responsible for different management qualities between farms
    double management_coefficient;
    ///
    double rent_offer_old;
    double tacs;


    // CONTAINERS and POINTERS
    /**   inum_vector records the number of investments in an investments object,
    e.g. 2 pig sties or 3 machinery 1
    inum_vector has the size of InvestList
    */
    vector<int >   inum_vector;
    /**   reference to market's product objects
    this is necessary to retrieve market prices for planning
    calculations, such as the equity interest rate (which is assumed
    to be the short term saving interest rate), or the current
    price for milk quota
    */
    vector<RegProductInfo>* product_cat;
    /**   reference to region invest catalog
    to retrieve information on investment objects
    */
    vector<RegInvestObjectInfo>* invest_cat;
    // List of pointer to plots the farm has
    list<RegPlotInfo* >  PlotList;

    // list of farm investments
    RegInvestList*       FarmInvestList;
    // list of farm products
    RegProductList*      FarmProductList;
    // pointer to Region
    RegRegionInfo        *region;

    // PRIVATE METHODS
    /**   The objects to invest in are determined in the
          MIP Lp. Capital constraints are adjusted (liquidity, borrowed
          capital and assets.

          @param ordernumber number of is this object in the InvestCatalog
          @param quantity denotes the number of investments in this object.
    */
    void    invest(int ordernumber, int quantity, bool test);
    /// farm is closed and land is released to the land market
    void    closeDown();
    /**   method does a planning calculation for the next period which. New
          investment objects and their costs effects are determined, and
          production is estimated on the basis of this 'anticipated'
          organisation. Function returns objective function value.
    */
    virtual double  anticipateNewPeriod();
    /**   withdraw capital. The minimum withdrawal is given in the
    dialog by a withdrawal factor per family labour unit. If the
          difference between the minimum withdrawal and the income is
          greater than zero then the actual value withdrawn is the
          minimum withdrawal plus 70% of the difference.
          @param inc income
    */
    double  withdrawCapital(double inc );
    /**   Determines the amount of capital to be borrowed in the Lp.
          \begin{description}
    financing_rule = liquidity
    + 0.7 * land_assets
    + 0.3 * total residual equity capital share of
    all investment objects
          \end{description}
    */
    void    calculateFinancingRule();
    /**   \begin{description}
    long term liquity available to the farm
    liquidty = equity capital
    - land_assets
    - total residual equity capital share in all
    investment objects
          \end{description}
    */
    void    calculateLiquidity();
   
    /// @return type 0:arable 1: grass
    bool disinvestPlotOfType(int type);
    /**   @param rent hectares of type rented; e.g. initial grasslandrented * Plotsize
          @param state state of plot
          @param type type of plot (grassland, arable)
          @return bool has the farm acquired enough land?
    */
    bool getInitialOwnedPlot(int type);
    bool getInitialRentedPlot(double rent,int type);

    void adjustVarCosts();

    /// returns costs saving factor for machinery
    double getCostFactorMachinery(int group) const;

    double getAverageNumberOfNeighbours() const;
    RegFarmInfo* obj_backup;
    bool flat_copy;

public:
	void setReinvestLUcap();
	double& getRefReinvestLUcap();
	map<string, int> getRestrictedInvests();
	bool restrictInvestments();
	bool allowInvest();
	void setAllowInvest(bool);
	void setRestrictInv();

	double& refYoungFarmerYears();
	void saveYoungFarmerPay();
	double getYoungFarmerPay() const ;
	void updateYoungFarmerLand();
	void updateYoungFarmer();

	int getGenerationChange() const {
		return GenChange_demograph;
	}
	double getRandomNormalRange(string, std::normal_distribution<>&, double, double);

	//soil service
	void initCarbons(vector<double>, vector<double>);
	void calAvCarbons();
	void calDeltaCarbons();
	void updateCarbons(RegPlotInfo, bool);
	void updateCarbons();
	RegProductList* getProductList();
	vector<double> getAvCarbons();
	vector<double> getVarCarbons();
	void setAvCarbons(vector<double> avc);
	void setVarCarbons(vector<double> vc);
	void updateYield();

	void increaseCarbonOfType(int,int);
	void decreaseCarbonOfType(int,int);

	//DCX
    //Farm location
	void setFarmStead();
 
	/**   constructor.
          @param reg pointer to region which manages plots
          @param G const pointer to global variables intialises globals
          @param PCat reference to ProductCatalog initialises the product list
          @param ICat reference to InvestCatalog
          @param lporig reference to original LP matrix
          @param pop farm type
          @param number farm number
          @param fc farm class
          @param farmname name of farm
    */
    RegFarmInfo() {
        flat_copy=false;
        obj_backup=NULL;
    };
    RegFarmInfo(RegRegionInfo * reg,
                RegGlobalsInfo* G,
                vector<RegProductInfo >& PCat,
                vector<RegInvestObjectInfo >& ICat ,
                RegLpInfo* lporig,
                short int pop,
                int number,
                int fc,
                string farmname,
                int farmerwerbsform);
    virtual RegFarmInfo* clone(RegGlobalsInfo* G,RegRegionInfo * reg,
                               vector<RegProductInfo> &PCat,
                               vector<RegInvestObjectInfo >& ICat);
    virtual RegFarmInfo* create();
    virtual RegFarmInfo* clone();
    /// Destructor
    ~RegFarmInfo();
    void backup();
    void restore();
    virtual void assign();
    // PUBLIC POINTERS
    /// pointer to plot the farm is on
    RegPlotInfo* farm_plot;
    /// pointer to plot that the farms wants to rent
    RegPlotInfo* wanted_plot;
    /// pointer to labour class that manages farm labour
    RegLabourInfo  *labour;
    // pointer to MIP
    RegLpInfo            *lp;

	// PUBLIC METHODS
    // Ga
    void cacheLandInput();
    void restoreLandInput();
    double getObjective(double land_input);
    double getObjective();
    double getValueOfPlots(vector<int>& plots);

    ///
    double getFarmTacsOfWantedPlotOfType(int type) const;
    double getTacsOfWantedPlotOfType(int type) const;
    void calculateEstimationForBidding();
    RegPlotInfo* getWantedPlotOfType(int type)const ;
    void calculateShadowPriceForLandOfType(int type,double premium_entitlement);

    /// returns the return for the products of a particular farm type
    double getGmOfFarmType() const;
    /** add investments to InvestList
        @param cn catalog number
        @param q  quantity of investments of number cn
        @param cap capacity of investment
    */
    void     addInvestments(int cn, int q, int cap);
    /// resets labour in labour object
    void    resetFarmVariables();
    void increaseLandCapacityOfType(int type,int no_of_plots);
    void decreaseLandCapacityOfType(int type,int no_of_plots);
    void setNoOfPlotsOfType(int type,int no_of_plots);
    int getNoOfPlotsOfType(int type) const;
    virtual void demandForLandOfType(int type,int count);
    /**   Demand function for land of 'type' arable or grassland.
          type = 0 (arable land), type = 1 (grassland)
          This method determines the 'wanted_plot' which the farm
          aims to rent. The resulting 'rent_offer' is multiplied
    by the RENT_ADJUST_COEFFICIENT = 0.75 which lowers the rent offer to
          reflect the fact that not the exact offer is paid later on
    */
    virtual void     demandForLand(RegPlotInfo* p);
    /// update var. costs in FarmProductList by the management_coefficient
    void     updateCosts();
    /**   set the actual rent which is paid for a plot.
          @param aALOffer average arable land offer
          @param ratioAl ratio of newly rented arable plots
    to total number of arable plots
          @param aGLOffer average grassland offer
          @param ratioGl ratio of newly rented grassland plots
    to total number of grassland plots

          For newly rented plots the rent paid is
          sqrt(rentOffer*averageOffer), and for plots which were
          newRent = sqrt[rentOffer*averageOffer]
          rented before by the farm the rent paid is adjusted
          according to
          newRent = pow(averageOffer,ratio)*pow(rentOffer, 1-ratio)

          For a more detailed description see RegManagerInfo .
    */
    void     adjustPaidRent(vector<double> av_offer_of_type,
                            vector<double> ratio_of_type);
    /// called at the beginning of iteration
    virtual void     newRentingProcess(int period);
    /**   Lp routine in which the number and kind of investments are
          determined from RegLp. 'inum_vector' returns the number of investments
          in each object. inum_vector has the size of invest_cat.
    */
    double   doLpInvest();
    /**   calculate the farm's financial results for this period
          \begin{enumerate}
    \item depreciation (fix_costs)
    \item gross margin of products (gm_products)
    \item profit = gm_products
    + bonus
    - total_maintenance
    - fix_costs
    - overhead costs
    - farm_rent_expenditure
    - long term interest costs
    +(- short term interest costs
    - farm hired labour variable pay)
    \item farm_income = profit
    + farm rent expenditure
    + short term interest costs
    + long term interest costs
    + farm hired labour variable pay
    + farm hired labour fix pay
    - farm fix labour income
    - farm variable labour income
    - short term interest earned
    \item opportunity costs of land
    \item land remuneration
    \item economic profit
    \item calculate Liquity
    \item update Financing Rule
    \item bonus payment
          \end{enumerate}

          The prices which are taken as the basis are this period's prices
          which are the expected prices of the last period.
    */
    void     periodResults(int period);
    void     periodResultsForRemovedFarms();

    /**   Planning routine in which the farm decides whether to continue
          production or to stop.
          The expected profit of the next period is
          \begin{description}
    expected profit = objective function value
    - farm rent expenditure
    - farm distance costs
    - total maintenance costs
    - overhead costs
          \end{description}
          Opportunity costs are determined as
          \begin{description}
    oppcosts = farm family labour * oppcostslabour
    + liquidity * interest_rate
    + oppcosts of land
    + oppcosts of quotarent
          \end{description}
          Oppcosts are increased by 15 percent when a generation change
          occurs.

          If oppcosts are less than the expected yield, the farm closes.
          @param sector sector is passed to add up sunk costs
    */

    virtual void     futureOfFarm( int period);
    /**   called after the farm is initiated. A random farm age is
          assigned, and the investment objects are given a random age.
    */
    void     setAsynchronousFarmAge();
    /// set states of rented plots and update farm variables
    void     setRentedPlot(RegPlotInfo* p, double rent, double tacs);
    /// set states of owned plots and update farm variables
    void     setOwnedPlot(RegPlotInfo* p);
    /// occupies and update farm variables
    void occupyPlot(RegPlotInfo* p);
    /// updates all Lp links
    void     updateLpValues();
    /// dec direct payment to previous value
    void     decDirectPayment(double payment);
    /// inc direct payment
    void     incDirectPayment(double payment);
    /// set direct payment
    void     setDirectPayment(double payment);
    double   getDirectPayment() const;
    /// set direct payment per plot
    void setDirectPaymentPerPlot(double payment);
    void modulateIncomePayment();
    void setOldReferenceDirectPayment(double payment) {
        reference_income_payment_farm_old=payment;
    }
    int getDisplayModulation() const {return display_modulation;};
    void setReferenceDirectPayment(double payment) {
        reference_income_payment_farm=payment;
    }
    double getOldReferenceDirectPayment()  const{
        return reference_income_payment_farm_old;
    }
    double getReferenceDirectPayment() const {
        return reference_income_payment_farm;
    }
    vector <double> getReferencePeriodProduction()  const;
	
    void fixReferencePeriod();

    void calculateReferencePeriod();

    /**   In this method it is checked whether it is profitable to
          let rented land on the land market. This is done in the
          methods disinvestPlotsOfType(int ). Investment objects which
          have reached their max. lifetime are removed from the
          InvestList. Their liquidation value is zero. Capacities are
          updated.
    */
    void    disInvest();
    int getCapacityOfType(int t) const {
        return static_cast<int> (FarmInvestList->getCapacityOfType(t));
    };
    /**   initial allocation of plots to farm, the number of plots is
          determined from the input file
          @return true if farm has acquired the number of plots given
          in the input file
    */
    bool     allokateInitialLand();
    /// count number of rented plots
    int      countRentedPlots() ;
    int      countNewRentedPlots() ;
    int countRentedPlotsOfType(int type) const;
    int countNewRentedPlotsOfType(int type) const;

    /// revenue of farm type specific products
    double getRevenueOfFarmType() const;

    double doLpWithPrice();
    double doLpWithPriceExpectation();
    double doProductionLp();

	//  PUBLIC VARIABLE ACCESS METHODS
    double   getLabourHa() const;
    double getFarmRentExpenditure()  const{
        return farm_rent_exp;
    }
    double getFarmNewRentExpenditure();

    // antonello
    bool getPreviouslyRentedByAgent(RegPlotInfo* p) const;
    double getFarmRentExpOfType(int type) const;
    double getFarmNewRentExpOfType(int type) const;
    double getAvRentOfType(int type) const;
    double getAvNewRentOfType(int type) const;
    int getNewInvestmentsOfCatalogNumber(int i) const {
        return FarmInvestList->getNewInvestmentsOfCatalogNumber(i);
    }
    int getInvestmentsOfCatalogNumber(int i)  const{
        return FarmInvestList->getInvestmentsOfCatalogNumber(i);
    }
    double getAverageAgeOfInvestmentsOfCatalogNumber(int i)  const{
        return FarmInvestList->getAverageAgeOfInvestmentsOfCatalogNumber(i);
    }
    int getNewInvestmentExpenditure()  const{
        return FarmInvestList->getNewInvestmentExpenditure();
    }
    double getRentOffer(RegPlotInfo*);
    double getRentOffer() const {
        return rent_offer;
    }
    void setMilkQuota(double m) {
        milk_quota = m;
    }
    double getMilkQuota() const {
        return milk_quota;
    }
    void setLandAssets(double l) {
        land_assets=l;
    }
    double getLandAssets() const {
        return land_assets;
    }
    void setInitialFamLu(double m);
    int getFarmType() {
        return farm_type;
    }
    void setFarmType(int ft) {
        farm_type = ft;
    }
    int   getFarmId() const {
        return farm_id;
    }
    void setFarmId(int fi) {
        farm_id = fi;
    }
    void setFarmColour(int fc) {
        farm_colour = fc;
    }
    void setFarmAge(int a) {
        farm_age = a;
    }
    int   getFarmAge() const {
        return farm_age;
    }
    double getAssets() const {
        return assets;
    }
    void setAssets(double a) {
        assets = a;
    }
    double getLtBorrowedCapital() const {
        return lt_borrowed_capital;
    }
    double getStBorrowedCapital() const {
        return st_borrowed_capital;
    }
    void setLtBorrowedCapital(double bc) {
        lt_borrowed_capital = bc;
    }
    double getLtBorrowedCapitalOld() const {
        return lt_borrowed_capital;
    }
    void setLtBorrowedCapitalOld(double bco) {
        lt_borrowed_capital = bco;
    }
    double getWithdrawal() const {
        return withdrawal;
    }
    void setWithdrawal(double w) {
        withdrawal = w;
    }
    double getCapitalInput() const {
        return capital_input;
    }
    void setCapitalInput(double ci) {
        capital_input = ci;
    }
    double getDepreciation() const {
        return depreciation;
    }
    void setDepreciation(double d) {
        depreciation = d;
    }
    double getFarmDistanceCosts() const {
        return farm_distance_costs;
    }
    void setFarmDistanceCosts(double dc) {
        farm_distance_costs = dc;
    }
    double getProfit() const {
        return profit;
    }
    void setProfit(double p) {
        profit = p;
    }
    double getLiquidity() const {
        return liquidity;
    }
    void setLiquidity(double l) {
        liquidity = l;
    }
    double getSunkCostsLabor() const {
        return sunk_costs_labor;
    }

    double getInterestCosts() const {
        return interest_costs;
    }
    void setInterestCosts(double ic) {
        interest_costs = ic;
    }
    double getLandInput() const {
        return land_input;
    }
    void setLandInput(double li2) {
        land_input = li2;
    }
    double getLandRemuneration() const {
        return land_remuneration;
    }
    void setLandRemuneration(double lr) {
        land_remuneration = lr;
    }
    double getEquityCapital() const {
        return equity_capital;
    }
    void setEquityCapital(double ec) {
        equity_capital = ec;
    }
    double getValueAdded() const {
        return value_added;
    }
    void setValueAdded(double va) {
        value_added = va;
    }
    double getAnnuity() const {
        return annuity;
    }
    void setAnnuity(double a) {
        annuity = a;
    }
    int getClosed() const {
        return closed;
    }
    int getFarmClosed()  const{
        return closed;
    };
    void setClosed(int c) {
        closed = c;
    }
    double getFarmHiredLabourVarPay() const {
        return farm_hired_labour_var_pay;
    }
    double getFarmHiredLabourFixPay() const {
        return farm_hired_labour_fix_pay;
    }
    double getFarmFactorRemunerationFix() const {
        return farm_factor_remuneration_fix;
    }
    double getFarmFactorRemunerationVar() const {
        return farm_factor_remuneration_var;
    }
    double getFarmRentExp() const {
        return farm_rent_exp;
    }
    double getStInterestCosts() const {
        return st_interest_costs;
    }
    double getLtInterestCosts() const {
        return lt_interest_costs;
    }
    double getStInterestReceived() const {
        return st_interest_received;
    }
    double getTotalIncome() const {
        return total_income;
    }
    double getTotalMaintenance() const {
        return total_maintenance;
    }
    double getGm() const {
        return gm;
    }
    double getGmProducts() const {
        return gm_products;
    }
    double getGmAgriculture() const {
        return gm_agriculture;
    }

    // antonello
    double getInitialOwnedLand() const;

    /// get units produced of product group (to calculate used land)
    double getUnitsProducedOfGroup(int) const;

    double getTotalLU(string cl) const;
    double getTotalLU() const;

    int  getNumberOfPlots() const {
        return PlotList.size();
    }
    int getNumberOfNewInvestmentsWoLabour() const {
        return number_new_investments_wo_labour;
    }
    double getUnitsOfProduct(int i) const;
	
    double getVarCostsOfProduct(int i) const ;
	
    int getOldFarmClass() const {
        return old_farm_class;
    }
    int getFarmClassChange() const;
    int getFarmClass() const{
        return farm_class;
    }
    RegPlotInfo* getFarmPlot() {
        return farm_plot;
    }
    double getEconomicProfit() const {
        return economic_profit;
    }
    string getFarmName() const {
        return farm_name;
    }
    RegLabourInfo* getLabourInfo() const {
        return labour;
    }

    // antonello
    void setInitialRentOfType(double r, int type) {
        initial_rent_of_type[type]=r;
    }
    void setInitialLand(double*);
    void setInitialLand(double);
    void setInitialOwnedLandOfType(double,int);
    void setInitialRentedLandOfType(double,int);
    /// release of a plot. Gives back an iterator to the next plot in the list
    list<RegPlotInfo* >::iterator releasePlot(list<RegPlotInfo* >::iterator p);

    void setRelInvestAge(double ria) {
        rel_invest_age = ria;
    }
    double getIncomePaymentFarm() const {
        return income_payment_farm;
    }
    double getModulatedIncomePaymentFarm() const {
        return modulated_income_payment;
    }
    double getAveragePremium() const {
        return average_premium;
    }
    double getBonus() const {
        return bonus;
    }
    double getOverheads() const {
        return overheads;
    }
    double getRevenue() const {
        return revenue;
    }
    int getFullTime() const {
        return (int) full_time;
    }
    int getSwitchErwerbsform() const {
        return switch_erwerbsform;
    }
    int getLegalType() const {
        return legal_type;
    }
    double getAssetsProdWoLand() const {
        return assets_at_production_wo_land;
    }
    double getLabSub() const;
    double getCapitalInputDEA() const;
    double getOutputGrossMarginDEA() const;
    double getStandardGrossMargin() const;
    double getGrossMargin() const;
    double getOutputRevenue() const;
    double getLandInputOfType(int type)  const{
        return land_input_of_type[type];
    };
    double getRentedLandOfType(int type)  const{
        return rented_land_of_type[type];
    };
    double getNewRentedLandOfType(int type) const;

    void setSecondPrice(vector<double>& secondprice_region);
    double getEcChange() const {
        return ecchange;
    }
    double getManagementCoefficient() const {
        return management_coefficient;
    }

    double getEconomicLandRent() const;
    int    getEconomicSizeClass() const;
    int 	 getFarmSizeClass() const;
    int    getFarmColour() const {
        return farm_colour;
    };

    void releaseRentedPlots();
	
    vector< vector <double> > contiguous_plots;
    int getContiguousPlotsOfType(int type) const;
    double getAvSizeOfContiguousPlotOfType(int type) const;
    double getSizeOfContiguousPlotsOfType(int type) const;
    double getSizeOfContiguousPlotOfType(int type,int i) const;

    void countContiguousPlots();
    void clearTagsForContiguousPlots();
    void initVectorOfContiguousPlots();
};

class CheckIfRented {
public:
    bool operator ()(const RegPlotInfo* f) {
        if (!f)
            return true;
        else
            return false;
    }
};

//---------------------------------------------------------------------------
#endif
