import socket
import json,threading
import time

def soc_thread(soc,i):
    while (flag==False):
        time.sleep(0.1)
    t1=time.time()
    data = {
        'recver_id':0,
        'type' : 1,
        'username' : str(i),
        'password' : str(i)
    }
    soc.send(json.dumps(data).encode())
    soc.recv(1024)
    t2=time.time()
    print((t2-t1).se)

def test(send_msg,recv_message):
    s=json.dumps(send_msg,separators=(',',':'))
    r=json.dumps(recv_message,separators=(',',':'))
    soc.send(s.encode())
    recv = soc.recv(1024).decode()
    print("Send: " + s)
    print("Expc: "+ r)
    print("Recv: "+ str(recv) + " ......",end='' )
    if(recv==r):
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
    'code':2,
    'notice_id':1
}

test(send,recv)