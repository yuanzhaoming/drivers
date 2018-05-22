#include "do_data.h"
#include "inc.h"
#include "protocol.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h> //atoi()

#ifdef PROJECT_DEBUG
#if 0
#define DO_DEBUG
#endif
#endif /* PROJECT_DEBUG */

#define DO_ERROR

#ifdef DO_DEBUG
#define do_debug(fmt, ...) rt_kprintf("[DO][%d]"fmt"\n", __LINE__, ##__VA_ARGS__)
#else
#define do_debug(fmt, ...)
#endif /* DO_DEBUG */

#ifdef DO_ERROR
#define do_error(fmt, ...) rt_kprintf("[DO][ERR][%d]"fmt"\n", __LINE__, ##__VA_ARGS__)
#else
#define do_error(fmt, ...)
#endif /* DO_ERROR */

#define CMD_DATA_LINE_SIZE	256		//命令配置文件一行最大长度





