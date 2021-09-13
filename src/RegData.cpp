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

// RegData.cpp
//---------------------------------------------------------------------------
#undef UNICODE
#include <windows.h>

#include <fstream>
#include "RegData.h"
#include "RegFarm.h"
#include "RegResults.h"
#include <time.h>
#include <iomanip>

using namespace std;

void RegDataInfo::scenarioDate(ofstream& of) {
	of <<"#Senario: \t"<< g->Scenario << "\n";
	of << "#Simulation: \t"<< g->TimeStart << "\n";
}

//soil service carbon values
void RegDataInfo::printRegionSoilservice(ofstream &c_ofs, list <RegFarmInfo*> farmList, int period) {
	
	list <RegFarmInfo*>::iterator iter;
	
	int nos=g->NO_OF_SOIL_TYPES;
	vector<double> sumC, land, avC, varC;
	sumC.resize(nos); land.resize(nos); avC.resize(nos); varC.resize(nos);
	
	for (int i=0; i<g->NO_OF_SOIL_TYPES; ++i) {
		for (iter = farmList.begin(); iter!=farmList.end();++iter) {
			sumC[i]+=(*iter)->getAvCarbons()[i] * (*iter)->getLandInputOfType(i);
			land[i]+= (*iter)->getLandInputOfType(i);
		}
		avC[i]=sumC[i]/land[i];
	}

	sumC.clear();sumC.resize(nos);
	for (int i=0; i<g->NO_OF_SOIL_TYPES; ++i) {
		for (iter = farmList.begin(); iter!=farmList.end();++iter) {
			sumC[i]+=((*iter)->getAvCarbons()[i])*((*iter)->getAvCarbons()[i]) * (*iter)->getLandInputOfType(i);
		}
		varC[i]=sumC[i]/land[i]-avC[i]*avC[i];
	}

	c_ofs << "###########   \n";
	c_ofs << "\t" << g->Scenario << "\t" << g->V << "\t" << period << "\t"<< 999999 << "\t"<< "whole_region" << "\t";
	for (int i=0; i<g->NO_OF_SOIL_TYPES; ++i) {
		c_ofs << avC[i] << "\t" << varC[i] << "\t";
	}
	c_ofs << "\n###########    \n";
	
	return;
}

void RegDataInfo::initSoilservice(list <RegFarmInfo*> farmList){
	string carbonFile = "carbons.dat";
	ofstream c_ofs;
	string file=g->OUTPUTFILE +carbonFile;
	c_ofs.open(file.c_str(),ios::out|ios::trunc);
	c_ofs<<"\t"<< "scenario" << "\t" << "replication"<<"\t";
	c_ofs<<"iteration"<<"\t" << "farm_id" <<"\t"<< "farm_name"<<"\t";
	for (int j=0; j< g->NO_OF_SOIL_TYPES; ++j) {
		c_ofs << "mean_"+ g->NAMES_OF_SOIL_TYPES[j]<<"\t"<<"var_"+g->NAMES_OF_SOIL_TYPES[j]<<"\t";
	}

	int nprod = market->getNumProducts();
	vector<RegProductInfo> &prodcat = market->getProductCat();
	for (int i= 0; i<nprod; ++i) {
		if (prodcat[i].hasDynSoilservice()){
			c_ofs << prodcat[i].getName()+"_N" << "\t"<< prodcat[i].getName()+"_Yield" << "\t";
		}
	}

	c_ofs<<"\n";

	int period=-1;
	printRegionSoilservice(c_ofs, farmList, period);

	list <RegFarmInfo*>::iterator iter;
		
	for (iter = farmList.begin(); iter!=farmList.end();++iter) {
		bool erst=true;
		for (int i=0; i<g->NO_OF_SOIL_TYPES; ++i) {
			if (erst) {
				c_ofs<<"\t" << g->Scenario << "\t" << g->V << "\t" << "-1" <<"\t"<< (*iter)->getFarmId() <<"\t" << (*iter)->getFarmName() <<"\t";
				erst=false;
			}
			c_ofs << (*iter)->getAvCarbons()[i]<<"\t"<< (*iter)->getVarCarbons()[i] <<"\t";
		}
		for (int i= 0; i<nprod; ++i) {
			if (prodcat[i].hasDynSoilservice()){
				c_ofs<<(*iter)->getProductList()->getNoptOfNumber(i,(*iter))<<"\t" << (*iter)->getProductList()->getYieldOfNumber(i,*iter) << "\t";
			}
		}
		c_ofs << "\n";
	}
	
	c_ofs.close();
}

void RegDataInfo::printSoilservice(list <RegFarmInfo*> farmList, int period){
	string carbonFile = "carbons.dat";
	ofstream c_ofs;
	string file=g->OUTPUTFILE +carbonFile;
	c_ofs.open(file.c_str(),ios::out|ios::app);
		
	printRegionSoilservice(c_ofs,farmList, period);
	
	int nprod = market->getNumProducts();
	vector<RegProductInfo> &prodcat = market->getProductCat();

	list <RegFarmInfo*>::iterator iter;
	for (iter = farmList.begin(); iter!=farmList.end();++iter) {
		bool erst=true;
		for (int i=0; i<g->NO_OF_SOIL_TYPES; ++i) {
			if (erst) {
				c_ofs<<"\t" << g->Scenario << "\t" << g->V << "\t" << period <<"\t"<< (*iter)->getFarmId() <<"\t" << (*iter)->getFarmName() <<"\t";
				erst=false;
			}

			c_ofs << (*iter)->getAvCarbons()[i]<<"\t"<< (*iter)->getVarCarbons()[i] <<"\t";

		}
		
		for (int i= 0; i<nprod; ++i) {
			if (prodcat[i].hasDynSoilservice()){
				c_ofs<<(*iter)->getProductList()->getNoptOfNumber(i,(*iter))<<"\t" << (*iter)->getProductList()->getYieldOfNumber(i,*iter) << "\t";
			}
		}
		c_ofs << "\n";
	}
	c_ofs.close();
}

//--------------------------
// INITIALISE REGION RESULTS
//--------------------------
RegDataInfo::RegDataInfo(RegGlobalsInfo* G, RegMarketInfo* markt) :g(G), market(markt) {
    counter = 0;
}

