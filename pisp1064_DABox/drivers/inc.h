/*
 * inc.h
 *
 *  Created on: 2016-4-22
 * Corporation: NanJingQingJin
 *      Author: wanghb
 * Description: ����ͨ�ýṹ��������������Դ
 */

#ifndef INC_H_
#define INC_H_

#include <stdio.h>

#define PROJECT_DEBUG

//����ȫ�ִ�ӡ��Ϣ,������Կ��عر�ʱ��Ҫ��Ϣ�޷���ӡ
#define DEBUG_GLOBAL
#define ERROR_GLOBAL

#ifdef DEBUG_GLOBAL
#define debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...)
#endif /* MYSQL_DEBUG */




#endif /* INC_H_ */
