/*************************************************************************
	> File Name: ipc-fifo-types.h
	> Author: 
	> Mail: 
	> Created Time: 2015年02月02日 星期一 10时53分55秒
 ************************************************************************/

#ifndef _IPC_FIFO_TYPES_H
#define _IPC_FIFO_TYPES_H
// Include Headers
#include <stdint.h>
// Define Macros
#define TT_FILE_MODE        (0644)
#define TT_FIFO_SERVER      "/tmp/fifo-server"
#define TT_FIFO_CLIENT      "/tmp/fifo-client."
#define TT_MESSAGE_HEAD     ((uint8_t)4)
#define TT_MESSAGE_SIZE     ((uint16_t)2048 - 1)

#undef  TRUE
#define TRUE                ((uint8_t)1)

#undef  FALSE
#define FALSE               ((uint8_t)0)
// Typedef
typedef struct {
  uint16_t  messageLen;
  uint16_t  messageType;
  uint8_t   message[TT_MESSAGE_SIZE + 1];
}TtMessage;
#endif
