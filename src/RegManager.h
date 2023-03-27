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

#ifndef RegManagerH
#define RegManagerH
#include <list>
#include <algorithm>
#include "RegResults.h"
#include "RegStructure.h"
#include "RegProduct.h"
#include "RegInvest.h"
#include "RegMarket.h"
#include "RegData.h"
#include "RegFarm.h"
#include "RegGlobals.h"
#include "OutputControl.h"
#include "Evaluator.h"
#include "RegEnvInfo.h"
/** RegManagerInfo class.
    This class is 'the brain' of the programme. It manages the all necessary
    classes and data flows.
    @author Kathrin Happe, Alfons Balmann, Konrad Kellermann
    @version June 2001
*/

class RegManagerInfo {
public:
    /// constructor
    RegManagerInfo(RegGlobalsInfo*);
    RegManagerInfo() {
        flat_copy=false;
        obj_backup=NULL;
    };
    /// constructor
    virtual RegManagerInfo* clone(string name);
    virtual   void assign();
    /// destructor
    ~RegManagerInfo();

    RegGlobalsInfo* getGlobals() {
        return g;
    }

    RegSectorResultsInfo* getSector() {
        return Sector;
    }
    RegSectorResultsInfo  *Sector;
    vector<RegSectorResultsInfo*> sector_type;
    RegEnvInfo* Env;

	void calcMaxRents();
	void testLivestockInvRand();

    list<RegFarmInfo*> getFarmList() const {
        return FarmList;
   }

   int getNoOfFarms() {
        return FarmList.size();
    }
    int f,t,n;
    void    increaseLandCapacityOfType(int farm,int type,int no_of_plots);
    void    increaseLandCapacityOfTypel(int farm,int type,int no_of_plots);
    void    init();
    void    initRegion();
    void    initInvestmentCatalog();
    void    initMarket();
    void    setPremiumColRow();
    void    initPopulations();
    void    initMatrix();
    void    initMatrix0();
    virtual RegLpInfo*    createMatrix();

    virtual RegFarmInfo* createFarm(RegRegionInfo * reg,
                                    RegGlobalsInfo* G,
                                    vector<RegProductInfo >& PCat,
                                    vector<RegInvestObjectInfo >& ICat ,
                                    RegLpInfo* lporig,
                                    short int pop,
                                    int number,
                                    int fc,
                                    string farmname,
                                    int farmerwerbsform);
    void    initOutput();
    void    initGlobals(bool read);
    void    initGlobals2();
    void    initEnv();
    void    initEnv2();

	void	initCommandlineOptions();

    /// Simulation of one period
    virtual void step();
    void stepwhile();
    void simulate();
    int getIteration() {
        return iteration;
    };
    virtual void backup();
    virtual void restore();
    void printShadowPrices(int nop);
    virtual RegManagerInfo* create();
    virtual RegManagerInfo* clone();
    int getFreeLandPlotsOfType(int type) {
      return Region->getFreeLandPlotsOfType(type);
    }

	//NASG
	vector<double> NASG_maxRentOfTypes;
	double NASG_UAA;
	double NASG_avArea;
	void updateNASG();
	double NASG_RentOffer(double, RegFarmInfo* pfarm, int landtype);

	void outputManageCoeffDistrib();
    void outputMap(std::map<int, int>&);
	void outputFarmAgeDists();

	void updateYoungFarmerLand();
    RegRegionInfo* getRegion() const {
        return Region;
    }

    vector<RegInvestObjectInfo> getInvestCatalog() const {
        return InvestCatalog;
    }
    
protected:
    RegFarmInfo* RLfarm = nullptr;
    double get_beta();

	void outputRestrictedInvs(RegFarmInfo*);
	int nfarms_restrict_invest;

	void output_av_rents();
	void initRandomDistribs();

	//soil service
	void UpdateSoilserviceP();  //after production  
	void UpdateSoilserviceLA() ; // after land allocation

    bool flat_copy;
    string name;
    RegManagerInfo* obj_backup;
    void setLpChangesFromPoliySettings();
    void setLpChangesFromPoliySettingsNaming();
    string current_policy;
    
    int bidcount;
    
    /// pointer to globals
    RegGlobalsInfo* g;

    int  iteration;

    // The major classes are created dynamically  because  their
    // actual size and specification can only be determined after
    // the input data was read.

    /// pointer to Region
    RegRegionInfo           *Region;
    /// pointer to Output (farm and sector)
    RegDataInfo             *Data;
    /// pointer to market
    RegMarketInfo           *Market;
    /// instance of Lp
    RegLpInfo                *Mip;
    /// Policy Settings
    OutputControl* Policyoutput;
    /// Evaluator and Parser
    Evaluator* evaluator;
    /** Catalog of possible investments
        this is a vector because it is not expected to change much
        on farm level it is <list> because of easier insertion and removal
    */
    vector <RegInvestObjectInfo >  InvestCatalog;
    /// list of pointers to farms in region
    list   <RegFarmInfo* >   FarmList;
    /// list of pointers to removed farms
    list   <RegFarmInfo* >   RemovedFarmList;

    // methods

    /** In this function all figures to  new investment objects
        which affect the MIP matrix are calculated. This is necessary because
        the interest rates may change.
        \begin{itemize}
        \item aac average annual costs of investment
        \item liquidity effect of investment - this is the selffinanced share
        of the acquisition costs. The figure is relevant for the financing
        rule constraint because it is the part of the investment costs
        which is to be financed by the farm.
        \item average bound equity capital -  this is the average equity capital
        bound by the investment during its lifetime. The figure is relevant
        for the liquidity situation of a farm.
        \end{itemize}
    */
    void    updateInterestDependentValues();
    // reset labour at end of period
    void    ResetPeriodLabour();

