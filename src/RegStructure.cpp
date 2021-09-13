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

#include <stdlib.h>
#include <time.h>

#include "RegStructure.h"
#include "RegFarm.h"
#include "RegPlot.h"
#include "random.h"

using namespace std;

typedef vector<RegPlotInfo* >::iterator iter;

RegRegionInfo::RegRegionInfo(RegGlobalsInfo* G) :g(G) {
    obj_backup=NULL;
    flat_copy=false;
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        free_plots_of_type.push_back(0);
        plots_of_type.push_back(0);
        average_rent_of_type.push_back(0);
        average_new_rent_of_type.push_back(0);
        exp_average_rent_of_type.push_back(0);
        exp_average_new_rent_of_type.push_back(0);
    }
    // to count dead plots
    plots_of_type.push_back(0);
}

RegRegionInfo::RegRegionInfo(const RegRegionInfo& rh,RegGlobalsInfo* G):g(G) {
    flat_copy=false;
    obj_backup=NULL;
    plots.resize(rh.plots.size());
    for (unsigned int i = 0;i<rh.plots.size();i++) { // for each column
        plots[i]=new RegPlotInfo(*(rh.plots[i]),g,plots);
    }
    for (unsigned int i = 0;i<rh.plots.size();i++) { // for each column
        plots[i]->finish(*(rh.plots[i]));
    }
    contiguous_plots=rh.contiguous_plots;
    free_plots_of_type=rh.free_plots_of_type;
    plots_of_type=rh.plots_of_type;
    average_rent_of_type=rh.average_rent_of_type;
    average_new_rent_of_type=rh.average_new_rent_of_type;
    exp_average_rent_of_type=rh.exp_average_rent_of_type;
    exp_average_new_rent_of_type=rh.exp_average_new_rent_of_type;
    free_plots.resize(rh.free_plots.size());
    for (unsigned int i=0;i<rh.free_plots.size();i++) {
        free_plots[i]=plots[rh.free_plots[i]->getId()];
    }
    total_tacs=rh.total_tacs;
    fix_tacs=rh.fix_tacs;
    var_tacs=rh.var_tacs;
}

// destroy the region
RegRegionInfo::~RegRegionInfo() {
    if (!flat_copy) {
        for (iter i = plots.begin();
                i != plots.end();
                i++) {
            // delete each plot in the vector
            delete *i;
        }
    }
    // clear the vector
    plots.clear();
    if (obj_backup) delete obj_backup;
}

int
RegRegionInfo::cPoT(int t) {
    int c=0;
    for (unsigned int i=0;i<plots.size(); i++) {
        if (plots[i]->getState()!=-1) {
            if (plots[i]->getSoilType()==t) c++;
        }
    }
    return c;
}

int RegRegionInfo::getRandom_contractLength() {
	string name = "CONTRACTLENGTH";
	return g->getRandomInt(name,g->uni_int_distrib_contractLength);
}

int RegRegionInfo::getRandom_freePlot_initLand() {
	string name = "FREEPLOT_INITLAND";
	return g->getRandomInt(name,g->uni_int_distrib_freePlot_initLand);
}

int RegRegionInfo::getRandom_freePlot_rentPlot() {
	string name = "FREEPLOT_RENTPLOT";
	return g->getRandomInt(name,g->uni_int_distrib_freePlot_rentPlot);
}

void
RegRegionInfo::setRentedPlot(RegPlotInfo* p,RegFarmInfo* f) {
    p->setState(1, f,f->getFarmColour());
    occupyPlot(p,f);
	int cl = getRandom_contractLength();
	//g->MIN_CONTRACT_LENGTH + randlong() % (g->MAX_CONTRACT_LENGTH - g->MIN_CONTRACT_LENGTH);
	//cout << cl << "\t";
    // printf("Newley Rented:%d\n",cl);
    p->setContractLength(cl);
}


void
RegRegionInfo::occupyPlot(RegPlotInfo* p,RegFarmInfo* f) {
    p->setDistanceFromAgent(p->calculateDistance(f->getFarmPlot()));
    decreaseFreePlotsOfType(p->getSoilType());
    unsigned int i;
    for ( i=0;i<free_plots.size();i++) {
        if (free_plots[i]==p) break;
    }
    vector<RegPlotInfo*>::iterator del=free_plots.begin()+i;
    free_plots.erase(del);
}

