/**
 * @file systemcmd_proxy_interface.cpp
 * @author yuwangliang (astralrovers@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2020-01-25
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>
// net
#include <sys/socket.h>
#include <sys/un.h>

#include "systemcmd_private_protocol.h"
#include "systemcmd_proxy_interface.h"

namespace wotsen
{

#ifdef DEBUG

#define __DEBUG(fmt, args...) printf("[%s %s %d]" fmt, __FILE__, __func__, __LINE__, ##args)

#else

#define __DEBUG(fmt, args...) (void)0

#endif

#define _INVALID_NET_FD -1 // 无效网络套接字句柄

static int _proxy_sock = _INVALID_NET_FD; // 代理套接字句柄

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static bool create_proxy_sock(void);
static void close_proxy_sock(void);
static bool send_cmd(const char *cmd);
static bool get_proxy_result(void);

/**
 * @brief 关闭套接字
 *
 * @function close_proxy_sock 
 */
static void close_proxy_sock(void)
{
	if (_INVALID_NET_FD != _proxy_sock)
	{
		close(_proxy_sock);
		_proxy_sock = _INVALID_NET_FD;
	}

	return ;
}

/**
 * @brief Create a proxy sock object
 * 
 * @return true 
 * @return false 
 */
static bool create_proxy_sock(void)
{
	int reuse = 1;
	struct sockaddr_un address;

	// 建立过了就不再建立了
	if (_INVALID_NET_FD != _proxy_sock)
	{
		return true;
	}

	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		__DEBUG("%s\n", strerror(errno));
		return false;
	}

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, NETSYSTEM_PROXY_AF_UNIX_NODE);

	if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		close(sockfd);
		__DEBUG("%s\n", strerror(errno));
		return false;
	}

	_proxy_sock = sockfd;

	return true;
}

/**
 * @brief 发送命令
 *
 * @function        send_cmd 
 * @param[input]    cmd : 命令
 *
 * @return          true-发送成功，false-发送失败
 */
static bool send_cmd(const char *cmd)
{
	struct netsystem_proxy_protocol net_cmd; // 协议命令

	memset(&net_cmd, 0, sizeof(net_cmd));

	sprintf(net_cmd.cmd, "%s", cmd);

	__DEBUG("cmd : %s\n", net_cmd.cmd);

	return write(_proxy_sock, &net_cmd, sizeof(net_cmd)) == sizeof(net_cmd);
}

/**
 * @brief Get the proxy result object
 * 
 * @return true 
 * @return false 
 */
static bool get_proxy_result(void)
{
	struct netsystem_proxy_protocol net_cmd; // 协议命令
	char buf[300] = {0};

	memset(&net_cmd, 0, sizeof(net_cmd));
	memset(buf, 0, sizeof(buf));

	if (sizeof(net_cmd) != read(_proxy_sock, buf, sizeof(buf)))
	{
		__DEBUG("recv error\n");
		close_proxy_sock();
		return false;
	}

	memcpy(&net_cmd, buf, sizeof(net_cmd));

    __DEBUG("执行结果：%d %s\n", net_cmd.ret, net_cmd.cmd);

	if (e_system_exec_ok != net_cmd.ret)
	{
		printf("systemcmd proxy faild : [%x] %s\n", net_cmd.ret, net_cmd.cmd);
		return false;
	}

	return true;
}

/**
 * @brief 系统命令代理
 * 
 * @param cmd 命令字符串
 * @return true 成功
 * @return false 失败
 */
bool systemcmd_proxy(const char *cmd)
{
	if (NULL == cmd)
	{
		__DEBUG("cmd null\n");
		return false;
	}

	if (strlen(cmd) > MAX_SYSTEM_CMD_LEN)
	{
		__DEBUG("e_systemcmd_proxy_cmd_toolong[%ld > %d]\n",
				strlen(cmd), MAX_SYSTEM_CMD_LEN);
		return false;
	}

	pthread_mutex_lock(&mutex);

	// 创建连接
	if (!create_proxy_sock())
	{
		pthread_mutex_unlock(&mutex);
		return false;
	}

	// 发送命令
	if (!send_cmd(cmd))
	{
		close_proxy_sock();
		__DEBUG("send_cmd faild\n");
		pthread_mutex_unlock(&mutex);
		return false;
	}

	pthread_mutex_unlock(&mutex);

	// 获取执行结果
	return get_proxy_result();
}

/**
 * @brief Get the systemcmd proxy version object
 * 
 * @return const char* 
 */
const char *get_systemcmd_proxy_version(void)
{
	const static char *version= SYSTEMCMD_PROXY_VERSION;

	return version;
}

} // namespace wotsen