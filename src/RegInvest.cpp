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

// RegInvest.cpp
//---------------------------------------------------------------------------
#include "RegInvest.h"
#include "RegGlobals.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "textinput.h"
#include "random.h"

//////////////////////
// RegInvestObjectInfo
//////////////////////

RegInvestObjectInfo::RegInvestObjectInfo(RegGlobalsInfo *G)
        :g(G) {
    invest_type = 0;
    invest_age = 0;
    average_cost = 0;
    liq_effect = 0;
    bound_equity_capital = 0;
    maintenance_costs = 0;
    affects_product_group = -1;
    bc_interest=0;
    interest_reduction=0;
    reduced_bc_interest=0;
    technical_change_effect=0;
}
RegInvestObjectInfo::~RegInvestObjectInfo() {}
void
RegInvestObjectInfo::setBcInterest(double bc_interest) {
    this->bc_interest=bc_interest;
    reduced_bc_interest=bc_interest-interest_reduction;
}
void
RegInvestObjectInfo::setInterestReduction(double interest_reduction) {
    this->interest_reduction=interest_reduction;
    reduced_bc_interest=bc_interest-interest_reduction;
}
void
RegInvestObjectInfo::setReducedBcInterest(double reduced_bc_interest) {
    this->reduced_bc_interest=reduced_bc_interest;
    interest_reduction=bc_interest-reduced_bc_interest;
}

void
RegInvestObjectInfo::setAttrib(int cn,
                               double ac,
                               unsigned int el,
                               double ls,
                               double ls2,
                               double c,
                               double mc,
                               int pg,
                               unsigned int it,
                               double tc,
                               string n
                              ) {
    catalog_number = cn;
    acquisition_costs = ac;
    economic_life = el;
    labour_substitution = ls;
    land_substitution = ls2;
    capacity = c;
    invest_type = it;
    name = n;
    affects_product_group = pg;
    residual_value = ac;
    maintenance_costs = mc;
    technical_change_effect = tc;
    residual_ec_share = ac * g->SHARE_SELF_FINANCE;
    residual_bc_share = ac * (1-g->SHARE_SELF_FINANCE);
}


void
RegInvestObjectInfo::incrementInvestAge() {
    invest_age++;
}

string
RegInvestObjectInfo::debug() {
    std::stringstream r;
    r << "Name:\t" << name << "\t" <<
                   "Catalog Number:\t" << catalog_number << "\t" <<
                   "Type:\t" << invest_type << "\t" <<
                   "Economic Life:\t" << economic_life << "\t" <<
                   "Invest Age:\t" << invest_age << "\t" <<
                   "Acquisition Costs:\t" << acquisition_costs << "\t" <<
                   "Average Costs:\t" << average_cost << "\t" <<
                   "Liquity Effect:\t" << liq_effect << "\t" <<
                   "Bound Equity Capital:\t" << bound_equity_capital << "\t" <<
                   "Capacity:\t" << capacity << "\t" <<
                   "Land Substitution:\t" << land_substitution << "\t" <<
                   "Labour Subtitution:\t" << labour_substitution << "\t" <<
                   "Residual Value:\t" <<  residual_value << "\t" <<
                   "Maintenance Costs:\t" << setprecision(7) << fixed << maintenance_costs << "\t";

    return r.str();
}

