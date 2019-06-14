#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char* argv[]) {
  FILE* fp = fopen(argv[1], "r");
  if (fp == NULL) {
    perror("Failed: ");
    return 1;
  }
  fseek(fp, 0, SEEK_END);
  //get length of file
  int length = ftell(fp);
  printf("%d\n", length);

  fclose(fp);
  return 0;
}
