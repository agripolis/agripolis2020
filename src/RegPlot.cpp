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

// RegPlot.cpp
//---------------------------------------------------------------------------
#include "RegPlot.h"
#include "RegFarm.h"
#include "RegMessages.h"
#include <algorithm>
#include <iostream>
using namespace std;

//soil service
double RegPlotInfo::getCarbon(){
	return carbon;
}

void RegPlotInfo::setCarbon(double c) {
	carbon=c;
	return;
}

//	CONSTRUCTOR
RegPlotInfo::RegPlotInfo(RegGlobalsInfo* G,
                         int pos,
                         int xcol,
                         int yrow,
                         vector<RegPlotInfo*>& r) :g(G), region(&r) {
	carbon = 0;
    paid_tacs=0;
    tacs=0;
    PA.state = 0;
    PA.number = pos;
    PA.col = xcol;
    PA.row = yrow;
    newley_rented = false;
    rent_paid = -1;
    second_offer = rent_paid;
    distance_from_agent = -1;
    rented_by_agent = -1;
    previously_rented_by_agent=-1;
    previously_rented_by_legal_type=-1;
    previously_paid_by_agent=0;
    payment_entitlement = 0;
    initial_payment_entitlement = 0;
    occupied_by_agent = -1;
    previously_occupied_by_agent = -1;
    distance_costs = 0;
    contract_length=0;
    obj_backup=NULL;

    update=false;
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        plot_p.push_back(0);
    }

    tagv=false;
    checked = false;
}
RegPlotInfo::RegPlotInfo(RegPlotInfo& rh,
                         RegGlobalsInfo* G,
                         vector<RegPlotInfo*>& r) :region(&r) {
    obj_backup=NULL;
    g=G;
    PA=rh.PA;
    paid_tacs=rh.paid_tacs;
    tacs=rh.tacs;
    contract_length=rh.contract_length;
    distance_from_agent=rh.distance_from_agent;
    rent_paid=rh.rent_paid;
    second_offer = rh.second_offer;
    payment_entitlement=rh.payment_entitlement;
    initial_payment_entitlement=rh.initial_payment_entitlement;
    rented_by_agent=rh.rented_by_agent;
    occupied_by_agent=rh.occupied_by_agent;
    previously_rented_by_agent=rh.previously_rented_by_agent;
    previously_occupied_by_agent=rh.previously_occupied_by_agent;
    distance_costs=rh.distance_costs;
    newley_rented=rh.newley_rented;

    tagv=rh.tagv;
    update=rh.update;
    plot_id=rh.plot_id;
    checked=rh.checked;
    plot_p=rh.plot_p;
    rented_by_legal_type=rh.rented_by_legal_type;
    previously_rented_by_legal_type=rh.previously_rented_by_legal_type;
    previously_paid_by_agent=rh.previously_paid_by_agent;
    free_plots=rh.free_plots;

    dist=rh.dist;
}
void
RegPlotInfo::finish(RegPlotInfo& rh) {
    for (unsigned int i=0;i<rh.free_plots.size();i++) {
        for (unsigned int j=0;j<rh.free_plots[i].size();j++) {
            free_plots[i][j].plot=(*region)[rh.free_plots[i][j].plot->getId()];
        }
    }
    for (unsigned int j=0;j<rh.contiguous_plot.size();j++) {
        contiguous_plot.push_back((*region)[rh.contiguous_plot[j]->getId()]);
    }
    pl_n.resize(rh.pl_n.size());
    if (rh.pl_n.size()>0) {
        for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
            pl_n[i].resize(rh.pl_n[i].size());
        }
        for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
            for (unsigned int j=0;j<rh.pl_n[i].size();j++) {
                pl_n[i][j]=(*region)[rh.pl_n[i][j]->getId()];
            }
        }
    }
}
void RegPlotInfo::backup() {
    obj_backup=new RegPlotInfo(*this);
}
void RegPlotInfo::restore() {
    RegPlotInfo* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
}
//--------------
//	DESTRUCTOR
//--------------
RegPlotInfo::~RegPlotInfo() {
    if (obj_backup) delete obj_backup;
}
//------------------------
//	DISTANCE BETWEEN PLOTS
//------------------------
double
RegPlotInfo::calculateDistance(const RegPlotInfo* P) {
    double d;
    if (P != 0) {
        double dx = min(abs(PA.col - P->PA.col),(g->NO_COLS - abs(PA.col - P->PA.col)));
        double dy = min(abs((PA.row - P->PA.row)),(g->NO_ROWS - abs(PA.row - P->PA.row)));
        d = sqrt((dx * dx) + (dy * dy));
        return d;
    } else
        return 0;
}

