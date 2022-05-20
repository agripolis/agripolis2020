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

// RegMarket.cpp
#include <iomanip>
#include <algorithm>
#include <set>
#include "RegMarket.h"
#include "RegResults.h"
#include "textinput.h"

using namespace std;
RegMarketInfo::RegMarketInfo(RegGlobalsInfo* globals) :g(globals) {
    obj_backup=NULL;
}

RegMarketInfo::~RegMarketInfo() {
    if (obj_backup) delete obj_backup;
}

RegMarketInfo::RegMarketInfo(const RegMarketInfo& rh,RegGlobalsInfo* G):g(G) {
    obj_backup=NULL;
    num_products=rh.num_products;
    priceflex_vector=rh.priceflex_vector;
    price_change_vector=rh.price_change_vector;
    orig_price_vector=rh.orig_price_vector;
    orig_var_costs_vector=rh.orig_var_costs_vector;
    lives_units_vector=rh.lives_units_vector;
    min_price_vector=rh.min_price_vector;
    price_support_vector=rh.price_support_vector;
    price_difference_vector=rh.price_difference_vector;
    price_vector=rh.price_vector;
    price_expectation_vector=rh.price_expectation_vector;
    product_cat=rh.product_cat;
    exp_price_change_vector=rh.exp_price_change_vector;
}

void
RegMarketInfo::createMarket(RegEnvInfo* Env) {
	set<int> groups;
    if (marketdata.premiumName.compare("")!=0)
        g->premiumName=marketdata.premiumName;
    else g->premiumName = "PREMIUM";

    num_products=marketdata.products.size();
	g->setNumberOfProducts(num_products); 

    double p,pc,vc,lun;
    product_cat.resize(num_products);
    int counter=0;
	for (int i=0;i<num_products;i++) {
		int gr = marketdata.products[i].group;
		if (gr>=0 && !groups.count(gr))
			groups.insert(gr);

        //product id
        product_id[marketdata.products[i].name]= i;

        orig_price_vector.push_back(p=marketdata.products[i].price);
        orig_var_costs_vector.push_back(vc = marketdata.products[i].varcost);
        lives_units_vector.push_back(lun = marketdata.products[i].luperplace);
        priceflex_vector.push_back(marketdata.products[i].priceflex);
        exp_price_change_vector.push_back(1);
        pc = marketdata.products[i].changerate;
        price_change_vector.push_back(pc);
        product_cat[counter].setAttrib(i,
                marketdata.products[i].type,marketdata.products[i].group,
				marketdata.products[i].name,marketdata.products[i].stdName,marketdata.products[i].prodbytype);
        product_cat[counter].setLU(lun);
        product_cat[counter].setPrice(p);
        product_cat[counter].setPriceExpectation(p);
        product_cat[counter].setVarCost(vc);
        double lab = marketdata.products[i].labour;
        product_cat[counter].setLabour(lab);
        price_vector.push_back(p);
        price_expectation_vector.push_back(p);
        bool leg= marketdata.products[i].premium;

        
        if (leg) {
            product_cat[counter].setPremiumLegitimation(true);
//            product_cat[counter].setPremiumCol(xl->readIntValueAt(premium_col,i));
//            product_cat[counter].setPremiumRow(xl->readIntValueAt(premium_row,i));
            product_cat[counter].setPremium(marketdata.products[i].initprem);
			product_cat[counter].setPolicyPremium(marketdata.products[i].initprem);
            product_cat[counter].setReferencePremium(marketdata.products[i].refprem);
            product_cat[counter].setReferencePremiumCalcTime(marketdata.products[i].xyears);
        } else {
            product_cat[counter].setPremiumLegitimation(false);
        }
        bool  ps=marketdata.products[i].pricesupp;
        if (ps) {
            product_cat[counter].setPriceSupport(true);
        } else {
            product_cat[counter].setPriceSupport(false);
        }
        min_price_vector.push_back(0);
        price_support_vector.push_back(0);
        price_difference_vector.push_back(0);
        if (product_cat[counter].getProductType()==1)
            g->setEqInterest(product_cat[counter].getPrice());
        counter++;
    }

	g->PRODUCTGROUPS = groups.size();
	g->INVEST_GROUPS = g->PRODUCTGROUPS;
	//cout << "#groups: " << groups.size() << "\n";

    if (g->ENV_MODELING) {
        int counter=0;
        for (int i=0; i< num_products; i++) {
                product_cat[counter].setEnvAttrib(
                    envmarketdata.envproducts[i].n, envmarketdata.envproducts[i].p2o5,
                    envmarketdata.envproducts[i].k2o,envmarketdata.envproducts[i].fungicide,
                    envmarketdata.envproducts[i].herbicide,envmarketdata.envproducts[i].insecticide,
                    envmarketdata.envproducts[i].water,envmarketdata.envproducts[i].soil_loss);

                Env->addMktIDToHabitat(counter, envmarketdata.envproducts[i].biohabitat);
                counter++;
        }
    }

	//init soil service values
	if (g->HAS_SOILSERVICE) {
		map <string,int> prodId;
		for (unsigned int i=0; i< marketdata.products.size(); i++){
			prodId[marketdata.products[i].name]=i;
			getProductCat()[i].setSoilservice(false);
		}
	
		int sz = yielddata.size();
		map <string,int> soilId;
		for (int i=0; i< g->NO_OF_SOIL_TYPES;++i)
			soilId[g->NAMES_OF_SOIL_TYPES[i]]=i;

		for (int i=0; i<sz; ++i) {
			getProductCat()[prodId[yielddata[i].name]].setSoilservice(yielddata[i].a, yielddata[i].b, 
					yielddata[i].c, yielddata[i].d, 
					yielddata[i].e, yielddata[i].f, yielddata[i].c_plat,
					yielddata[i].gamma, soilId[yielddata[i].soiltype], true, yielddata[i].dyn,
					yielddata[i].p, yielddata[i].k, yielddata[i].pesticide, yielddata[i].energyvar);
		}
	}
}

