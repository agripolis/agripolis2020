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

#ifndef RegPlotInformationH
#define RegPlotInformationH
class RegPlotInfo;
class RegPlotInformationInfo {
public:
    RegPlotInformationInfo();
    double tc;
    double farm_tac;
    double tac;
    double pe;
    double alternative_search_value;
    RegPlotInfo* plot;
    double costs();
    double alternativeSeachCosts() const;
    bool operator<=(RegPlotInformationInfo&  p1);
    bool operator>(RegPlotInformationInfo&  p1);
};

#endif
