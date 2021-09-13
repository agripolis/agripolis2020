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

//---------------------------------------------------------------------------
#ifndef RegLinkH
#define RegLinkH
#include "RegProduct.h"
#include "RegFarm.h"
//class RegFarmInfo;
#include "RegInvest.h"

//---------------------------------------------------------------------------
//Link Objekts connect MIP values mat_val , rhs and obj  to their sources 

//Types of Links:
// 0:Market, 1:Invest, 2:Reference, 3:Number
class RegProductList;
class RegLinkObject {
public:
    RegLinkObject();
    ~RegLinkObject() {};
    virtual bool trigger();
    virtual int getSourceNumber();
    virtual int getDestNumber();
    virtual int getDestKind();
    virtual int getType();
    virtual string debug()=0;
protected:
    double res_value;
    int value_kind;
    int dest_kind;  //0: mat_val;
    //1: rhs;
    //2: obj;
    double *dest;
    int dest_number;
    int source_number;
    double factor;
    int type;
};


//soil service link
class RegLinkYieldObject : public RegLinkObject {
private:
    RegGlobalsInfo* g;
    RegFarmInfo * source;
public:
    ~RegLinkYieldObject() {};
    RegLinkYieldObject(int,int,int,int,double);
    void init(RegFarmInfo*,double*);
    bool trigger();

    string debug();
};

class RegLinkLandObject : public RegLinkObject {
private:
    RegGlobalsInfo* g;
    RegFarmInfo * source;
public:
    ~RegLinkLandObject() {};
    RegLinkLandObject(int,int,int,double);
    void init(RegFarmInfo*,double*);
    bool trigger();

    string debug();
};

class RegLinkMarketObject : public RegLinkObject {
private:
    RegProductList * source;
//    int value_kind; //0: getCostsOfNumber();
    //1: getpriceOfNumber();
    //2: getpriceExpectationOfNumber();
    //3: Grossmargin
public:
    ~RegLinkMarketObject() {};
    RegLinkMarketObject(int,int,int,int,double);
    void init(RegProductList*,double*);
    bool trigger();

    string debug();
};
class RegLinkInvestObject : public RegLinkObject {
private:
    RegInvestList * source;
public:
    ~RegLinkInvestObject() {};
    RegLinkInvestObject(int,int,int,int,double);
    void init(RegInvestList*,double*);
    bool trigger();

    string debug();
};

class RegLinkReferenceObject : public RegLinkObject {
private:
    double *source;

public:
    ~RegLinkReferenceObject() {};
    RegLinkReferenceObject(int,int,int,double);
    void init(double*,double*);
    bool trigger();

    string debug();
};
class RegLinkNumberObject : public RegLinkObject {
private:
    double value;
public:
    ~RegLinkNumberObject() {};
    RegLinkNumberObject(int,int,double);
    void init(double*);
    bool trigger();

    string debug();
};
#endif
