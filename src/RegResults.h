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
// RegResults.h
// basic regional model
// Author: Kathrin Happe, University of Hohenheim
// Copyright (c) 1999

//---------------------------------------------------------------------------
//	RegSector -- agricultural sector class
//---------------------------------------------------------------------------

#ifndef RegSectorResultsH
#define RegSectorResultsH

#include <vector>
#include <list>
#include <iostream>
#include "RegGlobals.h"
#include "RegProduct.h"
#include "RegInvest.h"
#include "RegFarm.h"
class RegSectorResultsInfo {
private:
    RegGlobalsInfo* g;
    vector<double>  total_units_produced;
    vector<double>  total_varcosts_of_product;
    RegSectorResultsInfo* obj_backup;
   
public:
    vector<double> av_size_of_contiguous_plots_per_type;
    vector<double> std_size_of_contiguous_plots_per_type;
    vector<double> av_no_of_contiguous_plots_per_type;
    vector<int> total_capacities;

    vector<double> region_av_size_of_contiguous_plots_per_type;
    vector<double> region_std_size_of_contiguous_plots_per_type;
    vector<double> region_av_no_of_contiguous_plots_per_type;
    void backup();
    void restore();
    int farm_closings;              // number of farms that closed during the iteration
    int farm_closings_hoc;          // number of farms that close during generation change
    int period;                     // simulation period; one iteration
    int total_number_of_plots;      // number of plots in the region
    int total_number_of_farms;      // number of farms in the region
    int total_rented_plots;         // total number of rented plots
    int total_new_rented_plots;         // total number of rented plots
    double total_used_land;
    vector<int> total_rented_plots_of_type;
    vector<int> total_new_rented_plots_of_type;
    int total_fix_onfarm_labour;
    double total_var_offfarm_labour;
    int total_fix_offfarm_labour;
    double total_var_onfarm_labour;
    double total_labour_input_hours;

    double total_livestock_units;
    double total_lu_ruminants;
    double total_lu_granivores;
    double total_labour_input_ha;           // total labour hours expressed in lu per ha
    double total_av_labour_input_hours;
    double total_av_labour_input_ha;
    double total_sd_labour_input_ha;
    double total_fix_labour_ha;             // average labour units per ha
    double total_sd_fix_labour;       // standard deviation
    double total_family_labour;
    double total_av_family_labour;
    double total_sd_family_labour;
    double total_hired_labour_fix_pay;
    double total_hired_labour_var_pay;
    double total_factor_remuneration_fix;
    double total_factor_remuneration_var;
    double total_economic_profit;
    double total_economic_land_rent;
    double total_economic_land_rent_sc;
    double total_ec_land_rent;
    double total_profit;
    double total_total_income;
    int   total_investment_expenditure;
    double total_withdrawal;         
    double total_equity_capital;     //
    double total_ecchange;     //
    double total_liquidity;
    double total_assets;
    double total_land_assets;
    double total_number_of_investments;
    double total_borrowed_capital;
    double total_depreciation;
    double total_rent;
    double total_new_rent;
    vector<double> total_rent_of_type;
    vector<double> total_new_rent_of_type;
    double total_distance_costs;
    double total_value_added;
    double total_land_input;
    double total_capital_input;
    double total_land_remuneration;
    double total_lt_interest_costs;
    double total_st_interest_costs;
    double total_st_interest_received;
    double real_sunk_costs_labour;
    // SECTOR CALCULATIONS
    double av_value_added_ha;
    double total_gm_dea;


    void setTotalProduction(const RegFarmInfo* const);
    void setTotalLandInput(const RegFarmInfo* const);
    void periodResultsSector(const vector<RegInvestObjectInfo >& ,
                             const RegRegionInfo& ,
                             const list<RegFarmInfo* >&,
                             int period);
    void periodResultsSectorAfterDisinvest(const vector<RegInvestObjectInfo >& investcat,const list<RegFarmInfo* >&);

    void resetSector();
    void setRealSunkCostsLabour(double );
    double getTotalLandRemuneration() const {
        return total_land_remuneration;
    }
    double getTotalLandInput() const {
        return total_land_input;
    }
    int getTotalNumberOfPlots() const {
        return total_number_of_plots;
    }
    int getTotalNumberOfFarms() const {
        return total_number_of_farms;
    }


    int getFarmClosings() const {
        return farm_closings;
    }
    int getFarmClosingsHoc() const {
        return farm_closings_hoc;
    }
//@    int getTotalLuPerHa() const {return total_lu_per_ha;}
//@    double getTotalAverageLuPerHa() const {return total_average_lu_per_ha;}
//@    double getTotalSdLuPerHa() const {return total_sd_lu_per_ha;}
    double getTotalProfit() const {
        return total_profit;
    }
    double getTotalWithdrawal() const {
        return total_withdrawal;
    }
    double getTotalEquityCapital() const {
        return total_equity_capital;
    }
    double getTotalLiquidity() const {
        return total_liquidity;
    }
    double getTotalAssets() const {
        return total_assets;
    }
    double getTotalLandAssets() const {
        return total_land_assets;
    }
    double getTotalUnitsProduced(int i) const {
        return total_units_produced[i];
    }
    double getTotalVarCostsOfProduct(int i) const {
        return total_varcosts_of_product[i];
    }
    double getTotalGmDEA() const {
        return total_gm_dea;
    }

    double economicLandRentsc(double refincome);

    double economicLandRent(double refincome);
    double averageEconomicLandRent() const;
    double averageProfit() const;
    double averageRent() const;
    double averageNewRent() const;
    double averageFarmSize() const;
    double averageAWUDemand() const;
    double averageLivestockDensity() const;
    double averageLivestockDensityRuminants() const;
    double averageLivestockDensityGranivores() const;
    double averageNewInvestExp() const;
    double averageCapitalHa() const;
    double averageProfitHa(double off_lab) const;
    double averageValueAddedHa() const;
    double averageRentOfType(int type) const;
    double averageNewRentOfType(int type) const;

    double old_land_input;
    double old_assets;
    double old_sunc_labour;
    int check_type;
    bool checkFarm(RegFarmInfo* farm);
    RegSectorResultsInfo(RegGlobalsInfo*,int c);
    RegSectorResultsInfo(const RegSectorResultsInfo&,RegGlobalsInfo*);
    ~RegSectorResultsInfo();
};

//---------------------------------------------------------------------------
#endif
