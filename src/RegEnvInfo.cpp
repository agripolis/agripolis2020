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

//---------------------------------------------------------------------------
#include <fstream>

#include "RegEnvInfo.h"
#include "RegPlot.h"

#include "textinput.h"

RegEnvInfo::RegEnvInfo(const RegEnvInfo& rh,RegGlobalsInfo* G) :g(G) {
    *this=rh;
    g=G;
}

RegEnvInfo::RegEnvInfo(RegGlobalsInfo* G):g(G) {
    iteration=0;
}

void
RegEnvInfo::initEnv() {
  if (g->ENV_MODELING) {
        nHabitats = envdata.biohabs.size();
        zCoef = envdata.zcoef;
        speciesByHabitat.resize(nHabitats);
        mktIDsByHabitat.resize(nHabitats);
        haByHabitat.resize(nHabitats);
        cCoeffByHabitat.resize(nHabitats);

        {int i=0;
        for (vector<biohab>::iterator it = envdata.biohabs.begin();
                it != envdata.biohabs.end(); it++) {
            habitatLabels.push_back((*it).name);
            speciesByHabitat[i++]= (*it).value;
        }
        }

        groups=envdata.numGroups;
        for (int i=0;i<groups;i++) {
            vector<int> acts;
            vector<string> sacts;
            vector<int> soils;
            vector<double> q;

            no_soils.push_back(envdata.groups[i].numSoils);
            for (int j=0;j<no_soils[i];j++) {
                string sn = envdata.groups[i].soilNames[j];
                int k=0;
                while (sn.compare(globdata.namesOfSoilTypes[k])!=0)
                    k++;

                soils.push_back(k);
            }

            no_acts.push_back(envdata.groups[i].acts.size());

            {vector < act > :: iterator it= envdata.groups[i].acts.begin();
            for (int j=0 ;j<no_acts[i];j++) {
                sacts.push_back((*it).name);
                it++;
            }}

            {vector <act> :: iterator it= envdata.groups[i].acts.begin() ;
            for (int j=0;j<no_acts[i];j++) {
                q.push_back((*it).q);
                it++;
            }}

            associated_soils.push_back(soils);
            associated_q.push_back(q);
            sassociated_activities.push_back(sacts);

        }
    }
}//

