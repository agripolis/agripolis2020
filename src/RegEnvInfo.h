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

#ifndef RegEnvInfoH
#define RegEnvInfoH

#include "RegGlobals.h"

#include "RegMarket.h"
class RegFarmInfo;
class RegMarketInfo;
using namespace std;
class RegEnvInfo {
public:
    RegEnvInfo(RegGlobalsInfo*);
    RegEnvInfo(const RegEnvInfo&,RegGlobalsInfo*);
    void backup();
    void restore();
    void initEnv();
    void initEnvOutput(RegMarketInfo* market);
    void associateActivities(list<RegFarmInfo*>&);
    // soil type, farm, cont_plot, activitie
    vector< vector< vector < vector < double > > > > results;
    vector< vector< double > > mean;
    vector< vector< double > > std;
    vector< vector< int > > count;
    void sumHaProducedByHabitat(int mktID_h, double producedHa_h);
    void resetHaProducedByHabitat();
    void calculateCCoefficients();
    void calculateSpeciesByHabitat();
    void addMktIDToHabitat(int mktID_h, int habitatID_h);
    int getNHabitats() {
        return nHabitats;
    };
    vector <string> getHabitatLabels() {
        return habitatLabels;
    };
    double getProducedHaByHabitat(int habitatID_h) {
        return haByHabitat[habitatID_h];
    };
    double getSpeciesByHabitat(int habitatID_h) {
        return speciesByHabitat[habitatID_h];
    };
    double getAvSizeOfContPlotsOfType(int t) {
        return av_size_of_contiguous_plots[t];
    }
    double getTotalLandOfType(int t) {
        return total_land_of_type[t];
    }

    vector< vector<int> > associated_activities;
    vector< vector<string> > sassociated_activities;
private:
    RegEnvInfo* obj_backup;
    void calculateStatistics();
    void print();
    RegGlobalsInfo* g;
    vector<double> av_size_of_contiguous_plots;
    vector<double> total_land_of_type;
    vector< vector< double> >calculateProd(int,RegFarmInfo*);
    vector< vector<int> > associated_soils;
    vector< vector<double> > associated_q;
    vector<int> no_acts;
    vector<int> no_soils;
    int groups;
    vector<int> residual_activities;
    int nHabitats;
    double zCoef; //z coefficient for environmantal species indicator
    vector<string> habitatLabels;
    vector<double> speciesByHabitat;
    vector<double> haByHabitat;
    vector<double> cCoeffByHabitat;
    vector < vector <int> > mktIDsByHabitat;

    int iteration;
};
//---------------------------------------------------------------------------
#endif
