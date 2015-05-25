/*************************************************************************
	> File Name: ipc-fifo-recv.c
	> Author: 
	> Mail: 
	> Created Time: 2015年01月31日 星期六 10时33分27秒
 ************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "ipc-fifo-types.h"

#define ipcPrintf(...)  printf(__VA_ARGS__)

static uint8_t ttMessageHandler(TtMessage *message);

int main(int arg, char **argv)
{
  int ttReadFifo;
  int result;
  TtMessage message;
  
  arg = arg;
  argv = argv;

  result = mkfifo(TT_FIFO_SERVER, TT_FILE_MODE);
  if (result < 0) {
    ipcPrintf("fifo is existing\n");
  }

  ipcPrintf("Start open fifo\n");
  ttReadFifo = open(TT_FIFO_SERVER, O_RDONLY);
  open(TT_FIFO_SERVER, O_WRONLY);
 
  ipcPrintf("Start ipc fifo recv:\n");
  while (1) {
    uint16_t len;
    
    len = read(ttReadFifo, &message, TT_MESSAGE_HEAD);
    if (len != TT_MESSAGE_HEAD) {

    }
    ipcPrintf("Recv messageLen: %d\n", message.messageLen);
    /*message.messageLen = message.messageLen < TT_MESSAGE_SIZE? message.messageLen: TT_MESSAGE_SIZE;*/
    len = read(ttReadFifo, message.message, message.messageLen);
    message.message[len] = '\0';

    ttMessageHandler(&message);
  }

  unlink(TT_FIFO_SERVER);
  return 0;
}

static uint8_t ttMessageHandler(TtMessage *message)
{
  uint16_t len;
  uint8_t  *p;
  char      clientFifoName[32];
  int       clientFifo;

  p = message->message;
  while (*p != '#' && *p != '\0') {
    p++;
  }
  if (*p != '\0') {
    *p = '\0';
  } else {
    return FALSE;
  }

  memset(clientFifoName, 0, 32);
  sprintf(clientFifoName, "%s", message->message);
  ipcPrintf("clientFifoName:%s\n", clientFifoName);

  {
    uint16_t len;
    uint16_t i;
    p++;
    len = message->messageLen - (p - message->message);
  
    message->messageLen  = len;
    printf("Len: %d\n", len);
    printf("/*****Recv Data*****/\n");
    for (i = 0; i < len; i++) {
      printf("%02x ", p[i]);
      message->message[i] = p[i];
    }
    printf("\nEnd\n");
  }

 
  usleep(1000*2);

  clientFifo     = open(clientFifoName, O_WRONLY | O_NONBLOCK);
  if (clientFifo < 0) {
    ipcPrintf("Fail to open client fifo\n");
    ipcPrintf("errno:%d\n", errno);
    return FALSE;
  }

  //memset(message->message, 0, TT_MESSAGE_SIZE);
  /*message->messageLen  = sprintf(message->message, "I have receive your message");*/
  /*message->messageLen  = sprintf(message->message, "12");*/
  message->messageType = 1;
  len = message->messageLen + TT_MESSAGE_HEAD;
  write(clientFifo, message, len);

  close(clientFifo);
  return TRUE;
}