void RegEnvInfo::initEnvOutput(RegMarketInfo* market) {
    for (int i=0;i<groups;i++) {
        ofstream out;
        stringstream file;
        file<<g->OUTPUTFILE<<"landscape_statistics_of_group"<<i<<".dat";
        out.open(file.str().c_str(),ios::out|ios::trunc);
        out << "iteration" << "\t";
        for (int j=0;j<no_acts[i];j++) {
            out << market->getNameOfProduct(associated_activities[i][j]).c_str() << "\t" << "" << "\t";
        }
        out << "RESIDUAL" << "\t" << "" << "\t";
        out <<"\n" << "" << "\t";
        for (int j=0;j<no_acts[i]+1;j++) {
            out << "mean" << "\t" << "std" << "\t" << "count" << "\t";
        }
        out << "\n";
        out.close();
    }
}
vector < vector< double> >
RegEnvInfo::calculateProd(int group, RegFarmInfo* farm) {
    int no_cont_plots=0;
    for (unsigned int j=0;j<associated_soils[group].size();j++) {
        no_cont_plots+=farm->contiguous_plots[associated_soils[group][j]].size();
    }
    if (no_cont_plots>0) {
        int ass_acts=associated_activities[group].size();
        int max_plots=0;
        int red_plots=0;
        if (no_cont_plots*ass_acts > 2000) {
            max_plots=2000/ass_acts;
            red_plots=no_cont_plots-max_plots;
            no_cont_plots=max_plots;
        }
        int temp_red_plots=red_plots;

        int cols=no_cont_plots*ass_acts;
        int rows=no_cont_plots+ass_acts;
        double* obj=new double[cols];//[] = { 0,0,0,0,0,0,0,0,0 };
        double* rhs = new double[rows];//{ 35,25,50,50,40,20 };
        int counter=0;
        double max_act=farm->getUnitsOfProduct(associated_activities[group][0]);
        for (unsigned int i=0;i<associated_activities[group].size();i++) {
            rhs[counter]=farm->getUnitsOfProduct(associated_activities[group][i]);
            if (rhs[counter]>max_act) {
                max_act=rhs[counter];
            }
            counter++;
        }
        for (int j=0;j<no_soils[group];j++) {
            for (unsigned int i=0;i<farm->contiguous_plots[associated_soils[group][j]].size();i++) {
                double pl=farm->contiguous_plots[associated_soils[group][j]][i];
                if (temp_red_plots==0) {
                    rhs[counter]=pl;
                    counter++;
                } else {
                    if (pl>g->PLOT_SIZE) {
                        rhs[counter]=pl;
                        counter++;
                    } else {
                        temp_red_plots--;
                    }
                }
            }
        }
        char* sense= new char[rows];
        for (int i=0;i<rows;i++) {
            sense[i]='E';
        }
        long* matbeg= new long[cols];//{ 0, 2, 4, 6, 8, 10, 12, 14,16  };
        long* matcnt= new long[cols];//[] = { 2, 2, 2, 2, 2,2,2,2,2 };
        double* lb= new double[cols];//[] = { 0,0,0,0,0,0,0,0,0};
        double* ub= new double[cols];//[] = { INFBOUND,INFBOUND,INFBOUND,INFBOUND,INFBOUND,INFBOUND,INFBOUND,INFBOUND,INFBOUND};
        int c=0;
        for (int i=0;i<cols;i++) {
            obj[i]=0;
            matbeg[i]=c;
            c+=2;
            matcnt[i]=2;
            lb[i]=0;
            ub[i]=+1E30;
        }
        long* matind= new long[cols*2];//[] = { 0, 3, 1, 3, 2, 3, 0, 4, 1, 4,2,4,0,5,1,5,2,5 };

        int indc=ass_acts;
        c=0;
        for (int j=0;j<ass_acts;j++) {
            for (int i=0;i<no_cont_plots;i++) {
                matind[c]=j;
                c++;
                matind[c]=indc+i;
                c++;
            }
        }
        double* matval= new double[cols*2];//[] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
        for (int i=0;i<cols*2;i++) {
            matval[i]=1;
        }

        long* qmatbeg= new long[cols];//[] = { 0, 1 ,2,3,4,5,6,7,8};
        long* qmatcnt= new long[cols];//[] = { 1,1,1,1,1,1,1,1,1};
        long* qmatind= new long[cols];//[] = { 0, 1, 2, 3, 4, 5,6,7,8};
        double* qmatval= new double[cols];//[] = {2,2,2,2,2,2,2,2,2};//{1,1,1,1,1,1,1,1,1};//2,2,2,2,2,2,2,2,2};
        double* init= new double[cols];

        for (int i=0;i<cols;i++) {
            qmatbeg[i]=i;
            qmatcnt[i]=1;
            qmatind[i]=i;
            init[i]=max_act;
        }

        counter=0;
        for (int j=0;j<no_acts[group];j++) {
            for (int i=0;i<no_cont_plots;i++) {
                qmatval[counter]=associated_q[group][j];
                counter++;
            }
        }

		//[]={100,100,100,100,100,100,100,100,100};//{20,20,20,20,20,20,20,20,20};
        /* The Q matrix specifies the covariance between each pair of assets */

        //long stat;
        double objval=0;
        double* x =new double[cols];
//      HPROBLEM lp;

        printf("\nExample QP problem\n");

        /* set up the LP portion of the problem. The LP portion
        of the objective is all 0's here; it could be elaborated
        to include transaction costs or other factors. */
        /*        setintparam (lp, PARAM_ARGCK, 0);

                lp = loadlp ("", cols, rows, -1, obj, rhs, sense,
                             matbeg, matcnt, matind, matval, lb, ub, NULL, cols, rows, cols*2);
                long pnc=0;
                long pnr=0;
                long pni=0;
                INTARG type;
         */
        /* now set up the Q matrix to define the quadratic objective */

//        int e=loadquad (lp, qmatbeg, qmatcnt, qmatind, qmatval, cols,init);

        /* solve the problem; obtain and display the solution */
//        int erge=getproblimits(NULL,0,&pnr,&pnc,&pni);
//        e=optimize (lp);
//        e=solution (lp, &stat, &objval, x, NULL, NULL, NULL);
        /* once more: don't forget to free memory */
//        unloadprob (&lp);
        bool debug=true;
        vector<vector<double> > res;
        temp_red_plots=red_plots;
        counter=0;
        for (int j=0;j<no_soils[group];j++) {
            for (unsigned int i=0;i<farm->contiguous_plots[associated_soils[group][j]].size();i++) {
                double size=farm->contiguous_plots[associated_soils[group][j]][i];
                if (temp_red_plots>0 && size==g->PLOT_SIZE) {
                    temp_red_plots--;
                } else {
                    vector <double> tmp;
                    double sum=0;
                    for (int k=0;k<ass_acts;k++) {
                        tmp.push_back(x[k*no_cont_plots+counter]);
                        sum+=x[k*no_cont_plots+counter];
                    }
                    counter++;
                    tmp.push_back(size-sum);
                    res.push_back(tmp);
                }
            }
        }
        // Calculate remaining activities
        vector<double> rem_act;
        for (int i=0;i<ass_acts;i++) {
            double res_s=0;
            for (unsigned int j=0;j<res.size();j++) {
                res_s+=res[j][i];
            }
            double res_diff=farm->getUnitsOfProduct(associated_activities[group][i])-res_s;
            rem_act.push_back(res_diff);
        }
        for (int j=0;j<red_plots;j++) {
            vector<double> tmp;
            for (int i=0;i<ass_acts+1;i++) {
                tmp.push_back(0);
            }
            for (int k=0;k<ass_acts;k++) {
                if (rem_act[k]>0) {
                    rem_act[k]-=g->PLOT_SIZE;
                    tmp[k]=g->PLOT_SIZE;
                    break;
                }
            }
            res.push_back(tmp);
        }

        if (debug) {
            ofstream out;
            stringstream file;
            file<<g->OUTPUTFILE<<"associated_production_of_type"<<group<<".dat";
            out.open(file.str().c_str(),ios::out|ios::app);
            out << iteration << "\t" << farm->getFarmId() <<"\t" << "" << "\t";
            for (int i=0;i<ass_acts;i++) {
                out  <<farm->getUnitsOfProduct(associated_activities[group][i]) <<"\t";
            }
            out << objval << "\n";
            counter=0;
            temp_red_plots=red_plots;
            for (int j=0;j<no_soils[group];j++) {
                for (unsigned int i=0;i<farm->contiguous_plots[associated_soils[group][j]].size();i++) {
                    double pl=farm->contiguous_plots[associated_soils[group][j]][i];
                    if (temp_red_plots>0 && pl==g->PLOT_SIZE) {
                        temp_red_plots--;
                    } else {
                        out << iteration << "\t" << farm->getFarmId() << "\t" << pl << "\t";
                        for (int k=0;k<ass_acts;k++) {
                            out << x[k*no_cont_plots+counter] << "\t";
                        }
                        counter++;
                        out << "\n";
                    }
                }
            }

            vector<double> rem_act2;
            for (int i=0;i<ass_acts;i++) {
                double res_s=0;
                for (unsigned int j=0;j<res.size()-red_plots;j++) {
                    res_s+=res[j][i];
                }
                double res_diff=farm->getUnitsOfProduct(associated_activities[group][i])-res_s;
                rem_act2.push_back(res_diff);
            }
            for (int j=0;j<red_plots;j++) {
                vector<double> tmp;
                for (int i=0;i<ass_acts+1;i++) {
                    tmp.push_back(0);
                }
                for (int k=0;k<ass_acts;k++) {
                    if (rem_act2[k]>0) {
                        rem_act2[k]-=g->PLOT_SIZE;
                        tmp[k]=g->PLOT_SIZE;
                        break;
                    }
                }
                out << iteration << "\t" << farm->getFarmId() << "\t" << g->PLOT_SIZE << "\t";
                for (int l=0;l<ass_acts;l++) {
                    out << tmp[l] << "\t";
                }
                out << "\n";
            }
            out.close();
        }


        delete[] x;
        delete[] obj;
        delete[] rhs;
        delete[] sense;
        delete[] matbeg;
        delete[] matcnt;
        delete[] matind;
        delete[] matval;
        delete[] lb;
        delete[] ub;
        delete[] qmatbeg;
        delete[] qmatcnt;
        delete[] qmatind;
        delete[] init;
        delete[] qmatval;
        return res;
    }
    vector< vector < double > > e;
    return e;
}


