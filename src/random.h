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

#ifndef MT_RANDH
#define MT_RANDH

void randInit(unsigned long int x );

long int randlong(void);

extern long int mtRandMin ;
extern long int mtRandMax ;

double rand_normal(double mean, double stddev);
#endif
