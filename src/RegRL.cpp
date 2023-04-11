/*************************************************************************
* This file is part of AgriPoliS
*
* AgriPoliS: An Agricultural Policy Simulator
*
* Copyright (c) 2023, Alfons Balmann, Kathrin Happe, Konrad Kellermann et al.
* (cf. AUTHORS.md) at Leibniz Institute of Agricultural Development in 
* Transition Economies
*
* SPDX-License-Identifier: MIT
**************************************************************************/

#include "RegRL.h"
#include "RegPlot.h"
#include "RegInvest.h"
#include <zmq.hpp>
#include <google/protobuf/text_format.h>
#include "pydata.pb.h"


    static int numSoils;
    static RLdata cachedRLdata;
    //static vector<int> newRentedplots;
    vector<double> recentRents; //farm
    vector<double> avRecentRents; //region
    static const int nyear = 5;

    static double plotlen;
    static double x00;
    static double y00;
    static double ncols;
    static double nrows;

    bool newIteration = true;

    static bool zmqinited = false;

    static void toPb(rl::RLData& data) {
        data.set_age(cachedRLdata.age);
        data.set_liquidity(cachedRLdata.liquidity);
        data.set_management(cachedRLdata.management);
        data.set_nfarms10km(cachedRLdata.nfarms10km);
        *data.mutable_avnewrents() = { cachedRLdata.avNewRents.begin(), cachedRLdata.avNewRents.end() };
        *data.mutable_recentrents() = { cachedRLdata.recentRents.begin(), cachedRLdata.recentRents.end() };
        *data.mutable_nfreeplots10km() = { cachedRLdata.nfreeplots10km.begin(), cachedRLdata.nfreeplots10km.end() };
        
        for (int i = 0; i < cachedRLdata.restPlotsOfType.size(); ++i) {
            rl::RLData_plots pls;
            *pls.mutable_n() = {cachedRLdata.restPlotsOfType[i].begin(), cachedRLdata.restPlotsOfType[i].end()};
            (*data.mutable_restplotsoftype()).Add(move(pls));
        }
       
        for (auto i: cachedRLdata.restInvests) {
            rl::RLData_invlife il;
            il.set_num(i.second.first);
            il.set_life(i.second.second);
            (*data.mutable_restinvests())[i.first]=move(il);
        }
    }

    static zmq::context_t context(1);
    static zmq::socket_t sender_socket(context, ZMQ_PUSH);
    static zmq::socket_t receiver_socket(context, zmq::socket_type::pull);

    void initzmq() {
        sender_socket.connect("tcp://localhost:5557");
        receiver_socket.bind("tcp://0.0.0.0:5555");
        zmqinited = true;
        cout << "zmq inited\n";
    }

    static void send() {
        GOOGLE_PROTOBUF_VERIFY_VERSION;

        //zmq::context_t context(1);

        // sender
        //zmq::socket_t sender_socket(context, ZMQ_PUSH);
        //sender_socket.connect("tcp://localhost:5557");

        //zmq::socket_t receiver_socket(context, zmq::socket_type::pull);
        //receiver_socket.bind("tcp://0.0.0.0:555");

        //zmq::message_t context_msg(sizeof(context));
        //std::memcpy(context_msg.data(), &context, sizeof(context));
        //sender_socket.send(context_msg);

        std::chrono::seconds twosec(2);
        int max = 10;

        rl::RLData data;
        
        int i = cachedRLdata.iter;//zero is default for protobuf;
        if (i >= max)
            return;
        //while (true) {
            toPb(data);
            std::string text;
            google::protobuf::TextFormat::PrintToString(data, &text);
            //std::cout << "C0: " << text << std::endl;
                         
            std::string msg_str;
            data.SerializeToString(&msg_str);

            zmq::message_t message(msg_str.size());
            std::memcpy((void*)message.data(), msg_str.c_str(), msg_str.size());
            sender_socket.send(message);// , zmq::send_flags::dontwait);

           //zmq::message_t context_msg;
           //receiver_socket.recv(&context_msg);
           //std::memcpy(&context, context_msg.data(), sizeof(context));

            //std::this_thread::sleep_for(twosec);

            zmq::message_t received_message;
            receiver_socket.recv(&received_message);

            //std::vector<double> ds;
            //std::vector<std::string> ss;

            //mypackage::MyData rdata;
            std::string rmsg_str(static_cast<char*>(received_message.data()), received_message.size());

            data.ParseFromString(rmsg_str);
            //ds = { rdata.data().begin(), rdata.data().end() };
            //ss = { rdata.tags().begin(), rdata.tags().end() };
            ++i;
            if (data.age() != cachedRLdata.age) {
                std::cout << "c: age not correct! " << data.age() << "\n";
                exit(3);
            }

            //std::string text;
            google::protobuf::TextFormat::PrintToString(data, &text);
            std::cout << "C: " << text << std::endl;

        //}

        google::protobuf::ShutdownProtobufLibrary();
    }

    static vector<vector<int>> calcRestPlots(list<RegPlotInfo*> pls) {
        vector<vector<int>> res;
        res.resize(numSoils);
        for (auto i = 0; i < numSoils; ++i)
            res[i].resize(nyear);//+1);

        for (auto t : pls) {
            int  restlen = (*t).getContractLength();
            int type = (*t).getSoilType();
            if (restlen > 0 && restlen <= nyear)
                res[type][restlen - 1]++;
            //else res[type][nyear]++;
        }
        return res;
    }

    static map<int, pair<int, double>> calcRestInvests(list<RegInvestObjectInfo> invs) {
        map<int, pair<int, double>> res;
        for (auto t : invs) {
            int id = t.getCatalogNumber();
            int age = t.getInvestAge();
            int life = t.getEconomicLife();
            if (id <= 1)
                continue;
            if (res[id].first > 0) {
                double r = res[id].second * res[id].first + life - age;
                int c = res[id].first++;

                res[id] = pair(c, r / c);
            }
            else {
                res[id] = pair(1, life - age);
            }
        }
        return res;
    }

    //square of distance
    static double dist2(double x0, double y0, double x, double y, double cols, double rows, double len) {
        double dx, dy;
        dx = min(abs(x - x0), x > x0 ? x0 + cols - x : x + cols - x0);
        dy = min(abs(y - y0), y > y0 ? y0 + rows - y : y + rows - y0);

        return pow(dx * len, 2) + pow(dy * len, 2);
    }

    static void calc0(RegFarmInfo* f, RegManagerInfo* m) {
        plotlen = sqrt(m->getGlobals()->PLOT_SIZE * 10000);
        x00 = f->getFarmPlot()->getCol();
        y00 = f->getFarmPlot()->getRow();
        ncols = m->getGlobals()->NO_COLS;
        nrows = m->getGlobals()->NO_ROWS;
    }

    static int  calcNfarms(RegFarmInfo* f, RegManagerInfo* m, double dist) {
        int  res = 0;

        double x, y;
        list<RegFarmInfo*> farms = m->getFarmList();
        for (auto& tf : farms) {
            if (tf == f) continue;
            x = tf->getFarmPlot()->getCol();
            y = tf->getFarmPlot()->getRow();

            if (dist2(x00, y00, x, y, ncols, nrows, plotlen) <= pow(dist, 2))
                ++res;
        }

        return res;
    }

    static vector<int> calcNfreeplots(RegFarmInfo* f, RegManagerInfo* m, double dist) {
        vector<int> res;
        res.resize(numSoils);

        double x, y;
        vector<RegPlotInfo*> freeplots = m->getRegion()->getFreeplots();
        for (auto f : freeplots) {
            x = f->getCol();
            y = f->getRow();
            int t = f->getSoilType();
            if (dist2(x00, y00, x, y, ncols, nrows, plotlen) <= pow(dist, 2))
                ++res[t];
        }

        return res;
    }

    static vector<double> calcAvRents() {
        /*vector<double> res;
        RegRegionInfo* reg = m->getRegion();
        int n = numSoils;
        res.resize(n);
        for (auto i = 0; i < n; ++i) {
            res[i] = reg->getAvRentOfType(i);
        }
        return res;
        //*/
        return avRecentRents;
    }

    static vector<double> calcAvNewRents() {
        /*vector<double> res;
        RegRegionInfo* reg = m->getRegion();
        int n = numSoils;
        res.resize(n);
        newRentedplots.resize(n);

        for (auto i = 0; i < n; ++i) {
            res[i] = reg->getAvNewRentOfType(i);
            newRentedplots[i] = reg->getNewlyRentedPlotsOfType(i);
        }
        return res;
        //*/
        return avRecentRents;
    }

    static vector<double> calcRecentRents() {
        /*vector<double> res;
        res.resize(numSoils);
        for (auto i = 0; i < numSoils; ++i) {
            res[i] = f->getAvNewRentOfType(i);
        }
        return res;
        //*/
        return recentRents;
    }

    RLdata getRLdata(RegFarmInfo* f, RegManagerInfo* m) {
        if (!newIteration)
            return cachedRLdata;

        newIteration = false;
        numSoils = m->getGlobals()->NO_OF_SOIL_TYPES;
        calc0(f, m);

        RLdata rlData;

        rlData.iter = m->getIteration();

        list<RegPlotInfo*> plots = f->getPlotList();
        rlData.restPlotsOfType = calcRestPlots(plots);
        rlData.age = f->getFarmAge();
        rlData.liquidity = f->getLiquidity();
        rlData.management = f->getManagementCoefficient();
        RegInvestList* invlist = f->getFarmInvestList();
        list<RegInvestObjectInfo> invs = invlist->getFarmInvests();
        rlData.restInvests = calcRestInvests(invs);
        rlData.recentRents = calcRecentRents();

        rlData.nfarms10km = calcNfarms(f, m, 10000);
        rlData.nfreeplots10km = calcNfreeplots(f, m, 10000);
        //rlData.avRents = calcAvRents();
        rlData.avNewRents = calcAvNewRents();

        cachedRLdata = rlData;

        
        return rlData;
    }

    static void outputPlots(vector<vector<int>> vps, vector<string> names, ofstream &out) {
        out << "plots with rest contract <= 5 years:\n";
        for (auto i = 0; i < vps.size(); ++i) {
            out << "\t" << names[i] << ": ";
            for (auto i2 = 0; i2 < vps[i].size(); ++i2) {
                out << "\t" << vps[i][i2];
            }
            out << "\n";
        }
    }

    static void outputInvests(map<int, pair<int, double>> restinvs, vector<RegInvestObjectInfo> invcat, ofstream &out) {
        vector<string> idnames;
        for (auto i = 0; i < invcat.size(); ++i) {
            idnames.push_back(invcat[i].getName());
        }
        out << "rest life of investments (name, num, rest life): \n";
        for (auto& [id, p] : restinvs) {
            out << "\t" << idnames[id] << "\t" << p.first << "\t" << p.second << "\n";
        }
    }

    static void outputFreeplots(vector<int> fps, vector<string> names, ofstream &out) {
        out << "number of free plots within 10km:\n ";
        for (auto i = 0; i < fps.size(); ++i) {
            out << "\t" << names[i] << "\t" << fps[i] << ";";
        }
        out << "\n";
    }


    static void outputRents(vector<double> rents, vector<string> names, ofstream &out, bool forfarm = false) {
        if (!forfarm)
            out << "region recent rents:\n ";
        else
            out << "farm recent rents:\n ";

        for (auto i = 0; i < names.size(); ++i) {
            out << "\t" << names[i] << "\t" << rents[i] << ";";
        }
        out << "\n";

    }

    void output(RLdata rldata, RegManagerInfo* m, string fname="") {
        if (fname.size() == 0)
            return;

        ofstream out;
        out.open(fname, ios::app);
        if (!out.is_open()) {
            cout << "can not open: " << fname << endl;
        }

        vector<string> soilnames = m->getGlobals()->NAMES_OF_SOIL_TYPES;
        outputPlots(rldata.restPlotsOfType, soilnames, out);
        out << "age: " << rldata.age << "\n";
        out << "liquidity: " << rldata.liquidity << "\n";
        out << "management: " << rldata.management << "\n";
        outputInvests(rldata.restInvests, m->getInvestCatalog(), out);
        outputRents(rldata.recentRents, soilnames, out, true);

        out << "number of farms within 10km: " << rldata.nfarms10km << "\n";
        outputFreeplots(rldata.nfreeplots10km, soilnames, out);
        //outputRents(rldata.avRents, soilnames, out);
        outputRents(rldata.avNewRents, soilnames, out);
        out << "\n\n";
        out.close();

        if (!zmqinited)
            initzmq();
        send();
    }
