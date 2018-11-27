import socket
import json,threading
import re

host = '118.25.27.241'
port =9981

soc1=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc1.connect((host,port))
soc2=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc2.connect((host,port))

send={
    'type':2,
    'username':'test1',
    'password':'test1'
}

soc1.send(json.dumps(send).encode())
soc1.recv(1024)

send={
    'type':2,
    'username':'test2',
    'password':'test2'
}

soc2.send(json.dumps(send).encode())
soc2.recv(1024)

soc1=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc1.connect((host,port))
soc2=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc2.connect((host,port))

send={
    'type':1,
    'username':'test1',
    'password':'test1'
}

soc1.send(json.dumps(send).encode())
soc1.recv(1024)

send={
    'type':1,
    'username':'test2',
    'password':'test2'
}

#{"type":1,"username":"test1","password":"test1"}

soc2.send(json.dumps(send).encode())
soc2.recv(1024)

#############################################

send={
    'type':5,
    'code':1,
    'recver_id':2,
    'content':'hello'
}

#{"type":5, "code":1, "recver_id":2,  "content":"hello"}

soc1.send(json.dumps(send).encode())
print(soc2.recv(1024).decode())
print(soc1.recv(1024).decode())

send={
    'type':5,
    'code':2,
}

soc1.send(json.dumps(send).encode())
print(soc1.recv(1024).decode())

send={
    'type':5,
    'code':3,
    'user_id':2
}

soc1.send(json.dumps(send).encode())
print(soc1.recv(1024).decode())