void
RegDataInfo::initialisation(vector<RegInvestObjectInfo>& invest_cat,
                            vector<RegProductInfo>& product_cat, RegEnvInfo* Env) {
//DCX
    WIN32_FIND_DATA FindFileData;
    size_t pos = g->OUTPUTFILE.find_last_not_of("\\");
    string dirname = g->OUTPUTFILE.substr(0,pos+1);
    HANDLE hfind = FindFirstFile(dirname.c_str(), &FindFileData);
    if ( hfind == INVALID_HANDLE_VALUE) {               //not found
        if (!CreateDirectory(dirname.c_str(),NULL)){
            cerr << "ERROR: " << g->OUTPUTFILE.c_str() << " can not be created ! " << endl;
            exit(2);
        }
    }

	 if (g->SECTOROUTPUT) {
        if(g->PRINT_SEC_RES)
        initSectorResults(invest_cat,product_cat);
        if(g->PRINT_SEC_COSTS)
        initSectorVarCosts(product_cat);
        if(g->PRINT_SEC_PRICE)
        initSectorPrices(product_cat);
        if(g->PRINT_SEC_EXP_PRICE)
        initExpectedSectorPrices(product_cat);
        if(g->PRINT_SEC_COND)
        initCondensedSectorOutput();
    }
    if (g->CALC_LEGAL_TYPES) {
                initLegalTypeOutput(invest_cat,product_cat);
    }
    if (g->FARMOUTPUT) {
        if(g->PRINT_FARM_RES)
        initFarmResults(invest_cat,product_cat);
        if(g->PRINT_FARM_INV)
        initFarmInvestment(invest_cat);
        if(g->PRINT_FARM_PROD)
        initFarmProduction(product_cat);
        if(g->PRINT_FARM_COSTS)
        initFarmVarCosts(product_cat);
        if (g->CALCULATE_CONTIGUOUS_PLOTS) {
                if(g->PRINT_CONT_PLOTS)
            initContiguousPlotsOutput(product_cat);
        }
    }
    if (g->ENV_MODELING) {
        initFarmEnvDataUsage();
        initSpeciesOut(Env);
    }
  //    }
}
void
RegDataInfo::initFarmEnvDataUsage() {
    string part2="farm_envusage.dat";
    string file=g->OUTPUTFILE +part2;

    envusageout.open(file.c_str(),ios::out|ios::trunc);
    envusageout << "iteration\t"
    << "farm_ID\t"
    << "N\t"
    << "P2O5\t"
    << "K2O\t"
    << "Fungicides\t"
    << "Herbicides\t"
    << "Insecticides\t"
    << "Water Usage\t"
    << "Soil loss\t";
    envusageout << "\n";
    envusageout.close();
}


void
RegDataInfo::initFarmVarCosts(const vector<RegProductInfo>& product_cat) {
    string part2="farm_vc.dat";
    string file=g->OUTPUTFILE +part2;
    varcostsout.open(file.c_str(),ios::out|ios::trunc);
    varcostsout << "iteration\t"
    << "farm_ID\t";
    for (unsigned int i=0;i<product_cat.size();i++) {
        // output costs of product
        varcostsout << "varcosts_" << product_cat[i].getName().c_str() << "\t";
    }
    varcostsout << "\n";
    varcostsout.close();
}
void
RegDataInfo::printFarmVarCosts(const RegFarmInfo* farm,
                               const vector<RegProductInfo>& product_cat,
                               int period) {
    varcostsout.setf(ios_base::fmtflags(0), ios_base::floatfield);
    //varcostsout.precision(8);
    varcostsout.setf(ios_base::left, ios_base::adjustfield);
    varcostsout << setw(11) << (double)period << "\t"
    << setw(11) << (double)farm->getFarmId()<< "\t";
    for (unsigned int i = 0; i < product_cat.size(); i++) {
        varcostsout << setw(11) << farm->getVarCostsOfProduct(i) << "\t";
    }
    varcostsout << "\n";
}
void
RegDataInfo::openFarmOutput() {
    farm_results.clear();
    string part2;
    string file;
    part2="farm_production.dat";
    file=g->OUTPUTFILE +part2;
    if(g->PRINT_FARM_PROD)
    farmprodout.open(file.c_str(), ios::app);
    part2="farm_investment.dat";
    file=g->OUTPUTFILE +part2;
    if(g->PRINT_FARM_INV)
    farminvest.open(file.c_str(), ios::app);
    part2="farm_vc.dat";
    file=g->OUTPUTFILE +part2;
    if(g->PRINT_FARM_COSTS)
    varcostsout.open(file.c_str(), ios::app);
    if (g->ENV_MODELING) {
        part2="farm_envusage.dat";
        file=g->OUTPUTFILE +part2;
        envusageout.open(file.c_str(), ios::app);
    }
}
void
RegDataInfo::openFarmStandardOutput() {
    string part2="farm_standard_indicators.dat";
    string file=g->OUTPUTFILE +part2;
	farmout.open(file.c_str(), ios::app);
}
void
RegDataInfo::closeFarmStandardOutput() {
    if(g->PRINT_FARM_RES)
    farmout.close();
}

void
RegDataInfo::closeFarmOutput() {
    if(g->PRINT_FARM_INV)
    farminvest.close();
    if(g->PRINT_FARM_PROD)
    farmprodout.close();
    if(g->PRINT_FARM_COSTS)
    varcostsout.close();
    envusageout.close();
}

// SECTOR
void
RegDataInfo::initSectorPrices(const vector<RegProductInfo>& product_cat) {
    string part2="sec_prices.dat";
    string file=g->OUTPUTFILE +part2;

    secpricesout.open(file.c_str(),ios::out|ios::trunc);
	//scenarioDate(secpricesout);

    secpricesout << "scenario\t"
    << "replication\t"
    << "iteration\t";
    for (unsigned int i=0;i<product_cat.size();i++) {
        secpricesout << product_cat[i].getName().c_str()  << "\t" ;
    }
    secpricesout << "\n";
    secpricesout.close();
}

void
RegDataInfo::initSpeciesOut(RegEnvInfo* Env) {
    string part2="sec_envspecies.dat";
    string file=g->OUTPUTFILE +part2;
    speciesout.open(file.c_str(),ios::out|ios::trunc);
    speciesout << "Iteration\t";
    vector <string> habitatLabels = Env->getHabitatLabels();
    for (int i=0;i<Env->getNHabitats();i++) {
        string label= "ha_"+habitatLabels[i];
        speciesout << label.c_str()  << "\t" ;
    }
    for (int i=0;i<Env->getNHabitats();i++) {
        string label= "species_"+habitatLabels[i];
        speciesout << label.c_str()  << "\t" ;
    }
    speciesout << "\n";
    speciesout.close();
}
void
RegDataInfo::printSpeciesOut(RegEnvInfo* Env, int iteration_h) {
    string part2="sec_envspecies.dat";
    string file=g->OUTPUTFILE +part2;
    speciesout.open(file.c_str(), ios::app);
    speciesout <<setw(11) << (double) iteration_h << "\t";
    for (int i=0;i<Env->getNHabitats();i++) {
        speciesout <<setw(11) << (double) Env->getProducedHaByHabitat(i)  << "\t" ;
    }
    for (int i=0;i<Env->getNHabitats();i++) {
        speciesout <<setw(11) << (double) Env->getSpeciesByHabitat(i)  << "\t" ;
    }
    speciesout << "\n";
    speciesout.close();
};

