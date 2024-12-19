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
#ifndef RegLabourH
#define RegLabourH

#include "RegGlobals.h"

/** RegLabourInfo class.
    The class manages the labour capacity on the farm. It is important
    to distinguish between family labour capacity, expressed in hours,
    and fixed and hired labour capacities. Family labour capacity can
    be used either onfarm or offered as fix or var labour
    on the labour market. For offfarm labour it is assumed that contracts
    are for one year only. As for hired labour, this can be fix or
    var, too. Contracts are also assumed to be for one year.

    It is important to further distinguish between family_labour
    (expressed in hours) and labour_capacity. Labour_capacity is the sum
    of family_labour plus all labour substitution originating from
    machinery. Labour_capacity is interpreted as the capacity which
    is initially available on the farm, it is the value which is
    in the capacity constraint of the first MIP in a period. This
    is important, because all further labour movements are determined
    within the MIP.

    @author Kathrin Happe, Alfons Balmann, Konrad Kellermann
    @version June 2001
*/


class RegLabourInfo {
private:
    RegGlobalsInfo* g;
    // fix off farm labour hours
    double fix_offfarm_labour;
    // income of fix off farm labour
    double fix_offfarm_pay;
    // fix on farm labour hours
    double fix_onfarm_labour;
    // income of fix onfarm labour
    double fix_onfarm_pay;
    // var off farm labour hours
    double var_offfarm_labour;
    // income of var off farm labour (only by family labour units)
    double var_offfarm_pay;
    // var on farm labour hours
    double var_onfarm_labour;
    // income of var onfarm labour
    double var_onfarm_pay;
    // family labour hours
    double family_labour;
    // adjusted family labour
    double adjustfam_labour;
    // total input of labour hours on farm
    double labour_input_hours;
    RegLabourInfo* obj_backup;
public:
    void backup();
    void restore();

    /** initially corresponds to family labour + labour substitution.
        If labour substitution is zero, then labour_capacity = family_labour.
        The method is called by RegFarmInfo
    */
    double labour_capacity;
    /** initially family_labour, labour_input_hours and
        labour_capacity = f*MAX_H_LU
        @param f family labour units
    */
    void setInitialFamLu(double f);

    /// make family labour more diverse, take management coefficient
    void adjustFamLu(double f, double coeff);

    /** increase fix_offfarm_labour by fofffl,
        decrease labour_input_hours by fofffl
    */
    void addFixOfffarmLabour(double fofffl);
    double getFixOfffarmLabour();

    /// increase fix_offfarm_pay by fofffp
    void addFixOfffarmPay(double fofffp);
    double getFixOfffarmPay();

    /** increase fix_onfarm_labour by fonfl
        increase labour_input_hours by  fonfl
    */
    void addFixOnfarmLabour(double fonfl);
    double getFixOnfarmLabour();

    /// increase fix onfarm pay by fonfp
    void addFixOnfarmPay(double fonfp);
    double getFixOnfarmPay();

    /** increase var_offfarm_labour by vofffl
        decrease labour_input_hours by vofffl
    */
    void addVarOfffarmLabour(double vofffl);
    double getVarOfffarmLabour();

    /// increase var_offfarm_pay by vofffp
    void addVarOfffarmPay(double vofffp);
    double getVarOfffarmPay();

    /** increase var_onfarm_labour by vonfl
        decrease labour_input_hours by vonfl
    */
    void addVarOnfarmLabour(double vonfl);
    double getVarOnfarmLabour();
    void addVarOnfarmPay(double vonfp);
    double getVarOnfarmPay();
    double getFamilyLabour();
    double getLabourInputHours();
    void setLabourInputHours(double labour_input_hours);
    void setLabourCapacity(double );
    double getLabourCapacity() {
        return labour_capacity;
    }
    void addLabourSubstitution(double ls);

    /** all values except family_labour are set to zero,
        labour_input_hours and labour_capacity are set
        to family_labour
    */
    void setBackLabour();

    /// constructor
    RegLabourInfo(RegGlobalsInfo*);
    RegLabourInfo(RegLabourInfo&,RegGlobalsInfo*);
    /// destructor
    ~RegLabourInfo();
};

//---------------------------------------------------------------------------
#endif
