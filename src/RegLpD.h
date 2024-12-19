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

//---------------------------------------------------------------------------
// RegLP.h
//---------------------------------------------------------------------------
//	RegLP -- LP class
//---------------------------------------------------------------------------
#ifndef RegLPH
#define RegLPH
//---------------------------------------------------------------------------
#include "solverwahl.h"

#include <vector>
#include <stdlib.h>
#include <fstream>
#include "RegGlobals.h"
#include "RegProduct.h"
#include "RegStructure.h"
#include "RegInvest.h"
#include "RegLink.h"

#include "textinput.h"

//solver routines
#ifndef GNU_SOLVER
    #define LPCWSTR const wchar_t*
    #include "Frontkey.h"
    #include "MipBuild.h"

    #define FRONTMIPISINSTALLED
#endif

//#ifndef FRONTMIPISINSTALLED
#ifdef GNU_SOLVER
extern "C" {
    #define INFBOUND 1E30
    #include "glpk.h"
}
#endif

#include "RegLabour.h"

//---------------------------------------------------------------------------

/** RegLpInfo class.
    @short class that defines the Lp routine and all functions to manage
    the Lp matrix, constraints, etc.
    @author Kathrin Happe, Alfons Balmann, Konrad Kellermann
    @version June 2001
*/

class RegLinkInvestObject;
class RegLinkMarketObject;

class RegRegionInfo;
class RegProductList;

class RegLinkYieldObject;

class RegLinkReferenceObject;
class RegLinkNumberObject;
class RegLinkLandObject;
class RegLinkObject;
class RegFarmInfo;

class RegLpInfo {
protected:
	RegFarmInfo* farm;
	//set ubs of investments to 0
	void resetInvsUbs();
	void resetRestrictedInvsUbs(set<string>);

	//young farmer
	void updateBoundsYoungFarmer();

    bool flat_copy;
    /// pointer to globals
    RegGlobalsInfo* g;
    // non-zero coefficient value, size nzspace
    vector<double> mat_val;
    // status of the solution
    long     stat;
    // objective function value
    double   objval;
    // -1: minimisation; 1: maximisation
    long     objsen;
    // number of non-zero coefficients
    int      nzspace;


    /** number of rows in constraint matrix, not
        including the objective function or bounds on variables
    */
    long    numrows;
    // number of cols in constraint matrix
    long    numcols;
    // product columns
    long    prodcols;

    // capacity values (created dynamically on basis of input file)
    vector<double> rhs;
    // array with objective function coefficients
    vector<double> obj;
    // array containing the sense of each constraint
    vector<char> sense;
    // array containing the lower bound on each variable
    vector<double> lb;
    // array containing the upper bounds each variable
    vector<double> ub;
    // array that contains the optimal values of the primal variables
    vector<double> x;
    // array indicating the type of variables in the IP problems.
    vector<char> ctype;
    // INITIAL LINK LISTS
    // they are necessary for initialisation, but not thereafter

    /** points to investment link objects which affect investment related
        values in the MIP (investments, capacities)
    */
    vector<RegLinkInvestObject*> invest_links;
    /// points to market link objects which affect market values in MIP
    vector<RegLinkMarketObject*> market_links;
    /** reference links are all links which refer to a specific farm
        variable. These are
    */
    vector<RegLinkReferenceObject*> reference_links;
  
    vector<RegLinkNumberObject*> number_links;
    vector<RegLinkLandObject *> land_links;

    // Lists containing the destinations of values in the MIP
    vector<RegLinkObject *> mat_links;
    vector<RegLinkObject *> cap_links;
    vector<RegLinkObject *> obj_links;
    //
    vector<RegLinkObject *> incomepay_links;
    /** method that connects sources to destinations in the MIP
        values are retrieved from the input file and written at
        the appropriate position in the MIP
        @param dn destination
        @param dk destination kind
   */
    RegLinkObject* readLink(int dn, int dk);

	//soil service yield links
	vector<RegLinkYieldObject *> yield_links;


    //standardwerte für die Links
    void stdMatLink() ;
    void stdObjLink() ;

