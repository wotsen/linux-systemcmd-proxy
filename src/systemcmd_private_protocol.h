/**
 * @file systemcmd_private_protocol.h
 * @author yuwangliang (astralrovers@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2020-01-26
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#pragma once
namespace wotsen
{

#define NETSYSTEM_SERVICE_PORT 6010	// 服务端口
#define NETSYSTEM_PROXY_AF_UNIX_NODE "/tmp/netsystem" // AF_UNIX 使用的文件节点

#define MAX_SYSTEM_CMD_LEN 254 // 最长系统命令

#define SYSTEMCMD_PROXY_VERSION "1.0.0"

/**
 * @brief 内部执行结果返回码
 */
enum e_systemcmd_proxy_cmd_ret
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

} // namespace wotsen
