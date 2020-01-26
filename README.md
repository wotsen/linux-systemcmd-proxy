# linux-systemcmd-net-proxy

# 目录

[TOC]

## 目的

用于解决项目当中执行`system()`系统调用时会默认开启子进程，造成资源浪费，特别是大型项目中，
这里采用`systemcmd`代理服务器进程，专用于处理`system`调用，应用程序与代理服务器之间采用本地网络套接字通信.

## 原理

应用调用代理接口，代理接口内部进行与代理服务器之间的通信，将命令发送到服务器，服务器执行发送过来的命令，调用`system()`
进行执行，并且返回执行结果。

代理接口等待服务器返回执行结果，之后将执行结果返回给应用，这里只做简单返回:成功/失败.

**注意**：代理服务器一次只处理一条命令请求，不做并发支持.

## 使用

### 1. 编译

- 正常编译安装
  
  ```shell
  make && make install
  ```

- 带调试信息
  
  ```shell
  make DEBUG=true && make install
  ```

- 指定安装路径
  
  ```shell
  make && make install MAKE_INSTALL_PREFIX=.
  ```

- 编译测试，在编译安装后执行以下：
  
  ```shell
  make cli_test
  ```

  启动服务器`./systemcmd_proxyd`
  启动客户端`./cli_test ls`执行ls命令.

### 2. 集成

- 安装后的代理服务器程序`systemcmd_proxyd`在指定路径下的`bin`目录.

- 库`libsystemcmd_proxy_cli.a`在指定路径`lib`目录.

- 头文件`systemcmd_proxy_interface.h`在指定路径`include`目录.

使用时添加头文件和库.

### 3. 接口

```cpp
namespace wotsen
{

/**
 * @brief 系统命令代理
 *
 * @param cmd 命令字符串
 * @return true 成功
 * @return false 失败
 */
bool systemcmd_proxy(const char *cmd);

/**
 * @brief Get the systemcmd proxy version object
 *
 * @return const char*
 */
const char *get_systemcmd_proxy_version(void);

} // namespace wotsen
```