vector <RegInvestObjectInfo >
RegInvestObjectInfo::initInvestmentCatalog() {
    vector <RegInvestObjectInfo > InvestCatalog;

    int noi = investdata.invests.size();
    for (int i=0;i<noi;i++) {
            RegInvestObjectInfo *invest = new RegInvestObjectInfo(g);
            invest->setAttrib(i,
                    investdata.invests[i].cost,
                    static_cast<int>(investdata.invests[i].life),
                    investdata.invests[i].labsub,
                    investdata.invests[i].landsub,
                    investdata.invests[i].capacity,
                    investdata.invests[i].maintenance,
                    investdata.invests[i].group,
                    investdata.invests[i].type,
                    investdata.invests[i].techchange,
                    investdata.invests[i].name);

//      invest->setBcInterest(g->INTEREST_RATE);//(*invest_cat)[cat_num].getBcInterest());
            InvestCatalog.push_back(*invest);
            if (invest->getInvestType()>g->NUMBER_OF_INVESTTYPES)
                g->setNumberOfInvestType(invest->getInvestType());
            delete invest;
    }
    g->setNumberOfInvestType(g->NUMBER_OF_INVESTTYPES+1);

    for (unsigned int i=0;i<InvestCatalog.size();i++) {
        int t=InvestCatalog[i].getInvestType();
        if (InvestCatalog[i].getCapacity()!=0) {
            double min_lsub=InvestCatalog[i].getLabourSubstitution()/InvestCatalog[i].getCapacity();
            for (unsigned int j=0;j<InvestCatalog.size();j++) {
                if (t==InvestCatalog[j].getInvestType()) {
                    if ((InvestCatalog[j].getLabourSubstitution()/InvestCatalog[j].getCapacity())<min_lsub) {
                        min_lsub=(InvestCatalog[j].getLabourSubstitution()/InvestCatalog[j].getCapacity());
                    }
                }
            }
            InvestCatalog[i].setNormLabourSubstitution(min_lsub);
        }
    }

   // calculate average costs, average bound equity capital

   return InvestCatalog;
}

/////////////////
// REGINVESTLIST
/////////////////

RegInvestList::RegInvestList(RegGlobalsInfo *G, vector<RegInvestObjectInfo>&I)
        :g(G),invest_cat(&I),labSubstitution(0) {
    // 9 investment types
    obj_backup=NULL;
    newley_invested.resize((*invest_cat).size());
}
RegInvestList::RegInvestList(RegInvestList& rh,
                             RegGlobalsInfo* G,
                             vector<RegInvestObjectInfo>& I)
        :g(G),invest_cat(&I),labSubstitution(0) {
    obj_backup=NULL;
    farm_invests=rh.farm_invests;
    newley_invested=rh.newley_invested;
}
RegInvestList::~RegInvestList() {
    if (obj_backup) delete obj_backup;
}


void
RegInvestList::setBackNewleyInvestedVector() {
    for (unsigned int i=0;i<newley_invested.size();i++)
        newley_invested[i]=0;
}

void
RegInvestList::add(RegInvestObjectInfo i) {
    farm_invests.push_back(i);
    newley_invested[i.getCatalogNumber()]++;
}
void
RegInvestList::addTest(RegInvestObjectInfo i) {
    i.setInvestAge(-1);
    farm_invests.push_back(i);
}

void
RegInvestList::debug(string filename) {
    ofstream out;
    out.open(filename.c_str(),ios::trunc);
    out << "Invest List\n";
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            invest++) {
        out << (*invest).debug().c_str() << "\n";
    }
    out.close();
}

void
RegInvestList::debug1(string filename, double mc) {
    ofstream out;
    out.open(filename.c_str(),ios::trunc);
    out << "Invest List\n";
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            invest++) {
        out << (*invest).debug().c_str() << "\n";
    }
    out << g->tIter << "  " << setprecision(6) << fixed << mc << "\n";
    out.close();
}


