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

//---------------------------------------------------------------------------
#ifndef RegDataH
#define RegDataH

#include <fstream>
#include "RegFarm.h"
#include "RegGlobals.h"
#include "RegInvest.h"
#include "RegProduct.h"
#include "RegMarket.h"

/** RegDataInfo class.
    The class manages the data output for farms and the region.
    @author Kathrin Happe, Alfons Balmann, Konrad Kellermann
    @version June 2001
*/

class RegDataInfo {
private:
	void printRegionSoilservice(ofstream& ofs, list<RegFarmInfo*> farmList, int period);
    //Globals
    RegGlobalsInfo* g;

	RegMarketInfo* market;

    /// output stream for farm data
    ofstream farmout;
    /// output stream for farm  investment data
    ofstream farminvest;
    /// output stream for farm production data
    ofstream farmprodout;
    /// output stream for sector data
    ofstream secout;
    /// output stream for farm cost data
    ofstream varcostsout;
    /// output stream for sector var costs data
    ofstream secvarcostsout;
    /// output stream for environmantal data usage at a farm level
    ofstream envusageout;
    /// output stream for threatened species
    ofstream speciesout;
    /// output stream for sector price data
    ofstream secpricesout;
    /// output stream for expected sector price data
    ofstream exsecpricesout;
    /// output stream for policy data
    ofstream policyout;
    /// condensed sector output
    ofstream condsecout;
    /// condensed farm output
    ofstream condfarmout;


    vector<string> sector_names;
    vector<double> sector_values;
    int counter;
    int farm_results_c;
    vector< vector <double> > farm_results;
public:
	void scenarioDate(ofstream&);
	//soil service 
	void initSoilservice(list <RegFarmInfo*> farmList);
	void printSoilservice(list <RegFarmInfo*> , int);

    /// output farm data
    void printFarmResults(const RegFarmInfo* farm,
                          const vector<RegInvestObjectInfo >& invest_cat,
                          const vector<RegProductInfo >& product_cat,
                          int period,int c);
    void cacheFarmResults(const RegFarmInfo* farm,
                          const vector<RegInvestObjectInfo >& invest_cat,
                          const vector<RegProductInfo >& product_cat,
                          int period);
    void printFarmVarCosts(const RegFarmInfo* farm,
                           const vector<RegProductInfo>& product_cat,
                           int period);

    void initFarmVarCosts(const vector<RegProductInfo>& product_cat);
    void initContiguousPlotsOutput(const vector<RegProductInfo>& product_cat);
    void printContiguousPlotsOutput(const RegFarmInfo* farm, int period);
    void printRegionContiguousPlotsOutput(const RegRegionInfo* farm, int period);

    void openFarmOutput();
    void closeFarmOutput();
    void openFarmStandardOutput();
    void closeFarmStandardOutput();
    void initLegalTypeOutput(vector<RegInvestObjectInfo>& invest_cat,vector<RegProductInfo>& product_cat);
    void initialisation(vector<RegInvestObjectInfo>&,vector<RegProductInfo>&,RegEnvInfo* Env);
    void initFarmResults(vector<RegInvestObjectInfo>&,vector<RegProductInfo>&);
    void initSectorResults(vector<RegInvestObjectInfo>&,vector<RegProductInfo>&);
    void initSpeciesOut(RegEnvInfo* Env);
    void printSpeciesOut(RegEnvInfo* Env, int iteration_h);
    void printSectorResults(const RegSectorResultsInfo& ,
                            vector<RegProductInfo>& ,int);
    void printLegalTypeResults(const RegSectorResultsInfo& ,vector<RegSectorResultsInfo*>& sector_type,
                         vector<RegProductInfo>& ,vector <RegInvestObjectInfo >&,int);
    void initSectorVarCosts(const vector<RegProductInfo>& product_cat);
    void initSectorPrices(const vector<RegProductInfo>& product_cat);
    void initExpectedSectorPrices(const vector<RegProductInfo>& product_cat);
    void printSectorVarCosts( const RegSectorResultsInfo& sector,
                              vector<RegProductInfo>& product_cat, int period);
    void printSectorPrices( const RegSectorResultsInfo& sector,
                            vector<RegProductInfo>& product_cat, int period);
    void printExpectedSectorPrices( const RegSectorResultsInfo& sector,
                                    vector<RegProductInfo>& product_cat, int period);
    void printFarmInvestment(const RegFarmInfo* farm,
                             const vector<RegInvestObjectInfo >& invest_cat, int period);
    void initFarmInvestment(const vector<RegInvestObjectInfo >& invest_cat);

    void openPolicyOutput();
    void closePolicyOutput();
    void initPolicyOutput();
    void printPolicyOutput(string);
    void initCondensedSectorOutput();
    void printCondensedSectorOutput(const RegSectorResultsInfo& ,vector<RegProductInfo>& product_cat,vector <RegInvestObjectInfo >& invest_cat, int period);
    void initFarmProduction(vector<RegProductInfo>& product_cat);
    void printFarmProduction(const RegFarmInfo* farm, vector<RegProductInfo>& product_cat, int period);
    void printEnvDataUsage(const RegFarmInfo* farm, vector<RegProductInfo >& product_cat, int period);
    void initFarmEnvDataUsage();

    vector<double> getSectorValues() {
        return sector_values;
    }
    vector<string> getSectorNames() {
        return sector_names;
    }

     RegDataInfo(RegGlobalsInfo*, RegMarketInfo*);
     ~RegDataInfo() {}
};

#endif
