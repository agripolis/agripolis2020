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

#ifndef EvaluatorH
#define EvaluatorH
#include  <sstream>
#include "evalwrap.h"
#include "evalkern.h"
using namespace std;
class Evaluator {
public:
    Evaluator(string name);
    ~Evaluator();
    void setVariable(string part_name,int number,double value);
    void setVariable(string name,double value);

	//-1 : not found;
	int indOfVariable(string name, int num);
	int indOfVariable(string name);

    double getVariable(string part_name,int number);
    double getVariable(string name);
    void addFunctionBase(string);
    void clearFunctionBase();
    void evaluate();
    int getNoVariables();
    string getVariableName(int no);
    double getVariable(int no);
    void backup();
    void restore();
private:
    Evaluator* obj_backup;
    string function_base;
    string name;

    /* Symbol table */
    VariableDescriptor variable[N_VARIABLES];
    int nVariables;
    ErrorRecord errorRecord;
    evalKernel_pcb_type evalKernel_pcb;
    CharStack cs;
    ArgStack as;
};

#endif
