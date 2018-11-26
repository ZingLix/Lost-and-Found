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
    - [ ] 发布公告
    - [ ] 修改物品信息
    - [ ] 撤销公告
    - [ ] 对发起的认领进行认证
    - [ ] 接受、拒绝认领

## 表设计

- user：用户

|字段名|描述|类型|说明|
|:---:|:---:|:---:|:---:|
|username|用户名|varchar(64)|unique index|
|password|密码|varchar(64)||
|user_id|用户id|bigint unsigned|primary key|

- userinfo：用户信息 

|字段名|描述|类型|说明|
|:---:|:---:|:---:|:---:|
|user_id|用户id|bigint unsigned|primary key|
|email|邮箱|varchar(64)||
|phone|手机|char(11)||
|description|用户描述|varchar(512)||

- item：物品

|字段名|描述|类型|说明|
|:---:|:---:|:---:|:---:|
|item_id|物品id|bigint unsigned|primary key|
|item_name|物品名称|varchar(64)||
|item_info|物品信息|varchar(512)||
|lost_location|丢失位置|varchar(128)||

- message：私信 

|字段名|描述|类型|说明|
|:---:|:---:|:---:|:---:|
|seq_id|消息序列号|bigint unsigned|primary key|
|sender_id|发送者id|bigint unsigned||
|recver_id|接收者id|bigint unsigned||
|content|消息内容|varchar(512)||

- item_notice：捡到的人发布的认领公告 

|字段名|描述|类型|说明|
|:---:|:---:|:---:|:---:|
|notice_id|公告id|bigint unsigned|primary key|
|finder_id|发布公告的用户id|bigint unsigned||
|item_id|丢失物品的id|bigint unsigned||

- application：失主发起的认领申请 

|字段名|描述|类型|说明|
|:---:|:---:|:---:|:---:|
|application_seq|申请流水号|bigint unsigned|primary key|
|applicant_id|申请者id|bigint unsigned||
|notice_id|公告id|bigint unsigned||
|status|申请状态|smallint unsigned||

- notice_info：公告信息

|字段名|描述|类型|说明|
|:---:|:---:|:---:|:---:|
|notice_id|公告id|bigint unsigned|primary key|
|contact_id|联系人id|bigint unsigned||
|time|发布时间|timestamp||
