# Reinforcement Learning extension for AgriPoliS
# 2023

import subprocess
from config import *
from env import *
import pydata_pb2 as md
from ag import *
from util import *


import random
import queue
import os

res=queue.PriorityQueue(QSIZE)
initzmq()

def testing(e):
    print("evaluating run ...\n")
    closed=False
    agpt=subprocess.Popen([agpy, inputfiles, mkscenario(inputfiles+temp_scenario, e) ], 
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True)
    for r in range(runs):
        if not closed:
            data=recv_message()
            st = get_state(data)
            #print(st)

            beta = get_action()
            send_beta(beta)

            rew = recv_ec()
            #print("beta, rew: ", beta, rew)
            
            rc = recv_closed()
            if rc>0:
                closed=True

        else:
            st=[-1]
            beta=-1
            rew= recv_ec()

    agpt.wait()
    #agpout, agperr = agpt.communicate()
    #print(agpout)
    #print(agperr)

def get_ep(e):
    for s in range(simus):
        closed=False
        #subprocess.run(agripolis, iniputfiles])
        #agp=subprocess.Popen([python, agpy,  str(s)], 
        agp=subprocess.Popen([agpy, inputfiles ], 
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True)

        ep=[]
        eprew=0
        for r in range(runs):
            if not closed:
                data=recv_message()
                st = get_state(data)
                #print(st)

                beta = get_action()
                send_beta(beta)

                rew = recv_ec()
                #print("beta, rew: ", beta, rew)
                
                rc = recv_closed()
                if rc>0:
                    closed=True

            else:
                st=[-1]
                beta=-1
                rew= recv_ec()


            ep.append((st, beta,  rew))
            eprew += rew

          
        res.put((-eprew, ep))
        agp.wait()
        #agpout, agperr = agp.communicate()
        #print(agpout)
        #print(agperr)


    #print(res.queue)
    x=0
    while x < topn  and not res.empty():
       ep=res.get()[1]
       print(ep)
       #yield ep
       x+=1 

    res.queue.clear()

    learning()
    testing(e)

    #if better:
    outputModel()

for e in range(epochs):
    print(f"======= epoch {e} =========") 
    get_ep(e)

closezmq()
