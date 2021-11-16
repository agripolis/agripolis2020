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

#ifndef OutputControlH
#define OutputControlH
#include <iostream>
#include <vector>
#include <fstream>
#include <vector>
#include "RegGlobals.h"
using namespace std;
class OutputControl {
public:

    OutputControl(RegGlobalsInfo*);
    OutputControl(const OutputControl&,RegGlobalsInfo*);
    ~OutputControl();

    void openFarmdata(string);
    // Method to acess a specified col of a specified period
    vector< double> getColOfPeriod(int,unsigned int);
    double getSumColOfPeriod(int,int);
    vector< vector<double> > getColOfAllPeriods(int);
    void close();
    void groupByProfit(vector<double>&,int);
    void groupBySize(vector<double>&,int);
    vector<double> getInfoOfSpecificFarm(vector<int>& cols,int farm_id,int period);
    int getActualPeriod();
    double getAvColOfPeriod(int col,int period,int farmtype,int full_time);
    int getCountFarmsOfPeriod(int period,int farmtype,int full_time);
    string policy_input;
    string getPolicySettings(int);
	string getPolicySettingsSep(int);
	
private:
    RegGlobalsInfo* g;
    bool op;
    ifstream out;
    vector< vector <double> > cache;
    vector<int> cacheNumber;
    // -1 wenn nicht , Nummer sonst
    int isInCache(int);
    vector<int> number_of_farms;
    vector<long> begin_of_periods;
    vector< vector <int> > sort_by_profit;
    vector< vector <int> > sort_by_farmsize;
    void quicksort(vector<double>&,vector<int>&,int,int);
    void swap(vector<double>&,vector<int>&,int,int);
    void swap(vector<double>&,int,int);
    int partition( vector<double>&,vector<int>&,int,int);
    void groupBy(vector<double>&,int,int);
};
#endif
