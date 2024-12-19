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

// RegLink.cpp
//---------------------------------------------------------------------------
#include <sstream>
#include "RegLink.h"
#include "RegFarm.h"
RegLinkObject::RegLinkObject():source_number(0), type(-1) {};
int RegLinkObject::getDestKind() {
    return dest_kind;
}
int RegLinkObject::getSourceNumber() {
    return source_number;
}
int RegLinkObject::getDestNumber() {
    return dest_number;
}
int RegLinkObject::getType() {
    return type;
}
bool RegLinkObject::trigger() {
    bool retval=(dest[dest_number]!= res_value) && (dest_kind==0 || dest_kind==2);
    dest[dest_number]= res_value;
    return retval;

}

RegLinkMarketObject::RegLinkMarketObject(int dn, int dk,int sn, int vk,double f) {
    type=0;
    dest_number=dn;
    dest_kind=dk;
    source_number=sn;
    value_kind=vk;
    factor=f;
}
void
RegLinkMarketObject::init(RegProductList* s,double* d) {
    source=s;
    dest=d;
}

bool
RegLinkMarketObject::trigger() {
    switch (value_kind) {
    case 0:
        res_value=source->getVarCostsOfNumber(source_number)*factor;
        break;
    case 1:
        res_value=source->getPriceOfNumber(source_number)*factor;
        break;
    case 2:
        res_value= source->getPriceExpectationOfNumber(source_number)*factor;
        break;
    case 3:
        res_value=source->getGmOrGmExpectedOfNumber(source_number)*factor;
        break;
    /*case 4:
        res_value= source->getNoptOfNumber(source_number)*factor;
        break;
	//*/

    default:
        res_value=9999;
    }
    return RegLinkObject::trigger();
}
string
RegLinkMarketObject::debug() {
    string r;
    switch (value_kind) {
    case 0:
        r="costs";
        break;
    case 1:
        r="price";
        break;
    case 2:
        r="price expectation";
        break;
    case 3:
        r="gm";
        break;
    default:
        r=" undef. ";
    }
    stringstream  x;
    x << "Link from market."<<r<<" number "<<source_number<<" to ";
    switch (dest_kind) {
    case 0:
        r=" mat_val ";
        break;
    case 1:
        r=" rhs ";
        break;
    case 2:
        r=" obj ";
        break;
    default:
        r=" undef. ";
    }
    x<< r<<"number"<<dest_number<<"\n";
    return x.str();
}


//soil service
RegLinkYieldObject::RegLinkYieldObject(int dn, int dk,int sn,int vk,double f) {
    type=0;
    dest_number=dn;
    dest_kind=dk;
    source_number=sn;
	value_kind=vk;
    factor=f;
}

void
RegLinkYieldObject::init(RegFarmInfo* s,double* d) {
    source=s;
    dest=d;
}

bool
RegLinkYieldObject::trigger() {
	RegProductList * t = source->getProductList();
	double nopt = t->getNoptOfNumber(source_number,source);
	res_value = t->getYieldOfNumber(source_number, source);//*factor;
	res_value/=-1;  //??

	switch (value_kind) {
    case 0:  res_value = res_value*factor; 
			break;
	case 1:  res_value = t->getPOfNumber(source_number)*nopt*factor;
			break;
	case 2: res_value = t->getKOfNumber(source_number)*nopt*factor;
			break;
	case 3: res_value = t->getPesticideOfNumber(source_number)*nopt*factor;
			break;

	case 4: res_value *= -t->getEnergyvarOfNumber(source_number)*factor;
			break;

	case 5: res_value= nopt*factor;
			break; 
	
	default: res_value=-9999;
	}
			
    return    RegLinkObject::trigger();

}
string
RegLinkYieldObject::debug() {
    return string("Yield Link");

}
//=========================


RegLinkLandObject::RegLinkLandObject(int dn, int dk,int sn,double f) {
    type=0;
    dest_number=dn;
    dest_kind=dk;
    source_number=sn;
    factor=f;
}


void
RegLinkLandObject::init(RegFarmInfo* s,double* d) {
    source=s;
    dest=d;
}

bool
RegLinkLandObject::trigger() {
    res_value=source->getLandInputOfType(source_number)*factor;
    return    RegLinkObject::trigger();

}
string
RegLinkLandObject::debug() {
    return string("");

}

//===================Invest ======================
RegLinkInvestObject::RegLinkInvestObject(int dn, int dk,int sn, int vk,double f) {
    type=1;
    dest_number=dn;
    dest_kind=dk;
    source_number=sn;
    value_kind=vk;
    factor=f;
}

void
RegLinkInvestObject::init(RegInvestList* s,double* d) {
    source=s;
    dest=d;
}

bool
RegLinkInvestObject::trigger() {
    switch (value_kind) {
    case 0:
        res_value=source->getCapacityOfType(source_number)*factor;
        break;
    case 1:
        res_value=source->getLiqEffectOfNumber(source_number)*factor;
        break;
    case 2:
        res_value=source->getBoundEquityCapitalOfNumber(source_number)*factor;
        break;
    case 3:
        res_value=source->getAverageCostOfNumber(source_number)*factor;
        break;
    case 4:
        res_value=source->getNormalizedCapacityOfType(source_number)*factor;
        break;
    default:
        res_value=9999;
    }
    return    RegLinkObject::trigger();

    // debug();
}
string
RegLinkInvestObject::debug() {
    string r;
    stringstream x;
    switch (value_kind) {
    case 0:
        r="capacity";
        break;
    case 1:
        r="liquidity_effect";
        break;
    case 2:
        r="bound equity capital";
        break;
    case 3:
        r="average costs";
        break;
    case 4:
        r="normalized capacity";
        break;
    default:
        r=" undef. ";
    }
    x<<"Link from invest."<<r<<" number "<<source_number<<" to ";
    switch (dest_kind) {
    case 0:
        r=" mat_val ";
        break;
    case 1:
        r=" rhs ";
        break;
    case 2:
        r=" obj ";
        break;
    default:
        r=" undef. ";
    }
    x<<r<<"number"<<dest_number<<"\n";
    return x.str();
}
RegLinkReferenceObject::RegLinkReferenceObject(int dn,int dk,int sn,double f) {
    type=2;
    dest_number=dn;
    dest_kind=dk;
    source_number=sn;
    factor = f;
}

void
RegLinkReferenceObject::init(double* s,double* d) {
    dest=d;
    source=s;
}

bool
RegLinkReferenceObject::trigger() {
    res_value = *source;
    return    RegLinkObject::trigger();

}
RegLinkNumberObject::RegLinkNumberObject(int dn,int dk,double v) {
    type=3;
    dest_number=dn;
    dest_kind=dk;
    value =v;
}
string
RegLinkReferenceObject::debug() {
    string r ="ReferenceLink:not implemented";
    return r;
}
string
RegLinkNumberObject::debug() {
    string r ="NumberLink:not implemented";
    return r;
}

void
RegLinkNumberObject::init(double* d) {
    dest=d;
}

bool
RegLinkNumberObject::trigger() {
    res_value = value;
    return    RegLinkObject::trigger();
}
