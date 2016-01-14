/*************************************************************************
	> File Name: hex2ascii.c
	> Author: 
	> Mail: 
	> Created Time: 2015年05月22日 星期五 16时44分56秒
 ************************************************************************/

#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static uint8_t  getNumber(char data);

#define FILE_NAME  "./hex.txt"
#define BUF_SIZE  (uint32_t) 1024
int main(int argc, char **argv)
{
  uint32_t i;
  uint32_t len;
  uint32_t tmp;
  uint8_t *file;
  uint8_t *input;
  struct stat sb;
  int fd;
  int result;

  fd = open(FILE_NAME, O_RDWR);
  if (fd < 0) {
    perror("Fail to open file");
    exit(0);
  }

  result = fstat(fd, &sb);
  if (result < 0) {
    perror("Fail to get file state");
    exit(0);
  }

  file = (uint8_t *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (file == (void *)-1) {
    perror("Fail to mmap");
    exit(0);
  }
  close(fd);

  i = 0;
  tmp = 0;
  input = file;
  //printf("%s\n", input);
  printf("/* START to Print */\n");
  while(input[i] != 0 && i < sb.st_size) {
    uint8_t j;
    j = getNumber(input[i]);
    /*printf("%02x\n", j);*/
    if (j != 0xFF) {
      tmp <<= 4;
      tmp |= j;
    } else {
      printf("%c", tmp);
      tmp = 0;
    }
    i++;
  }
  printf("\n/* End */\n");
  result = munmap((void *)file, sb.st_size);
  if (result < 0) {
    perror("Fail to munmap");
    exit(0);
  }
  return 0;
}

static uint8_t  getNumber(char data)
{
  if ('0' <= data && data <= '9') {
    return data - '0';
  } 

  if ('a' <= data && data <= 'f') {
    return data - 'a' + 10;
  } 

  if ('A' <= data && data <= 'F') {
    return data - 'A' + 10;
  } 

  return 0xFF;
}
// just for test