void
RegInvestList::add(int cat_num, int quantity, int cap) {
    // determine the other attributes for each of the farm's investments
    // objects which have a quantity of non-zero from the invest_cat
    // invest_type, ac_costs, etc.
    if (quantity > 0) {
        double factor = 1;
        // adjustment of capacities necessary because the actual farm capacities
        // often do not match the capacities in invest_cat
        if ((*invest_cat)[cat_num].getInvestType() > 1) { // if type is not labour
            double cap_orig = (*invest_cat)[cat_num].getCapacity();
            factor = ((double)cap)/cap_orig;
        }
        for (int i = 0; i < quantity; i++) {
            RegInvestObjectInfo* invest = new RegInvestObjectInfo(g);
            invest->setAttrib((*invest_cat)[cat_num].getCatalogNumber(),
                              (*invest_cat)[cat_num].getAcquisitionCosts() * factor,
                              (*invest_cat)[cat_num].getEconomicLife(),
                              (*invest_cat)[cat_num].getLabourSubstitution() * factor,
                              (*invest_cat)[cat_num].getLandSubstitution() * factor,
                              cap,
                              (*invest_cat)[cat_num].getMaintenanceCosts() * factor,
                              (*invest_cat)[cat_num].getAffectsProductGroup(),
                              (*invest_cat)[cat_num].getInvestType(),
                              (*invest_cat)[cat_num].getTechnicalChangeEffect(),
                              (*invest_cat)[cat_num].getName());
            invest->setAverageCost((*invest_cat)[cat_num].getAverageCost() * factor);
            invest->setNormLabourSubstitution((*invest_cat)[cat_num].getNormLabourSubstitution() );
            invest->setLiqEffect((*invest_cat)[cat_num].getLiqEffect() * factor);
            invest->setBoundEquityCapital((*invest_cat)[cat_num].getBoundEquityCapital() * factor);
            invest->setBcInterest(g->INTEREST_RATE);//(*invest_cat)[cat_num].getBcInterest());
            invest->setInterestReduction((*invest_cat)[cat_num].getInterestReduction());
            invest->setResidualBcShare((*invest_cat)[cat_num].getResidualBcShare() * factor);
            // push back copy of invest
            farm_invests.push_back(*(invest));
            // delete pointer to original object
            delete invest;
        }
    }
}
void
RegInvestList::getOlder() {
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            invest++) {
        // increment age of each investment
        (*invest).incrementInvestAge();
    }
}
void RegInvestList::resetRemovedInvs() {
	removed_invs.clear();
}

map<string, int> RegInvestList::getRemovedInvests() {
	return removed_invs;
}

void
RegInvestList::removeInvestment() {
    list<RegInvestObjectInfo >::iterator invest;
    // REMOVE INVESTMENTS FROM INVESTLIST
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            invest++) {
        if ((*invest).getEconomicLife() <= (*invest).getInvestAge()) {
            // investment is dead -1 
            (*invest).setInvestAge(-1);
			removed_invs[(*invest).getName()] += 1;
        }
    }
    // remove all Investments with invest_age = -1 from FarmInvestList
    // effect on capacities at the beginning of new iteration
    if (!farm_invests.empty()) {
        list<RegInvestObjectInfo >::iterator removed;
        removed = remove_if(farm_invests.begin(), farm_invests.end(), CheckAge());
        farm_invests.erase(removed, farm_invests.end());
    }
}
int
RegInvestList::getNewInvestmentExpenditure() {
    int exp = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        if (((*invest).getInvestAge() == 1) && ((*invest).getAcquisitionCosts()  > 0))
            exp +=static_cast<int>( (*invest).getAcquisitionCosts());
    }
    return exp;
}
int
RegInvestList::getInvestmentsOfCatalogNumber(int t) {
    int numbers = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        if ((*invest).getCatalogNumber() == t)
            numbers++;
    }
    return numbers;
}
double
RegInvestList::getAverageAgeOfInvestmentsOfCatalogNumber(int t) {
    int numbers = 0;
    int age = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        if ((*invest).getCatalogNumber() == t) {
            numbers++;
            age += (*invest).getInvestAge();
        }
    }
    double avage = age/numbers;
    return avage;
}
double
RegInvestList::getLandSubstitution() {
    double lsub = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        lsub += (*invest).getLandSubstitution();
    }
    return lsub;
}
double
RegInvestList::getLabourSubstitution() {
    double lsub = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        lsub += (*invest).getLabourSubstitution();
    }
    return lsub;
}
double
RegInvestList::getLSWithoutLabour() {
    double lsub = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        if ((*invest).getCatalogNumber()==g->FIXED_HIRED_LAB)
            break;
        if ((*invest).getCatalogNumber()==g->FIXED_OFFFARM_LAB)
            break;
        lsub += (*invest).getLabourSubstitution();
        // exclude labour
    }
    return lsub;
}

