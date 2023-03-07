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

#ifndef RegPlotH
#define RegPlotH

#include "RegMessages.h"
#include "RegGlobals.h"
#include "RegFarm.h"
#include "RegPlotInformation.h"

/** RegPlotInfo class.
    The class manages each individual plot in the region
    @author Kathrin Happe, Alfons Balmann, Konrad Kellermann
    @version June 2001
*/

class RegPlotInfo {
private:
	//Soil service
	double carbon;

    int contract_length;
    //Globals
    RegGlobalsInfo* g;
    /// manages visual state of plot
    /** state a plot can take
        \begin{itemize}
            \item -1: dead plot
            \item 0: idle  plot
            \item 1: rented plot
            \item 2: plot is farmstead
            \item 3: plot is owned land
        \end{itemize}
    */
    struct PlA PA;
    /// distance from plot to farmstead of owner (tenant)
    double   distance_from_agent;
    /// rent paid for this plot
    double   rent_paid;

    /// second highest offer
    double second_offer;

    /// entitlement for direct payment
    double   payment_entitlement;
    /// initial payment entitlement
    double   initial_payment_entitlement;
    /// plot is rented by which farmNumber
    int      rented_by_agent;
    int      rented_by_legal_type;
    int      previously_rented_by_legal_type;
    double previously_paid_by_agent;
    /// number of farm this plot is farmstead of
    int      occupied_by_agent;
    /// plot is rented by which farmNumber
    int      previously_rented_by_agent;
    /// number of farm this plot is farmstead of
    int      previously_occupied_by_agent;
    /// distance costs associated with this plot
    double   distance_costs;
    ///flag for the desicion if plot is newley rented
    bool newley_rented;
//    bool Draw;
//    HWND Handle;
    double paid_tacs;
    double tacs;
    /// vector to store the free plots in the region
    /// this is done on individual level because the vector is then sorted  according the value of a plot
    vector< vector<RegPlotInformationInfo> > free_plots;
    vector<RegPlotInfo*> contiguous_plot;
    /// reference to pointer of plots
    vector <RegPlotInfo*>* region;
    /** check status of plot n whether it is of type type
        @param type 1: arable land, 2: grassland
        @param n in dex of plots in Region
        @return bool true: if plot is of type
                     false: if plot is not of type
    */
    bool checkPlot(int n, int type);
    bool tagv;

    //Fast plot search
    vector < vector<RegPlotInfo*> > pl_n;
    vector<int> plot_p;
    vector < vector<double> > dist;

    bool update;
    int plot_id;
    RegPlotInfo* obj_backup;
public:
	//soil service
	double getCarbon();
	void setCarbon(double);

	void finish(RegPlotInfo& rh);
    void backup();
    void restore();
    bool getUpdate() {
        return update;
    };
    void setUpdate(bool update) {
        this->update=update;
    }
    void resetPlotPointer() {
        for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
            plot_p[i]=0;
        }
    };
    void initFreePlots(RegFarmInfo* farm);
    void clearContiguousPlot();
    void untagContiguousPlot();
    RegPlotInformationInfo findMostPreferablePlotOfType(int type);
	RegPlotInformationInfo findMostPreferablePlotOfType(int type, RegFarmInfo* farm);

    RegPlotInformationInfo calculateValue(RegPlotInfo* farm_plot, RegFarmInfo* farm);
    RegPlotInformationInfo getValue(RegPlotInfo* plot, RegFarmInfo* farm);
    int identifyContiguousPlot(bool check_soiltype, bool ckeck_farmnumber,bool ckeck_previous_farmnumber,int farmnumberob=-1,bool mark_update=false);
    /** constructor
        @param G pointer to globals
        @param pos number of plot
        @param xcol column position of plot
        @param yrow row position of plot
        @param reference to vector of pointers to plots

        Region is initialised with reference to vector of
        pointers to plots passed from RegRegionInfo
    */
    RegPlotInfo(RegGlobalsInfo* G,
                int pos,
                int xcol,
                int yrow,
                vector<RegPlotInfo*>& r);
    RegPlotInfo(RegPlotInfo& rh,
                RegGlobalsInfo* G,
                vector<RegPlotInfo*>& r);
    /// destructor
    ~RegPlotInfo();

   int getId() {
        return plot_id;
    };
    void setId(int id) {
        plot_id=id;
    };
    double getPaidTacs() {
        return paid_tacs;
    };
    void setPaidTacs(double t) {
        paid_tacs=t;
    };
    double getTacs() {
        return tacs;
    };
    void setTacs(double t) {
        tacs=t;
    };

    /// itendify whether plot belongs to same farm
    int identifyPlotsSameState();
    int identifyPlotsSameStateAndFarm(const int& farmnumber);
    /// itendify whether plot belongs to same farm
    int identifyPreviousPlotsSameState(const int& farmnumber);
    double calcShareOfContiguousPlot(int farmnumber);
    int countContiguousPlots(int farmnumber);
    int countContiguousPlots();
    /// flag for counting algorithm
    bool checked;
    /** method that returns pointer to free plot of type
        (1:arable, 2:grassland).
        The algorithm is such that first the direct neighbours
        are checked and then the horizon is rotated and the
        4 diagonal neighbours are checked
    */
    RegPlotInfo* findFreePlotOfType(int type);

