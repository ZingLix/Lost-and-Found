import socket
import json,threading
import re

n =  0

def test(send_msg,expect_message,description):
    global n
    print("# TEST " + str(n) +" : "+ description)
    n=n+1
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

# test 0
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

test(send,recv,'登陆失败')
soc.close()
soc=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc.connect((host,port))

# test 1

send={
    'type':2,
    'username':'test',
    'password':'test'
}

recv={
    'type':2,
    'code':1,
}

test(send,recv,'注册')
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
    'code':2,
    'content':'Duplicate username.'
}

test(send,recv,'注册失败-用户名重复')
soc.close()
soc=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc.connect((host,port))


# test 3

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

test(send,recv,'登陆成功')

# test 4

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

test(send,recv,'添加公告')

# test 5

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

test(send,recv,'拉取公告')

# test 6

send={
    'type':11,
    'code':3,
    'notice_id':1
}

recv={
    'type':11,
    'code':13,
    'application_seq':1
}

test(send,recv,'发起认领')

# test 7

send={
    'type':11,
    'code':4,
    'notice_id':1
}

recv={
    'type':11,
    'code':14,
    'application_info':[
        [1,1,1,0,1,"手机"]
    ]
}

test(send,recv,'拉取申请列表')

# test 8

send={
    'type':11,
    'code':5,
    'application_seq':1,
    'status':1
}

recv={
    'type':11,
    'code':15,
}

test(send,recv,'接受申请')

# test 9

send={
    'type':11,
    'code':4,
    'notice_id':1
}

recv={
    'type':11,
    'code':14,
    'application_info':[
        [1,1,1,1,1,"手机"]
    ]
}

test(send,recv,'接受申请后状态检查')

# test 10

send={
    'type':11,
    'code':6,
    'notice_id':1,
}

recv={
    'type':11,
    'code':16,
}

test(send,recv,'撤销公告')

# test 11

send={
    'type':11,
    'code':2,
}

recv={
    'type':11,
    'code':12,
    'notice_info':[
        [1,"手机",3]
    ]
}
test(send,recv,'撤销公告后状态检查')

# test 12

send={
    'type':11,
    'code':7,
    'application_seq':1,
}

recv={
    'type':11,
    'code':17,
}

test(send,recv,'撤销申请')

# test 13

send={
    'type':11,
    'code':4,
    'notice_id':1
}

recv={
    'type':11,
    'code':14,
    'application_info':[
        [1,1,1,3,1,"手机"]
    ]
}

test(send,recv,'撤销申请后状态检查')

# test 14

send={
    'type':3,
    'code':1,
    'item_id':1
}

recv={
    'type':3,
    'code':11,
    'item_id':1,
    'item_name':'手机',
    'item_info':'',
    'lost_location':'教室'
}

test(send,recv,'获取物品信息')

# test 15 

send={
    'type':3,
    'code':2,
    'item_id':1,
    'item_name':'手机',
    'item_info':'tttt',
    'lost_location':'食堂'
}

recv={
    'type':3,
    'code':12,
}

test(send,recv,'修改物品信息')

# test 16

send={
    'type':3,
    'code':1,
    'item_id':1
}

recv={
    'type':3,
    'code':11,
    'item_id':1,
    'item_name':'手机',
    'item_info':'tttt',
    'lost_location':'食堂'
}

test(send,recv,'修改物品信息后检查')


# test 17

send={
    'type':4,
    'code':1,
    'user_id':1,
}

recv={
    'type':4,
    'code':11,
    'user_id':1,
    'email':'',
    'phone':'',
    'description':''
}

test(send,recv,'获取用户信息')

# test 18

send={
    'type':4,
    'code':2,
    'user_id':1,
    'email':'test@test.com',
    'phone':'13818888888',
    'description':'heihei'
}

recv={
    'type':4,
    'code':12,
}

test(send,recv,'修改用户信息')

# test 19

send={
    'type':4,
    'code':1,
    'user_id':1,
}

recv={
    'type':4,
    'code':11,
    'user_id':1,
    'email':'test@test.com',
    'phone':'13818888888',
    'description':'heihei'
}

test(send,recv,'修改用户信息后检查')

# test 20

send={
    'type':11,
    'code':8,
    'keyword':'手'
}

recv={
    'type':11,
    'code':18,
    'notice_info':[
        [1,"手机",3]
    ]
}

test(send,recv,'搜索')