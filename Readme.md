# 失物招领系统

## 功能分析

- 用户：
    - [x] 登陆、登出系统
    - [x] 注册
    - [ ] 录入、修改用户信息
    - [ ] 浏览物品信息
    - [ ] 搜索物品
    - [ ] 私信联系
- 丢失者：
    - [ ] 发起认领
    - [ ] 回复认证
    - [ ] 撤销认领
- 捡到的人：
    - [x] 发布公告
    - [ ] 修改物品信息
    - [ ] 撤销公告
    - [ ] 对发起的认领进行认证
    - [ ] 接受、拒绝认领

## 表设计

### user：用户

|字段名|描述|类型|说明|NULL|
|:---:|:---:|:---:|:---:|:---|
|username|用户名|varchar(64)|unique index|N
|password|密码|varchar(64)||N
|user_id|用户id|bigint unsigned|primary key, auto_increment|N

### userinfo：用户信息 

|字段名|描述|类型|说明|NULL|
|:---:|:---:|:---:|:---:|:---|
|user_id|用户id|bigint unsigned|primary key|N
|email|邮箱|varchar(64)||Y
|phone|手机|char(11)||Y
|description|用户描述|varchar(512)||Y

### item：物品

|字段名|描述|类型|说明|NULL|
|:---:|:---:|:---:|:---:|:---|
|item_id|物品id|bigint unsigned|primary key, auto_increment|N
|item_name|物品名称|varchar(64)||N
|item_info|物品信息|varchar(512)||Y
|lost_location|丢失位置|varchar(128)||N

### message：私信 

|字段名|描述|类型|说明|NULL|
|:---:|:---:|:---:|:---:|:---|
|msg_seq_id|消息序列号|bigint unsigned|primary key, auto_increment|N
|sender_id|发送者id|bigint unsigned||N
|recver_id|接收者id|bigint unsigned||N
|content|消息内容|varchar(512)||N

### item_notice：捡到的人发布的认领公告 

|字段名|描述|类型|说明|NULL|
|:---:|:---:|:---:|:---:|:---|
|notice_id|公告id|bigint unsigned|primary key, auto_increment|N
|finder_id|发布公告的用户id|bigint unsigned||N
|item_id|丢失物品的id|bigint unsigned||N
|status|认领状态|smallint unsigned|default 0|N

### application：失主发起的认领申请 

|字段名|描述|类型|说明|NULL|
|:---:|:---:|:---:|:---:|:---|
|application_seq|申请流水号|bigint unsigned|primary key, auto_increment|N
|applicant_id|申请者id|bigint unsigned||N
|notice_id|公告id|bigint unsigned||N
|status|申请状态|smallint unsigned||N

### notice_info：公告信息

|字段名|描述|类型|说明|NULL|
|:---:|:---:|:---:|:---:|:---|
|notice_id|公告id|bigint unsigned|primary key|N
|contact_id|联系人id|bigint unsigned||N
|time|发布时间|timestamp|DEFAULT CURRENT_TIMESTAMP|N
