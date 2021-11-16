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

// RegResults.cpp
//---------------------------------------------------------------------------
//#include <list>
#include "RegResults.h"
#include "RegFarm.h"
#include "RegStructure.h"
//---------------------------------------------------------------------------

RegSectorResultsInfo::RegSectorResultsInfo(const RegSectorResultsInfo& rh,RegGlobalsInfo* G):g(G) {
    obj_backup=NULL;
    *this=rh;
    g=G;
}
RegSectorResultsInfo::RegSectorResultsInfo(RegGlobalsInfo* G,int c) :g(G) {
    obj_backup=NULL;
    period = 0;
    old_land_input=0;
    old_assets=0;
    old_sunc_labour=0;
    check_type=c;
//   resetSector();

//DCX INIT
total_assets = 0;
total_land_input = 0;
real_sunk_costs_labour = 0;

}

RegSectorResultsInfo::~RegSectorResultsInfo() {
    if (obj_backup) delete obj_backup;
}

void
RegSectorResultsInfo::resetSector() {
    period = 0;
    total_number_of_plots = 0;
    total_number_of_farms = 0;
    total_rented_plots = 0;
    total_new_rented_plots = 0;
    total_used_land = 0;
    total_rented_plots_of_type.clear();
    total_new_rented_plots_of_type.clear();
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        total_rented_plots_of_type.push_back(0);
        total_new_rented_plots_of_type.push_back(0);
    }
    farm_closings = 0;
    farm_closings_hoc = 0;
    total_capacities.clear();
    for (int i=0;i<g->NUMBER_OF_INVESTTYPES;i++) {
        total_capacities.push_back(0);
    }
    total_fix_onfarm_labour =0;
    total_var_onfarm_labour = 0;
    total_fix_offfarm_labour =0;
    total_var_offfarm_labour = 0;
    total_labour_input_hours = 0;
    total_livestock_units = 0;
    total_lu_ruminants = 0;
    total_lu_granivores = 0;
    total_labour_input_ha = 0;
    total_av_labour_input_hours = 0;
    total_av_labour_input_ha = 0;
    total_sd_labour_input_ha = 0;
    total_fix_labour_ha = 0;
    total_sd_fix_labour = 0;
    total_family_labour = 0;
    total_av_family_labour = 0;
    total_sd_family_labour = 0;
    total_hired_labour_fix_pay = 0;
    total_hired_labour_var_pay = 0;
    total_factor_remuneration_fix = 0;
    total_factor_remuneration_var = 0;
    total_economic_profit = 0;
    total_economic_land_rent=0;
    total_economic_land_rent_sc=0;
    total_ec_land_rent = 0;

    total_profit = 0;
    total_total_income = 0;
    total_investment_expenditure = 0;
    total_withdrawal = 0;
    total_equity_capital = 0;
    total_ecchange = 0;
    total_liquidity = 0;
    old_assets=total_assets;

    total_assets = 0;
    total_land_assets = 0;
    total_number_of_investments = 0;
    total_borrowed_capital = 0;
    total_lt_interest_costs = 0;
    total_st_interest_costs = 0;
    total_st_interest_received = 0;
    total_depreciation = 0;
    total_rent = 0;
    total_new_rent = 0;
    total_rent_of_type.clear();
    total_new_rent_of_type.clear();
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        total_rent_of_type.push_back(0);
        total_new_rent_of_type.push_back(0);
    }
    total_new_rent_of_type.clear();
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        total_new_rent_of_type.push_back(0);
    }
    total_distance_costs = 0;
    total_value_added = 0;
    old_land_input=total_land_input;

    total_land_input = 0;
    total_capital_input = 0;
    total_land_remuneration = 0;
    old_sunc_labour=real_sunk_costs_labour;
    real_sunk_costs_labour = 0;
    total_units_produced.resize(g->MAXPRODUCTS);
    total_varcosts_of_product.resize(g->MAXPRODUCTS);


    for (int i = 0; i < g->MAXPRODUCTS; i++) {
        total_units_produced[i] = 0;
        total_varcosts_of_product[i] = 0;
    }

    total_gm_dea = 0;
    av_size_of_contiguous_plots_per_type.clear();
    std_size_of_contiguous_plots_per_type.clear();
    av_no_of_contiguous_plots_per_type.clear();
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        std_size_of_contiguous_plots_per_type.push_back(0);
        av_size_of_contiguous_plots_per_type.push_back(0);
        av_no_of_contiguous_plots_per_type.push_back(0);
    }
    region_av_size_of_contiguous_plots_per_type.clear();
    region_std_size_of_contiguous_plots_per_type.clear();
    region_av_no_of_contiguous_plots_per_type.clear();
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        region_av_size_of_contiguous_plots_per_type.push_back(0);
        region_std_size_of_contiguous_plots_per_type.push_back(0);
        region_av_no_of_contiguous_plots_per_type.push_back(0);
    }
}


