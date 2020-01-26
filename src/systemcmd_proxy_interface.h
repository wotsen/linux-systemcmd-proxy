/**
 * @file systemcmd_proxy_interface.h
 * @author yuwangliang (astralrovers@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2020-01-25
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#pragma once

namespace wotsen
{

/**
 * @brief 系统命令代理
 * 
 * @param cmd 命令字符串
 * @return true 成功
 * @return false 失败
 */
bool systemcmd_proxy(const char *cmd);

/**
 * @brief Get the systemcmd proxy version object
 * 
 * @return const char* 
 */
const char *get_systemcmd_proxy_version(void);

} // namespace wotsen
