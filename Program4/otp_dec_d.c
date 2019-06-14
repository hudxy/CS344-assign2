//Hudson Dean
//Server Code For Decryption
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues
void decryption(char* cipher, char* key, int length);

int main(int argc, char *argv[])
{
  int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
  socklen_t sizeOfClientInfo;
  char buffer[256];
  struct sockaddr_in serverAddress, clientAddress;

  if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

  // Set up the address struct for this process (the server)
  memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
  portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
  serverAddress.sin_family = AF_INET; // Create a network-capable socket
  serverAddress.sin_port = htons(portNumber); // Store the port number
  serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

  // Set up the socket
  listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
  if (listenSocketFD < 0) error("ERROR opening socket");

  // Enable the socket to begin listening
  if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
  error("ERROR on binding");
  listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

  // Always listen for new connections
  while(1) {

    // Accept a connection, blocking if one is not available until one connects
    sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
    establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
    if (establishedConnectionFD < 0) error("ERROR on accept");

    //spawn child to decrypt message
    pid_t child = fork();
    if(child == -1) {
      perror("Hull Breach!");

    } else if (child == 0) {

      //get the verification from the client and parse it for information
      memset(buffer, '\0', 256);
      charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket
      if (charsRead < 0) error("ERROR reading from socket");
      if (strncmp(buffer, "lan", 3) == 0) {

        //get size of incoming message
        int length = atoi(&buffer[3]);

        //send a Success message back to the client
        charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
        if (charsRead < 0) error("ERROR writing to socket");

        //setup variables
        char cipher[length];
        char key[length];
        char ctemp[length+1];
        char ktemp[length+1];
        memset(cipher, '\0', length);
        memset(key, '\0', length);
        memset(ctemp, '\0', length+1);
        memset(ktemp, '\0', length+1);

        //recieve cipher
        int accumulator = 0;
        while(accumulator != length) {
          memset(cipher, '\0', length);
          charsRead = recv(establishedConnectionFD, cipher, length-1, 0); // Read the client's message from the socket
          if (charsRead < 0) error("ERROR reading from socket");
          //store message in buffer in case file is too big to send over 1 TCP packet
          sprintf(ctemp, "%s%s", ctemp, cipher);
          //count characters read to ensure total characters sent from client are recieved
          accumulator = charsRead + accumulator;
        }

        //send a Success message back to the client
        charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
        if (charsRead < 0) error("ERROR writing to socket");

        //recieve key
        accumulator = 0;
        while(accumulator != length) {
          memset(key, '\0', length);
          charsRead = recv(establishedConnectionFD, key, length-1, 0); // Read the client's message from the socket
          if (charsRead < 0) error("ERROR reading from socket");
          //store message in buffer in case file is too big to send over 1 TCP packet
          sprintf(ktemp, "%s%s", ktemp, key);
          //count characters read to ensure total characters sent from client are recieved
          accumulator = charsRead + accumulator;
        }

        //send text to be decrypted (will return decryption in ctemp)
        decryption(ctemp, ktemp, length-1);

        //send the decrypted message back to the client
        charsRead = send(establishedConnectionFD, ctemp, length, 0);

        if (charsRead < 0) error("ERROR writing to socket");
        close(establishedConnectionFD); // Close the existing socket which is connected to the client
        return 0;

      } else {
        //unsuccessful verification of client
        perror("ERROR: otp_enc cannot use otp_dec_d");
        //send a END message back to the client
        charsRead = send(establishedConnectionFD, "END", 3, 0); // Send failure back
        if (charsRead < 0) error("ERROR writing to socket");
        close(establishedConnectionFD); // Close the existing socket which is connected to the client
        return(1);
      }
    }
  }
  close(listenSocketFD); // Close the listening socket
  return 0;
}

//decrypts a cipher using the key
void decryption(char* cipher, char* key, int length) {
  int i;
  for (i = 0; i < length; i++) {
    int cipherChar = (int)cipher[i];
    int keyChar = (int)key[i];

    // decrypt cipher
    if(cipherChar != 32) {
      cipherChar -= 65;
    }
    else {
      cipherChar = 26;
    }

    if(keyChar != 32) {
      keyChar -= 65;
    }
    else {
      keyChar = 26;
    }

    cipherChar -= keyChar;

    cipherChar = (27 + cipherChar) % 27;

    //convert back to ascii capital letters
    cipherChar += 65;

    //If ASCII value is 91, change to space character (32)
    if(cipherChar == 91) {
      cipherChar = 32;
    //If ASCII value is 64, change to space character (32)
    } else if(cipherChar == 64) {
      cipherChar = 32;
    //If ASCII value is 57, change to T character (84)
    } else if(cipherChar == 57) {
      cipherChar = 84;
    }
    //save the character
    cipher[i] = (char)cipherChar;
  }
}