double
RegPlotInfo::calculateDistanceCosts(const RegPlotInfo* P) {
    double dist=sqrt(g->PLOT_SIZE)/10;
    return dist*g->TRANSPORT_COSTS*calculateDistance(P);
}

void
RegPlotInfo::setDistanceFromAgent(double dfa) {
    // distance_from_agent is otherwise never needed
    distance_from_agent = dfa;
    // normalisation of distance to be expressed in km
    // 10 is conversion factor from ha in km
    // dist is the border length of a plot
    double dist=sqrt(g->PLOT_SIZE)/10;
    distance_costs = dist*g->TRANSPORT_COSTS*dfa;
}
//-------------------------------------
//	FIND FREE PLOT CLOSEST TO THIS CELL
//-------------------------------------
bool
RegPlotInfo::checkPlot(int n,int type) {
//    double dist; // cardinalpoints
//    dist = calculateDistance((*region)[n]);
//    (*region)[n]->setDistanceFromAgent(dist);
    // check for plot n whether it is of type type
    if (type==(*region)[n]->getSoilType() && (*region)[n]->getState()==0) return true;
    else return false;
}

RegPlotInfo*
RegPlotInfo::findFreePlotOfType(int type) {
    if (g->FAST_PLOT_SEARCH && pl_n.size()>0) {
        for (unsigned int i=plot_p[type];i<pl_n[type].size();i++) {
            if (pl_n[type][i]->getState()==0) {
                plot_p[type]=i;
                return pl_n[type][i];
            }
        }
        plot_p[type]=pl_n[type].size();
        return NULL;

    } else {
        // find free plot in (*region)
        // mechanism is that 4 direct neighbours are checked an then
        // the region is rotated and the 4 diagonal neighbours are
        // checked. This is repeated for the VISION of the farm,
        // which is currently the whole region

        int j;
        for (int i=1;i<=g->VISION;i++) {
            j=0;
            do {
                int n,o,s,w;  // coordinates Nord, Ost, Sued, West
                //plot to the north
                n=((PA.row-i+g->NO_ROWS)%g->NO_ROWS)+(((PA.col+j+g->NO_COLS)%g->NO_COLS)*g->NO_ROWS);
                if (checkPlot(n,type)) {
                    return (*region)[n];
                }
                // plot to the east
                o=((PA.row+j+g->NO_ROWS)%g->NO_ROWS)+(((PA.col+i+g->NO_COLS)%g->NO_COLS)*g->NO_ROWS);
                if (checkPlot(o,type)) {
                    return (*region)[o];
                }
                // plot to the south
                s=((PA.row+i+g->NO_ROWS)%g->NO_ROWS)+(((PA.col-j+g->NO_COLS)%g->NO_COLS)*g->NO_ROWS);
                if (checkPlot(s,type)) {
                    return (*region)[s];
                }
                // plot to the west
                w=((PA.row-j+g->NO_ROWS)%g->NO_ROWS)+(((PA.col-i+g->NO_COLS)%g->NO_COLS)*g->NO_ROWS);
                if (checkPlot(w,type)) {
                    return (*region)[w];
                }
                if (j > 0) { // rotates horizon to the right and left
                    j*=-1;
                } else {
                    j*=-1;
                    j+=1;
                }
            } while (j <= i);
        }
        // if no free plot is left
        return NULL;
    }
}

bool alternative_comp(const RegPlotInformationInfo &p1, const RegPlotInformationInfo &p2)
{
	return (p1.alternativeSeachCosts() < p2.alternativeSeachCosts());
}

