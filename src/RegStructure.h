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

#ifndef RegStructureH
#define RegStructureH
#include "RegGlobals.h"
#include "RegFarm.h"
/** RegRegionInfo class.
    The class manages the region, ie. the plots
    @author Kathrin Happe, Alfons Balmann, Konrad Kellermann
    @version June 2001
*/
class RegPlotInfo;
class RegFarmInfo;
class RegRegionInfo {
private:
    /// pointer to globals
    RegGlobalsInfo* g;

    vector<int> free_plots_of_type;
    vector<int> plots_of_type;
    vector<double> average_rent_of_type;
    vector<double> exp_average_rent_of_type;
    double average_rent;
    vector<double> average_new_rent_of_type;
    vector<double> exp_average_new_rent_of_type;
    double average_new_rent;
    RegRegionInfo* obj_backup;
    bool flat_copy;
    double total_tacs;
    double var_tacs;
    double fix_tacs;
public:
	int getRandom_contractLength();
	int getRandom_freePlot_initLand();
	int getRandom_freePlot_rentPlot();
    double calcPaidTacs();
    double calcTacs();
    void setTacs();

    /** choose a random plot out of the the_plots-vector and
        check for type of plot
        1: arable land  2: grass land
        caution!!!
        in all other classes, the type numbering is 0 for arable
        and 1 for grassland
    */
    void backup();
    void restore();
    void resetUpdate();
    void setUpdate();
    RegPlotInfo* getRandomFreePlotOfType(int type);
    RegPlotInfo* getRandomPlotOfType(int type);
    int getNumberOfFreePlots() {
        return free_plots.size();
    };
    void setRentedPlot(RegPlotInfo* p,RegFarmInfo* f);
    void setOwnedPlot(RegPlotInfo* p,RegFarmInfo* f);
    void setFarmsteadPlot(RegPlotInfo* p,RegFarmInfo* f);
    void releasePlot(RegPlotInfo* p);
    void occupyPlot(RegPlotInfo* p, RegFarmInfo* f);
    int cPoT(int i);
    void initPlotSearch();

	/// vector of pointer to all plots in the region
    vector<RegPlotInfo* > plots;
    vector<RegPlotInfo* > free_plots;

    // antonello
    int getNumberOfLandPlotsOfType(int type) {
        return plots_of_type[type];
    }
    int getFreeLandPlotsOfType(int type) {
        return free_plots_of_type[type];
    }


    void increaseFreePlotsOfType(int type) {
        if (type<g->NO_OF_SOIL_TYPES) free_plots_of_type[type]++;
    };
    void decreaseFreePlotsOfType(int type) {
        if (type<g->NO_OF_SOIL_TYPES) free_plots_of_type[type]--;
    };

    double getAvRentOfType(int type) {
        return average_rent_of_type[type];
    };
    double getAvRent() {
        return average_rent;
    };

    double getAvNewRentOfType(int type) {
        return average_new_rent_of_type[type];
    };
    double getAvNewRent() {
        return average_new_rent;
    };
    double getExpAvNewRentOfType(int type) {
        return exp_average_new_rent_of_type[type];
    };
    double getExpAvRentOfType(int type) {
        return exp_average_rent_of_type[type];;
    };
    void setExpAvNewRentOfType(int i,double t) {
        exp_average_new_rent_of_type[i]=t;
    };
    void setExpAvRentOfType(int i,double t) {
        exp_average_rent_of_type[i]=t;
    };
    void setNewRentFirstPeriod() {
        exp_average_new_rent_of_type=exp_average_rent_of_type;
        average_new_rent_of_type=average_rent_of_type;
    }
    /** CREATE PLOTS dynamically
        create array of plots of size NO_ROWS * NO_COLS
        The vector 'plots' contains the plots which
        are actually farmed. The size of the region, however
        is REGION_OVERSIZE times bigger than 'plots'
        This is done to achieve a better
        spatial distribution of plots to farms

        Inititally all plots are arable plots. Then, according
        to the number of grassland plots read in, the corresponding
        number of arable plots are randomly chosen and their
        type is changed to grassland.
    */
    void initialisation();

    /** Finish initialisation process
        In this method correct number of arable and grassland plots
        are set and equally, the number of free arable and grassland
        plots is reduced.
        Plots which are neither arable nor grassland are set 'dead'
        (black plots). They cannot  be activated for production
        purposes

        Thereafter, the total rent paid for rented arable and
        grassland plots is determined. Actually, this is only
        necessary, if we calculate the rent offer as a moving
        average of past rent offer. This is not implemented, though.
        Therefore it is NOT RELEVANT at the moment.
    */
    void finish();
    // average rent for the whole region
    void calculateAverageRent();
	void calculateAverageNewRent();

	void calcMaxRents();
	void updateMaxRents(double,int);
	void outputMaxRents();
	void resetMaxRents();

    /// modulate direct payments to plot
	void modulateDirectPayments(){};
    /// set regional ha payment
    void setHaPaymentPerPlot(double payment);

    RegPlotInfo* getRandomFreePlot();


    vector< vector <double> > contiguous_plots;
    int getContiguousPlotsOfType(int type) const;
    double getAvSizeOfContiguousPlotOfType(int type) const;
    double getStdSizeOfContiguousPlotOfType(int type) const;
    double getSizeOfContiguousPlotsOfType(int type);
    void countContiguousPlots();
    void clearTagsForContiguousPlots();
    void initVectorOfContiguousPlots();
    void countFreePlotsOfType();
    void countPlotsOfType();
    void setIdlePlotsDead();
    /// set n of dead plots to idle plots of soil type t
    void setDeadPlotsToType(int n,int t);
    RegRegionInfo() {
        flat_copy=false;
        obj_backup=NULL;
    };
    // constructor
    RegRegionInfo(RegGlobalsInfo* G);
    RegRegionInfo(const RegRegionInfo&,RegGlobalsInfo* G);
    // destructor
    ~RegRegionInfo();
};

//---------------------------------------------------------------------------
#endif