double
RegInvestList::getLSWithoutLabour1() {
    double lsub = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        if ((*invest).getCatalogNumber()==g->FIXED_HIRED_LAB)
            break;
        if ((*invest).getCatalogNumber()==g->FIXED_OFFFARM_LAB)
            break;
        lsub += (*invest).getLabourSubstitution();
        // exclude labour
    }
    labSubstitution=-lsub;
    return lsub;
}
double
RegInvestList::getTotalMaintenance() {
    double maint = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        maint += (*invest).getMaintenanceCosts();
    }
    return maint;
}
double
RegInvestList::getTotalResEcShare() {
    double ecres = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        ecres += (*invest).getResidualEcShare();
    }
    return ecres;

}
double
RegInvestList::getTotalResEcShareWithoutLabour() {
    double ecres = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        if ((*invest).getCatalogNumber() !=g->FIXED_OFFFARM_LAB && (*invest).getCatalogNumber() !=g->FIXED_HIRED_LAB)
            ecres += (*invest).getResidualEcShare();
    }
    return ecres;

}

double
RegInvestList::depreciateCapital(double* repay, double* fixincome,double* value_added,
                                 double* annuity, bool sunkcostrelevant,
                                 bool older, const double& equity_interest) {
    double getolder = 0;              // get older by number of years
    double depr = 0;
    double eq_interest = equity_interest;// Market->getProduct(0).getPrice();   // equity capital interest
    double re_payment = 0;
    double labincome = 0;
    double ann = 0;
    double v = g->SHARE_SELF_FINANCE;
    double totaldepreciation = 0;

    list<RegInvestObjectInfo >::iterator invest;

    if (older == true) getolder = 1;

    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            invest++) {
        double bc_interest=(*invest).getReducedBcInterest();
        // increment age of each investment
        if (older == true) {
            (*invest).incrementInvestAge();
        }

        // WITH EFFECT OF SUNK COSTS (relevant at end of period)
        // get depreciation depending on relevance for decision
        // investment objects except for off-farm labour
        if ( (sunkcostrelevant == true) && ((*invest).getAcquisitionCosts() > 0) ) {
            double acqcosts = (*invest).getAcquisitionCosts();
            int investage = (*invest).getInvestAge();
            int n = (*invest).getEconomicLife();
            
            double residualvalue = (*invest).getResidualValue();
            double ec_share = (*invest).getResidualEcShare();

            ec_share = v * acqcosts * capitalReturnFactor(eq_interest, n) / capitalReturnFactor(eq_interest, n-(investage-1));
            double bc_share = (1-v) * acqcosts * capitalReturnFactor(bc_interest, n) / capitalReturnFactor(bc_interest, n-(investage-1));
            
            residualvalue =  acqcosts * ( 1 - ( v * pow( (1 + eq_interest), investage) *
                                                ( capitalReturnFactor(eq_interest, n) - eq_interest) /
                                                capitalReturnFactor(eq_interest, investage)
                                              )
                                          - ( (1 - v) * pow( (1 + bc_interest), investage) *
                                              ( capitalReturnFactor(bc_interest, n) - bc_interest) /
                                              capitalReturnFactor(bc_interest, investage)
                                            )
                                        );
            (*invest).setResidualValue(residualvalue);
            (*invest).setResidualEcShare(ec_share);
            (*invest).setResidualBcShare(bc_share);
            // ONLY FOR CONSTANT MAINTENANCE COSTS
            depr = ( acqcosts *
                     (
                         v * pow((1 + eq_interest), investage - getolder) *
                         (capitalReturnFactor(eq_interest, n) - eq_interest) +
                         (1 - v) * pow((1 + bc_interest), investage - getolder) *
                         (capitalReturnFactor(bc_interest, n) - bc_interest))
                   );

            totaldepreciation += depr;

            ann += acqcosts * (v * capitalReturnFactor(eq_interest,n)
                               + (1-v) * capitalReturnFactor(bc_interest,n)
                              );

            // plus borrowed capital share of maintenance costs
            re_payment += ( acqcosts *
                            (1-v) * pow((1 + bc_interest), investage - getolder) *
                            (capitalReturnFactor(bc_interest, n) - bc_interest));

            // Fremd AK
            if ((*invest).getInvestType() == 0) {
                *value_added += ( acqcosts *
                                  (v * pow((1 + eq_interest), investage - getolder) *
                                   (capitalReturnFactor(eq_interest, n) - eq_interest) +
                                   (1 - v) * pow((1 + bc_interest), investage - getolder) *
                                   (capitalReturnFactor(bc_interest, n) - bc_interest))
                                ) ;
            }
        }
        if ( (sunkcostrelevant == true) && ((*invest).getAcquisitionCosts() < 0) ) {
            double acqcosts = (*invest).getAcquisitionCosts();
            int investage = (*invest).getInvestAge();
            int n = (*invest).getEconomicLife();

            labincome += ( acqcosts *
                           (v * pow((1 + eq_interest), investage - getolder) *
                            (capitalReturnFactor(eq_interest, n) - eq_interest) +
                            (1 - v) * pow((1 + bc_interest), investage - getolder) *
                            (capitalReturnFactor(bc_interest, n) - bc_interest))
                         );

			*value_added += ( acqcosts *
                              (v  * pow((1 + eq_interest), investage - getolder) *
                               (capitalReturnFactor(eq_interest, n) - eq_interest) +
                               (1 - v) * pow((1 + bc_interest), investage - getolder) *
                               (capitalReturnFactor(bc_interest, n) - bc_interest))
                            );
        }
        //
        // WITHOUT EFFECT OF SUNK COSTS (for planning in futureOfFarm)
        //
        // without sunk cost for off-farm labour
        // no effect on value added
        if ( (sunkcostrelevant == false)&& ((*invest).getAcquisitionCosts() < 0) ) {
            double acqcosts = (*invest).getAcquisitionCosts();
            int investage = (*invest).getInvestAge();
            int n = (*invest).getEconomicLife();

            labincome += ( acqcosts *
                           (v * pow((1 + eq_interest), investage - getolder) *
                            (capitalReturnFactor(eq_interest, n) - eq_interest) +
                            (1 - v) * pow((1 + bc_interest), investage - getolder) *
                            (capitalReturnFactor(bc_interest, n) - bc_interest))
                         );
        }
    }
  
    //    debug("deprdebug.txt");
    *fixincome = labincome;
    *repay = re_payment;
    *annuity = ann;
    return totaldepreciation;
}