RegPlotInformationInfo
RegPlotInfo::findMostPreferablePlotOfType(int type, RegFarmInfo* farm) {
	if (g->WEIGHTED_PLOT_SEARCH) {
		RegPlotInformationInfo pinfo_best;
		RegPlotInformationInfo pinfo_curr;
		bool first = true;
		for (unsigned int j = 0; j < (*region).size(); j++) {
			if (checkPlot(j, type)) {
				pinfo_curr = calculateValue((*region)[j], farm);
				if (first) {
					pinfo_best = pinfo_curr;
					first = false;
				}
				else {
					if (g->SWEDEN) {
						if (alternative_comp(pinfo_curr, pinfo_best))
							pinfo_best = pinfo_curr;
					}
					else {
						if (pinfo_best > pinfo_curr)
							pinfo_best = pinfo_curr;
					}
				}
			}
		}
		return pinfo_best;
	}
	else
		return findMostPreferablePlotOfType(type);
}

RegPlotInformationInfo
RegPlotInfo::findMostPreferablePlotOfType(int type) {
    if (g->WEIGHTED_PLOT_SEARCH) {
        if (free_plots[type].size()==0) {
            return RegPlotInformationInfo();
        }
        if (free_plots[type][0].plot->getState()==0)
            return free_plots[type][0];
        else     {
            for (unsigned int i=0;i<free_plots[type].size();i++) {
                if (free_plots[type][i].plot->getState()==0) {
                    return free_plots[type][i];
                }
            }
            return RegPlotInformationInfo();
        }
    } else {
        RegPlotInformationInfo pi;
        pi.plot= findFreePlotOfType(type);
        if (pi.plot!=0) {
            pi.tc=calculateDistanceCosts(pi.plot);
            pi.pe=pi.plot->getPaymentEntitlement();
            return pi;
        } else {
            return pi;
        }
    }
}

//-------------------------------------
//  MODIFY THE STATE OF A PLOT
//      -1: initialisation
//       0: idle
//       1: rented
//       2: agent_location
//       3: owned
//-------------------------------------
void
RegPlotInfo::setState(const int passState,RegFarmInfo* farm, int colour) {
    PA.state = passState;
    switch (PA.state) {
    case -1: // dead_plot
        rent_paid          = 0;
        second_offer = rent_paid;
        rented_by_agent    = -1;
        occupied_by_agent  = -1;
        rented_by_legal_type=-1;
        distance_costs     = 0;
        newley_rented      = false;
        break;
    case 0: // idle plot
        rent_paid          = 0;
        second_offer = rent_paid;
        previously_rented_by_agent    = rented_by_agent;
        previously_occupied_by_agent  = occupied_by_agent;
        previously_rented_by_legal_type=rented_by_legal_type;
        previously_paid_by_agent=rent_paid;
        rented_by_agent    = -1;
        rented_by_legal_type=-1;
        occupied_by_agent  = -1;
        distance_costs     = 0;
        paid_tacs     = 0;
        newley_rented      = false;
        break;
    case 1: // rented plot
        rented_by_agent    = farm->getFarmId();
        if (previously_rented_by_agent==-1)
            previously_rented_by_agent=farm->getFarmId();
        occupied_by_agent  = -1;
        rented_by_legal_type=farm->getLegalType();
        break;
    case 2: // plot is farmstead
        rent_paid          = 0;
        second_offer = rent_paid;
        rented_by_agent    = -1;
        rented_by_legal_type=farm->getLegalType();
        occupied_by_agent  = farm->getFarmId();
        if (previously_occupied_by_agent==-1)
            previously_occupied_by_agent=farm->getFarmId();
        newley_rented      = false;
        break;
    case 3: // plot is owened Land
        rent_paid          = 0;//farm.getRentOffer();
        second_offer = rent_paid;
        rented_by_agent    = -1;
        rented_by_legal_type=farm->getLegalType();
        occupied_by_agent  = farm->getFarmId();
        if (previously_occupied_by_agent==-1)
            previously_occupied_by_agent=farm->getFarmId();
        break;

    default:
        rent_paid          = 0;
        second_offer = rent_paid;
        rented_by_agent    = -1;
        occupied_by_agent  = -1;
        distance_costs     = 0;
        newley_rented      = false;
    }
    PA.farm_id=occupied_by_agent;
    // draw plot immediately if the object Draw exists.
}
void
RegPlotInfo::setPaymentEntitlement(double pe) {
    payment_entitlement = pe;
    initial_payment_entitlement = pe;
}
// identify the plots in the neighbourhood that have state state

