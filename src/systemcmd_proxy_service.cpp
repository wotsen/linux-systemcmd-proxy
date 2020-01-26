/**
 * @file systemcmd_proxy_service.cpp
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
#include <stdlib.h>
#include <signal.h>
// net
#include <sys/socket.h>
#include <sys/un.h>

#include "systemcmd_private_protocol.h"

#define _INVALID_NET_FD -1 // 无效网络套接字句柄

static int _proxy_sock = _INVALID_NET_FD; // 代理套接字句柄

static bool create_proxy_sock(void);
static void wait_cmd_connect(void);
static void response_cmd(const int sock);

/**
 * @brief 地址重用
 * 
 * @param sockfd 
 * @return true 
 * @return false 
 */
static bool addr_reuse(const int sockfd)
{
	int reuse = 1;

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse) < 0)
	{
		perror("Setsock failed");
		return false;
	}

	return true;
}

/**
 * @brief 地址绑定
 * 
 * @param sockfd 
 * @return true 
 * @return false 
 */
static bool bind_addr(const int sockfd)
{
	struct sockaddr_un address;

	memset(&address, 0, sizeof(address));

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, NETSYSTEM_PROXY_AF_UNIX_NODE);

	if (bind(sockfd, (struct sockaddr *)&address, (socklen_t)sizeof(address)) < 0)
	{
		perror("Bind failed");
		return false;
	}

	return true;
}

/**
 * @brief 监听请求
 * 
 * @param sockfd 
 * @return true 
 * @return false 
 */
static bool listen_requset(const int sockfd)
{
	if (listen(sockfd, 20) < 0)
	{
		perror("Listen failed");
		return false;
	}

	return true;
}

/**
 * @brief Create a proxy sock object
 * 
 * @return true 
 * @return false 
 */
static bool create_proxy_sock(void)
{
	// 建立过了就不再建立了
	if (_INVALID_NET_FD != _proxy_sock)
	{
		return true;
	}
	
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		return false;
	}

	if (!addr_reuse(sockfd))
	{
		close(sockfd);
		return false;
	}	

	unlink(NETSYSTEM_PROXY_AF_UNIX_NODE);

	if (!bind_addr(sockfd) || !listen_requset(sockfd))
	{
		close(sockfd);
		unlink(NETSYSTEM_PROXY_AF_UNIX_NODE);
		return false;
	}

	_proxy_sock = sockfd;

	return true;
}

/**
 * @brief system 执行结果
 * 
 * @param net_cmd 
 */
static void system_cmd_exec_result(struct netsystem_proxy_protocol *net_cmd)
{
    if (e_system_err == net_cmd->ret)
    {
        sprintf(net_cmd->cmd, "system error!");
    }
    else
    {
        if (WIFEXITED(net_cmd->ret))
        {
            if (e_system_exec_ok == WEXITSTATUS(net_cmd->ret))
            {
                net_cmd->ret = e_system_exec_ok;
                sprintf(net_cmd->cmd, "run shell script successfully.");
            }
            else
            {
                sprintf(net_cmd->cmd, "run shell script fail, script exit code = [%d]", WEXITSTATUS(net_cmd->ret));
                net_cmd->ret = e_system_exec_fail;
            }
        }
        else
        {
            sprintf(net_cmd->cmd, "exit status = [%d]", WEXITSTATUS(net_cmd->ret));
            net_cmd->ret = e_system_exec_expection;
        }
    }

	return ;
}

/**
 * @brief 响应命令
 * 
 * @param sock 
 */
static void response_cmd(const int sock)
{
	struct netsystem_proxy_protocol net_cmd;
	char buf[300] = {0};

	memset(&net_cmd, 0, sizeof(net_cmd));
	memset(buf, 0, sizeof(buf));

	if (read(sock, buf, sizeof(buf)) < 0)
	{
		net_cmd.ret = e_system_errno;
		sprintf(net_cmd.cmd, "%s", strerror(errno));
	}
	else
	{
		memcpy(&net_cmd, buf, sizeof(net_cmd));

		// 执行命令
		net_cmd.ret = system(net_cmd.cmd);
		
		// 保存执行结果
		system_cmd_exec_result(&net_cmd);
	}

	// 相应命令请求
	int ret = write(sock, &net_cmd, sizeof(net_cmd));

	return ;
}

/**
 * @brief 等待连接
 * 
 */
static void wait_cmd_connect(void)
{
	int client_sockfd;
	struct sockaddr_un client_addr;
	socklen_t len = sizeof(client_addr);

	while (1)
	{
		printf("server waiting:\n");
		/* 接受一个客户端的链接 */
		client_sockfd = accept(_proxy_sock, (struct sockaddr *)&client_addr, &len);

		response_cmd(client_sockfd);

		/* 关闭socket */
		close(client_sockfd);
	}

	close(_proxy_sock);

	return ;
}

int main(void)
{
	if (!create_proxy_sock())
	{
		printf("create_proxy_sock err\n");
		return 0;
	}

    signal(SIGPIPE, SIG_IGN);

	wait_cmd_connect();

	return 0;
}