void
RegMarketInfo::debug(string filename) {
    ofstream out;
    out.open(filename.c_str(),ios::trunc);
    out << "orig_price_vector\n";
    for (unsigned int i=0;i<orig_price_vector.size();i++) {
        out << orig_price_vector[i] << "\n";
    }
    out << "var_costs_vector\n";
    for (unsigned int i=0;i<orig_var_costs_vector.size();i++) {
        out << orig_var_costs_vector[i] << "\n";
    }
    out << "priceflex_vector\n";
    for (unsigned int i=0;i<priceflex_vector.size();i++) {
        out << priceflex_vector[i] << "\n";
    }
    out << "price_change_vector\n";
    for (unsigned int i=0;i<price_change_vector.size();i++) {
        out << price_change_vector[i] << "\n";
    }
    out << "product_cat\n";
    for (unsigned int i=0;i<product_cat.size();i++) {
        out << product_cat[i].getCatalogNumber() << "\t"
        << product_cat[i].getPrice() << "\t"
        << product_cat[i].getVarCost() << "\t"
        << product_cat[i].getProductType() << "\n";
    }
    out.close();
}

void
RegMarketInfo::calculatePricevectors(RegSectorResultsInfo& Sector, int iteration) {
	vector<double> prod;
    double mansell_price_i = 0;
	prod.resize(product_cat.size());
	for (int i = 0; i < (int)product_cat.size(); i++) {
		string aname = product_cat[i].getStdName();
		if (g->FIX_PRICES) {
			price_vector[i] = orig_price_vector[i];
		}
		else {
			double unitsproduced = Sector.getTotalUnitsProduced(i);
			double totallandha = Sector.getTotalLandInput();
			prod[i] = Sector.getTotalUnitsProduced(i);
			// price function like in Alfons original code.
			// adjust production of sows because of pig replacement
			if (!aname.compare("SOW")) { // sow production
				prod[i] = (unitsproduced - Sector.getTotalUnitsProduced(g->stdNameIndexs["FATTENINGPIGS"])*g->PIGS_PER_PLACE / g->PIGLETS_PER_SOW);
				if (prod[i] <= 0)
					prod[i] = 1;
			}
			else {
				prod[i] /= totallandha;
			}
			
			// MANURE MARKET
			if (!aname.compare("MANSELL") || !aname.compare("MANBUY")) {
				double totalmanuresold = Sector.getTotalUnitsProduced(g->stdNameIndexs["MANSELL"]);
				double totalmanurebought = Sector.getTotalUnitsProduced(g->stdNameIndexs["MANBUY"]);
				double excessmanure = 0;
				double changeperha = g->CHANGEPERHA;  
				double pricechange = 0;
				double exprice = 0;
				if (totallandha != 0) {
					excessmanure = (totalmanuresold - totalmanurebought) / totallandha;
				}
				if (i == g->stdNameIndexs["MANSELL"]) { // sell
					pricechange = price_vector[i] - excessmanure * changeperha;
				}
				else {  //g->MANBUY
					pricechange = price_vector[i] + excessmanure * changeperha;
				}
				exprice = (orig_price_vector[i] + pricechange) / 2;
				if (exprice > 0) exprice = 0;
				price_vector[i] = exprice;
			}

            //Manure price Emsland, April 2022
            else if (!aname.compare("MANSELL_EMS") || !aname.compare("MANBUY_EMS")) {
                if (!aname.compare("MANSELL_EMS")) {
                  double ts = Sector.getTotalUnitsProduced(g->stdNameIndexs["MANSELL_EMS"]);
                  double tb = Sector.getTotalUnitsProduced(g->stdNameIndexs["MANBUY_EMS"]);
                  double excessmanure = (ts-tb-g->Manure_Price_D)/(ts+tb+1);
                  double pricechange = 0;
                  pricechange = -excessmanure * g->Manure_Price_X;

                  price_vector[i] = price_vector[i] * (1 + pricechange);
                  mansell_price_i = price_vector[i];
                }
                else {//BUY  !!BUY must be below SELL
                  price_vector[i] = (-1)*mansell_price_i - g->Manure_Transport_Cost; 
                }

            }

			//MILK QUOTA
			else if (!aname.compare("GETQUOTA")) { // lease milk quota
				double pricechange = 0;
				// milk produced in sector
				double totalmilk = Sector.getTotalUnitsProduced(g->stdNameIndexs["MILK"]) * g->MILKPROD;
				pricechange = price_vector[i] * price_change_vector[i];
				if (totalmilk > (g->MILKUPPERLIMIT * g->REGION_MILK_QUOTA)) {
					pricechange *= g->CHANGEUP;//0.98;
				}
				if (totalmilk < (g->MILKLOWERLIMIT * g->REGION_MILK_QUOTA)) {
					pricechange *= g->CHANGEDOWN;//1.12;
				}
				price_vector[i] = pricechange; // lease
			}

			else if (!aname.compare("LETQUOTA")) { // let milk quota
				double pricechange = 0;
				// milk produced in sector
				double totalmilk = Sector.getTotalUnitsProduced(g->stdNameIndexs["MILK"]) * g->MILKPROD;
				pricechange = price_vector[i] * price_change_vector[i];
				if (totalmilk > (g->MILKUPPERLIMIT * g->REGION_MILK_QUOTA)) {
					pricechange *= g->CHANGEUP;//0.98;
				}
				if (totalmilk < (g->MILKLOWERLIMIT * g->REGION_MILK_QUOTA)) {
					pricechange *= g->CHANGEDOWN;//1.12;
				}
				price_vector[i] = pricechange; // let

				g->QUOTA_PRICE = price_vector[i];
			}

			//MILK QUOTA SWEDEN
			else if (!aname.compare("GETQUOTA_SWEDEN")) {  //i==g->GETQUOTA) { // lease milk quota
				double pricechange = 0;
				// milk produced in sector
				double totalmilk = Sector.getTotalUnitsProduced(g->stdNameIndexs["MILK"]) * g->MILKPROD;
				double excess_milk = totalmilk / g->REGION_MILK_QUOTA;
				// Adjust milk price by rate of excess production
				pricechange = price_vector[i] * excess_milk;
				//To adjust for radical reduction in milk output via variable price function
				pricechange *= price_change_vector[i];

				price_vector[i] = pricechange; // lease
			}
			else if (!aname.compare("LETQUOTA_SWEDEN")) {//i==g->LETQUOTA) { // let milk quota
				double pricechange = 0;
				// milk produced in sector
				double totalmilk = Sector.getTotalUnitsProduced(g->stdNameIndexs["MILK"]) * g->MILKPROD;
				double excess_milk = totalmilk / g->REGION_MILK_QUOTA;
				// Adjust milk price by rate of excess production
				pricechange = price_vector[i] * excess_milk;
				//To adjust for radical reduction in milk output via variable price function
				pricechange *= price_change_vector[i];
				price_vector[i] = pricechange; // let
				g->QUOTA_PRICE = price_vector[i];
			}

			//    SUCKLER CALF MARKET (Västerbotten)
			// assume that calf demand must be met by calf supply in region
			// but supply can be greater than demand as it is possible to export calves
			else if (!aname.compare("SELLCALF_SUCK") || !aname.compare("BUYCALF_SUCK")) {
				// calculate excess suckler calf demand
				int buyind = g->stdNameIndexs["BUYCALF_SUCK"];
				int sellind = g->stdNameIndexs["SELLCALF_SUCK"];
				double excess_suck_calf = Sector.getTotalUnitsProduced(buyind) - Sector.getTotalUnitsProduced(sellind);
				// if there is excess in supply
				if (excess_suck_calf>0) {
					//increase price
					//the price for selling must be lower than for buying to avoid circular trades
					price_vector[i] *= 1.0;  // 1.0 Västerbotten
				}
				else {
					//otherwise reduce price
					price_vector[i] *= 1.0;  //1.0 Västerbotten
				}
			}

			//    SUCKLER CALF MARKET (Joenkeping)
			// assume that calf demand must be met by calf supply in region
			// but supply can be greater than demand as it is possible to export calves
			else if (!aname.compare("SELLCALF_SUCK_JOENKEPING") || !aname.compare("BUYCALF_SUCK_JOENKEPING")) {
				// calculate excess suckler calf demand
				int buyind = g->stdNameIndexs["BUYCALF_SUCK_JOENKEPING"];
				int sellind = g->stdNameIndexs["SELLCALF_SUCK_JOENKEPING"];
				double excess_suck_calf = Sector.getTotalUnitsProduced(buyind) - Sector.getTotalUnitsProduced(sellind);
				// if there is excess in supply
				if (excess_suck_calf>0) {
					//increase price
					//the price for selling must be lower than for buying to avoid circular trades
					price_vector[i] *= 1.01;  //
				}
				else {
					//otherwise reduce price
					price_vector[i] *= 0.99;  //1.0 Västerbotten
				}
			}
			// DAIRY CALF MARKET
			//Jump in the loop when we are at the calf activities
			else if (!aname.compare("SELLCALF_DAIRY") || !aname.compare("BUYCALF_DAIRY")) {
				// calculate excess dairy calf demand
				double  excess_dairy_calf;
				//DCX
				if (Sector.getTotalUnitsProduced(g->stdNameIndexs["SELLCALF_DAIRY"])>0)
					excess_dairy_calf = Sector.getTotalUnitsProduced(g->stdNameIndexs["BUYCALF_DAIRY"]) / Sector.getTotalUnitsProduced(g->stdNameIndexs["SELLCALF_DAIRY"]);
				else excess_dairy_calf = 2;

				// if excess demand   > 10%
				price_vector[i] *= excess_dairy_calf;

				//adjust for imminent policy change
				price_vector[i] *= price_change_vector[i];
			}
			else {
				/*if (i!=g->MANSELL && i!=g->MANBUY
				&& i!=g->GETQUOTA			&& i!=g->LETQUOTA
				&& i!=g->BUYCALF_SUCK		&& i!=g->SELLCALF_SUCK
				&& i!=g->BUYCALF_DAIRY	&& i!=g->SELLCALF_DAIRY) {
				//*/
				if (g->USE_VARIABLE_PRICE_CHANGE) {
					price_vector[i] = price_vector[i] * price_change_vector[i];
				}
				else {
					if (unitsproduced > 0) {
						price_vector[i] = orig_price_vector[i]
							* pow(price_change_vector[i], (iteration + 1))
							* pow(prod[i], -priceflex_vector[i]);
					}
					else {
						price_vector[i] = orig_price_vector[i]
							* pow(price_change_vector[i], (iteration + 1));
					}
				}
			}
		} //else 

		  //price_expectation_vector		    
		  //if (price_vector[i]*price_expectation_vector[i] < 0)  ;
		if (price_vector[i] < 0) {
			price_expectation_vector[i] = -pow(price_vector[i] * price_expectation_vector[i], 0.5);
		}
		else {
			price_expectation_vector[i] = pow(price_vector[i] * price_expectation_vector[i], 0.5);
		}
/*		if (g->USE_VARIABLE_PRICE_CHANGE) {        // bei Preisschwankungen auskommentieren von hier
			price_expectation_vector[i] *= exp_price_change_vector[i];
		}
		else {
			price_expectation_vector[i] *= price_change_vector[i];
		}
		//*/
	}
	return;
}