void
RegDataInfo::initContiguousPlotsOutput(const vector<RegProductInfo>& product_cat) {
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        stringstream file;
        file<<g->OUTPUTFILE<<"contiguous_plots_of_type_"<<i<<".dat";
        secpricesout.open(file.str().c_str(),ios::out|ios::trunc);
        secpricesout << "iteration\tcontiguius_plots\n";
        secpricesout.close();
        file.str("");
        file<<g->OUTPUTFILE<<"region_contiguous_plots_of_type_"<<i<<".dat";
        secpricesout.open(file.str().c_str(),ios::out|ios::trunc);
        secpricesout << "iteration\tcontiguius_plots\n";
        secpricesout.close();
        file.str("");
        file<<g->OUTPUTFILE<<"gams_contiguous_plots_of_type_"<<i<<".dat";
        secpricesout.open(file.str().c_str(),ios::out|ios::trunc);
        secpricesout << "iteration.farm_id\tcontiguius_plots\n";
        secpricesout.close();
        file.str("");
        file<<g->OUTPUTFILE<<"gams_activity_levels_of_type_"<<i<<".dat";
        secpricesout.open(file.str().c_str(),ios::out|ios::trunc);
        secpricesout << "iteration.farm_id\t";
        if (i==0) {
            secpricesout << product_cat[5].getName().c_str()  << "\t" ;
            secpricesout << product_cat[6].getName().c_str()  << "\t" ;
            secpricesout << product_cat[7].getName().c_str()  << "\t" ;
            secpricesout << product_cat[8].getName().c_str()  << "\t" ;
            secpricesout << product_cat[17].getName().c_str()  << "\t" ;
            secpricesout << product_cat[23].getName().c_str()  << "\t" ;
        } else {
            secpricesout << product_cat[9].getName().c_str()  << "\t" ;
            secpricesout << product_cat[10].getName().c_str()  << "\t" ;
            secpricesout << product_cat[26].getName().c_str()  << "\t" ;
        }
        secpricesout << "\n";
        secpricesout.close();
    }
}

void
RegDataInfo::printRegionContiguousPlotsOutput(const RegRegionInfo* region, int period) {

    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        stringstream file;
        file<<g->OUTPUTFILE<<"region_contiguous_plots_of_type_"<<i<<".dat";
        secpricesout.open(file.str().c_str(),ios::app);

        for (unsigned int j=0;j<region->contiguous_plots[i].size();j++) {
            secpricesout << setw(11) << period  << "\t";
            secpricesout << setw(11) << region->contiguous_plots[i][j] << "\n";
        }
        secpricesout.close();
    }
}

void
RegDataInfo::printContiguousPlotsOutput(const RegFarmInfo* farm, int period) {

    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        stringstream file;
        file<<g->OUTPUTFILE<<"contiguous_plots_of_type_"<<i<<".dat";
        secpricesout.open(file.str().c_str(),ios::app);
        for (unsigned int j=0;j<farm->contiguous_plots[i].size();j++) {
            secpricesout << setw(11) << period  << "\t";
            secpricesout << setw(11) << farm->contiguous_plots[i][j] << "\n";
        }
        secpricesout.close();


        file.str("");
        file<<g->OUTPUTFILE<<"gams_contiguous_plots_of_type"<<i<<".dat";
        secpricesout.open(file.str().c_str(),ios::app);
        secpricesout << period << "." << (int)farm->getFarmId() << "\t";
        for (unsigned int j=0;j<farm->contiguous_plots[i].size();j++) {
            secpricesout  << farm->contiguous_plots[i][j] << "\t";
        }
        secpricesout << "\n";
        secpricesout.close();

        file.str("");
        file<<g->OUTPUTFILE<<"gams_activity_levels_of_type_"<<i<<".dat";
        secpricesout.open(file.str().c_str(),ios::app);
        secpricesout << period << "." << (int)farm->getFarmId() << "\t";
        if (i==0) {
            secpricesout << farm->getUnitsOfProduct(5)  << "\t" ;
            secpricesout << farm->getUnitsOfProduct(6)  << "\t" ;
            secpricesout << farm->getUnitsOfProduct(7) << "\t" ;
            secpricesout << farm->getUnitsOfProduct(8) << "\t" ;
            secpricesout << farm->getUnitsOfProduct(17) << "\t" ;
            secpricesout << farm->getUnitsOfProduct(23) << "\t" ;
        } else {
            secpricesout << farm->getUnitsOfProduct(9) << "\t" ;
            secpricesout << farm->getUnitsOfProduct(10) << "\t" ;
            secpricesout << farm->getUnitsOfProduct(26) << "\t" ;
        }
        secpricesout << "\n";
        secpricesout.close();
    }
}

void
RegDataInfo::initExpectedSectorPrices(const vector<RegProductInfo>& product_cat) {
    string part2="sec_exprices.dat";
    string file=g->OUTPUTFILE +part2;

    exsecpricesout.open(file.c_str(),ios::out|ios::trunc);
	//scenarioDate(exsecpricesout);

    exsecpricesout << "scenario\t"<< "iteration\t";
    for (unsigned int i=0;i<product_cat.size();i++) {
        exsecpricesout << product_cat[i].getName().c_str()  << "\t" ;
    }
    exsecpricesout << "\n";
    exsecpricesout.close();
}
void
RegDataInfo::initSectorVarCosts(const vector<RegProductInfo>& product_cat) {
    string part2="sec_vc.dat";
    string file=g->OUTPUTFILE +part2;

    secvarcostsout.open(file.c_str(),ios::out|ios::trunc);
    secvarcostsout << "iteration\t";
    for (unsigned int i=0;i<product_cat.size();i++) {
        secvarcostsout << product_cat[i].getName().c_str() << "\t" ;
    }
    for (unsigned int i=0;i<product_cat.size();i++) {
        secvarcostsout << "s_varcosts_" << product_cat[i].getCatalogNumber() << "\t";
    }
    secvarcostsout << "\n";
    secvarcostsout.close();
}
void
RegDataInfo::printSectorPrices( const RegSectorResultsInfo& sector,
                                vector<RegProductInfo>& product_cat, int period) {
    string part2="sec_prices.dat";
    string file=g->OUTPUTFILE +part2;

    secpricesout.open(file.c_str(), ios::app);
    secpricesout << setw(11) << g->Scenario << "\t"
    << setw(11) << (double)g->V << "\t"
    << setw(11) << (double)period  << "\t";
    for (unsigned int i = 0; i < product_cat.size(); i++) {
//         secpricesout << setw(11) << product_cat[i].getPriceExpectation() << "\t";
        secpricesout << setw(11) << product_cat[i].getPrice() << "\t";
    }
    secpricesout << "\n";
    secpricesout.close();
}
void
RegDataInfo::printExpectedSectorPrices( const RegSectorResultsInfo& sector,
                                        vector<RegProductInfo>& product_cat, int period) {
    string part2="sec_exprices.dat";
    string file=g->OUTPUTFILE +part2;

    secpricesout.open(file.c_str(), ios::app);
    secpricesout << g->Scenario << "\t"
		<< setw(11) << (double)period  << "\t";
    for (unsigned int i = 0; i < product_cat.size(); i++) {
        secpricesout << setw(11) << product_cat[i].getPriceExpectation() << "\t";
//         secpricesout << setw(11) << product_cat[i].getPrice() << "\t";
    }
    secpricesout << "\n";
    secpricesout.close();
}

void
RegDataInfo::printSectorVarCosts( const RegSectorResultsInfo& sector,
                                  vector<RegProductInfo>& product_cat, int period) {
    string part2="sec_vc.dat";
    string file=g->OUTPUTFILE +part2;

    secvarcostsout.open(file.c_str(), ios::app);
    secvarcostsout << setw(11) << (double)period  << "\t";
    for (unsigned int i = 0; i < product_cat.size(); i++) {
        secvarcostsout << setw(11) << sector.getTotalUnitsProduced(i) << "\t";
    }
    for (unsigned int i = 0; i < product_cat.size(); i++) {
        secvarcostsout << setw(11) << sector.getTotalVarCostsOfProduct(i) << "\t";
    }
    secvarcostsout << "\n";
    secvarcostsout.close();
}

