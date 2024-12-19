/*************************************************************************
* This file is part of AgriPoliS
*
* AgriPoliS: An Agricultural Policy Simulator
*
* Copyright (c) 2024, Alfons Balmann, Kathrin Happe, Konrad Kellermann et al.
* (cf. AUTHORS.md) at Leibniz Institute of Agricultural Development in 
* Transition Economies
*
* SPDX-License-Identifier: MIT
**************************************************************************/

#ifndef RegMarketH
#define RegMarketH
#include <vector>
#include "RegGlobals.h"
#include "RegProduct.h"
#include "RegEnvInfo.h"
#include "Evaluator.h"

#include <map>

class RegEnvInfo; //forward declaration

class RegSectorResultsInfo;

/** RegMarketInfo class.
    RegMarketInfo defines the product market for
    products produced on farms in the region.
    @author Kathrin Happe, Alfons Balmann, Konrad Kellermann
    @version June 2001
*/
class RegProductInfo;
class RegMarketInfo {
private:
    RegGlobalsInfo* g;
    /// number of products in region
    int num_products;

    // ALL VECTOR OF SIZE num_products!!!

    /// price flexibility vector
    vector <double> priceflex_vector;
    /// change of gross margin - creates adjustment pressure
    vector <double> price_change_vector;
    /// used fpr expgenous given proce trend to insure that farms realize the price change one year before its taking place
    vector <double> exp_price_change_vector;

    /// initial product prices
    vector <double> orig_price_vector;
    /// initial var costs of products
    vector <double> orig_var_costs_vector;
    /// livestock units
    vector <double> lives_units_vector;
    /// vector of minimum prices
    vector <double> min_price_vector;
    vector <double> price_support_vector;
    vector <double> price_difference_vector;
    /// current price vecto
    vector <double> price_vector;
    /// expected prices
    vector <double> price_expectation_vector;

    /// catalog of products in the region
    vector <RegProductInfo> product_cat;

    /** Method determines the price support as compared to the actual price vector.
        There are two kinds of price support.

        First, if prices are increased by a number (in price_support_vector), and
        if this is bigger than the minimum price, return the upper deviation to
        the actual price

        Second, prices may be below the minimum price, then the latter
        becomes effective.

        @param cn catalog number of product in product_cat
        @return price support or minimum price
    */
    double getPriceDifferenceOfProduct(int cn);
    RegMarketInfo* obj_backup;
//DCX
	void calculatePricevectors(RegSectorResultsInfo& Sector, int iteration);

public:
	double getLUperPlaceOfGroup(int group);
    map <string, int> product_id;

    /// constructor
    RegMarketInfo(RegGlobalsInfo*);
    RegMarketInfo(const RegMarketInfo&,RegGlobalsInfo*);
    ~RegMarketInfo();
    void backup();
    void restore();

    int     getNumProducts() const {
        return num_products;
    }
    void    setNumProducts(int num) {
        num_products = num;
    }

    /// supply input values to variables
    void    createMarket(RegEnvInfo*);
    /** @return product_cat
    */
    vector <RegProductInfo>& getProductCat();

    RegProductInfo getProductNumber(int);
    string getNameOfProduct(int i);

    // product groups
    vector<string> getProductGroups();
    vector<string> getNamesOfProductGroup(string name);
    vector<double> getTotalCostsOfGroup(string name);
    vector<double> getPricesOfGroup(string name);

    // Methods for premium payments
    vector<string> getNamesOfPremiumProducts();
    vector<double> getPremiumOfProducts();
    vector<int> getColsOfPremiumProducts();
    vector<int> getRowsOfPremiumProducts();
    vector<int> getCatNumberOfPremiumProducts();
    void setPremium(vector<double>&);

    //Methods for Price Support
    vector<string> getNamesOfPriceSupportProducts();
    vector<double> getPriceSupportOfProducts();
    vector<double> getMinPriceOfProducts();
    vector<double> getPriceDifferenceOfProducts();
    vector<int> getCatNumberOfPriceSupportProducts();
    void activatePriceSupport();

    void setPriceSupport(vector<double>&);
    void setMinPrice(vector<double>&);
    void setPriceChangeOfProduct(int i,double pc);
    void priceFunction(RegSectorResultsInfo& ,Evaluator*,int );
    void outputPrice(int);
    void debug(string);
    void setInitialPriceOfType(int i,double p);
};
//---------------------------------------------------------------------------
#endif
