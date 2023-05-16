# Reinforcement Learning extension for AgriPoliS
# 2023

import pydata_pb2 as md
import zmq
from config import runs
import random
from ag import get_value

context = zmq.Context()
send_socket = context.socket(zmq.PUSH)

send_addr = "tcp://localhost:5557"
send_socket.connect(send_addr)

recv_socket = context.socket(zmq.PULL)
recv_addr = "tcp://0.0.0.0:5555"
recv_socket.bind(recv_addr)


def send_message(data):
   # Serialize the Person message to a byte string
    message_data = data #data.SerializeToString()

    # Send the byte string over the socket
    send_socket.send(message_data)

def send_ec(ec):
    msg=str(ec)
    send_socket.send_string(msg)

def recv_beta():
    recv_message=recv_socket.recv_string()
    beta=float(recv_message)
    return beta

def test_data(iter):
    data=md.RLData()
    #data.iter=iter
    data.age=iter+1 
    data.liquidity=random.random()
    data.management=random.random()
    data.nfarms10km=random.randint(1,6)
    
    plots1=data.restPlotsOfType.add()
    plots1.n.extend([1,2,0,0,0])
    plots2=data.restPlotsOfType.add()
    plots2.n.extend([0,1,0,0,0])
   
    inv1 = data.restInvests[3]
    inv1.num=1
    inv1.life=3.
    inv2 = data.restInvests[4]
    inv2.num=1
    inv2.life=2.

    data.recentRents.extend([350,200])
    data.nfreeplots10km.extend([22,12])
    data.avNewRents.extend([340, 180])
    return data
    
for i in range(runs):
    data=test_data(i)
    print("agp: ", data)
    
    data_msg=data.SerializeToString()
    send_message(data_msg)
        
    beta = recv_beta()
    print("agp: beta", beta)

    ec=get_value()
    print("agp: ec", ec)

    send_ec(ec)


send_socket.close()
recv_socket.close()
context.term()