int
RegPlotInfo::identifyPlotsSameState() {
    int c = identifyContiguousPlot(true,true,false);
    untagContiguousPlot();
    clearContiguousPlot();
    return c-1;
}
int
RegPlotInfo::identifyPreviousPlotsSameState(const int& farmnumber) {
    int c = identifyContiguousPlot(true,false,true,farmnumber);
    untagContiguousPlot();
    clearContiguousPlot();
    return c-1;
}
int
RegPlotInfo::identifyPlotsSameStateAndFarm(const int& farmnumber) {
    int c = identifyContiguousPlot(true,true,false,farmnumber);
    untagContiguousPlot();
    clearContiguousPlot();
    return c-1;
}

int
RegPlotInfo::identifyContiguousPlot(bool check_soiltype, bool check_farmnumber,bool check_previous_farmnumber,int farmnumberob,bool mark_update) {
    list<RegPlotInfo*> queue;
    contiguous_plot.push_back(this);
    queue.push_back(this);
    int counter=1;
    this->tag();
    if (mark_update) update =true;
    int soiltype=this->getSoilType();
    int farmnumber;
    if (occupied_by_agent!=-1)
        farmnumber=occupied_by_agent;
    else
        farmnumber=rented_by_agent;
    if (farmnumberob!=-1)
        farmnumber=farmnumberob;
    while (queue.size()>0) {
        int row=(*queue.begin())->getRow();
        int col=(*queue.begin())->getCol();
        queue.pop_front();
        int i=1;
        int n,o,s,w;  // coordinates Nord, Ost, Sued, West
        //plot to the north
        n=((row-i+g->NO_ROWS)%g->NO_ROWS)+(((col+g->NO_COLS)%g->NO_COLS)*g->NO_ROWS);
        if (   (((*region)[n]->getRentedByAgent()==farmnumber) || ((*region)[n]->getOccupiedByAgent() == farmnumber) || !check_farmnumber)  &&
                (((*region)[n]->getPreviouslyRentedByAgent()==farmnumber) || ((*region)[n]->getPreviouslyOccupiedByAgent() == farmnumber) || !check_previous_farmnumber )  &&
                (((*region)[n]->getSoilType()==soiltype) || !check_soiltype) &&
                ((*region)[n]->getTag()==false) &&
                ((*region)[n]->getState()!=-1)       ) {
            (*region)[n]->tag();
            if (mark_update) update =true;
            counter++;
            contiguous_plot.push_back((*region)[n]) ;
            queue.push_back((*region)[n]);
        }
        // plot to the east
        o=((row+g->NO_ROWS)%g->NO_ROWS)+(((col+i+g->NO_COLS)%g->NO_COLS)*g->NO_ROWS);
        if (   (((*region)[o]->getRentedByAgent()==farmnumber) || ((*region)[o]->getOccupiedByAgent() == farmnumber) || !check_farmnumber)  &&
                (((*region)[o]->getPreviouslyRentedByAgent()==farmnumber) || ((*region)[o]->getPreviouslyOccupiedByAgent() == farmnumber) || !check_previous_farmnumber)  &&
                (((*region)[o]->getSoilType()==soiltype) || !check_soiltype) &&
                ((*region)[o]->getTag()==false) &&
                ((*region)[o]->getState()!=-1)       ) {
            (*region)[o]->tag();
            if (mark_update) update =true;
            counter++;
            contiguous_plot.push_back((*region)[o]) ;
            queue.push_back((*region)[o]);
        }
        // plot to the south
        s=((row+i+g->NO_ROWS)%g->NO_ROWS)+(((col+g->NO_COLS)%g->NO_COLS)*g->NO_ROWS);
        if (   (((*region)[s]->getRentedByAgent()==farmnumber) || ((*region)[s]->getOccupiedByAgent() == farmnumber) || !check_farmnumber)  &&
                (((*region)[s]->getPreviouslyRentedByAgent()==farmnumber) || ((*region)[s]->getPreviouslyOccupiedByAgent() == farmnumber) || !check_previous_farmnumber)  &&
                (((*region)[s]->getSoilType()==soiltype) || !check_soiltype) &&
                ((*region)[s]->getTag()==false) &&
                ((*region)[s]->getState()!=-1)       ) {
            (*region)[s]->tag();
            if (mark_update) update =true;
            counter++;
            contiguous_plot.push_back((*region)[s]) ;
            queue.push_back((*region)[s]);
        }
        // plot to the west
        w=((row+g->NO_ROWS)%g->NO_ROWS)+(((col-i+g->NO_COLS)%g->NO_COLS)*g->NO_ROWS);
        if (   (((*region)[w]->getRentedByAgent()==farmnumber) || ((*region)[w]->getOccupiedByAgent() == farmnumber) || !check_farmnumber)  &&
                (((*region)[w]->getPreviouslyRentedByAgent()==farmnumber) || ((*region)[w]->getPreviouslyOccupiedByAgent() == farmnumber) || !check_previous_farmnumber)  &&            (((*region)[w]->getSoilType()==soiltype) || !check_soiltype) &&
                ((*region)[w]->getTag()==false) &&
                ((*region)[w]->getState()!=-1)       ) {
            (*region)[w]->tag();
            if (mark_update) update =true;
            counter++;
            contiguous_plot.push_back((*region)[w]) ;
            queue.push_back((*region)[w]);
        }
    }
    return counter;
}

