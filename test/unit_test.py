import socket
import json,threading
import re

def test(send_msg,expect_message):
    s=json.dumps(send_msg,separators=(',',':'))
    exp=json.dumps(expect_message,separators=(',',':'))
    soc.send(s.encode())
    recv = soc.recv(1024).decode()
    print("Send: " + s)
    print("Expc: "+ exp)
    print("Recv: "+ str(recv) + " ......",end='' )
    if(json.loads(recv)==json.loads(exp)):
        print("ok!")
    else:
        print("error!")
    print("  ")

flag=False
host = '118.25.27.241'
port =9981

soc=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc.connect((host,port))

# test 1
send={
    'type':1,
    'username':'test',
    'password':'test'
}

recv={
    'type':1,
    'code':2,
    'content':'Incorrect username or password.'
}

test(send,recv)
soc.close()
soc=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc.connect((host,port))

# test 2

send={
    'type':2,
    'username':'test',
    'password':'test'
}

recv={
    'type':2,
    'code':1,
}

test(send,recv)
soc.close()
soc=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc.connect((host,port))


# test 3

send={
    'type':2,
    'username':'test',
    'password':'test'
}

recv={
    'type':2,
    'code':2,
    'content':'Duplicate username.'
}

test(send,recv)
soc.close()
soc=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc.connect((host,port))


# test 4

send={
    'type':1,
    'username':'test',
    'password':'test'
}

recv={
    'type':1,
    'code':1,
    'user_id':1
}

test(send,recv)

# test 5

send={
    'type':11,
    'code':1,
    'item_name':'手机',
    'item_info':'',
    'lost_location':'教室'
}

recv={
    'type':11,
    'code':11,
    'notice_id':1
}

test(send,recv)

# test 6

send={
    'type':11,
    'code':2,
}

recv={
    'type':11,
    'code':12,
    'notice_info':[
        [1,"手机",0]
    ]
}

test(send,recv)