static string trim_numSuffix(string x){
	size_t pos = x.find_last_not_of("0123456789");
	if (pos != string::npos) 
		return x.substr(0,pos+1);
	else return x;
}

void
RegDataInfo::initSectorResults(vector<RegInvestObjectInfo>& invest_cat,vector<RegProductInfo>& product_cat) {
//DCX
	sector_names.clear();

    sector_names.push_back( "scenario");
    sector_names.push_back( "replication");
    sector_names.push_back( "period");
    sector_names.push_back( "plots_occupied");
    sector_names.push_back( "rented_plots");
    sector_names.push_back( "used_land");
    sector_names.push_back( "number_of_farms");
    sector_names.push_back( "farm_closings");
    sector_names.push_back( "family_labour");
    sector_names.push_back( "total_income");
    sector_names.push_back( "profit");
    sector_names.push_back( "economic_profit");
    sector_names.push_back( "withdrawal");
    sector_names.push_back( "equity_capital");
    sector_names.push_back( "liquidity");
    sector_names.push_back( "assets_wo_land");
    sector_names.push_back( "investment_expenditure");
    sector_names.push_back( "borrowed_cap");
    sector_names.push_back( "total_lt_interest_costs");
    sector_names.push_back( "st_interest_costs");
    sector_names.push_back( "st_interest_rev");
    sector_names.push_back( "hired_pay_fix");
    sector_names.push_back( "hired_pay_var");
    sector_names.push_back( "factor_rem_fix");
    sector_names.push_back( "factor_rem_var");
    sector_names.push_back( "depreciation");
    sector_names.push_back( "rent_exp");
    sector_names.push_back( "new_rent_exp");
    sector_names.push_back( "distance_costs");
    ;
    for (unsigned int i=0;i<product_cat.size();i++) {
        sector_names.push_back( product_cat[i].getName());
    }
    sector_names.push_back( "value_added");
    sector_names.push_back( "labour_input_h");
    sector_names.push_back( "land_input_ha");
    sector_names.push_back( "capital_input");
    sector_names.push_back( "real_sc_labour");
    sector_names.push_back( "sell_guelle_p");
    sector_names.push_back( "buy_guelle_p");
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        sector_names.push_back(g->NAMES_OF_SOIL_TYPES[i]+string("_rent"));
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        sector_names.push_back(string("rented_")+g->NAMES_OF_SOIL_TYPES[i]);
    }
    sector_names.push_back( "land_assets");
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        sector_names.push_back(g->NAMES_OF_SOIL_TYPES[i]+string("_new_rent"));
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        sector_names.push_back(string("new_rented_")+g->NAMES_OF_SOIL_TYPES[i]);
    }
    sector_names.push_back( "new_rented_plots");
	if (g->CALCULATE_CONTIGUOUS_PLOTS) {
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        stringstream s1,s2;
        s1<<"Contiguous_plots_of_type_" <<i;
        s2<<"Av_size_of_type_"<<i;
        sector_names.push_back(s2.str());
        sector_names.push_back(s1.str());
	}}

	int sz = invest_cat.size();
	vector<string> capacity_names;
	capacity_names.resize(g->NUMBER_OF_INVESTTYPES);
	for (int i=0; i<sz; ++i) {
		auto t = invest_cat[i].getInvestType();
		auto s = invest_cat[i].getName();
		capacity_names[t] = trim_numSuffix(s);
	}

	for (int i=0;i<g->NUMBER_OF_INVESTTYPES;i++) {
        sector_names.push_back( capacity_names[i]+"_capacity" );
    }
    sector_names.push_back( "global_strategy");

    string part2="sector.dat";
    string file=g->OUTPUTFILE +part2;

	secout.open(file.c_str(), ios::out|ios::trunc);
    // Formatierunge
    secout.setf(ios_base::fmtflags(0), ios_base::floatfield);
    //secout.precision(8);
    secout.setf(ios_base::left, ios_base::adjustfield);
	//scenarioDate(secout);

    for (unsigned int i=0;i<sector_names.size();i++) {
        secout << sector_names[i].c_str() << "\t";
    }
    secout << "\n";
    secout.close();
}

void
RegDataInfo::printSectorResults(const RegSectorResultsInfo& sector,
                                vector<RegProductInfo>& product_cat, int period) {
    sector_values.clear();

    // SECTOR RESULTS
    //sector_values.push_back( g->Scenario                         );
    sector_values.push_back( (double)g->V                             );
    sector_values.push_back( (double)period                           );
    sector_values.push_back( (double)sector.total_number_of_plots     );
    sector_values.push_back( (double)sector.total_rented_plots        );
    sector_values.push_back( sector.total_used_land       );
    sector_values.push_back( (double)sector.total_number_of_farms     );
    sector_values.push_back( (double)sector.farm_closings             );
    sector_values.push_back( sector.total_family_labour       );
    sector_values.push_back( sector.total_total_income        );
    sector_values.push_back( sector.total_profit              );
    sector_values.push_back( sector.total_economic_profit     );
    sector_values.push_back( sector.total_withdrawal          );
    sector_values.push_back( sector.total_equity_capital      );
    sector_values.push_back( sector.total_liquidity           );
    sector_values.push_back( sector.total_assets
                             - sector.total_land_assets  );
    sector_values.push_back( sector.total_investment_expenditure   );
    sector_values.push_back( sector.total_borrowed_capital    );
    sector_values.push_back( sector.total_lt_interest_costs         );
    sector_values.push_back( sector.total_st_interest_costs         );
    sector_values.push_back( sector.total_st_interest_received      );
    sector_values.push_back( sector.total_hired_labour_fix_pay      );
    sector_values.push_back( sector.total_hired_labour_var_pay      );
    sector_values.push_back( sector.total_factor_remuneration_fix   );
    sector_values.push_back( sector.total_factor_remuneration_var   );
    sector_values.push_back( sector.total_depreciation        );
    sector_values.push_back( sector.total_rent                );
    sector_values.push_back( sector.total_new_rent                );
    sector_values.push_back( sector.total_distance_costs      );

    for (unsigned int i = 0; i < product_cat.size(); i++) {
        sector_values.push_back( sector.getTotalUnitsProduced(i) );
    }

    sector_values.push_back( sector.total_value_added         );
    sector_values.push_back( sector.total_labour_input_hours  );
    sector_values.push_back( sector.total_land_input          );
    sector_values.push_back( sector.total_capital_input       );
    sector_values.push_back( sector.real_sunk_costs_labour    );

 //DCX
    bool ok = g->stdNameIndexs.find("MANSELL")!=end(g->stdNameIndexs);
	sector_values.push_back(ok?product_cat[g->stdNameIndexs["MANSELL"]].getPriceExpectation():0);
    sector_values.push_back(ok?product_cat[g->stdNameIndexs["MANBUY"]].getPriceExpectation():0);
	
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        sector_values.push_back( sector.total_rent_of_type[i]         );
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        sector_values.push_back( sector.total_rented_plots_of_type[i]*g->PLOT_SIZE         );
    }
    sector_values.push_back( sector.total_land_assets  );
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        sector_values.push_back( sector.total_new_rent_of_type[i]        );
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        sector_values.push_back( sector.total_new_rented_plots_of_type[i]*g->PLOT_SIZE         );
    }
    sector_values.push_back( (double)sector.total_new_rented_plots        );
    if (g->CALCULATE_CONTIGUOUS_PLOTS) {
        for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
            sector_values.push_back( sector.av_size_of_contiguous_plots_per_type[i] );
            sector_values.push_back( sector.av_no_of_contiguous_plots_per_type[i] );
        }
    } 

    for (int i=0;i<g->NUMBER_OF_INVESTTYPES;i++) {
        sector_values.push_back( sector.total_capacities[i] );
    }
    sector_values.push_back( g->GLOBAL_STRATEGY);
    // SECTOR RESULTS
    string part2="sector.dat";
    string file=g->OUTPUTFILE +part2;

    secout.open(file.c_str(), ios::app);
	secout.setf(ios_base::left, ios_base::adjustfield);
	secout << g->Scenario << "\t";
    for (unsigned int i=0;i<sector_values.size();i++) {
        secout  << setw(11) << sector_values[i]  << "\t";
    }
    secout << "\n";
    secout.close();
}

