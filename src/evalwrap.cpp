#define _CRT_SECURE_NO_WARNINGS
/*
 EVALWRAP.C  Version 1.1

 evaluateExpression: A Simple Expression Evaluator
 Copyright (c) 1996 - 1999 Parsifal Software, All
 Rights Reserved.

 The EVALWRAP.C module provides support functions for the parser function
 evalKernel(), defined by EVALKERN.SYN. It includes definitions of the
 functions called by evalKernel() as well as the definition of the
 evaluateExpression function, implemented as a wrapper function for
 evalKernel().

 This module consists of six parts:
   1. Error diagnostic procedures, including the definition of
      checkZero.
   2. Character stack procedures, including the definition of
      pushChar.
   3. Symbol table procedures, including the definition of
      locateVariable, which provides access to named variables. In this
      implementation, there are no predefined variables. If a variable
      is not found, it is added to the table and initialized to zero.
      The lookup uses a binary search.
   4. Argument stack procedures, including the definition of
      pushArg.
   5. Function call interface which provides access to
      the standard C library math functions.
      The interface consists of
      . a functionTable, each entry of which contains the name of a
        function and a wrapper function which calls the named
        function.
      . an implementation of callFunction which does a binary search of
        functionTable and then calls the appropriate wrapper function.
      Macros are used to simplify generation of the wrapper functions
      and the functionTable entries.
   6. Definition of the evaluateExpression wrapper function.

 For further information about this module, contact
   Parsifal Software
   http://www.parsifalsoft.com
   info@parsifalsoft.com
   1-800-879-2755, 1-508-358-2564
   P.O. Box 219
   Wayland, MA 01778
*/

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "evalwrap.h"
#include "evalkern.h"

static char* mystrdup(const char* str)
{
 return strcpy((char*)malloc( strlen(str) + 1),str);
}

/*********************************************************************

 Part 1. Error Diagnostics

*********************************************************************/

/* define an error record */

void diagnoseError(const char *msg,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb) {
    if (evalKernel_pcb.exit_flag == AG_RUNNING_CODE)   /* parser still running */
        evalKernel_pcb.exit_flag = AG_SEMANTIC_ERROR_CODE;      /* stop parse */
    errorRecord.message = const_cast<char*>(msg);
    errorRecord.line    = evalKernel_pcb.line;
    errorRecord.column  = evalKernel_pcb.column;
}

double checkZero(double value,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb) {
    if (value) return value;
    diagnoseError("Divide by Zero",errorRecord,evalKernel_pcb);
    return 1;
}


/*******************************************************************

Part 2. Accumulate variable names and function names

*******************************************************************/



static void resetCharStack(CharStack& cs) {
    cs.charStackTop = cs.charStack;
}

void pushChar(int c,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb,CharStack& cs) {              /* append char to name string */
    if (cs.charStackTop < cs.charStack+CHAR_STACK_LENGTH) {
        *(cs.charStackTop++) = (char) c;
        return;
    }
    /* buffer overflow, kill parse and issue diagnostic */
    diagnoseError("Character Stack Overflow",errorRecord,evalKernel_pcb);
}

static char *popString(int nChars,CharStack& cs) {                /* get string */
    *(cs.charStackTop) = 0;
    return cs.charStackTop -= nChars;
}


/**********************************************************************

Part 3. Symbol Table

**********************************************************************/


/* Callback function to locate named variable */

double *locateVariable(int nameLength,VariableDescriptor* variable, int& nVariables,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb,CharStack& cs,ArgStack& as) {   /* identify variable name */
    char *name = popString(nameLength,cs);
    int first = 0;
    int last = nVariables - 1;

    while (first <= last) {                           /* binary search */
        int middle = (first+last)/2;
        int flag = strcmp(name,variable[middle].name);
        if (flag == 0) return &variable[middle].value;
        if (flag < 0) last = middle-1;
        else first = middle+1;
    }
    /* name not found, check for room in table */
    if (nVariables >= N_VARIABLES) {
        /* table is full, kill parse and issue diagnostic */
        static double junk = 0;
        diagnoseError("Symbol Table Full",errorRecord,evalKernel_pcb);
        return &junk;
    }

    /* insert variable in table in sorted order */
    memmove(&variable[first+1],
            &variable[first],
            (nVariables-first)*sizeof(VariableDescriptor));
    nVariables++;
    variable[first].name = mystrdup(name);//_strdup(name);
    variable[first].value = 0;
    return &variable[first].value;
}


/*******************************************************************

Part 4. Accumulate list of function arguments

*******************************************************************/

static void resetArgStack(ArgStack& as) {
    as.argStackTop = as.argStack;
}

void pushArg(double x,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb,CharStack& cs,ArgStack& as) {                     /* store arg in list */
    if (as.argStackTop < as.argStack + ARG_STACK_LENGTH) {
        *(as.argStackTop++) = x;
        return;
    }
    /* too many args, kill parse and issue diagnostic */
    diagnoseError("Argument Stack Full",errorRecord,evalKernel_pcb);
}

