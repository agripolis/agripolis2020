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

#ifndef EVALDEFS_H
#define EVALDEFS_H

/*****************************************************************

 EVALDEFS.H

 Interface declarations for EVALKERN.SYN
 Copyright (c) 1996 - 1999 Parsifal Software. All Rights Reserved.

 For further information about this program, contact
   Parsifal Software
   http://www.parsifalsoft.com
   info@parsifalsoft.com
   1-800-879-2755, Voice/Fax 1-508-358-2564
   P.O. Box 219
   Wayland, MA 01778

*****************************************************************/


/*****************************************************************

Override AnaGram defaults

*****************************************************************/

/* override default definition of SYNTAX_ERROR */

#define SYNTAX_ERROR(a,b,c) diagnoseError(a,b,c)


/*****************************************************************

Define compile time constants

*****************************************************************/

/* define array sizes */
#define CHAR_STACK_LENGTH 1000          /* Length of char stack */
#define ARG_STACK_LENGTH   200          /* Length of arg stack  */
#define N_VARIABLES        10000          /* Size of symbol table */


/*****************************************************************
Define data types
*****************************************************************/

/* Define an error record */
typedef struct {
    char *message;                 /* identifies error */
    int line;                      /* location of error */
    int column;
}
ErrorRecord;
/* Define an error record */
typedef struct {
    char  charStack[CHAR_STACK_LENGTH+1];
    char *charStackTop;
}
CharStack;
typedef struct {
    double  argStack[ARG_STACK_LENGTH+1];
    double *argStackTop;
}
ArgStack;

/* Define a symbol table entry */
typedef struct {
    char   *name;
    double value;
}
VariableDescriptor;
typedef union {
    long alignment;
    char ag_vt_2[sizeof(int)];
    char ag_vt_4[sizeof(double)];
} evalKernel_vs_type;

typedef enum {
    evalKernel_white_space_token = 1, evalKernel_input_string_token = 4,
    evalKernel_expressions_token, evalKernel_eof_token,
    evalKernel_expression_token, evalKernel_conditional_expression_token = 10,
    evalKernel_logical_or_expression_token = 16,
    evalKernel_logical_and_expression_token = 19,
    evalKernel_equality_expression_token = 21,
    evalKernel_relational_expression_token = 23,
    evalKernel_additive_expression_token = 26,
    evalKernel_multiplicative_expression_token = 31,
    evalKernel_unary_expression_token = 34, evalKernel_factor_token = 37,
    evalKernel_primary_token, evalKernel_arguments_token = 43,
    evalKernel_argument_list_token, evalKernel_simple_real_token = 57,
    evalKernel_exponent_token = 60, evalKernel_integer_part_token,
    evalKernel_fraction_part_token = 63, evalKernel_digit_token = 65,
    evalKernel_letter_token, evalKernel_name_token = 75,
    evalKernel_real_token = 95
} evalKernel_token_type;


typedef struct {
    evalKernel_token_type token_number, reduction_token, error_frame_token;
    int input_code;
    int input_value;
    int line, column;
    int ssx, sn, error_frame_ssx;
    int drt, dssx, dsn;
    int ss[128];
    evalKernel_vs_type vs[128];
    int ag_ap;
    char *error_message;
    char read_flag;
    char exit_flag;
    int bts[128], btsx;
    unsigned char * pointer;
    unsigned char * la_ptr;
    const unsigned char *key_sp;
    int save_index, key_state;
    char ag_msg[82];
}
evalKernel_pcb_type;
/**********************************************************************

Function prototypes

**********************************************************************/

void    pushChar(int character,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb,CharStack& cs);
void    pushArg(double value,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb,CharStack& cs,ArgStack& as);
double  checkZero(double value,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb);
double *locateVariable(int nameLength,VariableDescriptor* variable,int& nVariables,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb,CharStack& cs,ArgStack& as);
double  callFunction(int nameLength, int argCount,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb,CharStack& cs,ArgStack& as);
void    diagnoseError(char *message,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb);
int     evaluateExpression(char *expressionString,VariableDescriptor* variable, int& nVariables,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb,CharStack& cs,ArgStack& as);

/**********************************************************************

Global data

**********************************************************************/

/* Support for error diagnostics */

#endif
