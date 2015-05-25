/*************************************************************************
  > File Name: ipc-share-memory.c
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
#include "ipc-fifo-types.h"
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"

//#define ipcPrintf(...)  printf(__VA_ARGS__)
#define ipcPrintf(...) 

extern int lua_recv_fifo(lua_State *L);
extern int lua_send_fifo(lua_State *L);
extern int luaopen_fifo(lua_State *L);

static const struct luaL_reg luafifolib[]={
  {"write",lua_send_fifo},
  {"read",lua_recv_fifo},
  {NULL,NULL}
};


static int ipcWriteForLua(const uint8_t *data, const int dataSize);
static int ipcReadForLua(uint8_t *data, int *dataSize,int timeout);

static int ipcRead(int fd, TtMessage *message);
static int ipcWrite(int fd, TtMessage *message);

// First version for Lua's IPC
char  fifoClientName[32];
static int ipcRead(int fd, TtMessage *message)
{
  uint16_t len;

  memset(message->message, 0, TT_MESSAGE_SIZE);
  len = read(fd, message, TT_MESSAGE_HEAD);
  if (len != TT_MESSAGE_HEAD) {
    ipcPrintf("Fail to Read fifoRead\n");
    exit(1);
  }

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

  len = message->messageLen + TT_MESSAGE_HEAD;
  // When the data lenght is greater than 512, 
  // the write action will be atom operation.
  len = len < 512? 512: len;
  return write(fd, message, len);
}

static int ipcWriteForLua(const uint8_t *data, const int dataSize)
{
  int fifoWrite;
  int result;
  uint8_t *p;
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

  fifoWrite = open(TT_FIFO_SERVER, O_WRONLY);
  if (fifoWrite == -1) {
    ipcPrintf("fail to open fifo\n");
    exit(1);
  }

  message.messageLen = sprintf(message.message, "%s#", fifoClientName);
  p = &message.message[message.messageLen];
  memcpy(p, data, dataSize);
  message.messageLen += dataSize;

  ipcWrite(fifoWrite, &message);

  close(fifoWrite);
  return TRUE;
}

static int ipcReadForLua(uint8_t *data, int *dataSize,int timeout)
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
    //*dataSize = sprintf(data, "recv timeout");
    result = FALSE;
  }

  close(fifoRead);
  unlink(fifoClientName);
  return result;
}

int lua_send_fifo(lua_State *L)
{
  int len=luaL_checkint(L,2);
  char *buffer=luaL_checkstring(L,1);

  if(0==len) {
    lua_pushnumber(L,1);
    return 1;
  }

  if(NULL==buffer) {
    lua_pushnumber(L,2);
    return 1; 
  }

  if(TRUE==ipcWriteForLua(buffer,len)) { 
    lua_pushnumber(L,0); 
  } else { 
    lua_pushnumber(L,3);
  } 

  return 1;
}

int lua_recv_fifo(lua_State *L)
{

  int   len=0;
  char  buffer[1024];
  int timeout=luaL_checkint(L,1);

  if(TRUE==ipcReadForLua(buffer,&len,timeout)) { 
    lua_pushnumber(L,0);
    lua_pushstring(L,buffer);
    lua_pushnumber(L,len); 
    return 3;
  }

  lua_pushnumber(L,3); 
  return 1;  
}

int luaopen_luafifo(lua_State *L)
{
  luaL_register(L,"luafifo",luafifolib);
  return 1;
}

