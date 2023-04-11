import pb.pydata_pb2 as md
import zmq

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
    data.ParseFromString(recv_message)
    return data
      
max=10
i=1

while True:
    rdata=recv_message()
    print("py: ", rdata)
          
    i+=1
    #rdata.age = rdata.age;
    send_message(rdata)
    if i>=max:
       print("py: fertig")
       break
         

send_socket.close()
recv_socket.close()
context.term()
