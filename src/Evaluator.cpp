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

#include <fstream>
#include <sstream>

#include "Evaluator.h"
using namespace std;
Evaluator::Evaluator(string name) {
    this->name=name;
    nVariables=0;
    cs.charStackTop=cs.charStack;
    as.argStackTop=as.argStack;
    obj_backup=NULL;
}
Evaluator::~Evaluator() {
    if (obj_backup) delete obj_backup;
}

void
Evaluator::setVariable(string part_name,int number,double value) {
    stringstream s;
    s<<part_name<<number;
    setVariable(s.str(),value);

}

void
Evaluator::setVariable(string name,double value) {
    stringstream s;
    s  << name<<"="<<value<<";\n";
    function_base+=s.str();

}
double
Evaluator::getVariable(string part_name,int number) {
    stringstream s;
    s<<part_name<<number;
    return getVariable(s.str());

}
double
Evaluator::getVariable(string name) {
    int i=0;
    for (i = 0; i < nVariables; i++) {
        string t=string(variable[i].name);
        if (t==name) return variable[i].value;
    }
    return 0;
}

int Evaluator::indOfVariable(string part_name,int number) {
    stringstream s;
    s<<part_name<<number;
    return indOfVariable(s.str());

}

//variable can not be removed !!
int Evaluator::indOfVariable(string name) {
    int res=-1;
	int i;
	if (function_base.find(name)!=string::npos){
    for (i = 0; i < nVariables; i++) {
        string t=string(variable[i].name);
        if (t==name) {
			res= i;
			break;
		}
    }
	}
    return res;
}

int
Evaluator::getNoVariables() {
    return nVariables;
}
string
Evaluator::getVariableName(int no) {
    if (no>=nVariables) return "";
    else return variable[no].name;
}
double
Evaluator::getVariable(int no) {
    if (no>=nVariables) return 0;
    else return variable[no].value;
}
void
Evaluator::addFunctionBase(string f) {
    function_base+=f;
}
void
Evaluator::clearFunctionBase() {
    function_base="";
}
void
Evaluator::evaluate() {
    ofstream out;
//    out.open((name+"_test.txt").c_str(),ios::trunc);
//    out << function_base.c_str();
//    out.close();
    int errorFlag = evaluateExpression((char*)(function_base.c_str()),variable,nVariables,errorRecord,evalKernel_pcb,cs,as);

    if (errorFlag) {
//        ofstream out;
        out.open((name+"_error.txt").c_str(),ios::trunc);
        out << errorRecord.message
        << " at line "
        << errorRecord.line
        << " , column "
        << errorRecord.column
        << "\n"
        << function_base.c_str();
        out.close();
    }
//DCX
    else out.close();

}
void
Evaluator::backup() {
    obj_backup=new Evaluator(*this);
}
void
Evaluator::restore() {
    Evaluator* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
}

