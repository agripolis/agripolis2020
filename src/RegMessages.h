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
#ifndef RegMessagesH
#define RegMessagesH

#include <string>
//class RegFarmInfo;
using namespace std;
// attributes of plot necessary for plotting
struct PlA {                  // Plot appearance
    int col;
    int row;
    int number;
    int state;
    int soil_type;
    string soil_name;
    int farm_id;
};

//---------------------------------------------------------------------------
#endif