void
RegDataInfo::closePolicyOutput() {
    policyout.close();
}
void
RegDataInfo::openPolicyOutput() {
    string part2="policy_output.dat";
    string file=g->OUTPUTFILE +part2;

    policyout.open(file.c_str(), ios::app);
}
void RegDataInfo::initPolicyOutput() {
}
void
RegDataInfo::printPolicyOutput(string o) {
    policyout  << o.c_str();
}

//CONDENSED SECTOR DATA

void
RegDataInfo::printCondensedSectorOutput(const RegSectorResultsInfo& sector, vector<RegProductInfo>& product_cat,vector <RegInvestObjectInfo >&  invest_cat,int period) {
    string part2="condsecout.dat";
    string filename=g->OUTPUTFILE +part2;
    condsecout.open(filename.c_str(), ios::app);
    condsecout << setw(11) //<< g->SIM_VERSION  << "\t"
    << setw(11) << g->Scenario << "\t"
    << setw(11) << (double)g->V << "\t"
    << setw(11) << (double)period  << "\t"
    << setw(11) << (double)sector.total_number_of_farms  << "\t"
    << setw(11) << (double)sector.farm_closings << "\t"
    << setw(11) << (double)sector.farm_closings_hoc << "\t"
    << setw(11) << (double)sector.total_economic_land_rent << "\t"
    << setw(11) << (double)sector.total_economic_land_rent_sc    << "\t"

    << setw(11) << (double)sector.averageProfit() << "\t"
    << setw(11) << (double)sector.averageRent() << "\t"
    << setw(11) << (double)sector.averageNewRent() << "\t"
    << setw(11) << (double)sector.averageFarmSize() << "\t"
    << setw(11) << (double)sector.averageAWUDemand() << "\t"
    << setw(11) << (double)sector.averageLivestockDensity() << "\t"
    << setw(11) << (double)sector.averageLivestockDensityRuminants() << "\t"
    << setw(11) << (double)sector.averageLivestockDensityGranivores() << "\t"
    << setw(11) << (double)sector.averageNewInvestExp() << "\t"
    << setw(11) << (double)sector.averageCapitalHa() << "\t"
    //<< setw(11) << (double)sector.averageProfitHa() << "\t"
    << setw(11) << (double)sector.averageProfitHa(  invest_cat[g->FIXED_HIRED_LAB].getAcquisitionCosts()  ) << "\t"
    << setw(11) << (double)sector.averageValueAddedHa() << "\t";
    condsecout << setw(11) << (double)sector.getTotalGmDEA() << "\t";
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        condsecout << setw(11) << (double)sector.averageRentOfType(i) << "\t";
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        condsecout << setw(11) << (double)sector.averageNewRentOfType(i) << "\t";
    }
    condsecout << setw(11) << (double)sector.total_land_input << "\t"
    << setw(11) << (double)sector.total_used_land << "\t"
    << setw(11) << (double)sector.total_labour_input_hours << "\t";

    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        condsecout << setw(11) << (double)sector.total_rented_plots_of_type[i]*g->PLOT_SIZE << "\t";
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        condsecout << setw(11) << (double)sector.total_new_rented_plots_of_type[i]*g->PLOT_SIZE << "\t";
    }
    condsecout << "\n";
    condsecout.close();
}
void
RegDataInfo::initCondensedSectorOutput() {
    string part2="condsecout.dat";
    string filename=g->OUTPUTFILE +part2;
    condsecout.open(filename.c_str(),ios::out|ios::app);
    condsecout.setf(ios_base::fmtflags(0), ios_base::floatfield);
    condsecout.precision(10);
    condsecout.setf(ios_base::left, ios_base::adjustfield);
    if (g->V==0) {
    condsecout << "scenario\t"
    << "replication\t"
    << "iteration\t"
    << "number_of_farms\t"
    << "farm_closings\t"
    << "farm_closings_hoc\t"
    << "ec_land_rent\t"
    << "economic land rent sc\t"
    << "av_profit_farm\t"
    << "av_rent\t"
    << "av_new_rent\t"
    << "av_farm_size\t"
    << "AWU_demand\t"
    << "av_Livestock_density\t"
    << "av_Livestock_density_ru\t"
    << "av_Livestock_density_gran\t"
    << "av_new_invest_exp\t"
    << "av_capital_ha\t"
    << "av_profit_ha\t"
    << "av_value_added_ha\t"
    << "total_gmDEA\t";
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        condsecout << ("average_rent_" +g->NAMES_OF_SOIL_TYPES[i]).c_str() << "\t";
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        condsecout << ("average_new_rent_" +g->NAMES_OF_SOIL_TYPES[i]).c_str() << "\t";
    }
    condsecout << "total_ha\t"
    << "used_land\t"
    << "total_labour_hrs\t";
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        condsecout << ("rented_" +g->NAMES_OF_SOIL_TYPES[i]).c_str() << "\t";
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        condsecout << ("new_rented_" +g->NAMES_OF_SOIL_TYPES[i]).c_str() << "\t";
    }
    condsecout << "\n";
    }
    condsecout.close();
}