double
RegInvestList::getNormalizedCapacityOfType(int t) {
    double lsub = 0;
    double lsub_norm = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        if ((*invest).getInvestType() == t) {
            lsub += (*invest).getLabourSubstitution();
            lsub_norm = (*invest).getNormLabourSubstitution();
        }
    }
    if (lsub_norm!=0)
        return lsub/lsub_norm;
    else
        return 0;
}

double
RegInvestList::getCapacityOfType(int t) {

    double cap = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        RegInvestObjectInfo p=(*invest);
        if ((*invest).getInvestType() == t)
            cap += (*invest).getCapacity();
    }
    return cap;
}

int RegInvestList::getRandomInvestAge() {
	string name = "INVESTAGE";
	
	return g->getRandomInt(name, g->uni_int_distrib_investAge);
}

void
RegInvestList::setAsynchronousInvestAge(int farm_age, double *assets,
                                        double *liquidity, double *lt_borrowed_capital,
                                        const double& equity_interest, const double& rel_invest_age ) {

    //int catnumber;
    int /*modulo, */investage;
   // int life;
    double v = g->SHARE_SELF_FINANCE;
    double ec_interest = equity_interest;

    *liquidity = 0;

    list<RegInvestObjectInfo >::iterator invest;

    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            invest++) {
        double acqcosts;
        int n;
        double bc_interest = (*invest).getReducedBcInterest();

        n = (*invest).getEconomicLife();

		int ir = getRandomInvestAge();
		//cout << ir << endl;
		investage = ir % n;// randlong() % n;

        acqcosts = (*invest).getAcquisitionCosts();
        if (investage < 1)
            investage = 1;
       
        // set values for full lifetime of invest objects
        *lt_borrowed_capital += acqcosts * (1 - g->SHARE_SELF_FINANCE);
        *assets += acqcosts; // assets for full lifetime of investment

        if ((investage > 0)&&(acqcosts > 0)) {
            double residualvalue, ec_share, delta, bc_share;

            (*invest).setInvestAge(investage);
            // according to Alfons equation of 15.03.2001

            ec_share = v * acqcosts * capitalReturnFactor(ec_interest, n) / capitalReturnFactor(ec_interest, (n-(investage-1)));
            
            bc_share = (1-v) * acqcosts * capitalReturnFactor(bc_interest, n) / capitalReturnFactor(bc_interest, n-(investage-1));

            residualvalue =  acqcosts * ( 1 - ( v * pow( (1 + ec_interest), investage) *
                                                ( capitalReturnFactor(ec_interest, n) - ec_interest) /
                                                capitalReturnFactor(ec_interest, investage)
                                              )
                                          - ( (1 - v) * pow( (1 + bc_interest), investage) *
                                              ( capitalReturnFactor(bc_interest, n) - bc_interest) /
                                              capitalReturnFactor(bc_interest, investage)
                                            )
                                        );
            (*invest).setResidualValue(residualvalue);
            (*invest).setResidualEcShare(ec_share);
            (*invest).setResidualBcShare(bc_share);

            delta = acqcosts - residualvalue;
            *assets -= delta;
            if (*assets<0)
                *assets=0;
            *lt_borrowed_capital -= delta * (1-v);
            if (*lt_borrowed_capital<0)
                *lt_borrowed_capital=0;
        }
    }
}