void
RegMarketInfo::priceFunction(RegSectorResultsInfo& Sector,Evaluator* evaluator, int iteration ) {
    if (g->USE_VARIABLE_PRICE_CHANGE) {
        for (unsigned int i=0;i<product_cat.size();i++) {
            price_change_vector[i]=exp_price_change_vector[i];
			int ind = evaluator->indOfVariable(getNameOfProduct(i)+"_price_change");
			if (ind>=0){
				exp_price_change_vector[i]=evaluator->getVariable(ind);//getNameOfProduct(i)+"_price_change");
				product_cat[i].setPriceChange(evaluator->getVariable(ind));
			}
			else exp_price_change_vector[i]= product_cat[i].getPriceChange();
        }
    }

	calculatePricevectors(Sector, iteration);

	for (unsigned int i=0; i<product_cat.size(); ++i) {
        product_cat[i].setPrice(price_vector[i]);
        product_cat[i].setPriceExpectation(price_expectation_vector[i]);

        if (product_cat[i].getPriceSupport()==true) {
            product_cat[i].setPrice(price_vector[i]*(1+price_support_vector[i]));
            product_cat[i].setPriceExpectation(price_expectation_vector[i]*(1+price_support_vector[i]));
            if (product_cat[i].getPrice()<min_price_vector[i])
                product_cat[i].setPrice(min_price_vector[i]);
            if (product_cat[i].getPriceExpectation()<min_price_vector[i])
                product_cat[i].setPriceExpectation(min_price_vector[i]);
            price_difference_vector[i]=getPriceDifferenceOfProduct(i);
        } else {
            product_cat[i].setPrice(price_vector[i]);
            product_cat[i].setPriceExpectation(price_expectation_vector[i]);
        }
    }
	return;
}