static double *popArgs(int nArgs,ArgStack& as) {                 /* fetch args */
    return as.argStackTop -= nArgs;
}


/**********************************************************************

 Part 5. Function Call Interface

 Define functionTable, each entry of which contains the ascii name of
 a function and a pointer to a wrapper function. The wrapper function
 checks the argument count and calls the real function.

 Then, define callFunction. Given the ascii name of a function,
 callFunction does a binary search of functionTable and on a successful
 search calls the corresponding wrapper function.

**********************************************************************/

/* define some macros to build the wrapper functions */

/*
 First, a macro to make a wrapper function for a function with one
 argument.
*/

#define WRAPPER_FUNCTION_1_ARG(FUN) \
double FUN##Wrapper(int argc, double *argv,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb) {\
  if (argc == 1) return FUN(argv[0]);\
  diagnoseError("Wrong Number of Arguments",errorRecord,evalKernel_pcb);\
  return 0;\
}

/*
 Now, a macro to make a wrapper function for a function with two
 arguments.
*/

#define WRAPPER_FUNCTION_2_ARGS(FUN) \
double FUN##Wrapper(int argc, double *argv,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb) {\
  if (argc==2) return FUN(argv[0], argv[1]);\
  diagnoseError("Wrong Number of Arguments",errorRecord,evalKernel_pcb);\
  return 0;\
}


/*
 Now define wrapper functions for the standard C library
 math functions.
*/

WRAPPER_FUNCTION_1_ARG(acos)
WRAPPER_FUNCTION_1_ARG(asin)
WRAPPER_FUNCTION_1_ARG(atan)
WRAPPER_FUNCTION_2_ARGS(atan2)
WRAPPER_FUNCTION_1_ARG(cos)
WRAPPER_FUNCTION_1_ARG(cosh)
WRAPPER_FUNCTION_1_ARG(exp)
WRAPPER_FUNCTION_1_ARG(fabs)
WRAPPER_FUNCTION_2_ARGS(fmod)
WRAPPER_FUNCTION_1_ARG(log)
WRAPPER_FUNCTION_1_ARG(log10)
WRAPPER_FUNCTION_1_ARG(sin)
WRAPPER_FUNCTION_1_ARG(sinh)
WRAPPER_FUNCTION_1_ARG(sqrt)
WRAPPER_FUNCTION_1_ARG(tan)
WRAPPER_FUNCTION_1_ARG(tanh)


/* A macro to make correct functionTable entries */
#define TABLE_ENTRY(FUN) {#FUN, FUN##Wrapper}

/* remember to fix this when you add more functions to the table */
#define N_FUNCTIONS 16

/* define the function table -- must be in sorted order! */
struct {
    const char *name;
    double (*function)(int, double[],ErrorRecord&,evalKernel_pcb_type&);
}
functionTable[N_FUNCTIONS] = {
                                 TABLE_ENTRY(acos),
                                 TABLE_ENTRY(asin),
                                 TABLE_ENTRY(atan),
                                 TABLE_ENTRY(atan2),
                                 TABLE_ENTRY(cos),
                                 TABLE_ENTRY(cosh),
                                 TABLE_ENTRY(exp),
                                 TABLE_ENTRY(fabs),
                                 TABLE_ENTRY(fmod),
                                 TABLE_ENTRY(log),
                                 TABLE_ENTRY(log10),
                                 TABLE_ENTRY(sin),
                                 TABLE_ENTRY(sinh),
                                 TABLE_ENTRY(sqrt),
                                 TABLE_ENTRY(tan),
                                 TABLE_ENTRY(tanh),
                             };


/* Finally, define the callback function to perform a function call */

double callFunction(int nameLength, int argCount,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb,CharStack& cs,ArgStack& as) {
    char *name = popString(nameLength,cs);
    double *argValues = popArgs(argCount,as);
    int first = 0;
    int last = N_FUNCTIONS-1;
    while (first <= last) {                     /* binary search */
        int middle = (first+last)/2;
        int flag = strcmp(name,functionTable[middle].name);
        if (flag == 0) return functionTable[middle].function(argCount, argValues,errorRecord,evalKernel_pcb);
        if (flag < 0) last = middle-1;
        else first = middle+1;
    }
    diagnoseError("Unknown Function",errorRecord,evalKernel_pcb);
    return 0;
}

/*******************************************************************

Part 6. Wrapper function definition

*******************************************************************/

int evaluateExpression(char *expressionString,VariableDescriptor* variable, int& nVariables,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb,CharStack& cs,ArgStack& as) {
    resetCharStack(cs);
    resetArgStack(as);
    evalKernel_pcb.pointer = (unsigned char *) expressionString;
    evalKernel(variable,nVariables,errorRecord,evalKernel_pcb,cs,as);
    return evalKernel_pcb.exit_flag != AG_SUCCESS_CODE;
}

/* End of evalwrap.c */
