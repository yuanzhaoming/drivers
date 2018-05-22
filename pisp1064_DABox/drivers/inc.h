/*
 * inc.h
 *
 *  Created on: 2016-4-22
 * Corporation: NanJingQingJin
 *      Author: wanghb
 * Description: 定义通用结构、变量或其他资源
 */

#ifndef INC_H_
#define INC_H_

#include <stdio.h>

#define PROJECT_DEBUG

//定义全局打印信息,避免调试开关关闭时必要信息无法打印
#define DEBUG_GLOBAL
#define ERROR_GLOBAL

#ifdef DEBUG_GLOBAL
#define debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...)
#endif /* MYSQL_DEBUG */




#endif /* INC_H_ */
