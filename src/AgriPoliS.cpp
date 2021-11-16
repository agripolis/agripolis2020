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
#include <chrono>
#include <ctime>
#include "RegManager.h"
#include "Agripolis.h"
//---------------------------------------------------------------------------

int main (int argc, char * argv[]) {
	auto start = std::chrono::system_clock::now();
	std::time_t time0 = std::chrono::system_clock::to_time_t(start);
	auto ctime_start =  std::ctime(&time0);
	cout << ctime_start << "\n";

	    gg= new RegGlobalsInfo();
		gg->TimeStart = ctime_start;

		gg->ARGC = argc;
		gg->ARGV = &argv[0];
		
		gg->readFromCommandLine();
		vector<string> args = gg->commandlineFILES;
		int nFiles = args.size();
        if (nFiles < 1) {
  		   cout << gg->UsageString; 
           return 1;
        }
	
		optiondir = args[0];
		if (nFiles > 1) gg->SCENARIOFILE = args[1];
		else gg->SCENARIOFILE = "scenario.txt";
		readScenario();
		//gg->DebMip = deb_mip();
		
		if (nFiles > 2 ) gg->V = atoi(args[2].c_str());
		if (gg->V !=0 ) { 
			gg->INIT_OUTPUT = false;
		} else {
			gg->INIT_OUTPUT = true;
		}
		gg->SEED = gg->V*119;

        options(optiondir);
		if (gg->DebMip) 
			deb_mip();
        
	   	RegManagerInfo *Manager = new RegManagerInfo(gg);
        Manager->simulate();
		auto end = std::chrono::system_clock::now();
		std::time_t end_time = std::chrono::system_clock::to_time_t(end);
		gg->TimeStop = std::ctime(&end_time);

		cout << gg->TimeStop << "\n";
        return 0;
}

//---------------------------------------------------------------------------
