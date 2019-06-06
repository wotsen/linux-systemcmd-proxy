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

void reset_netsystem_cmd_error_code(void)
{
    sprintf(_error_code, "ok");
}

char * get_netsystem_cmd_error_code(void)
{
    return _error_code;
}

static void close_proxy_sock(void)
{
	if (_INVALID_NET_FD != _proxy_sock)
	{
		close(_proxy_sock);
		_proxy_sock = _INVALID_NET_FD;
	}

	return ;
}

static enum e_netsystem_result create_proxy_sock(void)
{
	// 建立过了就不再建立了
	if (_INVALID_NET_FD != _proxy_sock)
	{
		return e_netsystem_ok;
	}

	int reuse = 1;
	struct sockaddr_un address;
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

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
		case -1:
			return e_netsystem_system_err;
			break;
		case 0:
			return e_netsystem_ok;
			break;
		case 1:
			return e_netsystem_exec_fail;
			break;
		case 2:
			return e_netsystem_child_pross;
			break;
		case 0xff:
			return e_netsystem_error;
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
    printf("执行结果：%d %s\n", net_cmd.ret, _error_code);

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

