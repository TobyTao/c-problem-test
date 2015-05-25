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
#include  <netinet/in.h>
#include  <sys/socket.h>
#include  <sys/wait.h>
#include  <arpa/inet.h>

#define   BUF_SIZE                ((uint16_t)2048)
#define   ipcSocketPrint(...)     printf(__VA_ARGS__)

static void ipcSocketServerInet(void);

int main(int arg, char **argv)
{
  arg = arg;
  argv = argv;

  ipcSocketServerInet();
  return 0;
}

static void ipcSocketServerInet(void)
{
  int server, client;
  int result;
  uint16_t len;
  uint8_t buf[BUF_SIZE];
  struct sockaddr_in clientAddress;
  struct sockaddr_in serverAddress;

  server = socket(AF_INET, SOCK_STREAM, 0);
  if (server == -1) {
    ipcSocketPrint("Fail to open socket\n");
    exit(1);
  }
  
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port   = htons(3218);
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  bzero(&(serverAddress.sin_zero), 8);

  result = bind(server, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr));
  if (result == -1) {
    ipcSocketPrint("fail to bind server socket");
    exit (1);
  }

  result = listen(server, 10);
  if (result == -1) {
    ipcSocketPrint("fail to listen");
    exit(1);
  }

  while (1) {
    socklen_t size;
    size = sizeof(struct sockaddr_in);
    client = accept(server, (struct sockaddr *)&clientAddress, &size);
    if (client == -1) {
      ipcSocketPrint("Fail to accept\n");
      exit(1);
    }
    if (0 == fork()) {
      ipcSocketPrint("Accept Success\n");
      memset(buf, 0, BUF_SIZE);
      len = read(client, buf, BUF_SIZE);
      ipcSocketPrint("Recv Message length: %d \n%s\n", len, buf);
      //write(client, "Hello Client, nice to meet you", 30);
      close(client);
      exit(0);
    }
    close(client);
  }
  close(server);
  wait(&result);
}

