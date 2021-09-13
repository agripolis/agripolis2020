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

#include <cstdlib>
#include <algorithm>
#include <regex>
//---------------------------------------------------------------------------
#include "OutputControl.h"

//---------------------------------------------------------------------------
#define DATABEGIN 1421
#define LINESIZE 1202                         //1421     2623        1202
#define WIDTH 12
#define TOTALFARMSIZE 5
#define PROFIT 13
#define FARMCLASS 2
#define ERWERBSFORM 97

OutputControl::OutputControl(RegGlobalsInfo* G) :g(G) {
    begin_of_periods.push_back(DATABEGIN);
    op=false;

    policy_input=g->POLICYFILE;
}
OutputControl::OutputControl(const OutputControl& rh,RegGlobalsInfo* G):g(G) {
    op=rh.op;
    cache=rh.cache;
    cacheNumber=rh.cacheNumber;
    number_of_farms=rh.number_of_farms;
    begin_of_periods=rh.begin_of_periods;
    sort_by_profit=rh.sort_by_profit;
    sort_by_farmsize=rh.sort_by_farmsize;
    g=G;
    policy_input=rh.policy_input;

}

OutputControl::~OutputControl() {
    for (unsigned int i=0;i<sort_by_profit.size();i++)
        sort_by_profit[i].clear();
    for (unsigned int i=0;i<sort_by_farmsize.size();i++)
        sort_by_farmsize[i].clear();
    sort_by_profit.clear();
    sort_by_farmsize.clear();
}
void
OutputControl::openFarmdata(string file) {
    cache.clear();
    cacheNumber.clear();
    out.open(file.c_str(), ios::in);

    // check if file exists
    // if(!out) {
    //     Application->MessageBox("The file does not exist", "Error!", MB_OK);
    // }
    out.seekg(0,ios::end);
    long end = static_cast<long>(out.tellg());
    int nof;
    if ((nof=(end-begin_of_periods[begin_of_periods.size()-1])/LINESIZE)>0) {
        number_of_farms.push_back(nof);
        begin_of_periods.push_back(end);
        vector<double> profit=getColOfPeriod(PROFIT,begin_of_periods.size()-2);
        vector<double> size=getColOfPeriod(TOTALFARMSIZE,begin_of_periods.size()-2);
        vector<int> sprofit;
        vector<int> ssize;
        for (int i=0;i<number_of_farms[number_of_farms.size()-1];i++) {
            sprofit.push_back(i);
            ssize.push_back(i);
        }
        quicksort(profit,sprofit,0,profit.size()-1);
        quicksort(size,ssize,0,size.size()-1);
        sort_by_profit.push_back(sprofit);
        sort_by_farmsize.push_back(ssize);

    }
    op=true;
}
int
OutputControl::isInCache(int number) {
    for (unsigned int i=0;i<cacheNumber.size();i++) {
        if (cacheNumber[i]==number)
            return i;
    }
    return -1;
}


void
OutputControl::close() {
    out.close();
}
double
OutputControl::getSumColOfPeriod(int c,int p) {
    vector<double> o=getColOfPeriod(c,p);
    double out=0;
    for (unsigned int i=0;i<o.size();i++) {
        out +=o[i];
    }
    return out;
}
vector<double>
OutputControl::getColOfPeriod(int col,unsigned int period) {
    if (period==number_of_farms.size()-1) {
        int c=isInCache(col);
        if (c!=-1)
            return cache[c];
    }
    vector<double> data;
    for (int i=0;i<number_of_farms[period];i++) {
        out.seekg(begin_of_periods[period]+col*WIDTH+i*LINESIZE);
        double d;
        out >> d;
        data.push_back(d);
    }
    if (period==number_of_farms.size()-1) {
        cache.push_back(data);
        cacheNumber.push_back(col);
    }
    return data;
}

