/*************************************************************************
* This file is part of AgriPoliS
*
* AgriPoliS: An Agricultural Policy Simulator
*
* Copyright (c) 2023, Alfons Balmann, Kathrin Happe, Konrad Kellermann et al.
* (cf. AUTHORS.md) at Leibniz Institute of Agricultural Development in 
* Transition Economies
*
* SPDX-License-Identifier: MIT
**************************************************************************/

#ifndef RegRLH
#define RegRLH

#include <tuple>
#include "RegFarm.h"
#include "RegGlobals.h"
#include "RegManager.h"

/**
  Data structure for Reinforcement Learning
*/

struct RLdata {
    int iter;
    //farm
    vector<vector<int>> restPlotsOfType; 
    int age;
    double liquidity;
    double management;
    //invest: id, num, restlife
    map<int, pair<int, double>> restInvests;
    vector<double> recentRents;
    
    //sector
    int nfarms10km;
    vector<int> nfreeplots10km;
    //vector<double> avRents;
    vector<double> avNewRents;
};

RLdata getRLdata(RegFarmInfo*, RegManagerInfo*);
void initzmq();
void output(RLdata, RegManagerInfo*, string);
#endif
