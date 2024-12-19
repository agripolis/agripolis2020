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

//Reading text input files, without policy setting file

#ifndef TEXTINPUT_H
#define  TEXTINPUT_H
#include <time.h>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <cctype>

using namespace std;

const string  DEMOGRAPH_FILE = "demographics.txt";
const string  YOUNGFARMER_FILE = "youngfarmer.txt";

const string  FARMDIR  =  "farms/";
const string  MIPDIR  =  "mip/";
const string  TXT  =  ".txt";
const string  TESTDIR  =  "test/";

const string  GLOBFILE  =  "globals.txt";
const string  TRANSLFILE  =  "translations.txt";
const string  MARKETFILE  =  "market.txt";
const string  INVESTFILE  =  "investments.txt";
const string  ENV_MARKETFILE  =  "env_market.txt";
const string  ENVIRONFILE  =  "environmental.txt";
const string  FARMSDATAFILE  =  "farmsdata.txt";

const string  MATRIXFILE  =  "matrix.txt";
const string  MATRIX_NEWFILE = "matrix_new.txt";
const string  CAPLFILE  =  "capacityLinks.txt";
const string  OBJLFILE  =  "objFuncLinks.txt";
const string  MATLFILE  =  "matrixLinks.txt";

const string YIELDFILE = "yield.txt";


extern string   inputdir;

struct lands
{
   vector < double> owned_land;
   vector < double> rented_land;
   vector < double> initial_rent_price;

   //soil service
	vector<double> carbon_mean;
	vector<double> carbon_std_dev;
} ;

struct farmsdata {
   int numOfFarms;
   vector <string  > names;
   vector <int> farm_types;
   vector <int> formsOrgs;
   vector <int> weightFacs;
   vector <double> land_inputs;
   map <string  , lands > alllands;
   vector <double> milk_quotas;
   vector <double> fam_lab_units;
   vector <double> off_fam_labs;
   vector <double> equity_capitals;
   vector <double> land_assets;
   vector <double> rel_invest_ages;
};

extern struct farmsdata farmsdata  ;

struct oneInvest {
    string   name;
    int quant;
    int capacity;
    oneInvest(): name(""), quant(0), capacity(0) {};
    oneInvest(string   n, int q, int c): name(n), quant(q), capacity(c){};

} ;

struct farminvestdata {
    vector < oneInvest >  initinvests;
};

extern map <string  , farminvestdata > farmsIinvest;

struct globdata {
   vector <string  > namesOfSoilTypes;
   map <string  , string  > globs;
} ;

extern struct globdata globdata;

struct transdata {
    map <string   , string  > trans;
};

extern struct transdata transdata  ;

struct matrixdata {
    vector <string  > rownames;
    vector <string  > colnames;
    vector <int> isInt;
    vector <vector<double> > mat;
} ;

extern struct matrixdata matrixdata  ;

struct aRestrict0 {
    string name;
    vector<string> terms0;
};

struct matrixdata_n {
    vector <string> VarNames0;
    vector <string> IntVars0;
    vector <aRestrict0> Restricts0;
} ;

extern struct matrixdata_n matrixdata_n  ;

struct aRestrict {
    string name;
    vector<pair<string, double>> terms;
};

struct matrixdata_new {
    vector <string> VarNames;
    vector <string> IntVars;
    vector <aRestrict> Restricts;
} ;

struct onelink {
    string   name;
    string   linktype;
    string   numbertype;
    string   valuetype;
    double factor;

    onelink(): numbertype("-"), factor(0){}
} ;


struct caplinkdata{
     vector <onelink> caplinks;
} ;

extern struct caplinkdata caplinkdata   ;

struct objlinkdata {
     vector<onelink> objlinks;
} ;

extern struct objlinkdata objlinkdata   ;

struct onematlink {
    string   row;
    string   col;
    string   linktype;
	string   numbertype;
    string   valuetype;
    double factor;
    onematlink():numbertype("-"), factor(0){};
} ;

struct matlinkdata {
     //map <string  ,double> mark_fac;
     vector <onematlink> matlinks;
};

extern struct matlinkdata   matlinkdata   ;

struct envproduct {
    string   name;
    double n;
    double p2o5;
    double k2o;
    double fungicide;
    double herbicide;
    double insecticide;
    double water;
    double soil_loss;
    int biohabitat;
};

struct envmarketdata {
     vector <envproduct> envproducts;
} ;

extern struct envmarketdata  envmarketdata    ;


struct oneinvest {
    int type;
    string   name;
    double cost;
    double life;
    double labsub;
    double landsub;
    double capacity;
    double maintenance;
    int group;
    double techchange;
	oneinvest() {}
	oneinvest(int typ, string nam, double cst, double labsubt) :type(typ), name(nam), cost(cst), life(1),
		labsub(labsubt), capacity(0), maintenance(0), group(-1), techchange(-1) {}
};

struct investdata {
     string FixOffFarmLabName;
     string FixHiredLabName;
      vector <oneinvest>  invests;
}  ;

extern struct investdata investdata     ;

struct oneproduct {
    int type;
    int group;
    string   name;
    string stdName;
    double price;
    double varcost;
    double labour;
    double priceflex;
    double changerate;
    string   prodbytype;
    double luperplace;
    bool premium;
    double initprem;
    bool pricesupp;
    int refprem;
    int xyears;
	oneproduct(){}
	oneproduct(int typ, string nam, double pric, double lab):type(typ),group(-1),name(nam),stdName("-"),
	         price(pric),varcost(0),labour(lab),priceflex(0),changerate(1),prodbytype("NON"),
	         luperplace(0),premium(false),initprem(0),pricesupp(false),refprem(0),xyears(0){}
};

struct oneyield {
    string   name;
    double a,b,c,d;
	double e,f,c_plat;
    double gamma;
	string   soiltype;
	bool dyn;
	double p, k, pesticide, energyvar;
};

extern vector <oneyield> yielddata;

struct marketdata {
    string   premiumName;
    vector <oneproduct> products;
}  ;

extern struct marketdata marketdata    ;

struct act {
    string   name;
    int q;
};

struct group {
    int numSoils;
    vector <string  > soilNames;
    vector < act > acts;
}  ;

struct biohab {
    string   name;
    double value;
};

struct envdata {
      double zcoef;
      vector < biohab > biohabs;
      int numGroups;
      vector <group> groups;
} ;

extern struct envdata  envdata ;

extern const string  colsmarket[];
extern const string  colsinvest[];
extern const string  transwords[];

void readfiles(string,bool  );
#endif
