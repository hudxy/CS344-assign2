//Hudson Dean
//Client Code For Encryption
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
  if (argc < 4) { fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); exit(0); } // Check usage & args

  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  struct hostent* serverHostInfo;

  // Set up the server address struct
  memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
  portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
  serverAddress.sin_family = AF_INET; // Create a network-capable socket
  serverAddress.sin_port = htons(portNumber); // Store the port number
  serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
  if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
  memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address


  //open plaintext file
  FILE* fp = fopen(argv[1], "r");
  if (fp == NULL) {
    perror("Failed: ");
    return 1;
  }
  fseek(fp, 0, SEEK_END);
  //get length of file
  int length = ftell(fp);
  //point file pointer back to beginning
  fseek(fp, 0, 0);

  //clear plaintext buffer
  char plaintext[length];
  memset(plaintext, '\0', length);
  //read plaintext into buffer
  fgets(plaintext, length, fp);

  //close plaintext file
  fclose(fp);

  //open key file
  fp = fopen(argv[2], "r");
  if (fp == NULL) {
    perror("Failed: ");
    return 1;
  }

  //clear key buffer
  char key[length];
  memset(key, '\0', length);
  //read key into buffer
  fgets(key, length, fp);

  //close key file
  fclose(fp);

  //clear and prepare buffer for sending
  char buffer[length];
  memset(buffer, '\0', length);
  sprintf(buffer, "len%d", length);
  //display error if plaintext file contains invalid ASCII char codes
  int i;
  for (i = 0; i < strlen(plaintext); i++) {
    if (plaintext[i] != 32 && (plaintext[i] < 65 || plaintext[i] > 90)) {
      perror("ERROR: Incompatible plaintext character");
      return 1;
    }
  }
  //display error if key file and cipher file are not of same length
  if (strlen(key) != strlen(plaintext)) {
    perror("ERROR: Keyfile has incompatible length");
    return 1;
  }

  // Set up the socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
  if (socketFD < 0) error("CLIENT: ERROR opening socket");

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
  error("CLIENT: ERROR connecting");


  //send verification to server
  charsWritten = send(socketFD, buffer, strlen(buffer)+1, 0); // Write to the server
  if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
  if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

  //get return message from server
  memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
  charsRead = recv(socketFD, buffer, 39, 0); // Read data from the socket, leaving \0 at end
  if (charsRead < 0) error("CLIENT: ERROR reading from socket");
  //if error message recieved from server, return
  if (strncmp(buffer, "END", 3) == 0) {
    return 1;
  }

  //send plaintext to server
  charsWritten = send(socketFD, plaintext, length, 0); // Write to the server
  if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
  if (charsWritten < length) printf("CLIENT: WARNING: Not all data written to socket!\n");


  //get return message from server
  memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
  charsRead = recv(socketFD, buffer, 39, 0); // Read data from the socket, leaving \0 at end
  if (charsRead < 0) error("CLIENT: ERROR reading from socket");

  //send key to server
  charsWritten = send(socketFD, key, length, 0); // Write to the server
  if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
  if (charsWritten < length) printf("CLIENT: WARNING: Not all data written to socket!\n");


  //get return message from server
  char temp[length];
  memset(temp, '\0', sizeof(temp));
  int accumulator = 0;
  while(accumulator != length) {
    memset(temp, '\0', sizeof(temp)); // Clear out the buffer again for reuse
    charsRead = recv(socketFD, temp, sizeof(temp), 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    //print buffer to stdout
    printf("%s", temp);
    fflush(stdin);
    //count characters read to ensure total characters sent from server are recieved
    accumulator += charsRead;
  }
  //print newline to stdout
  printf("\n");
  close(socketFD); // Close the socket
  return 0;
}
