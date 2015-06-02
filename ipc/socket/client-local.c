/*************************************************************************
	> File Name: client-local.c
	> Author: 
	> Mail: 
	> Created Time: 2015年01月31日 星期六 15时27分28秒
 ************************************************************************/

#include  <stdint.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <fcntl.h>
#include  <unistd.h>
#include  <error.h>
#include  <string.h>
#include  <sys/types.h>
#include  <netinet/in.h>
#include  <sys/socket.h>
#include  <sys/wait.h>
#include  <sys/un.h>
#include <arpa/inet.h>

#define       LINUX_DOMAIN    "./ipc-socket-domain"
#define       ipcSocketPrint(...)     printf(__VA_ARGS__)
#define       BUF_SIZE        ((uint16_t)2048)

static void ipcSocketClientLocal(int process);

int main(int arg, char **argv)
{
  uint16_t processNumber;
  uint16_t i;

 
  arg = arg;
  argv = argv;

  processNumber = 3;
  for (i = 0; i < processNumber; i++) {
    if (0 == fork()) {
      ipcSocketClientLocal(i);
    }
  }
  return 0;
}


static void ipcSocketClientLocal(int process)
{
  struct sockaddr_un serverAddress;
  int connectServer;
  int result;
  char buf[BUF_SIZE];
  uint16_t len;

  connectServer = socket(AF_LOCAL, SOCK_STREAM, 0);
  if (connectServer == -1) {
    ipcSocketPrint("fail to open socket");
    exit(1);
  }

  serverAddress.sun_family = AF_LOCAL;
  strcpy(serverAddress.sun_path, LINUX_DOMAIN);

  memset(buf, 0, BUF_SIZE);
  sprintf(buf, "Process: %d Hello Server!\n", process);
  result = connect(connectServer, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  if (result == -1) {
    ipcSocketPrint("fail to connectServer\n");
    exit(1);
  }
  write(connectServer, buf, strlen(buf));
  ipcSocketPrint("----Process: %d----\n", process);
  ipcSocketPrint("Send Message Len: %d\n%s\n", strlen(buf), buf);
  len = read(connectServer, buf, BUF_SIZE);
  ipcSocketPrint("Recv Message Len: %d\n%s\n", len, buf);
  close(connectServer);
  exit(0);
  return;
}

