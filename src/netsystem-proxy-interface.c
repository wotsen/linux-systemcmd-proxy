#include <stdio.h>
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

static bool create_proxy_sock(void);
static void close_proxy_sock(void);
static bool send_cmd(const char *cmd);
static enum e_netsystem_result get_proxy_result(void);
static enum e_netsystem_result retval_to_result(int retval);

static void close_proxy_sock(void)
{
	if (_INVALID_NET_FD != _proxy_sock)
	{
		close(_proxy_sock);
		_proxy_sock = _INVALID_NET_FD;
	}

	return ;
}
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

	if (sockfd < 0)
	{
		return false;
	}

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, NETSYSTEM_PROXY_AF_UNIX_NODE);

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);

	if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		close(sockfd);
		return false;
	}

	_proxy_sock = sockfd;

	return true;
}

static bool send_cmd(const char *cmd)
{
	struct netsystem_proxy_protocol net_cmd; // 协议命令

	memset(&net_cmd, 0, sizeof(net_cmd));

	sprintf(net_cmd.cmd, "%s", cmd);
	printf("cmd : %s\n", net_cmd.cmd);

	return write(_proxy_sock, &net_cmd, sizeof(net_cmd)) == sizeof(net_cmd);
}

static enum e_netsystem_result retval_to_result(int retval)
{
	switch (retval)
	{
		case 0:
			return e_netsystem_ok;
			break;
		default:
			break;
	}

	return e_netsystem_unknow;
}

static enum e_netsystem_result get_proxy_result(void)
{
	struct netsystem_proxy_protocol net_cmd; // 协议命令
	char buf[300] = {0};
	int len = 0;

	memset(&net_cmd, 0, sizeof(net_cmd));
	memset(buf, 0, sizeof(buf));

	if (sizeof(net_cmd) != read(_proxy_sock, buf, sizeof(buf)))
	{
		printf("recv error\n");
		close_proxy_sock();
		return e_netsystem_recv_err;
	}

	memcpy(&net_cmd, buf, sizeof(net_cmd));

	printf("执行结果：%d %s\n", net_cmd.ret, net_cmd.cmd);

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
	if (strlen(cmd) > MAX_SYSTEM_CMD_LEN)
	{
		return e_netsystem_cmd_toolong;
	}

	if (!create_proxy_sock())
	{
		return e_netsystem_sock_err;
	}

	if (!send_cmd(cmd))
	{
		close_proxy_sock();
		return e_netsystem_send_err;
	}

	return get_proxy_result();
}

int main(int argc, char **argv)
{
	if (2 != argc)
	{
		printf("use age:\n");
		printf("\t%s cmd\n", argv[0]);
		return 0;
	}

	net_system_cmd_proxy(argv[1]);

	return 0;
}