void
RegRegionInfo::setOwnedPlot(RegPlotInfo* p,RegFarmInfo* f) {
    p->setState(3, f,f->getFarmColour());
    occupyPlot(p,f);
    p->setContractLength(-1);
}
void
RegRegionInfo::setFarmsteadPlot(RegPlotInfo* p,RegFarmInfo* f) {
    p->setState(2, f,f->getFarmColour());
    occupyPlot(p,f);
    p->setContractLength(-1);
}
void
RegRegionInfo::releasePlot(RegPlotInfo* p) {
    increaseFreePlotsOfType(p->getSoilType());
    p->setState(0, 0, 0);
    p->setDistanceFromAgent(0);
    free_plots.push_back(p);
}

// initialise region
void
RegRegionInfo::initialisation() {
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        int lot=(int)((g->LAND_INPUT_OF_TYPE[i]*g->REGION_OVERSIZE)/g->PLOT_SIZE)   ;
        plots_of_type[i]=lot;
    }
    int non_ag_plots=static_cast<int>(g->NON_AG_LANDINPUT/g->PLOT_SIZE);

    plots.resize(g->NO_ROWS * g->NO_COLS);
    free_plots.resize(g->NO_ROWS * g->NO_COLS);
    
	// create plots
    int index;
    for (int c = 0; c < g->NO_COLS; c++) { // for each column
        for (int r = 0; r < g->NO_ROWS; r++) { //... and row
            index = c * g->NO_ROWS + r;    // create index number
            // ... and call setup function for each plot
            plots[index]=new RegPlotInfo(g,index,c,r,plots);
            // inititally all land is set to arable land
            plots[index]->setSoilType(0);
            plots[index]->setSoilName(g->NAMES_OF_SOIL_TYPES[0]);
            plots[index]->setId(index);
            free_plots[index]=plots[index];
        }
    }
    
    int type=g->NO_OF_SOIL_TYPES;
    for (int i=0;i<non_ag_plots;i++) {
        RegPlotInfo* free=getRandomFreePlotOfType(0);
        free->setSoilType(type);
        free->setSoilName("NON_AG");
        //    plots[index]->setState(-1,NULL,0);
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        for (int j=0;j<plots_of_type[i];j++) {
            RegPlotInfo* free=getRandomFreePlotOfType(0);
            free->setSoilType(i);
            free->setSoilName(g->NAMES_OF_SOIL_TYPES[i]);
        }
    }
    countFreePlotsOfType();
    countPlotsOfType();
}

void
RegRegionInfo::finish() {
    setIdlePlotsDead();
    countPlotsOfType();
}
void RegRegionInfo::countPlotsOfType() {
    for (int i=0;i<g->NO_OF_SOIL_TYPES+1;i++) {
        plots_of_type[i]=0;
    }
    int index=0;
    for (int c = 0; c < g->NO_COLS; c++) { // for each column
        for (int r = 0; r < g->NO_ROWS; r++) { //... and row
            index = c * g->NO_ROWS + r;  // create index number
            plots_of_type[plots[index]->getSoilType()]++;
        }
    }
}
void RegRegionInfo::countFreePlotsOfType() {
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        free_plots_of_type[i]=0;
    }
    int index=0;
    for (int c = 0; c < g->NO_COLS; c++) { // for each column
        for (int r = 0; r < g->NO_ROWS; r++) { //... and row
            index = c * g->NO_ROWS + r;    // create index number
            // ... and call setup function for each plot
            int t=plots[index]->getSoilType();
            increaseFreePlotsOfType(t);
        }
    }
}