void
RegDataInfo::initLegalTypeOutput(vector<RegInvestObjectInfo>& invest_cat,vector<RegProductInfo>& product_cat) {
//DCX   NICHT mit echten  sector_names verwechselt !
		vector<string> sector_names;
	if (g->V==0) {
        string part2="legal_type.dat";
        string filename=g->OUTPUTFILE +part2;

//DCX
condfarmout.open(filename.c_str(), ios::trunc);
//        condfarmout.open(filename.c_str(), ios::app);

        //if (g->CALC_LEGAL_TYPES) {
            for (unsigned int z=0;z<g->LEGAL_TYPES.size();z++) {
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"scenario");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"replication");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"period");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"land_occupied");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"rented_land");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"used_land");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"number_of_farms");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"farm_closings");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"family_labour");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"total_income");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"profit");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"economic_profit");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"withdrawal");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"equity_capital");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"liquidity");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"assets_wo_land");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"investment_expenditure");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"borrowed_cap");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"total_lt_interest_costs");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"st_interest_costs");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"st_interest_rev");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"hired_pay_fix");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"hired_pay_var");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"factor_rem_fix");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"factor_rem_var");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"depreciation");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"rent_exp");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"new_rent_exp");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"distance_costs");
                ;
                for (unsigned int i=0;i<product_cat.size();i++) {
                    sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+product_cat[i].getName());
                }
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"value_added");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"labour_input_h");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"land_input_ha");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"capital_input");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"real_sc_labour");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"sell_guelle_p");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"buy_guelle_p");
                for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                    sector_names.push_back(g->NAMES_OF_SOIL_TYPES[i]+string("_rent"));
                }
                for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                    sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+string("rented_")+g->NAMES_OF_SOIL_TYPES[i]);
                }
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"land_assets");
                for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                    sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+g->NAMES_OF_SOIL_TYPES[i]+string("_new_rent"));
                }
                for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                    sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+string("new_rented_")+g->NAMES_OF_SOIL_TYPES[i]);
                }
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"new_rented_plots");
                if (g->CALCULATE_CONTIGUOUS_PLOTS) {
                    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                        stringstream s1,s2;
		        s1<<"Contiguous_plots_of_type_" <<i;
                        s2<<"Av_size_of_type_" <<i;
                        sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+s2.str());
                        sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+s1.str());
                    }
                }
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"economic_land_rent");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"economic_land_rentsc");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"av_Livestock_density");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"av_Livestock_density_ru");
                sector_names.push_back(g->NAMES_OF_LEGAL_TYPES[z]+"av_Livestock_density_gran");

            }
        }

        for (unsigned int i=0;i<sector_names.size();i++) {
            condfarmout  <<  sector_names[i].c_str()  << "\t";
        }
        condfarmout << "\n";

        condfarmout.close();
}

void
RegDataInfo::printLegalTypeResults(const RegSectorResultsInfo& sector,vector<RegSectorResultsInfo*>& sector_type,
                             vector<RegProductInfo>& product_cat,vector<RegInvestObjectInfo>& invest_cat, int period) {

    vector<double> sector_values;

    // SECTOR RESULTS

    //if (g->CALC_LEGAL_TYPES) {
        for (unsigned int z=0;z<g->LEGAL_TYPES.size();z++) {
			//sector_values.push_back( g->SCENARIO               );
            sector_values.push_back( (double)g->V                   );
            sector_values.push_back( (double)period                           );
            sector_values.push_back( (double)sector_type[z]->total_number_of_plots*g->PLOT_SIZE     );
            sector_values.push_back( (double)sector_type[z]->total_rented_plots *g->PLOT_SIZE       );
            sector_values.push_back( (double)sector_type[z]->total_used_land       );
            sector_values.push_back( (double)sector_type[z]->total_number_of_farms     );
            sector_values.push_back( (double)sector_type[z]->farm_closings             );
            sector_values.push_back( sector_type[z]->total_family_labour       );
            sector_values.push_back( sector_type[z]->total_total_income        );
            sector_values.push_back( sector_type[z]->total_profit              );
            sector_values.push_back( sector_type[z]->total_economic_profit     );

            sector_values.push_back( sector_type[z]->total_withdrawal          );
            sector_values.push_back( sector_type[z]->total_equity_capital      );
            sector_values.push_back( sector_type[z]->total_liquidity           );
            sector_values.push_back( sector_type[z]->total_assets
                                     - sector_type[z]->total_land_assets  );
            sector_values.push_back( sector_type[z]->total_investment_expenditure   );
            sector_values.push_back( sector_type[z]->total_borrowed_capital    );
            sector_values.push_back( sector_type[z]->total_lt_interest_costs         );
            sector_values.push_back( sector_type[z]->total_st_interest_costs         );
            sector_values.push_back( sector_type[z]->total_st_interest_received      );
            sector_values.push_back( sector_type[z]->total_hired_labour_fix_pay      );
            sector_values.push_back( sector_type[z]->total_hired_labour_var_pay      );
            sector_values.push_back( sector_type[z]->total_factor_remuneration_fix   );
            sector_values.push_back( sector_type[z]->total_factor_remuneration_var   );
            sector_values.push_back( sector_type[z]->total_depreciation        );
            sector_values.push_back( sector_type[z]->total_rent                );
            sector_values.push_back( sector_type[z]->total_new_rent                );
            sector_values.push_back( sector_type[z]->total_distance_costs      );

            for (unsigned int i = 0; i < product_cat.size(); i++) {
                sector_values.push_back( sector_type[z]->getTotalUnitsProduced(i) );
            }

            sector_values.push_back( sector_type[z]->total_value_added         );
            sector_values.push_back( sector_type[z]->total_labour_input_hours  );
            sector_values.push_back( sector_type[z]->total_land_input          );
            sector_values.push_back( sector_type[z]->total_capital_input       );
            sector_values.push_back( sector_type[z]->real_sunk_costs_labour    );
//DCX
            bool ok = g->stdNameIndexs.find("MANSELL")!=end(g->stdNameIndexs);
            sector_values.push_back(ok ? product_cat[g->stdNameIndexs["MANSELL"]].getPriceExpectation():0 );
            sector_values.push_back(ok ? product_cat[g->stdNameIndexs["MANBUY"]].getPriceExpectation():0 );
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                sector_values.push_back( sector_type[z]->total_rent_of_type[i]         );
            }
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                sector_values.push_back( sector_type[z]->total_rented_plots_of_type[i]*g->PLOT_SIZE         );
            }
            sector_values.push_back( sector_type[z]->total_land_assets  );
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                sector_values.push_back( sector_type[z]->total_new_rent_of_type[i]*g->PLOT_SIZE         );
            }
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                sector_values.push_back( sector_type[z]->total_new_rented_plots_of_type[i]*g->PLOT_SIZE         );
            }
            sector_values.push_back( (double)sector_type[z]->total_new_rented_plots        );
            if (g->CALCULATE_CONTIGUOUS_PLOTS) {
                for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                    sector_values.push_back( sector_type[z]->av_no_of_contiguous_plots_per_type[i] );
                    sector_values.push_back( sector_type[z]->av_size_of_contiguous_plots_per_type[i] );
                }
            }
            sector_values.push_back( sector_type[z]->total_economic_land_rent );
            sector_values.push_back( sector_type[z]->total_economic_land_rent_sc );
            sector_values.push_back( sector_type[z]->averageLivestockDensity()  );
            sector_values.push_back( sector_type[z]->averageLivestockDensityRuminants()  );
            sector_values.push_back( sector_type[z]->averageLivestockDensityGranivores()  );
        }
//    }
//DCX
    string e="";//GetCurrentDir();
    printf(e.c_str());
    printf("\n");

    string part2="legal_type.dat";
    string filename=g->OUTPUTFILE +part2;
    secout.open(filename.c_str(), ios::app);

    //printf(filename.c_str());
    //printf("\n");
	secout << g->Scenario << "\t";
    for (unsigned int i=0;i<sector_values.size();i++) {
        secout   << sector_values[i]  << "\t";
    }
    secout << "\n";
    secout.close();
}