//    bool hasNeighbours(const int& farmid);
    double calculateDistance(const RegPlotInfo* P);
    double calculateDistanceCosts(const RegPlotInfo* P);
    /** The distance costs from plot to agent are calculated
        Here, the distance measure distance_from_agent is transformed
        to be expressed in terms of kilometres.
        @return distance (cardinal points)
    */
    void setDistanceFromAgent(double dfa);
    void setContractLength(int cl) {
        contract_length=cl;
    };
    int getContractLength() {
        return contract_length;
    };
    void decreaseContractLength() {
        contract_length--;
    };

	int getState()const {
        return PA.state;
    }
    /** change state of a plot
        @param s state
        @param farm pointer to farm that changes the state
        @param colour colour of plot

        State and colour are changed in the PA struct, that
        manages the visual state of the plot
    */
    void setState(const int passState,
                  RegFarmInfo* farm,
                  int colour);

    double getDistanceFromAgent() const {
        return distance_from_agent;
    }
    void setRentPaid(double rp) {
        rent_paid = rp;
    }
    double getRentPaid() const {
        return rent_paid;
    }

    void setSecondOffer(double so) {
        second_offer = so;
    }
    double getSecondOffer() const {
        return second_offer;
    }


    void setPaymentEntitlement(double pe);
    double getPaymentEntitlement() const {
        return payment_entitlement;
    }
    void setRentedByAgent(int rba) {
        rented_by_agent = rba;
    }
    int getPreviouslyRentedByLegalType() {
        return previously_rented_by_legal_type;
    };
    int getRentedByLegalType() {
        return rented_by_legal_type;
    };
    int getRentedByAgent() const {
        return rented_by_agent;
    }
    int getPreviouslyRentedByAgent() const {
        return previously_rented_by_agent;
    }
    void setOccupiedByAgent(int oba) {
        occupied_by_agent = oba;
    }
    int getOccupiedByAgent() const {
        return occupied_by_agent;
    }
    int getPreviouslyOccupiedByAgent() const {
        return previously_occupied_by_agent;
    }
    double getDistanceCosts() const {
        return distance_costs;
    }
    int getCol()const {
        return PA.col;
    }
    int getRow()const {
        return PA.row;
    }
    int getNumber()const {
        return PA.number;
    }
    bool getNewleyRented() const {
        return newley_rented;
    }
    void setNewleyRented(bool n) {
        newley_rented=n;
    }
    void setSoilType(int soil_type) {
        PA.soil_type=soil_type;
    };
    void setSoilName(string soil_name) {
        PA.soil_name=soil_name;
    };
    int getSoilType() {
        return PA.soil_type;
    }
    string getSoilName() {
        return PA.soil_name;
    }
    void tag() {
        tagv=true;
    }
    void unTag() {
        tagv=false;
    }
    bool getTag() {
        return tagv;
    }
    void finish(RegRegionInfo* r);

//DCX
    int getFarmId() {
        return PA.farm_id;
    }
};

//---------------------------------------------------------------------------
#endif