    //gemeinsames 
    RegLinkObject* mklink(onelink& lk, int dn, int dk);

    RegLpInfo* obj_backup;
public:
    /// Modify right hand side
        string printVar(double val, int no);
    void backup();
    void restore();
    /// Change sense
    void setSenseLessEqual(int row);
    void setSenseEqual(int row);
    void setSenseGreaterEqual(int row);
    /// Change bound
    void setUBoundZero(int col);
    void setUBoundInf(int col);
    void setUBound(int col, int val);
	void setLBoundZero(int col);
	void setLBound(int col, int val);

	int getColIndex(string);
	int getRowIndex(string);
	double getValOfIndex(int);

    /** Lp optimisation method
        This method is mostly defined by the Fronmip solver dll.
        @param PList pointer to FarmProductList in RegFarmInfo
        @param reference to inum_vector in RegFarmInfo, which keeps track
                of the number of investments in to one object
        @return objective function value
    */
    virtual double  Lp(RegProductList* PList,vector<int >& ninv, bool prod, int famlabour);

    //#ifndef FRONTMIPISINSTALLED
        /** Lp optimization method using glpk library */
        // double  LpGlpk(RegProductList* PList,vector<int >& ninv, bool prod, int famlabour);

#ifdef GNU_SOLVER //ndef FRONTMIPISINSTALLED
    /** Lp optimization method using glpk library */
    double  LpGlpk(RegProductList* PList,vector<int >& ninv, bool prod, int famlabour);
    void glp_solve();
#endif
    /** \begin{itemize}
            \item read in an assign values to variables
            \item create MIP matrix
            \item read and assign capacity, matrix and objective function links
            \item prepare MIP problem such that it can be passed to solver dll
        \end{itemize}
    */
    void    setupMatrix(RegGlobalsInfo* G);
    /** Because variables affecting the MIP problem change during
        runtime, this method updates these values. The respective
        actual values are retrieved
        \begin{itemize}
            \item invest_links->trigger() updates all values which are affected
                    by changed investment data
            \item market_links->trigger() updates all values affected by changed
                    market data (e.g. prices)
            \item reference_links->trigger() updates all references to specific
                    variables @see market_links
            \item number_links->trigger() always retrieves 0 as a value because
                    the values behind number links are not influenced
                    by changes in other classes or variables
        \end{itemize}
    */
    bool    updateLpValues();
    /// method to debug the MIP problem
    void debug(string);
    void initLinks(RegFarmInfo*,RegInvestList*,RegProductList*,RegLabourInfo*,
                   double*,double*,double*,double*,double*,double*,double*,double*,double*,double*,double*,double*,double*,double*,double*,double*);
    /// to update only the land links
    void updateLand();

	///update yield --soil service
	void updateYield();

    /// to update only the incomepay links
    void updatePaymentEntitlement();
    /// to update only the capacity links
    void updateCapacities();
    /// to update only the matrix links
    void updateMatrix();
    /// to update only the objective function links
    void updateObjectiveFunction();
    double LpWithPrice(RegProductList* PList, vector<int >& ninv, int maxofffarmlu);
    double LpWithPriceExpectation(RegProductList* PList, vector<int >& ninv, int maxofffarmlu);
    double LpProdPriceExpectation(RegProductList* PList, vector<int >& ninv, int maxofffarmlu);
    bool changeMatrix(int nel ,int* indexRow, int* indexCol, double* dels);
    void setCellValue(int c,int r,double val);
    /// Constructor
    double globalAllocation(list<RegFarmInfo*>& farms, RegRegionInfo* region, int iteration);
    void globalAllocationFromFile(list<RegFarmInfo*>& farms, RegRegionInfo* region,string file);
    RegLpInfo();
    virtual RegLpInfo*  clone();

    virtual RegLpInfo*  clone(RegGlobalsInfo*);
    virtual RegLpInfo*  create();
    void setFlatCopy() {
        flat_copy=true;
    }
    /// Destructor
    ~RegLpInfo();
};

#endif