void
RegDataInfo::initFarmResults(vector<RegInvestObjectInfo>& invest_cat,vector<RegProductInfo>& product_cat) {
    string part2="farm_standard_indicators.dat";
    string file=g->OUTPUTFILE +part2;

    farmout.open(file.c_str(),ios::out|ios::trunc);
	//scenarioDate(farmout);

    // FARM DATA
	farmout << "scenario\t"
		<< "replication\t"
		<< "iteration\t"
		<< "farm_ID\t"
		<< "farm_name\t"
		<< "closed\t"
		<< "full_time\t"
		<< "legal_type\t"
		<< "farm_age\t";
	if (g->ManagerDemographics||g->YoungFarmer)
		farmout << "generation_change\t";
	if (g->YoungFarmer)
		farmout << "pay_young_farmer\t";
    farmout << "farm_class\t"
		<< "management_coeff\t"
		<< "farm_size_class\t"
		<< "econ_size_class\t"

		// STRUCTURE
		<< "ec_size_ESU\t"
		<< "total_ha\t"
		<< "owned_ha\t"
		<< "used_land\t"

		// PRODUCTION
		<< "revenue\tlu\truminants\tgranivore\t"

		// COSTS
		<< "overheads\t"
		<< "maintenance\t"
		<< "annuity\t"
		<< "depreciation\t"
		<< "wages_paid\t"
		<< "rent_paid\t"
		<< "interest_paid\t";

    // SUBSIDIES
	if(g->LP_MOD) {
	farmout    << "coupled_subs_unmod\t"
    	<< "decoupled_subs_unmod\t"
    	<< "total_premium_mod\t";
} else {
farmout
    << "inc_payment_farm\t"
    << "coupled_subs\t";
}
    farmout
    // LAND
    << "econ_land_rent\t";
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        string soil="rent_" + g->NAMES_OF_SOIL_TYPES[i] + "\t";
        farmout << soil.c_str();
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        string soil="total_land_" + g->NAMES_OF_SOIL_TYPES[i] + "\t";
        farmout << soil.c_str();
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        string soil="rented_land_" + g->NAMES_OF_SOIL_TYPES[i] + "\t";
        farmout << soil.c_str();
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        string soil="new_rent_" + g->NAMES_OF_SOIL_TYPES[i] + "\t";
        farmout << soil.c_str();
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        string soil="new_rented_land_" + g->NAMES_OF_SOIL_TYPES[i] + "\t";
        farmout << soil.c_str();
    }
    farmout
    // BALANCE SHEET
    << "total_assets\t"
    << "total_fixed_assets\t"
    << "land_assets\t"
    << "liquidity\t"
    << "borrowed_capital\t"
    << "short_term_borrowed\t"

    // FINANCIAL SITUATION
    << "profit\t"
    << "equity_capital\t"
    << "change_in_equity\t"
    << "net_investment\t"

    // INCOME
    << "labour_input\t"
    << "family_labour\t"
    << "Withdrawal\t"
    << "farm_net_value_added\t"
    << "total_hh_income\t"
    << "off_farm_income\t"
    << "labour_substitution\t";

    // INVESTMENT
    if (g->CALCULATE_CONTIGUOUS_PLOTS) {
        for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
            stringstream s1,s2;
            s1<<"Contiguous_plots_of_type_"<<i;
            s2<<"Av_size_of_type_"<<i;
            farmout << s1.str() << "\t" << s2.str() << "\t";
        }
    }
    farmout << "global_strategy\t";
    farmout << "display_modulation\n";
    farmout.close();
}
void
RegDataInfo::initFarmProduction(vector<RegProductInfo>& product_cat) {
    string part2="farm_production.dat";
    string file=g->OUTPUTFILE +part2;

    farmprodout.open(file.c_str(),ios::out|ios::trunc);
	//scenarioDate(farmprodout);

    farmprodout << "scenario\t"
    << "replication\t"
    << "iteration\t"
    << "farm_ID\tfarm_name\tdisplay_modulation\t"
    << "ha\t";
    // FARM DATA
    for (unsigned int i=0;i<product_cat.size();i++) {
        farmprodout << product_cat[i].getName().c_str() << "\t";
    }
    farmprodout << "\n";
    farmprodout.close();
}
void
RegDataInfo::printFarmProduction(const RegFarmInfo* farm,
                                 vector<RegProductInfo>& product_cat,
                                 int period) {
    farmprodout.setf(ios_base::fmtflags(0), ios_base::floatfield);
    //farmprodout.precision(8);
    farmprodout.setf(ios_base::left, ios_base::adjustfield);

	farmprodout << setw(11) << g->Scenario << "\t"
    << setw(11) << (double)g->V << "\t"
    << setw(11) << (double)period  << "\t"
    << setw(11) << (double)farm->getFarmId()            << "\t"
    << setw(11) << (farm->getFarmName()).c_str()            << "\t"
    << setw(11) << farm->getDisplayModulation()            << "\t"
    << setw(11) << (double)farm->getNumberOfPlots()*g->PLOT_SIZE << "\t";
    for (unsigned int i = 0; i < product_cat.size(); i++) {
        farmprodout << setw(11) << farm->getUnitsOfProduct(i) << "\t";
    }
    farmprodout << "\n";

}
void
RegDataInfo::printEnvDataUsage(const RegFarmInfo* farm,
                               vector<RegProductInfo>& product_cat,
                               int period) {
    envusageout.setf(ios_base::fmtflags(0), ios_base::floatfield);
    //envusageout.precision(10);
    envusageout.setf(ios_base::left, ios_base::adjustfield);
    envusageout << setw(11) << (double)period  << "\t"
    << setw(11) << (double)farm->getFarmId() << "\t";

    double N_farmUsage=0;
    double P2O5_farmUsage=0;
    double K2O_farmUsage=0;
    double Fungicides_farmUsage=0;
    double Herbicides_farmUsage=0;
    double Insecticides_farmUsage=0;
    double Water_farmUsage=0;
    double Soil_farmLoss=0;

    for (unsigned int i = 0; i < product_cat.size(); i++) {
        N_farmUsage += farm->getUnitsOfProduct(i)*product_cat[i].getN_usage();
        P2O5_farmUsage += farm->getUnitsOfProduct(i)*product_cat[i].getP2O5_usage();
        K2O_farmUsage += farm->getUnitsOfProduct(i)*product_cat[i].getK2O_usage();
        Fungicides_farmUsage += farm->getUnitsOfProduct(i)*product_cat[i].getFungicides_usage();
        Herbicides_farmUsage += farm->getUnitsOfProduct(i)*product_cat[i].getHerbicides_usage();
        Insecticides_farmUsage += farm->getUnitsOfProduct(i)*product_cat[i].getInsecticides_usage();
        Water_farmUsage += farm->getUnitsOfProduct(i)*product_cat[i].getWater_usage();
        Soil_farmLoss += farm->getUnitsOfProduct(i)*product_cat[i].getSLossCoeff();
    }
    envusageout << setw(11) << (double)N_farmUsage  << "\t"
    << setw(11) << (double)P2O5_farmUsage << "\t"
    << setw(11) << (double)K2O_farmUsage << "\t"
    << setw(11) << (double)Fungicides_farmUsage << "\t"
    << setw(11) << (double)Herbicides_farmUsage << "\t"
    << setw(11) << (double)Insecticides_farmUsage << "\t"
    << setw(11) << (double)Water_farmUsage << "\t"
    << setw(11) << (double)Soil_farmLoss << "\t"
    << "\n";
}

void
RegDataInfo::initFarmInvestment(const vector<RegInvestObjectInfo >& invest_cat) {
    string part2="farm_investment.dat";
    string file=g->OUTPUTFILE +part2;

    farminvest.open(file.c_str(),ios::out|ios::trunc);
	//scenarioDate(farminvest);

    farminvest << "scenario\t"
    << "replication\t"
    << "iteration\t"
    << "farm_ID\tfarm_name\t"
    << "ha\t";
    // INVESTMENT
    for (unsigned int i=0;i<invest_cat.size();i++) {
        farminvest << invest_cat[i].getName().c_str() << "\t";
    }
    farminvest << "\n";
    farminvest.close();
}