double
RegInvestList::getOwnBcInterest() {
    // add up farm's total borrowed capital interest costs
    double interest = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        // we assume that off farm labour does not save on interest costs
        if ((*invest).getAcquisitionCosts()>0)
            interest += (*invest).getOwnBcInterest();
    }
    return interest;
}
double
RegInvestList::getPoliticalBcInterest() {
    double interest = 0;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        interest += (*invest).getPoliticalBcInterest();
    }
    return interest;
}
double
RegInvestList::getCostFactor(int prodgroup) {
    int count = 0;
    int countsame = 1;
    int a;
    int b=-1;
    double addfactor = 0;
    int age;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        age = (*invest).getInvestAge();
        if (((age==0)||(age==-1))
                &&((*invest).getAffectsProductGroup()==prodgroup)) { // new investment
            a = (*invest).getCatalogNumber();
            if (a==b) {
                // if there are at least 2 investments of a catalogue number
                // then the factor is
                // f = a + a^2 for 0 < a < 1
                addfactor += ((*invest).getTechnicalChangeEffect()
                              + (*invest).getTechnicalChangeEffect()*
                              (*invest).getTechnicalChangeEffect()
                             );
                countsame++;
            } else {
                addfactor += (*invest).getTechnicalChangeEffect();
            }
            count++;
            b = a;
        }
    }
    if (countsame > 1)
        return addfactor/countsame;
    else {
        if (count>0)
            return addfactor/count;
        else
            return 0;
    }
}
bool
RegInvestList::newMachinery(int prodgroup) {
    int age;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        age = (*invest).getInvestAge();
        if (((age==0)||(age==-1))
                &&((*invest).getAffectsProductGroup()==prodgroup)) { // new investment
            return true;
        } else {
            return false;
        }
    }
    return false;
}
int
RegInvestList::getNumberOfNewInvestmentsWithoutLabour() {
    int count = 0;
    int age;
    list<RegInvestObjectInfo >::iterator invest;
    for (invest = farm_invests.begin();
            invest != farm_invests.end();
            ++invest) {
        age = (*invest).getInvestAge();
        if ((age==0)&&((*invest).getAffectsProductGroup()!=g->PRODGROUPLAB)) { // new investment
            count++;
        }
    }
    return count;
}


int
RegInvestList::getNumberOfInvestTypes() {
    int max=0;
    for (unsigned int i=0;i<(*invest_cat).size();i++) {
        int t=(*invest_cat)[i].getInvestType();
        if (t>max) max=t;
    }
    return max+1;
}
void
RegInvestList::backup() {
    obj_backup=new RegInvestList(*this);
}
void
RegInvestList::restore() {
    RegInvestList* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
}