double
OutputControl::getAvColOfPeriod(int col,int period,int farmtype,int full_time) {
    int c=0;
    vector<double> data=getColOfPeriod(col,period);
    vector<double> fc=getColOfPeriod(FARMCLASS,period);
    vector<double> ft=getColOfPeriod(ERWERBSFORM,period);
    if (full_time<0) {
        for (unsigned int i=0;i<ft.size();i++) {
            ft[i]=full_time;
        }
    }
    if (farmtype<0) {
        for (unsigned int i=0;i<fc.size();i++) {
            fc[i]=farmtype;
        }
    }
    for (unsigned int i=0;i<fc.size();i++) {
        if (fc[i]!=farmtype || ft[i]!=full_time) {
            c++;
            data[i]=0;
        }
    }
    double sum=0;
    for (unsigned int i=0;i<data.size();i++) {
        sum+=data[i];
    }
    if (data.size()-c<=0)
        return 0;
    else
        return sum/(double)(data.size()-c);
}
int
OutputControl::getCountFarmsOfPeriod(int period,int farmtype,int full_time) {
    int c=0;
    vector<double> fc=getColOfPeriod(FARMCLASS,period);
    vector<double> ft=getColOfPeriod(ERWERBSFORM,period);
    if (full_time<0) {
        for (unsigned int i=0;i<ft.size();i++) {
            ft[i]=full_time;
        }
    }
    if (farmtype<0) {
        for (unsigned int i=0;i<fc.size();i++) {
            fc[i]=farmtype;
        }
    }
    for (unsigned int i=0;i<fc.size();i++) {
        if (fc[i]==farmtype && ft[i]==full_time) {
            c++;
        }
    }
    return c;
}

vector< vector<double> >
OutputControl::getColOfAllPeriods(int col) {
    vector< vector<double> > data;
    for (unsigned int i=0;i<number_of_farms.size();i++) {
        data.push_back(getColOfPeriod(col,i));
    }
    return data;
}
int
OutputControl::partition( vector<double>& a,vector<int>& b, int low, int high ) {
    int left, right;
    double pivot_item;
    int pivot_item2;
    pivot_item = a[low];
    pivot_item2 = b[low];
    left = low;
    right = high;
    while ( left < right ) {
        /* Move left while item < pivot */
        while ( a[left] <= pivot_item && left<high) left++;
        /* Move right while item > pivot */
        while ( a[right] > pivot_item && right>low) right--;
        if ( left < right ) swap(a,b,left,right);
    }
    /* right is final position for the pivot */
    a[low] = a[right];
    a[right] = pivot_item;
    b[low] = b[right];
    b[right] = pivot_item2;
    return right;
}
void
OutputControl::swap(vector<double>& a,vector<int>& b,int l,int r) {
    double tmp1=a[l];
    int tmp2=b[l];
    a[l]=a[r];
    b[l]=b[r];
    a[r]=tmp1;
    b[r]=tmp2;
}
void
OutputControl::swap(vector<double>& a,int l,int r) {
    double tmp1=a[l];
    a[l]=a[r];
    a[r]=tmp1;
}
void
OutputControl::quicksort(vector<double>& a,vector<int>& b, int low, int high ) {
    int pivot;
    /* Termination condition! */
    if ( high > low )   {
        pivot = partition( a, b ,low, high );
        quicksort( a, b, low, pivot-1 );
        quicksort( a, b, pivot+1, high );
    }
}
// 0:size 1:profit
void
OutputControl::groupBy(vector<double>& d,int t,int p) {
    vector<double> d2 = d;
    for (unsigned int i=0;i<d.size();i++) {
        int c;
        if (t==0)
            c=sort_by_farmsize[p][i];
        else
            c=sort_by_profit[p][i];
        d[i]=d2[c];
    }
}
void
OutputControl::groupBySize(vector<double>& d,int p) {
    groupBy(d,0,p);
}
void
OutputControl::groupByProfit(vector<double>& d,int p) {
    groupBy(d,1,p);
}

vector<double>
OutputControl::getInfoOfSpecificFarm(vector<int>& cols,int farm_id,int period) {
    vector<double> data;
    int line=0;
    bool found=false;
    for (int i=0;i<number_of_farms[period];i++) {
        out.seekg(begin_of_periods[period]+1*WIDTH+i*LINESIZE);
        double d;
        out >> d;
        if (d==farm_id) {
            line=i;
            found=true;
            break;
        }
    }
    if (found) {
        for (unsigned int i=0;i<cols.size();i++) {
            out.seekg(begin_of_periods[period]+cols[i]*WIDTH+line*LINESIZE);
            double d;
            out >> d;
            data.push_back(d);
        }
    }
    return data;
}
int
OutputControl::getActualPeriod() {
    return number_of_farms.size()-1;
}

static string trim(const std::string& str, const std::string& whitespace = "\t ")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

static void policyError(string str){
	cout<<"Error: "<<str<<endl;
	exit(5);
}