void
RegDataInfo::printFarmInvestment(const RegFarmInfo* farm,
                                 const vector<RegInvestObjectInfo >& invest_cat, int period) {
    // Formatierungen
    farminvest.setf(ios_base::fmtflags(0), ios_base::floatfield);
    //farminvest.precision(8);
    farminvest.setf(ios_base::left, ios_base::adjustfield);
    // FARM DATA
    farminvest  << setw(11) << g->Scenario << "\t"
    << setw(11) << (double)g->V << "\t"
    << setw(11) << (double)period  << "\t"
    << setw(11) << (double)farm->getFarmId()            << "\t"
    << setw(11) << (farm->getFarmName()).c_str()            << "\t"
    << setw(11) << (double)farm->getNumberOfPlots()*g->PLOT_SIZE << "\t";
    for (unsigned int i = 0; i < invest_cat.size(); i++) {
        farminvest << setw(11) << (double)farm->getNewInvestmentsOfCatalogNumber(i) << "\t";
        //			farminvest << setw(11) << (double)farm->getInvestmentsOfCatalogNumber(i) << "\t";
    }
    farminvest << "\n";
}

void
RegDataInfo::printFarmResults(const RegFarmInfo* farm,
                              const vector<RegInvestObjectInfo >& invest_cat,
                              const vector<RegProductInfo>& product_cat,
                              int period,int c) {
    farmout.setf(ios_base::fmtflags(0), ios_base::floatfield);
   // farmout.precision(8);
    farmout.setf(ios_base::left, ios_base::adjustfield);

//    for(int i=0;i<farm_results.size();i++) {
	farmout << g->Scenario << "\t";
    for (unsigned int j=0;j<farm_results[c].size();j++) {
        if (j==3) {
            farmout << setw(11) <<  farm->getFarmName().c_str() << "\t";
            farmout << setw(11) << farm->getFarmClosed() << "\t";
        }
        farmout << setw(11) << farm_results[c][j] << "\t";
    }
    farmout << "\n";
//    }
}


void
RegDataInfo::cacheFarmResults(const RegFarmInfo* farm,
                              const vector<RegInvestObjectInfo >& invest_cat,
                              const vector<RegProductInfo>& product_cat,
                              int period) {
    farm_results_c=0;
    // Formatierungen
    vector <double> res;
    //res.push_back(g->Scenario);
    res.push_back((double)g->V);
    res.push_back((double)period);
    res.push_back((double)farm->getFarmId()            );
    res.push_back((double)farm->getFullTime()          );
    res.push_back((int)farm->getLegalType()		    );
//    res.push_back(farm->getFarmName().c_str()          );
    res.push_back((double)farm->getFarmAge()           );
	if (g->ManagerDemographics||g->YoungFarmer)
		res.push_back(farm->getGenerationChange());
	if (g->YoungFarmer)
		res.push_back(farm->getYoungFarmerPay());
    res.push_back((int)farm->getFarmClass()    			);
    res.push_back((double)farm->getManagementCoefficient()    );
    res.push_back( farm->getFarmSizeClass()    		);
    res.push_back( farm->getEconomicSizeClass()    	);

    // STRUCTURE
    res.push_back(((double)farm->getStandardGrossMargin()/g->ESU) );
    res.push_back((double)farm->getLandInput());
    res.push_back(farm->getInitialOwnedLand()             );
    res.push_back(farm->getUnitsProducedOfGroup(0)             );

    // PRODUCTION
    res.push_back((double)farm->getOutputRevenue() 	);

    res.push_back((double)farm->getTotalLU());
            // total livestock untis
    res.push_back((double)farm->getTotalLU("GRASSLAND"));
            // total livestock untis
    res.push_back((double)farm->getTotalLU("PIG/POULTRY"));
    // COSTS
    res.push_back(farm->getOverheads()         		);
    res.push_back(farm->getTotalMaintenance()  		);
    res.push_back((double)farm->getAnnuity() 			);
    res.push_back(farm->getDepreciation()              );
    res.push_back((farm->getFarmHiredLabourFixPay()
                   + farm->getFarmHiredLabourVarPay()) );
    res.push_back(farm->getFarmRentExp()               );
    res.push_back((farm->getLtInterestCosts()
                   + farm->getStInterestCosts())      	);

    // SUBSIDIES
	if(g->LP_MOD) {
    	res.push_back(farm->getUnitsOfProduct(g->stdNameIndexs["COUPLED_PREM_UNMOD"]) );
    	res.push_back(farm->getUnitsOfProduct(g->stdNameIndexs["DECOUPLED_PREM_UNMOD"]) );
    	res.push_back(farm->getUnitsOfProduct(g->stdNameIndexs["TOTAL_PREM_MODULATED"]) );
	} else {
    	res.push_back(farm->getModulatedIncomePaymentFarm() );
    	res.push_back(farm->getUnitsOfProduct(g->stdNameIndexs["PREMIUM"]) );
	}

    // LAND
    res.push_back(farm->getEconomicLandRent() );
    ;
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        res.push_back(farm->getAvRentOfType(i) );
        ;
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        res.push_back(farm->getLandInputOfType(i) );
        ;
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        res.push_back(farm->getRentedLandOfType(i) );
        ;
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        res.push_back(farm->getAvNewRentOfType(i) );
        ;
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        res.push_back(farm->getNewRentedLandOfType(i) );
        ;
    }


    // BALANCE SHEET
    res.push_back(farm->getAssets()            );
    res.push_back(farm->getAssetsProdWoLand()  );
    res.push_back(farm->getLandAssets()		);
    res.push_back(farm->getLiquidity()			);
//			res.push_back((farm->getEquityCapital()
//							- farm->getAssets())             );
    res.push_back(farm->getLtBorrowedCapital()      );
    res.push_back(farm->getStBorrowedCapital()        );

    // FINANCIAL SITUATION
    res.push_back(farm->getProfit()            		);
    res.push_back(farm->getEquityCapital()     		);
    res.push_back((double)farm->getEcChange()    		);
    res.push_back(((double)farm->getNewInvestmentExpenditure()
                   - farm->getDepreciation())	    );

    // INCOME
    res.push_back(farm->labour->getLabourInputHours()     );
    res.push_back((double)farm->labour->getFamilyLabour() );
    res.push_back((double)farm->getWithdrawal()            );
    res.push_back(farm->getValueAdded()               	   );
    res.push_back(farm->getTotalIncome()       		   );
    res.push_back((farm->getFarmFactorRemunerationFix()+
                   farm->getFarmFactorRemunerationVar())  );
    res.push_back((double)farm->getLabSub()  );
    ;
    if (g->CALCULATE_CONTIGUOUS_PLOTS) {
        for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {

            res.push_back((double)farm->getContiguousPlotsOfType(i)  );
            res.push_back((double)farm->getAvSizeOfContiguousPlotOfType(i)  );
            ;
        }
    }
    res.push_back(g->GLOBAL_STRATEGY);
    res.push_back((double)farm->getDisplayModulation());

    farm_results.push_back(res);
}