void
RegSectorResultsInfo::setTotalProduction(const RegFarmInfo* const Farm) {
    // get number of plots of farms
    int nop = Farm->getNumberOfPlots();
    if (nop > 0) {
        int i;
        // determine the total units produced of each product in the region
        for (i = 0; i < g->MAXPRODUCTS; i++) {
            // units produced of product i by this farm
            // add up sector total production of product i
            total_units_produced[i] += Farm->getUnitsOfProduct(i);
            total_varcosts_of_product[i] += (Farm->getVarCostsOfProduct(i)
                                             * Farm->getUnitsOfProduct(i))   ;
        }
    }
}

void
RegSectorResultsInfo::setTotalLandInput(const RegFarmInfo* const Farm) {
    double land = Farm->getLandInput();
    total_land_input += land;
}

bool
RegSectorResultsInfo::checkFarm(RegFarmInfo* farm) {
    if (check_type==0) return true;
    return farm->getLegalType()==check_type;
}



void
RegSectorResultsInfo::periodResultsSector(const vector<RegInvestObjectInfo >& investcat,
        const RegRegionInfo& region,
        const list<RegFarmInfo* >& farms,
        int period) {
    vector<double > sector_capacities;
    list<RegFarmInfo* >::const_iterator afarm;
    total_number_of_farms = 0;

//    total_number_of_plots = g->NO_ROWS * g->NO_COLS;

    for (int i = 0; i < g->MAXPRODUCTS; i++) {
        // units produced of product i by this farm
        // add up sector total production of product i
        total_units_produced[i] = 0;
        total_varcosts_of_product[i] = 0;
    }
    for (afarm = farms.begin();
            afarm != farms.end();
            afarm++) {
        if (checkFarm(*afarm)) {
            total_number_of_farms++;
            if (g->CALCULATE_CONTIGUOUS_PLOTS) {
                for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                    int no=(*afarm)->getContiguousPlotsOfType(i);
                    for (int j=0;j<no;j++) {
                        av_size_of_contiguous_plots_per_type[i]+=(*afarm)->getSizeOfContiguousPlotOfType(i,j);
                    }
                    av_no_of_contiguous_plots_per_type[i]+=no;
                }
            }

            for (int i = 0; i < g->MAXPRODUCTS; i++) {
                // units produced of product i by this farm
                // add up sector total production of product i
                total_units_produced[i] += (*afarm)->getUnitsOfProduct(i);
                total_varcosts_of_product[i] += ((*afarm)->getVarCostsOfProduct(i)
                                                 * (*afarm)->getUnitsOfProduct(i))   ;
            }

            // update rent and distance for each farm
            // including rent for farm_plot

            // total number of plots occupied
            total_number_of_plots += (*afarm)->getNumberOfPlots();
            // total number of rented plots
            total_rented_plots += (*afarm)->countRentedPlots();
            total_new_rented_plots += (*afarm)->countNewRentedPlots();
            // total used land
            total_used_land += (*afarm)->getUnitsProducedOfGroup(0);
//cout << (*afarm)->getFarmId() << ": " << total_used_land << endl;
            // total number of rented arable plots
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                total_rented_plots_of_type[i]+=(*afarm)->countRentedPlotsOfType(i);
            }
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                total_new_rented_plots_of_type[i]+=(*afarm)->countNewRentedPlotsOfType(i);
            }
            total_rent += (*afarm)->getFarmRentExpenditure();
            total_new_rent += (*afarm)->getFarmNewRentExpenditure();
            // total rent arable land
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                total_rent_of_type[i]+=(*afarm)->getFarmRentExpOfType(i);
            }
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                total_new_rent_of_type[i]+=(*afarm)->getFarmNewRentExpOfType(i);
            }
            // total amount of distance costs
            total_distance_costs += (*afarm)->getFarmDistanceCosts();
            for (int i=0;i<g->NUMBER_OF_INVESTTYPES;i++) {
                total_capacities[i]+=(*afarm)->getCapacityOfType(i);
                ;
            }
            // LABOUR
            // total family labour  = ges_ak_besatz
            total_family_labour += (double) (*afarm)->labour->getFamilyLabour()/(double)g->MAX_H_LU;
            // total labour input in hours
            total_labour_input_hours += (*afarm)->labour->getLabourInputHours();
            // total livestock untis
            total_livestock_units += (*afarm)->getTotalLU();
            // total livestock untis
            total_lu_ruminants += (*afarm)->getTotalLU("GRASSLAND");
            // total livestock untis
            total_lu_granivores += (*afarm)->getTotalLU("PIG/POULTRY");
            // total labour hours expressed in labour units per ha
            total_labour_input_ha += (*afarm)->labour->getLabourInputHours();
            ;
            // total fix onfarm labour units
            total_fix_onfarm_labour = static_cast<int>((*afarm)->labour->getFixOnfarmLabour());
            // total var onfarm labour in hours
            total_var_onfarm_labour += (*afarm)->labour->getVarOnfarmLabour();
            // total fix offfarm labour units
            total_fix_offfarm_labour += static_cast<int>((*afarm)->labour->getFixOfffarmLabour());
            // total var offfarm labour in hours
            total_var_offfarm_labour += (*afarm)->labour->getVarOfffarmLabour();

            // sector capacities
            // intialize sector capacities
            sector_capacities.resize(g->NUMBER_OF_INVESTS);

            /*        for(unsigned i = 0; i < sector_capacities.size(); i++) {
            			// add-up farm capacities to sector capacities for
            			// each constraint i
            			sector_capacities[i] += (*afarm)->getCapacity(i);
            		}
            */
        }
    }


    if (total_number_of_farms > 0) {

        if (g->CALCULATE_CONTIGUOUS_PLOTS) {
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                av_size_of_contiguous_plots_per_type[i]/=av_no_of_contiguous_plots_per_type[i];
            }
        }


        // on the basis of the total labour input in hours
        total_av_labour_input_ha = ((double)total_labour_input_ha)/(double)(total_number_of_plots*g->PLOT_SIZE);
        // only family labour
        total_av_family_labour = ((double) total_family_labour)/total_number_of_farms;
    }


    double variance_labour_input_ha = 0;
    double variance_family_labour = 0;
    double farmfamilylabour;
    double farmlabour;
    total_land_input = 0;

    for (afarm = farms.begin();
            afarm != farms.end();
            afarm++) {
        if (checkFarm(*afarm)) {
            if ((*afarm)->labour->getFamilyLabour() > 0) {
                // variances for labour intensity
                farmfamilylabour = (double)(*afarm)->labour->getFamilyLabour()/(double)g->MAX_H_LU;
                farmlabour = (*afarm)->labour->getLabourInputHours()/(double)g->MAX_H_LU;
                if ((*afarm)->getNumberOfPlots()>0) {
                    farmlabour/=(double)((*afarm)->getNumberOfPlots()*g->PLOT_SIZE);
                    variance_family_labour += pow((farmfamilylabour - total_av_family_labour), 2);
                    variance_labour_input_ha += pow((farmlabour - total_av_labour_input_ha), 2);
                }   else {
                    int er=0;
                    er++;
                }

            }
            if (g->CALCULATE_CONTIGUOUS_PLOTS) {
                for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                    int no=(*afarm)->getContiguousPlotsOfType(i);
                    for (int j=0;j<no;j++) {
                        std_size_of_contiguous_plots_per_type[i]+=( av_size_of_contiguous_plots_per_type[i]-(*afarm)->getSizeOfContiguousPlotOfType(i,j))*( av_size_of_contiguous_plots_per_type[i]-(*afarm)->getSizeOfContiguousPlotOfType(i,j));
                    }
                }
            }


            // structural variables
            total_equity_capital += (*afarm)->getEquityCapital();
            total_ecchange += (*afarm)->getEcChange();
            total_liquidity += (*afarm)->getLiquidity();
            total_assets += (*afarm)->getAssets();
            total_land_assets += (*afarm)->getLandAssets();
            total_borrowed_capital += (*afarm)->getLtBorrowedCapital();
            total_hired_labour_fix_pay += (*afarm)->getFarmHiredLabourFixPay();
            total_hired_labour_var_pay += (*afarm)->getFarmHiredLabourVarPay();
            total_factor_remuneration_fix += (*afarm)->getFarmFactorRemunerationFix();
            total_factor_remuneration_var += (*afarm)->getFarmFactorRemunerationVar();
            total_lt_interest_costs += (*afarm)->getLtInterestCosts();
            total_value_added += (*afarm)->getValueAdded();
            total_land_input += (*afarm)->getLandInput();
            total_capital_input += (*afarm)->getCapitalInput();
            total_land_remuneration += (*afarm)->getLandRemuneration();
            total_profit += (*afarm)->getProfit();
            total_total_income += (*afarm)->getTotalIncome();
            total_economic_profit += (*afarm)->getEconomicProfit();
            total_withdrawal += (*afarm)->getWithdrawal();
            total_depreciation += (*afarm)->getDepreciation();
            total_number_of_investments += (*afarm)->getNumberOfNewInvestmentsWoLabour();
            total_investment_expenditure += (*afarm)->getNewInvestmentExpenditure();
            total_st_interest_costs += (*afarm)->getStInterestCosts();
            total_st_interest_received += (*afarm)->getStInterestReceived();
            total_gm_dea += (*afarm)->getOutputGrossMarginDEA();
        }
    }
    if (total_number_of_farms > 0) {

        total_sd_labour_input_ha = sqrt(variance_labour_input_ha/total_number_of_farms);
        total_sd_family_labour = sqrt(variance_family_labour/total_number_of_farms);
        if (g->CALCULATE_CONTIGUOUS_PLOTS) {
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                std_size_of_contiguous_plots_per_type[i]/=av_no_of_contiguous_plots_per_type[i];
                av_no_of_contiguous_plots_per_type[i]/=total_number_of_farms;
            }
        }

    }
    if (g->CALCULATE_CONTIGUOUS_PLOTS) {
        for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
            region_av_size_of_contiguous_plots_per_type[i]+=region.getAvSizeOfContiguousPlotOfType(i);
            region_std_size_of_contiguous_plots_per_type[i]+=region.getStdSizeOfContiguousPlotOfType(i);
            region_av_no_of_contiguous_plots_per_type[i]+=region.getContiguousPlotsOfType(i);
        }
    }
}

