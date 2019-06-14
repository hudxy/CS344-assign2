#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[]) {
  //Print error if not enough arguments are given
  if(argc < 2) {
    perror("Error: Not enough arguments.");
    return 1;
  }
  //seed random time
  srand(time(0));
  //Grab length of key from cmd line
  int keyLen = atoi(argv[1]);

  //create key buffer
  char* keyStr = (char*)malloc(keyLen * sizeof(char));
  memset(keyStr, '\0', keyLen * sizeof(char));
  int i;
  for(i = 0; i < keyLen; i++) {
    int randNum = rand() % 27;
    //get ASCII value
    randNum = randNum + 65;
    //Keep spaces the same
    if(randNum == 91) {
      randNum = 32;
    }
    keyStr[i] = (char)randNum;
  }
  //print key to output
  printf("%s\n", keyStr);

  //free memory
  free(keyStr);

  return 0;
}