void
RegMarketInfo::outputPrice(int it) {
    ofstream out;
    out.open("prices.txt",ios::app);
    out.setf(ios_base::fmtflags(0), ios_base::floatfield);
    out.precision(10);
    out.setf(ios_base::left, ios_base::adjustfield);

    if (it == 0) {
        out << "\t"
        << "price\t";
        out << "price_exp\t";
        out << "\n";
        out << "period\t";
        for (unsigned int i=0;i<product_cat.size();i++) {
            out << product_cat[i].getName().c_str() << "\t";
            out << "Exp_" << product_cat[i].getName().c_str() << "\t";
        }
        out << "\n";
    }
    out << setw(11) << (double)it << "\t";
    for (unsigned int i=0;i<product_cat.size();i++) {
        out << setw(11) << product_cat[i].getPrice() << "\t";
        out << setw(11) << product_cat[i].getPriceExpectation() << "\t";
    }
    out << "\n";
	out.close();
}

vector <RegProductInfo>&
RegMarketInfo::getProductCat() {
    return product_cat;
}
RegProductInfo
RegMarketInfo::getProductNumber(int i) {
    return product_cat[i];
}

string RegMarketInfo::getNameOfProduct(int i){
        return product_cat[i].getName();
};

vector<string>
RegMarketInfo::getProductGroups() {
    vector <string> n;
    for (unsigned int i=0;i<product_cat.size();i++) {
        bool hit=false;
        string name=product_cat[i].getClass();
        if (name!="NON") {
            for (unsigned int j=0;j<n.size();j++) {
                if (n[j]==name) hit=true;
            }
        } else {
            hit=true;
        }
        if (!hit) n.push_back(name);
    }
    return n;
}
vector<string>
RegMarketInfo::getNamesOfProductGroup(string name) {
    vector <string> n;
    for (unsigned int i=0;i<product_cat.size();i++) {
        string na=product_cat[i].getClass();
        if (na==name) n.push_back(product_cat[i].getName());
    }
    return n;

}
vector<double>
RegMarketInfo::getTotalCostsOfGroup(string name) {
    vector <double> n;
    for (unsigned int i=0;i<product_cat.size();i++) {
        string na=product_cat[i].getClass();
        if (na==name) n.push_back(product_cat[i].getVarCost());
    }
    return n;

}
vector<double>
RegMarketInfo::getPricesOfGroup(string name) {
    vector <double> n;
    for (unsigned int i=0;i<product_cat.size();i++) {
        string na=product_cat[i].getClass();
        if (na==name) n.push_back(product_cat[i].getPrice());
    }
    return n;
}
vector<string>
RegMarketInfo::getNamesOfPremiumProducts() {
    vector<string> names;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPremiumLegitimation()==true) {
            names.push_back(product_cat[i].getName());
        }
    }
    return names;
}
vector<double>
RegMarketInfo::getPremiumOfProducts() {
    vector<double> premium;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPremiumLegitimation()==true) {
            premium.push_back(product_cat[i].getPremium());
        }
    }
    return premium;
}
vector<int>
RegMarketInfo::getColsOfPremiumProducts() {
    vector<int> cols;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPremiumLegitimation()==true) {
            int x = product_cat[i].getPremiumCol();
            cols.push_back(x);
//            cols.push_back(product_cat[i].getPremiumCol());
        }
    }
    return cols;
}
vector<int>
RegMarketInfo::getRowsOfPremiumProducts() {
    vector<int> rows;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPremiumLegitimation()==true) {
            rows.push_back(product_cat[i].getPremiumRow());
        }
    }
    return rows;
}
vector<int>
RegMarketInfo::getCatNumberOfPremiumProducts() {
    vector<int> cn;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPremiumLegitimation()==true) {
            cn.push_back(product_cat[i].getCatalogNumber());
        }
    }
    return cn;
}
void
RegMarketInfo::setPremium(vector<double>& premium) {
    int counter=0;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPremiumLegitimation()==true) {
            product_cat[i].setPremium(premium[counter]);
            counter++;
        }
    }
}