void RegRegionInfo::setIdlePlotsDead() {
    int index=0;
    for (int c = 0; c < g->NO_COLS; c++) { // for each column
        for (int r = 0; r < g->NO_ROWS; r++) { //... and row
            index = c * g->NO_ROWS + r;  // create index number
            // if plot is idle set plot to dead and decrease the numbers
            // of free plots
            if (plots[index]->getState()==0) {
                decreaseFreePlotsOfType(plots[index]->getSoilType());
                // set all plots that are neither arable nor grassland
                // dead, ie. colour them black
                plots[index]->setState(-1,NULL,0);
                plots[index]->setSoilType(g->NO_OF_SOIL_TYPES);
                plots[index]->setSoilName("NON_AG");
                unsigned int i;
                for (i=0;i<free_plots.size();i++) {
                    if (free_plots[i]==plots[index]) {
                      break;
                    }
                }
                vector<RegPlotInfo*>::iterator del=free_plots.begin()+i;
                free_plots.erase(del);
            }
        }
    }
}

    /// set n of dead plots to idle plots of soil type t

void RegRegionInfo::setDeadPlotsToType(int n,int t) {
  if(plots_of_type[g->NO_OF_SOIL_TYPES]<n)
    n=plots_of_type[g->NO_OF_SOIL_TYPES];

  for(int i=0;i<n;i++) {
    RegPlotInfo* free=getRandomPlotOfType(g->NO_OF_SOIL_TYPES);
    free->setSoilType(t);
    free->setSoilName(g->NAMES_OF_SOIL_TYPES[t]);
    free->setState(0,NULL,0);
    increaseFreePlotsOfType(t);
    plots_of_type[t]++;
    plots_of_type[g->NO_OF_SOIL_TYPES]--;
    free_plots.push_back(free);
 }
}

void RegRegionInfo::resetMaxRents() {
	for (auto i = 0; i < g->NO_OF_SOIL_TYPES;++i) {
		g->MaxRents[i] = 0;
	}
}

void RegRegionInfo::outputMaxRents() {
	for (auto x : g->MaxRents)
		cout << x/g->PLOT_SIZE << "\t\t";
	cout << "\n";
	resetMaxRents();
}

void RegRegionInfo::updateMaxRents(double newrent, int typ) {
	if (newrent > g->MaxRents[typ])
		g->MaxRents[typ] = newrent;
}

void RegRegionInfo::calcMaxRents() {
	int index;
	double arent;
	for (int c = 0; c < g->NO_COLS; c++) { // for each column
		for (int r = 0; r < g->NO_ROWS; r++) { //... and row
			index = c * g->NO_ROWS + r;  // create index number
			int st = plots[index]->getSoilType();
			if (st<g->NO_OF_SOIL_TYPES && plots[index]->getState() == 1) {
				 arent = plots[index]->getRentPaid();
				 if (arent > g->MaxRents[st])
					 g->MaxRents[st] = arent;
			}
		}
	}
}

void RegRegionInfo::calculateAverageRent() {
    vector<int> used_plots;
    int no_plots=0;
    average_rent=0;
    used_plots.resize(g->NO_OF_SOIL_TYPES);
    average_rent_of_type.resize(g->NO_OF_SOIL_TYPES);
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        average_rent_of_type[i]=0;
        used_plots[i]=0;
    }

    int index;
    for (int c = 0; c < g->NO_COLS; c++) { // for each column
        for (int r = 0; r < g->NO_ROWS; r++) { //... and row
            index = c * g->NO_ROWS + r;  // create index number
            int st=plots[index]->getSoilType();
            if (st<g->NO_OF_SOIL_TYPES && plots[index]->getState()==1) {
                used_plots[st]++;
                no_plots++;
                average_rent+=plots[index]->getRentPaid();
                average_rent_of_type[st]+=plots[index]->getRentPaid();
            }
        }
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        if (used_plots[i]>0) {
            average_rent_of_type[i]/=((double)used_plots[i]*g->PLOT_SIZE);
        }
        exp_average_rent_of_type[i]=average_rent_of_type[i];
    }
    average_rent/=no_plots;
}
void RegRegionInfo::calculateAverageNewRent() {
    vector<int> used_new_plots;
    int no_plots=0;
    average_new_rent=0;
    used_new_plots.resize(g->NO_OF_SOIL_TYPES);
    average_new_rent_of_type.resize(g->NO_OF_SOIL_TYPES);
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        average_new_rent_of_type[i]=0;
        used_new_plots[i]=0;
    }

    int index;
    for (int c = 0; c < g->NO_COLS; c++) { // for each column
        for (int r = 0; r < g->NO_ROWS; r++) { //... and row
            index = c * g->NO_ROWS + r;  // create index number
            int st=plots[index]->getSoilType();
            if (st<g->NO_OF_SOIL_TYPES && plots[index]->getNewleyRented()==true) {
                used_new_plots[st]++;
                no_plots++;
                average_new_rent+=plots[index]->getRentPaid();
                average_new_rent_of_type[st]+=plots[index]->getRentPaid();
            }
        }
    }
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        if (used_new_plots[i]>0) {
            average_new_rent_of_type[i]/=(double)used_new_plots[i]*g->PLOT_SIZE;
        }
        exp_average_new_rent_of_type[i]=average_new_rent_of_type[i];
    }
    average_new_rent/=no_plots;
}

