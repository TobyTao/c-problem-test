/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 2015年01月31日 星期六 14时56分15秒
 ************************************************************************/

#include  <stdint.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <fcntl.h>
#include  <unistd.h>
#include  <error.h>
#include  <string.h>
#include  <sys/types.h>
//#include  <netinet/in.h>
#include  <sys/socket.h>
#include  <sys/wait.h>
#include  <sys/un.h>

#define   BUF_SIZE                ((uint16_t)2048)
#define   LINUX_DOMAIN            "./ipc-socket-domain" 
#define   ipcSocketPrint(...)     printf(__VA_ARGS__)

static void ipcSocketServerLocal(void);

int main(int arg, char **argv)
{
  arg = arg;
  argv = argv;

  ipcSocketServerLocal();
  return 0;
}

static void ipcSocketServerLocal(void)
{
  socklen_t addressLen;
  int server, client;
  int result;
  uint16_t len;
  uint8_t buf[BUF_SIZE];

  struct sockaddr_un clientAddress;
  struct sockaddr_un serverAddress;

  server = socket(AF_LOCAL, SOCK_STREAM, 0);
  if (server == -1) {
    ipcSocketPrint("Fail to open socket\n");
    exit(1);
  }
  
  serverAddress.sun_family = AF_LOCAL;
  strcpy(serverAddress.sun_path, LINUX_DOMAIN);
  unlink(LINUX_DOMAIN);
  result = bind(server, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  if (result == -1) {
    ipcSocketPrint("fail to bind server socket");
    exit (1);
  }

  result = listen(server, 100);
  if (result == -1) {
    ipcSocketPrint("fail to listen");
    exit(1);
  }
  while (1) {
    socklen_t size;
    memset(buf, 0, BUF_SIZE);
    size = sizeof(struct sockaddr_un);
    client = accept(server, (struct sockaddr *)&clientAddress, &size);
    if (client == -1) {
      continue;
    }
    if (0 == fork()) {
      len = read(client, buf, BUF_SIZE);
      ipcSocketPrint("Recv Message length: %d \n%s\n", len, buf);
      write(client, "Hello Client, nice to meet you", 30);
      close(client);
      exit(0);
    }
    close(client);
  }
  close(server);
  unlink(LINUX_DOMAIN);
}