vector<string>
RegMarketInfo::getNamesOfPriceSupportProducts() {
    vector<string> n;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPriceSupport()==true)
            n.push_back(product_cat[i].getName());
    }
    return n;
}
vector<double>
RegMarketInfo::getPriceSupportOfProducts() {
    vector<double> ps;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPriceSupport()==true)
            ps.push_back(price_support_vector[i]);
    }
    return ps;
}
vector<double>
RegMarketInfo::getMinPriceOfProducts() {
    vector<double> mp;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPriceSupport()==true)
            mp.push_back(min_price_vector[i]);
    }
    return mp;
}
vector<double>
RegMarketInfo::getPriceDifferenceOfProducts() {
    vector<double> pd;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPriceSupport()==true)
            pd.push_back(price_difference_vector[i]);
    }
    return pd;
}
vector<int>
RegMarketInfo::getCatNumberOfPriceSupportProducts() {
    vector<int> cn;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPriceSupport()==true)
            cn.push_back(product_cat[i].getCatalogNumber());
    }
    return cn;
}

void
RegMarketInfo::setPriceSupport(vector<double>& ps) {
    int counter=0;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPriceSupport()==true) {
            price_support_vector[i]=ps[counter];
            counter++;
            price_difference_vector[i]=getPriceDifferenceOfProduct(i);
        }
    }
}
void
RegMarketInfo::setMinPrice(vector<double>& mp) {
    int counter=0;
    for (unsigned int i=0;i<product_cat.size();i++) {
        if (product_cat[i].getPriceSupport()==true) {
            min_price_vector[i]=mp[counter];
            counter++;
            price_difference_vector[i]=getPriceDifferenceOfProduct(i);
        }
    }
}
double
RegMarketInfo::getPriceDifferenceOfProduct(int cn) {
    // two types of price support!!!
    // 1. higher prices
    if ((price_vector[cn]+price_vector[cn]*price_support_vector[cn])>min_price_vector[cn])
        return price_vector[cn]*price_support_vector[cn];
    else
        // 2. minimum price effective
        return min_price_vector[cn]-price_vector[cn];
}
void
RegMarketInfo::activatePriceSupport() {
    for (unsigned int i=0;i<price_vector.size();i++) {
        if (product_cat[i].getPriceSupport()==true) {
            product_cat[i].setPrice(price_vector[i]+price_vector[i]*price_support_vector[i]);
            if (product_cat[i].getPrice()<min_price_vector[i])
                product_cat[i].setPrice(min_price_vector[i]);
            product_cat[i].setPriceExpectation(product_cat[i].getPriceExpectation()/price_change_vector[i]);
            if (product_cat[i].getPriceExpectation()<min_price_vector[i])
                product_cat[i].setPriceExpectation(min_price_vector[i]);
            price_difference_vector[i]=getPriceDifferenceOfProduct(i);
        }
    }
}
void
RegMarketInfo::setPriceChangeOfProduct(int i,double pc) {
    price_change_vector[i]=pc;
}
void RegMarketInfo::setInitialPriceOfType(int i,double p) {
    for (int j=0;j<num_products;j++) {
        if (product_cat[j].getProductType()==i) {
            product_cat[j].setPrice(p);
            product_cat[j].setPriceExpectation(p);
            orig_price_vector[j]=p;
            price_expectation_vector[j]=p;
            price_vector[j]=p;
        }
    }
}

void
RegMarketInfo::backup() {
    obj_backup=new RegMarketInfo(*this);
}
void
RegMarketInfo::restore() {
    RegMarketInfo* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
}

double RegMarketInfo::getLUperPlaceOfGroup(int g) {
	for (auto x : product_cat) {
		if (x.getProductGroup() == g) {
			return x.getLU();
		}
	}
	return 0;
}