RegPlotInfo*
RegRegionInfo::getRandomFreePlotOfType(int type) {
    bool test=false;
    RegPlotInfo* p;
    do {
        test=false;
        //int randplot = randlong()%(g->NO_COLS * g->NO_ROWS);
		int randplot = getRandom_freePlot_initLand();
		//cout << randplot << "\t";

        p=plots[randplot];
//        p=the_plots[random(g->NO_COLS * g->NO_ROWS)];

        if (type==p->getSoilType()) test=true;

    } while (p->getState()!= 0 || !test);
    return p;
}
RegPlotInfo*
RegRegionInfo::getRandomPlotOfType(int type) {
    bool test=false;
    RegPlotInfo* p;
    do {
        test=false;
		int randplot = randlong() % (g->NO_COLS * g->NO_ROWS);
		//cout << randplot << "\t";

        p=plots[randplot];
//        p=the_plots[random(g->NO_COLS * g->NO_ROWS)];

        if (type==p->getSoilType()) test=true;

    } while ( !test);
    return p;
}

RegPlotInfo*
RegRegionInfo::getRandomFreePlot() {
    if (free_plots.size()==0) {
        return NULL;
    } else {
        int t=free_plots.size();
		//int r = randlong() % t;
		int r = getRandom_freePlot_rentPlot();
		//cout << r << "\t";

		r = r % t;
		return free_plots[r];
    }
}
void
RegRegionInfo::setHaPaymentPerPlot(double payment) {
    int index;
    for (int c = 0; c < g->NO_COLS; c++) { // for each column
        for (int r = 0; r < g->NO_ROWS; r++) { //... and row
            index = c * g->NO_ROWS + r;  // create index number
            plots[index]->setPaymentEntitlement(payment);
        }
    }
}
double
RegRegionInfo::calcPaidTacs() {
    double tacs=0;
    int index;
    for (int c = 0; c < g->NO_COLS; c++) { // for each column
        for (int r = 0; r < g->NO_ROWS; r++) { //... and row
            index = c * g->NO_ROWS + r;  // create index number
            tacs+=plots[index]->getPaidTacs();
        }
    }
    return tacs;
}
double
RegRegionInfo::calcTacs() {
    double tacs=0;
    int index;
    for (int c = 0; c < g->NO_COLS; c++) { // for each column
        for (int r = 0; r < g->NO_ROWS; r++) { //... and row
            index = c * g->NO_ROWS + r;  // create index number
            tacs+=plots[index]->getTacs();
        }
    }
    return tacs;
}
void
RegRegionInfo::setTacs() {
    fix_tacs=0;
    var_tacs=0;
    for (int c = 0; c < g->NO_COLS; c++) { // for each column
        for (int r = 0; r < g->NO_ROWS; r++) { //... and row
         int   index = c * g->NO_ROWS + r;  // create index number
         if(plots[index]->getState()>0) {
            int neighbours = plots[index]->identifyContiguousPlot(true,true,false);
            double bonus=0;
            if (g->FIXED_BONUS)
                bonus= g->FIXED_BONUS_VALUE*g->PLOT_SIZE;
            if (g->VARIABLE_BONUS)
                bonus+= ((double)neighbours*g->VARIABLE_BONUS_VALUE*(double)g->PLOT_SIZE);
            if (!g->LEGAL_TYPE_BONUS && !g->PREV_OWNER_BONUS) {
                plots[index]->setTacs(bonus);
            }
            if (g->LEGAL_TYPE_BONUS && !g->PREV_OWNER_BONUS) {
                if (plots[index]->getPreviouslyRentedByLegalType()==3)
                  plots[index]->setTacs(bonus);
                else
                  plots[index]->setTacs(0);
            }
            if (!g->LEGAL_TYPE_BONUS && g->PREV_OWNER_BONUS) {
                plots[index]->setTacs(bonus);
            }
            if (g->LEGAL_TYPE_BONUS && g->PREV_OWNER_BONUS) {
                if (plots[index]->getRentedByLegalType()==3)
                  plots[index]->setTacs(bonus);
                else
                  plots[index]->setTacs(0);
            }
        }
      }
    }
}

