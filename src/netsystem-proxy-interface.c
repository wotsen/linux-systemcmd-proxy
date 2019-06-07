/* 
 * Copyright (c) 2019 astralrovers.
 * 
 * Unpublished Copyright. All rights reserved. This material contains
 * proprietary information that should be used or copied only within
 * astralrovers, except with written permission of astralrovers.
 * 
 * @file netsystem-proxy-interface.c
 * @brief:   
 * @author astralrovers
 * @version 1.0
 * @date 2019-06-07
 */
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
// net
#include <sys/socket.h>
#include <sys/un.h>

#include "netsystem-proxy-protocol.h"
#include "netsystem-proxy-interface.h"

#define _INVALID_NET_FD -1 // 无效网络套接字句柄

static int _proxy_sock = _INVALID_NET_FD; // 代理套接字句柄
static char _error_code[MAX_SYSTEM_CMD_LEN] = "ok"; // 当前错误代码

static enum e_netsystem_result create_proxy_sock(void);
static void close_proxy_sock(void);
static bool send_cmd(const char *cmd);
static enum e_netsystem_result get_proxy_result(void);
static enum e_netsystem_result retval_to_result(int retval);

/**
 * @brief 重置错误代码
 *
 * @function reset_netsystem_cmd_error_code 
 */
void reset_netsystem_cmd_error_code(void)
{
    sprintf(_error_code, "ok");
}

/**
 * @brief 获取当前错误代码
 *
 * @function get_netsystem_cmd_error_code 
 *
 * @return 错误码字符串
 */
const char * get_netsystem_cmd_error_code(void)
{
    return _error_code;
}

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
 * @brief 创建套接字
 *
 * @function create_proxy_sock 
 *
 * @return 
 */
static enum e_netsystem_result create_proxy_sock(void)
{
	int reuse = 1;
	struct sockaddr_un address;
    int sockfd = _INVALID_NET_FD;

	// 建立过了就不再建立了
	if (_INVALID_NET_FD != _proxy_sock)
	{
		return e_netsystem_ok;
	}

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sockfd < 0)
	{
		return e_netsystem_sock_err;
	}

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, NETSYSTEM_PROXY_AF_UNIX_NODE);

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);

	if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		close(sockfd);
		return e_netsystem_srv_not_exist;
	}

	_proxy_sock = sockfd;

	return e_netsystem_ok;
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
#if defined(NETSYSTEM_CMD_DBG)
	printf("<%s %d> cmd : %s\n", __func__, __LINE__, net_cmd.cmd);
#endif

	return write(_proxy_sock, &net_cmd, sizeof(net_cmd)) == sizeof(net_cmd);
}

/**
 * @brief 执行结果转换
 *
 * @function retval_to_result 
 * @param[input] retval : 返回码
 *
 * @return 
 */
static enum e_netsystem_result retval_to_result(int retval)
{
	switch (retval)
	{
		case e_system_err:
			return e_netsystem_system_err;
			break;
		case e_system_exec_ok:
			return e_netsystem_ok;
			break;
		case e_system_exec_fail:
			return e_netsystem_exec_fail;
			break;
		case e_system_exec_expection:
			return e_netsystem_child_pross;
			break;
		case e_system_errno:
			return e_netsystem_error;
			break;
		default:
			break;
	}

	return e_netsystem_unknow;
}

/**
 * @brief 返回执行结果
 *
 * @function get_proxy_result 
 *
 * @return 
 */
static enum e_netsystem_result get_proxy_result(void)
{
	struct netsystem_proxy_protocol net_cmd; // 协议命令
	char buf[300] = {0};

	memset(&net_cmd, 0, sizeof(net_cmd));
	memset(buf, 0, sizeof(buf));

	if (sizeof(net_cmd) != read(_proxy_sock, buf, sizeof(buf)))
	{
		printf("recv error\n");
		close_proxy_sock();
		return e_netsystem_recv_err;
	}

	memcpy(&net_cmd, buf, sizeof(net_cmd));

    sprintf(_error_code, "%s", net_cmd.cmd);
#if defined(NETSYSTEM_CMD_DBG)
    printf("执行结果：%d %s\n", net_cmd.ret, _error_code);
#endif

	return retval_to_result(net_cmd.ret);
}

/**
 * [net_system_cmd_proxy description] 系统命令代理接口
 * @method net_system_cmd_proxy
 * @param  cmd                  [input] : 需要执行的命令
 * @return                      [out]   : 执行结果
 */
enum e_netsystem_result net_system_cmd_proxy(const char *cmd)
{
    enum e_netsystem_result ret = e_netsystem_ok;

	if (strlen(cmd) > MAX_SYSTEM_CMD_LEN)
	{
		return e_netsystem_cmd_toolong;
	}

	if (e_netsystem_ok != (ret = create_proxy_sock()))
	{
		return ret;
	}

	if (!send_cmd(cmd))
	{
		close_proxy_sock();
		return e_netsystem_send_err;
	}

	return get_proxy_result();
}

