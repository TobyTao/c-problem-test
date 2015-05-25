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
#include  <arpa/inet.h>


#define       LINUX_DOMAIN    "./ipc-socket-domain"
#define       ipcSocketPrint(...)     printf(__VA_ARGS__)
#define       BUF_SIZE        ((uint16_t)2048)

static void ipcSocketClientInet(void);

int main(int arg, char **argv)
{
  arg = arg;
  argv = argv;

  ipcSocketClientInet();
  return 0;
}


static void ipcSocketClientInet(void)
{
  int connectServer;
  int result;
  char buf[BUF_SIZE];
  uint16_t len;
  uint16_t i;
  struct sockaddr_in serverAddress;

  connectServer = socket(AF_INET, SOCK_STREAM, 0);
  if (connectServer == -1) {
    ipcSocketPrint("fail to open socket");
    exit(1);
  }

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port   = htons(3218);
  inet_aton("127.0.0.1", &serverAddress.sin_addr);

  for (i = 0; i < 3; i++) {
    if (0 == fork()) {
      usleep(10000);
      memset(buf, 0, BUF_SIZE);
      sprintf(buf, "Process: %d Hello Server!\n", i);
      result = connect(connectServer, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
      if (result == -1) {
        ipcSocketPrint("Process: %d fail to connectServer\n", i);
        exit(1);
      }

      write(connectServer, buf, strlen(buf));
      ipcSocketPrint("Send Message Len: %d\n%s\n", strlen(buf), buf);
      //len = read(connectServer, buf, BUF_SIZE);
      //ipcSocketPrint("Recv Message Len: %d\n%s\n", len, buf);
      close(connectServer);
      exit(0);
    }
  }

  wait(&result);
  return;
}
