# Reinforcement Learning extension for AgriPoliS
# 2023

import pydata_pb2 as md
from config import nInvs
import numpy as np
import os

def mkscenario(fn, ep, temp='<num>'):
    with open(fn,"rt") as fin:
        d=fin.read()
        d=d.replace(temp, str(ep))

    fn2=os.path.join(os.path.dirname(fn), "scenario-"+str(ep)+".txt")
    with open(fn2,"wt") as fin:
        fin.write(d)

    return os.path.basename(fn2)

def manage2int(m):
    if m<0.95:
        return 0
    elif m>1.05:
        return 2
    else:
        return 1

def rel2int(rel):
    if rel < 1: 
        return 0
    elif rel < 1.2:
        return 1
    elif rel < 1.5:
        return 2
    elif rel < 2:
        return 3
    else:
        return 4

def rel2intList(relv):
    return list(map(rel2int, relv))

def refinit():
    pass

def adjust(d):
    pass

def flat(d):
    res=[it for sub in d.restPlotsOfType for it in sub.n]
    res.append(d.age)
    res.append(d.liquidity)
    res.append(d.management)
    
    invs = np.zeros(nInvs)
    for k,v in d.restInvests.items():
        invs[k]= v.num*v.life
    res.extend(invs)


    res.extend(d.recentRents)
    res.append(d.nfarms10km)
    res.extend(d.nfreeplots10km)
    res.extend(d.avNewRents)

    return res
#    print(res)

def get_state(d):
    rld = md.RLData()
    rld.ParseFromString(d)
    return flat(rld)
    #return rld