void RegEnvInfo::associateActivities(list<RegFarmInfo*>& farms) {
    results.clear();
    list<RegFarmInfo* >::iterator farms_iter;
    for (int i=0;i<groups;i++) {
        vector< vector< vector < double > > > tmp;
        for (farms_iter = farms.begin();
                farms_iter != farms.end();
                farms_iter++) {
            tmp.push_back(calculateProd(i,(*farms_iter)));
        }
        results.push_back(tmp);
    }
    calculateStatistics();
    av_size_of_contiguous_plots.clear();
    total_land_of_type.clear();
    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        double sum=0;
        double count=0;
        for (farms_iter = farms.begin();
                farms_iter != farms.end();
                farms_iter++) {

            for (unsigned int j=0;j<(*farms_iter)->contiguous_plots[i].size();j++) {
                sum+=(*farms_iter)->contiguous_plots[i][j];
                count+=1;
            }
        }
        total_land_of_type.push_back(sum);
        av_size_of_contiguous_plots.push_back(sum/count);

    }
    print();
}

void RegEnvInfo::calculateStatistics() {
    mean.clear();
    std.clear();
    count.clear();
    for (int i=0;i<groups;i++) {
        // +1 for residual value
        int ass_acts=associated_activities[i].size()+1;
        vector <double> m;
        vector <double> s;
        vector <int> c;
        for (int j=0;j<ass_acts;j++) {
            m.push_back(0);
            s.push_back(0);
            c.push_back(0);
        }
        int nfarms=results[i].size();
        for (int j=0;j<nfarms;j++) {
            int nplots=results[i][j].size();
            for (int k=0;k<nplots;k++) {
                for (int l=0;l<ass_acts;l++) {
                    if (results[i][j][k][l]>0.4) {
                        c[l]++;
                        m[l]+=results[i][j][k][l];
                    }
                }
            }
        }
        for (int j=0;j<ass_acts;j++) {
            if (c[j]!=0)
                m[j]/=c[j];
            else
                m[j]=0;
        }
        for (int j=0;j<nfarms;j++) {
            int nplots=results[i][j].size();
            for (int k=0;k<nplots;k++) {
                for (int l=0;l<ass_acts;l++) {
                    if (results[i][j][k][l]>0.4) {
                        s[l]+=(results[i][j][k][l]-m[l])*(results[i][j][k][l]-m[l]);
                    }
                }
            }
        }
        for (int j=0;j<ass_acts;j++) {
            if (c[j]!=0)
                s[j]/=c[j];
            else
                s[j]=0;
        }
        mean.push_back(m);
        std.push_back(s);
        count.push_back(c);
    }
}