double
RegPlotInfo::calcShareOfContiguousPlot(int farmnumber) {
    double own=0;
    double total=0;
    identifyContiguousPlot(true,true,false);
    for (unsigned int j=0;j<contiguous_plot.size();j++) {
        if ( contiguous_plot[j]->getRentedByAgent()   == farmnumber || contiguous_plot[j]->getOccupiedByAgent() == farmnumber) {
            own+=1;
        }
        total+=1;
    }
    double share=own/total;
    untagContiguousPlot();
    clearContiguousPlot();
    return share;
}

int
RegPlotInfo::countContiguousPlots(int farmnumber) {
    int c=identifyContiguousPlot(true,true,false);
    clearContiguousPlot();
    return c;
}

int
RegPlotInfo::countContiguousPlots() {
    int c = identifyContiguousPlot(true,false,false);
    clearContiguousPlot();
    return c;
}

void
RegPlotInfo::finish(RegRegionInfo* r) {
    if (g->FAST_PLOT_SEARCH && PA.state==2)  {
        plot_p.clear();
        dist.clear();
        pl_n.clear();
        pl_n.resize(g->NO_OF_SOIL_TYPES);
        dist.resize(g->NO_OF_SOIL_TYPES);
        for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
            plot_p.push_back(0);
            pl_n[i].resize(r->getNumberOfLandPlotsOfType(i));
            dist[i].resize(r->getNumberOfLandPlotsOfType(i));
        }
        for (int t=0;t<g->NO_OF_SOIL_TYPES;t++) {
            int counter =0;
            for (int j=0;j<g->NO_COLS*g->NO_ROWS;j++) {
                if ((*region)[j]->getSoilType()==t) {
                    dist[t][counter]=calculateDistanceCosts((*region)[j]);
                    pl_n[t][counter]=(*region)[j];
                    counter++;
                }
            }
            
            vector<double>::iterator a=dist[t].begin();
            vector<RegPlotInfo*>::iterator b=pl_n[t].begin();
            quicksort(&(*a),&(*b),0,pl_n[t].size()-1);
			
        }
    }
}


