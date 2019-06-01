#ifndef __NETSYSTEM_PROXY_PROTOCOL_H__
#define __NETSYSTEM_PROXY_PROTOCOL_H__

#define NETSYSTEM_SERVICE_PORT 6010	// 服务端口
#define NETSYSTEM_PROXY_AF_UNIX_NODE "/tmp/netsystem" // AF_UNIX 使用的文件节点

#define MAX_SYSTEM_CMD_LEN 254 // 最长系统命令

struct netsystem_proxy_protocol
{
	char cmd[MAX_SYSTEM_CMD_LEN+1]; // 系统命令 留一字节放'\0'
	int ret; // 执行结果
};

#endif // endof : #ifndef __NETSYSTEM_PROXY_PROTOCOL_H__
