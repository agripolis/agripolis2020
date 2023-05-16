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

def get_ep():
    for s in range(simus):
        #subprocess.run(agripolis, iniputfiles])
        #agp=subprocess.Popen([python, agpy,  str(s)], 
        agp=subprocess.Popen([agpy, inputfiles ], 
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True)

        ep=[]
        eprew=0
        for r in range(runs):
            data=recv_message()
            st = get_state(data)
            #print(st)

            beta = get_action()
            send_beta(beta)

            rew = recv_ec()
            #print("beta, rew: ", beta, rew)

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
       #print(ep)
       yield ep
       x+=1 

    res.queue.clear()

for e in range(epochs):
    if True: # % 10 ==0:
        #pass
        print("============= epoch: ", e)

    for t in get_ep():
        print(t)
        pass
    
    learning()

    testing()


outputModel()


closezmq()