void
RegSectorResultsInfo::periodResultsSectorAfterDisinvest(const vector<RegInvestObjectInfo >& investcat,const list<RegFarmInfo* >& farms) {
    double refincome= (double)g->MAX_H_LU*1.25* ((-investcat[g->FIXED_OFFFARM_LAB].getAcquisitionCosts() )/( - investcat[g->FIXED_OFFFARM_LAB].getLabourSubstitution()));
    vector<double > sector_capacities;
    list<RegFarmInfo* >::const_iterator afarm;
    total_economic_land_rent=economicLandRent(refincome);
    total_economic_land_rent_sc=economicLandRentsc(refincome);
    for (afarm = farms.begin();
            afarm != farms.end();
            afarm++) {
        if (checkFarm(*afarm)) {
            real_sunk_costs_labour+=(*afarm)->getSunkCostsLabor();
            if ((*afarm)->getClosed())
                farm_closings++;
        }
    }
    for (afarm = farms.begin();
            afarm != farms.end();
            afarm++) {
        if ((*afarm)->getClosed()==2)
            farm_closings_hoc++;
    }
}




void
RegSectorResultsInfo::setRealSunkCostsLabour(double rscl) {
    real_sunk_costs_labour += rscl;
}
double
RegSectorResultsInfo::economicLandRentsc(double refincome) {
    if (old_land_input==0) return 0;

    double sc=(total_assets+total_depreciation-total_investment_expenditure)-old_assets;
    sc-=old_sunc_labour;
    double elr_sc=economicLandRent(refincome)+sc;
    return elr_sc;
}
double
RegSectorResultsInfo::economicLandRent(double refincome){
    if (total_land_input==0) return 0;
    double hhincome = (total_profit
                       + total_rent
                       + total_lt_interest_costs
                       + total_st_interest_costs
                       + total_hired_labour_fix_pay
                       + total_hired_labour_var_pay
                       + total_factor_remuneration_fix
                       + total_factor_remuneration_var);
    double wages = ( total_hired_labour_fix_pay
                     + total_hired_labour_var_pay );
    double oppinc = (total_family_labour*refincome);
    double bcinterest = (total_lt_interest_costs
                         + total_st_interest_costs);
    double ecinterest = ((total_equity_capital-total_land_assets-total_ecchange+total_withdrawal)*g->EQ_INTEREST);
    double quota = ((getTotalUnitsProduced(g->stdNameIndexs["MILK"])*g->MILKPROD
                     + getTotalUnitsProduced(g->stdNameIndexs["LETQUOTA"])
                     - getTotalUnitsProduced(g->stdNameIndexs["GETQUOTA"])
                    ) * g->QUOTA_PRICE);

    total_ec_land_rent =  hhincome
                       - wages
                       - oppinc
                       - bcinterest
                       - ecinterest
                       - quota;
    return total_ec_land_rent;
}
double
RegSectorResultsInfo::averageEconomicLandRent() const {
    return total_economic_land_rent/total_land_input;
}
double
RegSectorResultsInfo::averageProfit() const {
    return total_profit/total_number_of_farms;
}
double
RegSectorResultsInfo::averageRent()  const {
    return total_rent/(total_rented_plots * g->PLOT_SIZE);
}
double
RegSectorResultsInfo::averageNewRent()  const {
    if (total_new_rented_plots>0)
        return total_new_rent/(total_new_rented_plots * g->PLOT_SIZE);
    else
        return 0;
}
double
RegSectorResultsInfo::averageFarmSize() const {
    return total_land_input/total_number_of_farms;
}
double
RegSectorResultsInfo::averageAWUDemand() const {
    return total_labour_input_hours/g->MAX_H_LU/total_land_input*100;
}
double
RegSectorResultsInfo::averageLivestockDensity() const {
    return total_livestock_units/total_used_land;
}
double
RegSectorResultsInfo::averageLivestockDensityRuminants() const {
    return total_lu_ruminants/total_used_land;
}
double
RegSectorResultsInfo::averageLivestockDensityGranivores() const {
    return total_lu_granivores/total_used_land;
}

