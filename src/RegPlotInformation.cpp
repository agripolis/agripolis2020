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

#include "RegPlotInformation.h"
    RegPlotInformationInfo::RegPlotInformationInfo() {
        tc=0;
        tac=0;
        farm_tac=0;
        pe=0;
        plot=0;
        alternative_search_value=0;
    }
    double
    RegPlotInformationInfo::costs() {
        return tc + farm_tac;
    };
    double
    RegPlotInformationInfo::alternativeSeachCosts() const{
        return tc + alternative_search_value;
    };
    
	bool
    RegPlotInformationInfo::operator<=(RegPlotInformationInfo& p1) {
        return costs() <= p1.costs();
    }
    bool
    RegPlotInformationInfo::operator>(RegPlotInformationInfo& p1) {
        return costs() > p1.costs();
    }


