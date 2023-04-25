/*************************************************************************
* This file is part of AgriPoliS
*
* AgriPoliS: An Agricultural Policy Simulator
*
* Copyright (c) 2023, Alfons Balmann, Kathrin Happe, Konrad Kellermann et al.
* (cf. AUTHORS.md) at Leibniz Institute of Agricultural Development in
* Transition Economies
*
* SPDX-License-Identifier: MIT
**************************************************************************/

#include <iostream>
#include <string>
#include "RegRegr.h"

using namespace std;

//weighted regression r/sqrt(x+y)=a x/sqrt(x+y) + b y/sqrt(x+y)
static vector<double> fit(vector<double> f1, vector<double> f2, vector<double> r){
	
	double xx = 0;
	double xy = 0;
	double yy = 0;
	double rx = 0;
	double ry = 0;

	int sz = r.size();

	for (int i = 0; i < sz; ++i) {
		xx += (f1[i] * f1[i]) / (f1[i] + f2[i]);
		xy += f1[i] * f2[i] / (f1[i] + f2[i]);
		yy += f2[i] * f2[i] / (f1[i] + f2[i]);
		rx += f1[i] * r[i] ;
		ry += f2[i] * r[i] ;
	}

	double a = 0;
	double b = 0;
	a = (yy * rx - xy * ry) / (xx * yy - xy * xy);
	b = (xx * ry - xy * rx) / (xx * yy - xy * xy);

	vector<double> res{ a,b };
	return res;
}
	
static double fittotal(vector<double> f1, vector<double> f2, vector<double> r) {
	int sz = f1.size();
	double tot = 0;
	for (int i = 0; i < sz; ++i) {
		tot += f1[i] * r[0] + f2[i] * r[1];
	}
	return tot;
}

void updateEconLandRentsPy(RegGlobalsInfo* g) {
	bool debug = false;
	vector<double> r1=fit(g->Farm_Lands[0], g->Farm_Lands[1], g->Farm_Econ_Land_Rents);
			
	if (debug) {
		double tot_fit = fittotal(g->Farm_Lands[0], g->Farm_Lands[1], r1);
		std::cout << g->Total_Econ_Land_Rent << endl;
		std::cout << tot_fit << std::endl;
		std::cout << "delta: " << g->Total_Econ_Land_Rent - tot_fit << std::endl;
	}

	g->Econ_Land_Rents = r1;
}

