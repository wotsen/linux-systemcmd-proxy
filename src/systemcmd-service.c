// TODO : 网络linux系统命令执行代理，减小开销
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

#include "netsystem-proxy-protocol.h"

#define _INVALID_NET_FD -1 // 无效网络套接字句柄

static int _proxy_sock = _INVALID_NET_FD; // 代理套接字句柄

static bool create_proxy_sock(void);
static void wait_cmd_connect(void);
static void response_cmd(int sock);

static bool create_proxy_sock(void)
{
	// 建立过了就不再建立了
	if (_INVALID_NET_FD != _proxy_sock)
	{
		return true;
	}

	int reuse = 1;
	struct sockaddr_un address;
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	memset(&address, 0, sizeof(address));

	if (sockfd < 0)
	{
		return false;
	}

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, NETSYSTEM_PROXY_AF_UNIX_NODE);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse) < 0)
	{
		perror("Setsock failed");
		close(sockfd);
		return false;
	}

	unlink(NETSYSTEM_PROXY_AF_UNIX_NODE);

	if (bind(sockfd, (struct sockaddr *)&address, (socklen_t)sizeof(address)) < 0)
	{
		perror("Bind failed");
		close(sockfd);
		unlink(NETSYSTEM_PROXY_AF_UNIX_NODE);
		return false;
	}

	/* 监听 */
	if (listen(sockfd, 5) < 0)
	{
		perror("Listen failed");
		close(sockfd);
		unlink(NETSYSTEM_PROXY_AF_UNIX_NODE);
		return false;
	}

	_proxy_sock = sockfd;

	return true;
}

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

static void response_cmd(int sock)
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

		net_cmd.ret = system(net_cmd.cmd);
		
		system_cmd_exec_result(&net_cmd);
	}

	write(sock, &net_cmd, sizeof(net_cmd));

	return ;
}

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