double
RegRegionInfo::getAvSizeOfContiguousPlotOfType(int type) const{
    double size=0;
    for (unsigned int i=0;i<contiguous_plots[type].size();i++) {
        size+=contiguous_plots[type][i];
    }
    if (size==0)
        return 0;
    else
        return size/(double)contiguous_plots[type].size();
}
double
RegRegionInfo::getStdSizeOfContiguousPlotOfType(int type) const{
    double size=0;
    double av=getAvSizeOfContiguousPlotOfType(type);
    for (unsigned int i=0;i<contiguous_plots[type].size();i++) {
        size+=(av-contiguous_plots[type][i])*(av-contiguous_plots[type][i]);
    }
    if (size==0)
        return 0;
    else
        return size/(double)contiguous_plots[type].size();
}

double
RegRegionInfo::getSizeOfContiguousPlotsOfType(int type) {
    double size=0;
    for (unsigned int i=0;i<contiguous_plots[type].size();i++) {
        size+=contiguous_plots[type][i];
    }
    return size;
}
int RegRegionInfo::getContiguousPlotsOfType(int type) const{
    return contiguous_plots[type].size();
}

void RegRegionInfo::countContiguousPlots() {
    vector<RegPlotInfo* >::iterator i;
    clearTagsForContiguousPlots();
    initVectorOfContiguousPlots();
    i=plots.begin();
    int c=0;
    do {
        while (/*(*i)->getTag()==true && */i!=plots.end()) {
            if ((*i)->getTag()==false && (*i)->getState()!=-1) break;
            c++;
            i++;
        }
        if (i!=plots.end()) {
            if ((*i)->getTag()==false) {
                int nop=(*i)->countContiguousPlots();
                int st=(*i)->getSoilType();
                contiguous_plots[st].push_back(nop*g->PLOT_SIZE);
            }
        }
    } while (i!=plots.end());
    clearTagsForContiguousPlots();
}
void
RegRegionInfo::resetUpdate() {
    for (int i=0;i<g->NO_COLS*g->NO_ROWS;i++) {
        plots[i]->setUpdate(false);
    }
}
void
RegRegionInfo::setUpdate() {
    for (int i=0;i<g->NO_COLS*g->NO_ROWS;i++) {
        plots[i]->setUpdate(true);
    }
}
void RegRegionInfo::clearTagsForContiguousPlots() {
    vector<RegPlotInfo* >::iterator plot_iter;
    for (plot_iter = plots.begin();
            plot_iter != plots.end();
            ++plot_iter) {
        (*plot_iter)->unTag();
    }
}
void RegRegionInfo::initVectorOfContiguousPlots() {
    contiguous_plots.clear();
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        vector<double> temp;
        contiguous_plots.push_back(temp);
    }
}
void RegRegionInfo::initPlotSearch() {
    if (g->FAST_PLOT_SEARCH) {
        for (int i = 0; i < g->NO_COLS*g->NO_ROWS; i++) {
            plots[i]->finish(this);
        }
    }
}
void
RegRegionInfo::backup() {
    obj_backup=new RegRegionInfo(*this);
    obj_backup->flat_copy=true;
    for (unsigned int i=0;i<plots.size();i++) {
        plots[i]->backup();
    }
}
void
RegRegionInfo::restore() {
    RegRegionInfo* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
    for (unsigned int i=0;i<plots.size();i++) {
        plots[i]->restore();
    }
}
