/*************************************************************************
	> File Name: ipc-share-memory-send.c
	> Author: 
	> Mail: 
	> Created Time: 2015年01月31日 星期六 10时17分35秒
 ************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <errno.h>
#include "ipc-fifo-types.h"

#define INPUT_FILE    "./input_fifo.txt"
#define BUF_SIZE      ((uint16_t)2024)
#define PROCESS_NUMBER ((uint16_t)1)

#define ipcPrintf(...)  printf(__VA_ARGS__)

int ipcWriteForLua(const uint8_t *data, const int dataSize);
int ipcReadForLua(uint8_t *data, int *dataSize);
void ipcTimeoutSet(int time);

static int ipcRead(int fd, TtMessage *message);
static int ipcWrite(int fd, TtMessage *message);
static uint8_t ipcFifoTest(void);
int main(int arg, char **argv)
{
  int status;
  int processNumber;
  uint8_t i;

  if (arg == 2) {
    ipcPrintf("processNumber: %s\n", argv[1]);
    processNumber = atol(argv[1]);
  } else {
    processNumber = PROCESS_NUMBER;
  }

  for (i = 0; i < processNumber; i++) {
    if (0 == fork()) {
      ipcPrintf("process: %d    ", i);
      ipcFifoTest();
      exit(0);
    }
  }

  for (i = 0; i < PROCESS_NUMBER; i++) {
    wait(&status);
  }
  return 0;
}

// Test Function
static uint8_t ipcFifoTest(void)
{
  int inputFile;
  int len;
  uint8_t fileBuf[1024];

  inputFile = open(INPUT_FILE, O_RDONLY);
  if (inputFile == -1) {
    ipcPrintf("faile to open file\n");
    exit(1);
  }
  len = read(inputFile, fileBuf, 1000);

  ipcTimeoutSet(1000);

  if (TRUE == ipcWriteForLua(fileBuf, len)) {
    memset(fileBuf, 0, 1024);
    ipcReadForLua(fileBuf, &len);
    ipcPrintf("fileBuf Len: %d, %s\n", len, fileBuf);
  }

  close(inputFile);
  return 0;
}

// First version for Lua's IPC
char  fifoClientName[32];
int timeout;
static int ipcRead(int fd, TtMessage *message)
{
  uint16_t len;

  memset(message->message, 0, TT_MESSAGE_SIZE);
  len = read(fd, message, TT_MESSAGE_HEAD);
  if (len != TT_MESSAGE_HEAD) {
    ipcPrintf("Fail to Read fifoRead\n");
    exit(1);
  }

  message->messageLen = message->messageLen < TT_MESSAGE_SIZE? message->messageLen: TT_MESSAGE_SIZE;

  len = read(fd, message->message, message->messageLen);
  ipcPrintf("Read len: %d, Messagelen: %d\n", len, message->messageLen);
  if (len != message->messageLen) {
    ipcPrintf("Fail to Read Message\n");
    exit(1);
  }
  message->message[len] = '\0';
  return len + 1;
}

static int ipcWrite(int fd, TtMessage *message)
{
  uint16_t len;
  uint16_t sendLen;
  len = message->messageLen + TT_MESSAGE_HEAD;
  sendLen = write(fd, message, len);
  return (sendLen == len? TRUE: FALSE);
}

int ipcWriteForLua(const uint8_t *data, const int dataSize)
{
  uint8_t *p;
  int fifoWrite;
  int result;
  int   pid;
  TtMessage message;

  memset(message.message, 0, TT_MESSAGE_SIZE);
  memset(fifoClientName, 0, 32);
  pid = getpid();
  sprintf(fifoClientName, "%s%d", TT_FIFO_CLIENT, pid);
  result = mkfifo(fifoClientName, TT_FILE_MODE);
  if (result < 0) {
    ipcPrintf("Fail to mkfifo\n");
    return FALSE;
  }

  fifoWrite = open(TT_FIFO_SERVER, O_WRONLY | O_NONBLOCK);
  if (fifoWrite == -1) {
    ipcPrintf("fail to open fifo\n");
    exit(1);
  }

  message.messageLen = sprintf(message.message, "%s#", fifoClientName);
  p = &message.message[message.messageLen];
  memcpy(p, data, dataSize);
  message.messageLen += dataSize;

  result = ipcWrite(fifoWrite, &message);
  close(fifoWrite);
  if (result == FALSE) {
    ipcPrintf("Fail to Write, errno: %d\n", errno);
    return FALSE;
  }
  return result;
}

int ipcReadForLua(uint8_t *data, int *dataSize)
{
  TtMessage message;
  int   fifoRead;
  int   epollFd;
  int   result;
  struct epoll_event event;

  fifoRead = open(fifoClientName, O_RDONLY | O_NONBLOCK);
  if (fifoRead < 0) {
    ipcPrintf("Fail to open fifoClientName\n");
    exit(1);
  }

  event.events = EPOLLIN | EPOLLET;
  event.data.fd = fifoRead;
  epollFd = epoll_create(1);
  epoll_ctl(epollFd, EPOLL_CTL_ADD, fifoRead, &event);

  result = epoll_wait(epollFd, &event, 1, timeout);
  if (0 < result && event.data.fd == fifoRead) {
    ipcRead(fifoRead, &message);
    memcpy(data, message.message, message.messageLen);
    *dataSize = message.messageLen;
    result = TRUE;
  } else {
    *dataSize = sprintf(data, "recv timeout");
    result = FALSE;
  }

  close(fifoRead);
  unlink(fifoClientName);
  return result;
}

void ipcTimeoutSet(int time)
{
  timeout = time;
}

