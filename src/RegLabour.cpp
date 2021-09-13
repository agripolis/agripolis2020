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

// RegLabour.cpp
//---------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>

#include "RegLabour.h"
RegLabourInfo::RegLabourInfo(RegGlobalsInfo* G) :g(G) {
    fix_offfarm_labour=0;
    fix_offfarm_pay=0;
    fix_onfarm_labour=0;
    fix_onfarm_pay=0;
    var_offfarm_labour=0;
    var_offfarm_pay=0;
    var_onfarm_labour=0;
    var_onfarm_pay=0;
    family_labour =  2 * g->MAX_H_LU;
    labour_input_hours = family_labour;
    labour_capacity = family_labour;
    obj_backup=NULL;
}
RegLabourInfo::RegLabourInfo(RegLabourInfo& rh,RegGlobalsInfo* G) {
    *this=rh;
    g=G;
    obj_backup=NULL;
}
RegLabourInfo::~RegLabourInfo() {
    if (obj_backup) delete obj_backup;
}

void
RegLabourInfo::setInitialFamLu(double f) {
    adjustfam_labour = f * g->MAX_H_LU;
    family_labour =  adjustfam_labour;
    labour_input_hours = family_labour;
    labour_capacity = family_labour;
}

void
RegLabourInfo::adjustFamLu(double f, double coeff) {
    adjustfam_labour = /*coeff * */f * g->MAX_H_LU;

    family_labour = adjustfam_labour;
    labour_input_hours = family_labour;
    labour_capacity = family_labour;
}

void
RegLabourInfo::addFixOfffarmLabour(double fofffl) {
    fix_offfarm_labour += fofffl;
    labour_input_hours -=fofffl;
    if (labour_input_hours <0)
        labour_input_hours = 0;
}
void
RegLabourInfo::addFixOfffarmPay(double fofffp) {
    fix_offfarm_pay += fofffp;
}

double
RegLabourInfo::getFixOfffarmPay() {
    return fix_offfarm_pay;
}

void
RegLabourInfo::addFixOnfarmLabour(double fonfl) {
    fix_onfarm_labour += fonfl;
    labour_input_hours +=fonfl;
}
void
RegLabourInfo::addFixOnfarmPay(double fonfp) {
    fix_onfarm_pay += fonfp;
}
double
RegLabourInfo::getFixOnfarmPay() {
    return fix_onfarm_pay;
}

void
RegLabourInfo::addVarOfffarmLabour(double vofffl) {
    var_offfarm_labour += vofffl;
    labour_input_hours -= vofffl;
}
void
RegLabourInfo::addVarOfffarmPay(double vofffp) {
    var_offfarm_pay += vofffp;
}
double
RegLabourInfo::getVarOfffarmPay() {
    return var_offfarm_pay;
}

void
RegLabourInfo::addVarOnfarmLabour(double vonfl) {
    var_onfarm_labour += vonfl;
    labour_input_hours += vonfl;
}
void
RegLabourInfo::addVarOnfarmPay(double vonfp) {
    var_onfarm_pay += vonfp;
}
double
RegLabourInfo::getVarOnfarmPay() {
    return var_onfarm_pay;
}
void
RegLabourInfo::addLabourSubstitution(double ls) {
    labour_input_hours -= ls;
}

double
RegLabourInfo::getFamilyLabour() {
    return family_labour;
}
double
RegLabourInfo::getLabourInputHours() {
    return labour_input_hours;
}
void
RegLabourInfo::setLabourInputHours(double labour_input_hours) {
      this->labour_input_hours=labour_input_hours;
}
double
RegLabourInfo::getVarOnfarmLabour() {
    return var_onfarm_labour;
}
double
RegLabourInfo::getFixOfffarmLabour() {
    return fix_offfarm_labour;
}
double
RegLabourInfo::getFixOnfarmLabour() {
    return fix_onfarm_labour;
}
double
RegLabourInfo::getVarOfffarmLabour() {
    return var_offfarm_labour;
}
void
RegLabourInfo::setLabourCapacity(double lc) {
//   if(lc < 0)
//       labour_capacity = 0;
//   else
    labour_capacity = lc;
}
void
RegLabourInfo::setBackLabour() {
    labour_input_hours =  adjustfam_labour;//family_labour;
    labour_capacity = adjustfam_labour;// family_labour;
    fix_offfarm_labour=0;
    fix_offfarm_pay=0;
    fix_onfarm_labour=0;
    fix_onfarm_pay=0;
    var_offfarm_labour=0;
    var_offfarm_pay=0;
    var_onfarm_labour=0;
    var_onfarm_pay=0;
}
void
RegLabourInfo::backup() {
    obj_backup=new RegLabourInfo(*this);
}
void
RegLabourInfo::restore() {
    RegLabourInfo* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
}
