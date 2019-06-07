#ifndef __NETSYSTEM_PROXY_PROTOCOL_H__
#define __NETSYSTEM_PROXY_PROTOCOL_H__

#define NETSYSTEM_SERVICE_PORT 6010	// 服务端口
#define NETSYSTEM_PROXY_AF_UNIX_NODE "/tmp/netsystem" // AF_UNIX 使用的文件节点

#define MAX_SYSTEM_CMD_LEN 254 // 最长系统命令

/**
 * @brief 内部执行结果返回码
 */
enum e_netsystem_cmd_ret
{
    e_system_err = -1,           // 系统调用错误
    e_system_exec_ok = 0,        // 执行成功
    e_system_exec_fail = 1,      // 执行错误
    e_system_exec_expection = 2, // 执行异常
    e_system_errno = 0xff,       // 来自errno的错误
};

struct netsystem_proxy_protocol
{
	char cmd[MAX_SYSTEM_CMD_LEN+1]; // 系统命令 留一字节放'\0'
	int ret; // 执行结果
};

#endif // endof : #ifndef __NETSYSTEM_PROXY_PROTOCOL_H__
