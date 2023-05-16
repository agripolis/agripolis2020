# Reinforcement Learning extension for AgriPoliS
# 2023

import pydata_pb2 as md
import zmq

def initzmq():
    global context, send_socket, recv_socket
    context = zmq.Context()
    send_socket = context.socket(zmq.PUSH)

    send_addr = "tcp://localhost:5555"
    send_socket.connect(send_addr)

    recv_socket = context.socket(zmq.PULL)
    recv_addr = "tcp://0.0.0.0:5557"
    recv_socket.bind(recv_addr)


def send_message(data):
   # Serialize the Person message to a byte string
    message_data = data.SerializeToString()

    # Send the byte string over the socket
    send_socket.send(message_data)

def recv_message():
    data=md.RLData()
    recv_message=recv_socket.recv()
    #data.ParseFromString(recv_message)
    #print(data)
    return recv_message
    #return data
      
def recv_ec():
    recv_ec = recv_socket.recv_string()
    #print(float(recv_ec))
    return float(recv_ec) 

def send_beta(b):
    #print(b)
    msg=str(b)
    send_socket.send_string(msg)

def closezmq():
    global send_socket, recv_socket, context
    send_socket.close()
    recv_socket.close()
    context.term()
