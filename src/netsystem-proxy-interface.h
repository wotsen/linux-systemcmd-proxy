#ifndef __NETSYSTEM_PROXY_INTERFACE_H__
#define __NETSYSTEM_PROXY_INTERFACE_H__

enum e_netsystem_result
{
	e_netsystem_ok,					// 成功
	e_netsystem_sock_err,			// 本地套接字异常
	e_netsystem_send_err,			// 发送异常
	e_netsystem_srv_not_exist,		// 服务不存在
	e_netsystem_cmd_toolong,		// 命令过长
	e_netsystem_recv_err,			// 接受数据错误
	e_netsystem_unknow,				// 未知命令
};

enum e_netsystem_result net_system_cmd_proxy(const char *cmd);

#endif // endof : #ifndef __NETSYSTEM_PROXY_INTERFACE_H__