    // does necessary resetting activity to start a new iteration
    void    PreparationForPeriod();
    /** central method for land allocation
        \begin{itemize}
        \item \textbf{management coefficient}
        In iteration = 0 set effect of management coefficient. This has
        the effect that farms operate with different costs that are between
        +- 5% of the variable costs from the input file. The coefficient
        itself is chosen randomly for each farm . It is assumed
        that successful farmers have lower variable costs. The structure of
        costs, however is not changed, ie. the var cost reduction or
        increase is the same for all production activities of one farm.
        \item \textbf{renting process}
        The renting process is organised as an auction. In this auction,
        farms give bids for individual plots they want. The bids are collected
        and whoever bids highest receives the desired plot. Note, that
        it is not the same plot that farms are bidding for. It is
        only the offers which are compared.
        Arable and grassland can be rented separately. The renting is
        for one plot at a time. The process starts with arable land.
        LandAllocation() calls rentOnePlot() in which farms determine
        their wanted plot and make a bid for this plot. The farm with
        the highest bid is allocated the plot (setRentedPlot()). The
        renting process runs until there are no more positive bids.
        \item \textbf{rent adjustment}
        Rents for plots rented before are adjusted. This is done because
        rental contracts are hardly ever for one year only. In the model
        it is assumed that farms keep their rented land until it is no
        more profitable. But, the rent paid for these plots is adjusted
        every period.
        \begin{itemize}
        \item \textbf{newly rented plots}:
        newRent = sqrt(rentOffer*averageOffer),
        ie. the rent paid shifts towards the higher figure
        \item \textbf{`old' plots}:
        Here, the adjusted rent depends on the share of newly
        rented plots of a type. The higher the share, the more
        the rent adjustment is towards the higher figure
        newRent = pow(averageOffer, share)
        * pow(rentOffer, 1-share)
        \end{itemize}
        \item \textbf{calculate average rent in region}
        On the basis of the adjusted rents, the average regional rent
        is calculated.
        \end{itemize}
    */
    void    LandAllocation();
    double    RentStatistics();
    void    CapacityEstimationForBidding();
    /// modulation of direct payments
    void    ModulateDirectPayments();
    /// determine region per ha payment
    void    RegionalPerHaPayment();
    /// cost Adjustment
    void    CostAdjustment();
    /** calls doLpInvest() which solves MIP under consideration of investments
     */
    void    InvestmentDecision();
    /** doLpWithPrice() without option to invest, but the option to invest is
        included in the production decision
    */
    double    Production();
    /** set total land input in region, and determine price changes and
        adjustments.
    */
    void    UpdateMarket();
    /** compute period results for each farm
     */
    void    FarmPeriodResults();
    /** period results of removed farms
     */
    void    RemovedFarmPeriodResults();
    void    Disinvest();
    /** for each farm call function that does  planning calculation
        with respect to continuation of the farm
    */
    void    FutureOfFarms();
    /// upgrade species calculation and print results
    void    EnvSpeciesCalc();
    /// output farm data
    void    FarmOutput();
    /// compute sector results (add up all farm data)
    void    SectorResults();
    void    SectorResultsAfterDisinvest();
    /// output sector results
    void    SectorOutput();
    void    ProcessMessages();
    /** if farms are closed, remove them from FarmList
        We have thought about changing this to keep the closed farms in the
        list and have them receive rent income
    */
    void    RemoveFarms();

    /** @param type arable land or grassland
        @param offercount count the number of plots of type already rented
        @param totaloffer sum of all offers of type
        @return maxoffer the maximum bid for the plot allocated in this round
        \begin{itemize}
        \item each farm determines the desired plot of type
        \item the maximum bid for the wanted plots is determined. If farms
        have the same bid, then a pointer to the farms are stored in the
        list equal_bidder. As soon as one bid is higher, the list is
        cleared.
        \item Whoever has the highest bid receives its wanted plot
        (setRentedPlot()) and the counter offercount is increased and
        the rent offer is added to totaloffer.
        \end{itemize}
    */
    double rentOnePlot(vector<int>& count_rented_plots_of_type,int type);
    void setPolicyChanges();

    void readPolicyChanges0();
    void initPolicy();
	
	void setIncreasePrices();

    void readPolicyChanges();
    void setDecoupling();
    void calculateReferencePaymentPerFarm();
    void setRegionalDecoupling();
    void setFullyDecoupling();
    void setFarmspecificDecoupling();
    void setPremium();
    bool setModulationData();
    void calculateExpectedRentalPriceChange();

    double released_plots;
    double released_plots_IF;
    double released_plots_CF;
    double rented_plots_CF;
    double rented_plots_IF;
    double CF_to_CF;
    double CF_to_IF;
    double IF_to_CF;
    double IF_to_IF;
    double stay_CF;
    double stay_IF;
    double paid_tacs;
    double total_tacs;
    double stay_at_prev_owner;
    double stay_at_prev_owner_because_of_tacs;
private:
	bool debug;
};

/**  The class checks if a farm is closed. A pointer to a
     farm object is passed. The operator is used in the
     STL function remove_if which scans the FarmList
     for all farms which are closed.
*/
class CheckIfClosed {
public:
    bool operator ()(const RegFarmInfo* f) {
        if (!f)
            return true;
        else
            return false;
    }
};

//---------------------------------------------------------------------------
#endif