void
RegPlotInfo::initFreePlots(RegFarmInfo* farm) {
    free_plots.clear();
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        vector<RegPlotInformationInfo> tmp_p;
        for (unsigned int j=0;j<(*region).size();j++) {
            if (checkPlot(j,i)) {
                RegPlotInformationInfo pi=calculateValue((*region)[j],farm);
                tmp_p.push_back(pi);
             }
        }

        if(g->SWEDEN) {
          sort(tmp_p.begin(),tmp_p.end(),alternative_comp);
        } else {
		 if (tmp_p.size()>0)  //DCX
			 quicksort(&(*tmp_p.begin()),0,tmp_p.size()-1);

			//sort(tmp_p.begin(),tmp_p.end());
        }

        free_plots.push_back(tmp_p);
    }
}

RegPlotInformationInfo RegPlotInfo::getValue(RegPlotInfo* plot, RegFarmInfo* farm) {
    if (g->WEIGHTED_PLOT_SEARCH) {
        int type=plot->getSoilType();
        for (unsigned int i=0;i<free_plots[type].size();i++) {
            if (free_plots[type][i].plot->getId()==plot->getId())
                  return free_plots[type][i];

        }
        return calculateValue(plot,farm);
    } else {
        return calculateValue(plot,farm);
    }
}
RegPlotInformationInfo RegPlotInfo::calculateValue(RegPlotInfo* plot, RegFarmInfo* farm) {
    int farm_id=farm->getFarmId();
    RegPlotInformationInfo pc;
    pc.tc=calculateDistanceCosts(plot);
    pc.pe=plot->getPaymentEntitlement();
    pc.farm_tac=0;
    pc.tac=0;
    pc.plot=plot;
    if (g->INITIALISATION)
        return pc;
    if (g->WEIGHTED_PLOT_SEARCH) {
        if (g->USE_TC_FRAMEWORK) {
            pc.tac=plot->getTacs();
            if (!g->LEGAL_TYPE_BONUS && !g->PREV_OWNER_BONUS) {
                pc.farm_tac = plot->getTacs();
            }
            if (g->LEGAL_TYPE_BONUS && !g->PREV_OWNER_BONUS) {
                if (!(farm->getLegalType()==3))
                    pc.farm_tac = plot->getTacs();
                else
                    pc.farm_tac = 0;
            }
            if (!g->LEGAL_TYPE_BONUS && g->PREV_OWNER_BONUS) {
                if (!(plot->getPreviouslyRentedByAgent()==farm_id))
                    pc.farm_tac = plot->getTacs();
                else
                    pc.farm_tac = 0;
            }
            if (g->LEGAL_TYPE_BONUS && g->PREV_OWNER_BONUS) {
                if (!(plot->getPreviouslyRentedByAgent()==farm_id) && !(farm->getLegalType()==3))
                    pc.farm_tac = plot->getTacs();
                else
                    pc.farm_tac = 0;
            }

        } else {

        if (g->ENV_MODELING) {
                int neighbours;
                if (g->USE_HISTORICAL_CONTIGUOUS_PLOTS)  {
                    neighbours = plot->identifyPreviousPlotsSameState(farm_id);
                } else {
                    neighbours = plot->identifyPlotsSameStateAndFarm(farm_id);
                }
                pc.farm_tac=pc.tac = -neighbours * g->RENT_ADJUST_COEFFICIENT_N;
                if(g->SWEDEN) {
                  pc.alternative_search_value=-neighbours*g->WEIGHTED_PLOT_SEARCH_VALUE;
                }
                /*   if (neighbours>2) // mehr als 2 plots
                   if (neighbours < 2 && neighbours > 0) //1 - 2 plots
                       rent_offer = rent_offer*factor + g->RENT_ADJUST_COEFFICIENT_N;
                   if (neighbours == 0)
                       rent_offer = rent_offer * factor - g->RENT_ADJUST_COEFFICIENT_N;*/
            } else {
                pc.farm_tac=pc.tac = 0;
            }

        }
    }
    return pc;
}
void
RegPlotInfo::clearContiguousPlot() {
    contiguous_plot.clear();
}

void
RegPlotInfo::untagContiguousPlot() {
    for (unsigned int j=0;j<contiguous_plot.size();j++) {
        contiguous_plot[j]->unTag();
    }
}
