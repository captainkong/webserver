# webServer

高性能web服务器

## 核心组件

* 线程池

* sql连接池

* poller

* 应用层buffer

为什么需要应用层buffer?
为了性能考虑,客户socket需要使用非阻塞IO以降低事件循环的IO开销.
为了保证业务逻辑的完整性,每个TCP socket都需要维护一个读缓冲区和写缓冲区.

## 核心功能

* 注册
USE webserver;
CREATE TABLE `USERS`(
    uid int primary key auto_increment,
    name varchar(32) unique not null,
    password char(32) not null,
    reg_time    datetime not null
)ENGINE=InnoDB;

用户名需要是唯一的,在注册时需要使用ajax判断用户名是否可用.

参考项目
<https://github.com/markparticle/WebServer>

参考书籍
《Linux多线程服务端编程》