double
RegSectorResultsInfo::averageNewInvestExp() const {
    return total_investment_expenditure/total_number_of_farms;
}
double
RegSectorResultsInfo::averageCapitalHa() const {
    return total_capital_input/total_land_input;
}
double
RegSectorResultsInfo::averageProfitHa(double off_lab) const {
    return (total_profit - ((total_family_labour - ((total_fix_offfarm_labour + total_var_offfarm_labour)/g->MAX_H_LU))* 2 *
        (off_lab)))/total_used_land;
}
double
RegSectorResultsInfo::averageValueAddedHa() const {
    return total_value_added/total_land_input;
}
double
RegSectorResultsInfo::averageRentOfType(int type) const {
    if (total_rented_plots_of_type[type] > 0)
        return total_rent_of_type[type]/(total_rented_plots_of_type[type] * g->PLOT_SIZE);
    else
        return 0;
}
double
RegSectorResultsInfo::averageNewRentOfType(int type) const {
    if (total_new_rented_plots_of_type[type] > 0)
        return total_new_rent_of_type[type]/(total_new_rented_plots_of_type[type] * g->PLOT_SIZE);
    else
        return 0;
}
void
RegSectorResultsInfo::backup() {
    obj_backup=new RegSectorResultsInfo(*this);
}
void
RegSectorResultsInfo::restore() {
    RegSectorResultsInfo* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
}