void RegEnvInfo::print() {
    for (int i=0;i<groups;i++) {
        ofstream out;
        stringstream file;
        file<<g->OUTPUTFILE<<"landscape_statistics_of_group"<<i<<".dat";
        out.open(file.str().c_str(),ios::out|ios::app);
        out << iteration << "\t";
        for (unsigned int j=0;j<associated_activities[i].size()+1;j++) {
            out << mean[i][j] << "\t" << std[i][j] << "\t" << count[i][j] << "\t";
        }
        out <<"\n";
        out.close();
    }
    iteration++;
}

void
RegEnvInfo::addMktIDToHabitat(int mktID_h, int habitatID_h) {
    if (habitatID_h>=0) {
        vector <int> temp =mktIDsByHabitat[habitatID_h];
        temp.push_back(mktID_h);
        mktIDsByHabitat[habitatID_h]=temp;
    }
};

void
RegEnvInfo::sumHaProducedByHabitat(int mktID_h, double producedHa_h) {
    for (int i=0;i<nHabitats;i++) {
        for (unsigned int k=0; k<mktIDsByHabitat[i].size(); k++) {
            if (mktIDsByHabitat[i][k]== mktID_h) {
                haByHabitat[i] += producedHa_h;
                return;
            }
        }
    }
};

void
RegEnvInfo::resetHaProducedByHabitat() {
    for (int i=0; i<nHabitats; i++)
        haByHabitat[i]=0;
};

void
RegEnvInfo::calculateCCoefficients() {
    for (int i=0; i<nHabitats; i++) {
        double t=pow(haByHabitat[i],zCoef);
        if (t==0)
            cCoeffByHabitat[i]=0;
        else
            cCoeffByHabitat[i]=speciesByHabitat[i]/t;
    }
};

void
RegEnvInfo::calculateSpeciesByHabitat() {
    for (int i=0; i<nHabitats; i++)
        speciesByHabitat[i]=cCoeffByHabitat[i]*pow(haByHabitat[i],zCoef);
};
void
RegEnvInfo::backup() {
    obj_backup=new RegEnvInfo(*this);
}
void
RegEnvInfo::restore() {
    RegEnvInfo* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
}
