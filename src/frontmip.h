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

/**********************************************************************/
/* Frontline Systems Solver DLL (Dynamic Link Library) Version 3.0    */
/* Copyright 1995-98 Frontline Systems, Inc.  All Rights Reserved.    */
/* Version 3.0 includes definitions for Nonlinear Solver engines.     */
/* Include this header file in your C or C++ program for MS Windows.  */
/**********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __WIN32__    // changed this
#define _FAR_
#define _HUGE_
#define _CC  __stdcall
#else /* WIN16 */
#define _FAR_  _far
#define _HUGE_ _huge
#define _CC  __export __far __pascal
#endif

    typedef long   INTARG;  /* same in Win16 and Win32 */
    typedef double REALARG; /* same on all platforms...*/
    typedef unsigned char _FAR_ * LPBYTEARG;
    typedef INTARG  _FAR_ * LPINTARG;
    typedef REALARG _FAR_ * LPREALARG;

    /* Define huge pointers for int and REALARG types.  HPREALARG
       is used for matval[], and HPINTARG is used for matind[] to allow
       for more than 32768 nonzero coefficients, in 16-bit Windows. */
    typedef INTARG  _HUGE_ * HPINTARG;
    typedef REALARG _HUGE_ * HPREALARG;

#ifndef LPSTR
#define LPSTR char _FAR_ *
#endif
#ifndef NULL
#define NULL 0
#endif

    /* Define the SAFEARRAY type for arrays passed from/to Visual Basic */

// changed this
//#ifndef _WINDOWS_//_Windows// __WIN32__//  _WINDOWS_
    typedef struct  tagSAFEARRAYBOUND {
        unsigned long cElements;
        long lLbound;
    }
    SAFEARRAYBOUND;
    typedef struct  tagSAFEARRAY {
        unsigned short cDims;
        unsigned short fFeatures;
        unsigned long cbElements;
        unsigned long cLocks;
        void _FAR_ * pvData;
        SAFEARRAYBOUND rgsabound[ 1 ];
    }
    SAFEARRAY;
    typedef SAFEARRAY _FAR_ * LPSAFEARRAY;
// changed this
//#endif

    typedef LPSAFEARRAY _FAR_ * PARAM_SAFEARRAY; /* VB passes this */

    /* Define the value which represents "plus infinity" for bounds */
#define INFBOUND 1E30

    /* Define problem type codes - linear, quadratic, nonlinear, integer */
    typedef enum {
        PROB_LP     =  0,
        PROB_MIP    =  1,
        PROB_QP     =  4,
        PROB_QPMIP  =  8,
        PROB_NLP    =  12,
        PROB_NLPMIP =  16
    } PROBTYPE;

    /* Define return codes for the callback functions */
    typedef enum {
        PSTAT_CONTINUE        = 0,  /* Used in callback to continue solving     */
        PSTAT_OPTIMAL         = 1,  /* An optimal solution has been found       */
        PSTAT_INFEASIBLE      = 2,  /* No feasible solution could be found      */
        PSTAT_UNBOUNDED       = 3,  /* The objective value is unbounded         */
        PSTAT_IT_LIM_FEAS     = 5,  /* Iteration limit exceeded, feasible sol   */
        PSTAT_IT_LIM_INFEAS   = 6,  /* Iteration limit exceeded, no feasible sol*/
        PSTAT_TIME_LIM_FEAS   = 7,  /* Time limit exceeded, feasible solution   */
        PSTAT_TIME_LIM_INFEAS = 8,  /* Time limit exceeded, no feasible solution*/
        PSTAT_ABORT_FEAS      = 12, /* User aborted solve, feasible solution    */
        PSTAT_ABORT_INFEAS    = 13, /* User aborted solve, no feasible solution */

        PSTAT_FRACT_CHANGE    = 65, /* Objective function changing too slowly   */
        PSTAT_NO_REMEDIES     = 66, /* All remedies failed to find better point */
        PSTAT_FLOAT_ERROR     = 67, /* Error when evaluating problem functions  */
        PSTAT_MEM_LIM         = 68, /* Could not allocate enough main memory    */
        PSTAT_ENTRY_ERROR     = 69, /* Attempt to use DLL from 2 or more threads*/

        PSTAT_MIP_OPTIMAL         = 101,/* Optimal integer solution found       */
        PSTAT_MIP_OPTIMAL_TOL     = 102,/* Integer solution found within epgap  */
        PSTAT_MIP_INFEASIBLE      = 103,/* No feasible integer solution         */
        PSTAT_MIP_SOL_LIM         = 104,/* Limit of integer solutions exceeded  */
        PSTAT_MIP_NODE_LIM_FEAS   = 105,/* Node limit exceeded, integer solution*/
        PSTAT_MIP_NODE_LIM_INFEAS = 106,/* Node limit exceeded, no int solution */
        PSTAT_MIP_TIME_LIM_FEAS   = 107,/* Time limit exceeded, integer solution*/
        PSTAT_MIP_TIME_LIM_INFEAS = 108 /* Time limit exceeded, no int solution */
    } PSTAT;