static bool getValidLine(ifstream &ins, string &astr){ 
	if  (ins.eof()) return false;

	getline(ins,astr);
	astr=trim(astr);
	while (astr.length()<=0 || astr.at(0)=='#') {
		if (ins.eof()) break;
		getline(ins,astr);
		astr=trim(astr);
	}
	//*/
	if (ins.eof()) return false;
	else return true;
}

// in prods: (name=value;)*
static string appendStr(string app, string prods){
	string res;
	auto p1 = prods.find_first_not_of(' ');
	auto p2 = prods.find_first_of('=');
	if (p1 ==string::npos|| p2 == string::npos)
		policyError("Missing name !");
	while(1) {
	   res += trim(prods.substr(p1,p2-p1))+"_"+app+"=";
	   p1=p2+1;
	   p2 = prods.find(';',p1);
	   if (p2==string::npos) 
		   policyError("Missing semicolon !");

	   if (p2<=p1) 
		   policyError("Missing value !");

	   res+= trim(prods.substr(p1,p2-p1))+";";
	   p1=p2+1;
	   p2=prods.find('=',p1);
	   if (p2==string::npos)
		   break;
	}
	//*/
	return res;
}

static string readPolicy(ifstream &ins){
	string res;
	string aline;
	while (getValidLine(ins, aline)){
		string first, second;
		replace(aline.begin(),aline.end(),'"',' ');
		if (regex_match(aline, regex("[\t ]*iteration[\t ]*([0-9]+)[\t ]*", regex_constants::icase))){
			break;
		}
		
		auto p1=aline.find(':');
		first=trim(aline.substr(0,p1));
		second=trim(aline.substr(p1+1));
			
		if (first=="premium" ||first=="decoupling" || first == "price_change")
		    res += appendStr(first,second);
		else if (first=="ref_prem_percent")  {
			res += appendStr("reference_premium_percent", second);
		}else if (first=="tranch") { 
			second.erase(remove(second.begin(),second.end(),' '),second.end());
			res+=second;
		} else {//"others" !!!Others must be after all the other categories
			second.erase(remove(second.begin(),second.end(),' '),second.end());
			res+="others:"+second;
		}
		//*/
		//res += second;
	}
	return res;
}

string
OutputControl::getPolicySettingsSep(int p) {
    //string e=GetCurrentDir().c_str();
    ifstream in3;
    in3.open(policy_input.c_str(),ios::in);
//DCX
	if ( ! in3.is_open() ) {
       cerr << "Error while opening: " << policy_input << "\n";
	   exit(2);
    }
//
	string result;
	string aline;
	bool firstLine = true;
	int n;
	smatch m;
  while (!in3.eof()) {
 	if (!getValidLine(in3, aline)) return result;
	if (firstLine)  { // iteration line
		if (!regex_search(aline, m, regex("[\t ]*iteration[\t ]*([0-9]+)[\t ]*", regex_constants::icase))){
			policyError("Iteration line missing !");
		} else 
			n =atoi(m[1].str().c_str());
		firstLine = false;
	} else {

		bool found = regex_search(aline,m, regex("[\t ]*iteration[\t ]*([0-9]+)[\t ]*", regex_constants::icase));
		if (!found) 
			continue;
		else 
			n = atoi(m[1].str().c_str());
	}

	if (n<p) continue;
	if (n==p) {
	    result = readPolicy(in3);
	  break;
	}
	if (n>p) {
		  break;
	}
	//*/
  }
    
    in3.close();
    return result;
}

string
OutputControl::getPolicySettings(int p) {
    //string e=GetCurrentDir().c_str();
    ifstream in3;
    in3.open(policy_input.c_str(),ios::in);
//DCX
	if ( ! in3.is_open() ) {
       cerr << "Error while opening: " << policy_input << "\n";
	   exit(2);
    }
//
    char x[10000];
    for (int i=0;i<10000;i++) {
        x[i]=' ';
    }
    for (int i=0;i<=p;i++) {
        in3.getline(x,10000);
    }
    for (int i=0;i<10000;i++) {
        if (x[i]=='"') x[i]=' ';
    }
    in3.close();
    return string(x);

}
#undef DATABEGIN
#undef LINESIZE
#undef WIDTH
#undef TOTALFARMSIZE
#undef PROFIT
#undef FARMCLASS
#undef ERWERBSFORM




