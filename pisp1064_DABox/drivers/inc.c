#include "inc.h"
#include "do_data.h"
#include <stdio.h>

#ifdef PROJECT_DEBUG
#if 0
#define INC_DEBUG
#endif
#endif /* PROJECT_DEBUG */

#define INC_ERROR

#ifdef INC_DEBUG
#define inc_debug(fmt, ...) rt_kprintf("[INC][%d]"fmt"\n", __LINE__, ##__VA_ARGS__)
#else
#define inc_debug(fmt, ...)
#endif /* INC_DEBUG */

#ifdef INC_ERROR
#define inc_error(fmt, ...) rt_kprintf("[INC][ERR][%d]"fmt"\n", __LINE__, ##__VA_ARGS__)
#else
#define inc_error(fmt, ...)
#endif /* INC_ERROR */