#define PSTAT_USER_ABORT  PSTAT_ABORT_FEAS  /* may be returned by callback */

    /* Define symbolic names for optimization parameters */
    typedef enum {
        PARAM_ARGCK      =  990, /* on argument errors: 1-MsgBox, 0-retval only*/
        PARAM_ARRAY      =  995, /* arrays are: 0-C-style arrays, 1-SAFEARRAYs */
        PARAM_IISBND     =  999, /* IIS finder: 0-includes bnds, 1-omits bounds*/
        PARAM_ITLIM      = 1020, /* limit on LP iterations - per (sub)problem  */
        PARAM_NDLIM      = 2017, /* limit on Branch & Bound nodes explored     */
        PARAM_MIPLIM     = 2015, /* limit on Branch & Bound IP solutions found */
        PARAM_SCAIND     = 1033, /* scaling: -1-none, 0-normal, 1-aggressive   */
        PARAM_CRAIND     = 1007, /* crashing: 0-none, 1-crash initial basis    */
        PARAM_RELAX      = 2501, /* solve relaxation: 1-on, 0-use int vars     */
        PARAM_PREPRO     = 2502, /* B&B Preprocessing and Probing: 1-on, 0-off */
        PARAM_OPTFIX     = 2503, /* B&B Optimality Fixing:  1-on, 0-off        */
        PARAM_REORDR     = 2504, /* B&B Branch Variable Reordering: 1-on, 0-off*/
        PARAM_IMPBND     = 2505, /* B&B Bounds Improvement: 1-on, 0-off        */

        PARAM_TILIM      = 1038, /* global solution time limit  */
        PARAM_EPOPT      = 1014, /* LP optimality tolerance     */
        PARAM_EPPIV      = 1091, /* LP pivot tolerance          */
        PARAM_EPSOL      = 1092, /* LSLP solution tolerance     */
        PARAM_EPRHS      = 1016, /* LP feasibility tolerance    */
        PARAM_EPGAP      = 2009, /* integer tolerance/MIP gap   */
        PARAM_CUTLO      = 2006, /* known incumbent for max MIP */
        PARAM_CUTHI      = 2007, /* known incumbent for min MIP */

        PARAM_EPNEWT     = 5001, /* NLP constraint tolerance    */
        PARAM_EPCONV     = 5002, /* NLP slow change stopping tol*/
        PARAM_LINVAR     = 5010, /* NLP recognize linear vars   */
        PARAM_DERIV      = 5011, /* NLP derivative computation  */
        PARAM_ESTIM      = 5012, /* NLP estimates for basic vars*/
        PARAM_DIREC      = 5013  /* NLP search direction option */
    } PARAM;

    /* Define "wherefrom" codes for callback functions  */
    typedef enum {
        CALLBACK_PRIMAL  = 1,      /* callback on LP/NLP pivot  */
        CALLBACK_MIP     = 101     /* callback on MIP branch    */
    } CALLBACK_CODE;

    /* Define info request codes for callback functions */
    typedef enum {
        CBINFO_PRIMAL_OBJ     = 1, /* callback: objective       */
        CBINFO_PRIMAL_INFMEAS = 3, /* callback: infeasibility   */
        CBINFO_PRIMAL_FEAS    = 5, /* callback: whether feasible*/
        CBINFO_ITCOUNT        = 7, /* callback: iteration count */
        CBINFO_BEST_INTEGER   = 8, /* callback: MIP incumbent   */
        CBINFO_NODE_COUNT     = 10,/* callback: nodes explored  */
        CBINFO_MIP_ITERATIONS = 12 /* callback: MIP iterations  */
    } CBINFO;

    /* The problem structure:  loadlp() loads its arguments into a PROBLEM
       structure and returns an HPROBLEM pointer to this data structure.
       loadctype(), if called, allocates and copies the ctype member.  The
       structure itself is statically allocated, since at present the LSLP
       Solver can work with only one problem at a time.  unloadprob() just
       clears addresses in the structure members.
    */
    typedef struct PROBLEM {
        struct PROBLEM _FAR_ * lp; /* points to itself, for compatibility */
        LPSTR probname;
        INTARG probtype, numcols, numrows, objsen;
        LPREALARG objx, rhsx;
        LPBYTEARG sense;
        LPINTARG matbeg, matcnt;
        HPINTARG matind;
        HPREALARG matval;
        LPREALARG var, lb, ub, rngval;
        INTARG colspace, rowspace, nzspace;
        LPINTARG qmatbeg, qmatcnt;
        HPINTARG qmatind;
        HPREALARG qmatval;
        INTARG qnzspace;
        LPBYTEARG ctype, objtype, matvaltype;
        REALARG objective, sum_of_infeas, best_integer;
        INTARG iterations, branches, total_iters, integer_soln;
        INTARG stat;
        REALARG objval;
        LPREALARG x, piout, slack, dj;
        struct {
            INTARG begin, end;
            LPREALARG lower, upper;
        }
        obj;
        struct {
            INTARG begin, end;
            LPREALARG lower, upper;
        }
        rhs;
        struct {
            INTARG numrows, numcols, stat, write;
            LPINTARG rowind, rowbdstat, colind, colbdstat;
        }
        iis;
        struct {
            PARAM_SAFEARRAY objx, matbeg, matcnt, matind, matval;
            PARAM_SAFEARRAY objtype, matvaltype;
        }
        sa;
        INTARG (_CC * funceval)();
        INTARG (_CC * jacobian)();
    }
    PROBLEM;
    typedef struct PROBLEM _FAR_ * HPROBLEM;


    /* Define the procedure pointer type for callback functions */
    typedef INTARG (_CC * _CCPROC) (HPROBLEM lpinfo, INTARG wherefrom);

    /* Define the callbacks to compute function values, and the objective
       value, objective gradient and Jacobian matrix of partial derivatives */
    typedef INTARG (_CC * _FUNCEVAL) (HPROBLEM lp, INTARG numcols,
                                      INTARG numrows, LPREALARG objval, LPREALARG lhs, LPREALARG var,
                                      INTARG varone, INTARG vartwo);

    typedef INTARG (_CC * _JACOBIAN) (HPROBLEM lp, INTARG numcols,
                                      INTARG numrows, INTARG nzspace, LPREALARG objval, LPREALARG obj,
                                      LPINTARG matbeg, LPINTARG matcnt, HPINTARG matind, HPREALARG matval,
                                      LPREALARG var, LPBYTEARG objtype, LPBYTEARG matvaltype);

    /* These callback typedefs are useful for Visual Basic.
       They pass OLE SAFEARRAYs rather than C-style arrays */
    typedef INTARG (_CC * _SAFUNCEVAL) (HPROBLEM lp, INTARG numcols,
                                        INTARG numrows, LPREALARG objval, LPSAFEARRAY lhs, LPSAFEARRAY var,
                                        INTARG varone, INTARG vartwo);

    typedef INTARG (_CC * _SAJACOBIAN) (HPROBLEM lp, INTARG numcols,
                                        INTARG numrows, INTARG nzspace, LPREALARG objval, LPSAFEARRAY obj,
                                        LPSAFEARRAY matbeg, LPSAFEARRAY matcnt, LPSAFEARRAY matind, LPSAFEARRAY matval,
                                        LPSAFEARRAY var, LPSAFEARRAY objtype, LPSAFEARRAY matvaltype);


    /* Prototypes for the DLL's callable entry points... */


    /* Define the supported problem type/size entry point */
    INTARG _CC getproblimits (INTARG type,
                              LPINTARG numcols_p, LPINTARG numrows_p, LPINTARG numints_p);

    /* Define the entry points for the Linear & Quadratic Solver */
    HPROBLEM _CC loadlp (LPSTR probname,
                         INTARG numcols, INTARG numrows, INTARG objsen,
                         LPREALARG obj, LPREALARG rhs, LPBYTEARG sense,
                         LPINTARG matbeg, LPINTARG matcnt, HPINTARG matind, HPREALARG matval,
                         LPREALARG lb, LPREALARG ub, LPREALARG rngval,
                         INTARG colspace, INTARG rowspace, INTARG nzspace);

    INTARG _CC loadquad (HPROBLEM lp, LPINTARG qmatbeg, LPINTARG qmatcnt,
                         HPINTARG qmatind, HPREALARG qmatval, INTARG qnzspace, LPREALARG var);

    INTARG _CC loadctype (HPROBLEM lp, LPBYTEARG ctype); /* MIP problems */

    /* Define the entry points for the Nonlinear Solver */
    HPROBLEM _CC loadnlp (LPSTR probname,
                          INTARG numcols, INTARG numrows, INTARG objsen,
                          LPREALARG obj, LPREALARG rhs, LPBYTEARG sense,
                          LPINTARG matbeg, LPINTARG matcnt, HPINTARG matind, HPREALARG matval,
                          LPREALARG var, LPREALARG lb, LPREALARG ub, LPREALARG rngval,
                          INTARG nzspace, _FUNCEVAL funceval, _JACOBIAN jacobian);

    INTARG _CC loadnltype (HPROBLEM lp, LPBYTEARG objtype, LPBYTEARG matvaltype);

    INTARG _CC testnltype (HPROBLEM lp, INTARG numtests, LPREALARG testvals,
                           LPINTARG pstat, LPBYTEARG objtype, LPBYTEARG matvaltype);

    /* Define the optimize and solution routines */
    INTARG _CC optimize (HPROBLEM lp);

    INTARG _CC mipoptimize (HPROBLEM lp);

    INTARG _CC solution (HPROBLEM lp, LPINTARG pstat, LPREALARG pobj,
                         LPREALARG x, LPREALARG piout, LPREALARG slack, LPREALARG dj);

    INTARG _CC objsa (HPROBLEM lp, INTARG begidx, INTARG endidx,
                      LPREALARG lower, LPREALARG upper);

    INTARG _CC rhssa (HPROBLEM lp, INTARG begidx, INTARG endidx,
                      LPREALARG lower, LPREALARG upper);

    /* Define the IIS (Irreducibly Infeasible Set) finding routines */
    INTARG _CC findiis (HPROBLEM lp, LPINTARG iisnumrows_p, LPINTARG iisnumcols_p);

    INTARG _CC getiis (HPROBLEM lp, LPINTARG iisstat_p,
                       LPINTARG rowind, LPINTARG rowbdstat, LPINTARG iisnumrows_p,
                       LPINTARG colind, LPINTARG colbdstat, LPINTARG iisnumcols_p);

    /* unloadprob MUST be called to free memory...NOTE argument type change! */
    INTARG _CC unloadprob (HPROBLEM _FAR_ *lp_p);

    /* routines to set and get parameter values...new versions */

    INTARG _CC infointparam (INTARG whichparam,
                             LPINTARG defvalue_p, LPINTARG minvalue_p, LPINTARG maxvalue_p);

    INTARG _CC setintparam (INTARG whichparam, INTARG newvalue);

    INTARG _CC getintparam (INTARG whichparam, LPINTARG value_p);

    INTARG _CC infodblparam (INTARG whichparam,
                             LPREALARG defvalue_p, LPREALARG minvalue_p, LPREALARG maxvalue_p);

    INTARG _CC setdblparam (INTARG whichparam, REALARG newvalue);

    INTARG _CC getdblparam (INTARG whichparam, LPREALARG value_p);

    INTARG _CC setdefaults (void);

    /* compatibility routines for integer tolerance ("mip gap") */

    INTARG _CC setepgap (REALARG newvalue,
                         REALARG _FAR_ *ptoosmall, REALARG _FAR_ *ptoobig);

    void _CC getepgap (REALARG _FAR_ *pepgap);

    /* compatibility routines for reduced cost or "optimality" tolerance */

    INTARG _CC setepopt (REALARG newvalue,
                         REALARG _FAR_ *ptoosmall, REALARG _FAR_ *ptoobig);

    void _CC getepopt (REALARG _FAR_ *pepopt);

    /* routines to define and use the LP and MIP callback functions */

    INTARG _CC setlpcallbackfunc (_CCPROC callback);

    void _CC getlpcallbackfunc (_CCPROC *callback_p);

    INTARG _CC setmipcallbackfunc (_CCPROC callback);

    void _CC getmipcallbackfunc (_CCPROC *callback_p);

    INTARG _CC getcallbackinfo (HPROBLEM lpinfo, INTARG wherefrom,
                                INTARG infonumber, void _FAR_ *result_p);

    /* routines to write a file summarizing LP/QP problem in algebraic form */

    INTARG _CC lprewrite (HPROBLEM lp, LPSTR filename);

    INTARG _CC iiswrite (HPROBLEM lp, LPSTR filename);

#ifdef __cplusplus
};
#endif



