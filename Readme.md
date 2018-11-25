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

- user：用户 (username, password, user_id)
- userinfo：用户信息 (user_id, email, phone, description)
- item：物品 (item_id, item_name, item_info)
- message：私信 (sender_id, recver_id, content)
- item_notice：捡到的人发布的认领公告 (notice_id, finder_id, item_id)
- application：失主发起的认领申请 (applicant_id, notice_id, status)
- notice_info：公告信息(notice_id, location, time)
