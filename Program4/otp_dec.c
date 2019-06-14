//Hudson Dean
//Client Code For Decryption
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
  if (argc < 4) { fprintf(stderr,"USAGE: %s cipher key port\n", argv[0]); exit(0); } // Check usage & args

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

	//open cipher file
	FILE* fp = fopen(argv[1], "r");

	fseek(fp, 0, SEEK_END);
  if (fp == NULL) {
    perror("Failed: ");
    return 1;
  }
  //get length of file
	int length = ftell(fp);
  //point file pointer to beginning
	fseek(fp, 0, 0);

  //clear cipher buffer
	char cipher[length];
	memset(cipher, '\0', length);
  //read file into cipher
	fgets(cipher, length, fp);

	//close cipher file
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
  //read file into key
	fgets(key, length, fp);

	//close key file
	fclose(fp);

  //clear and prepare buffer for sending
	char buffer[length];
	memset(buffer, '\0', length);
	sprintf(buffer, "lan%d", length);
  //display error if key file and cipher file are not of same length
	if (strlen(key) != strlen(cipher)) {
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
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	//get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, 39, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
  //if error message from server, return
	if (strncmp(buffer, "END", 3) == 0) {
		return 1;
	}

	//send cipher to server
	charsWritten = send(socketFD, cipher, length, 0); // Write to the server
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
  int accumulator = 0;
  while(accumulator != length) {
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
    charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    //print buffer to stdout
    printf("%s", buffer);
    //count characters read to ensure total characters sent from server are recieved
    accumulator += charsRead;
  }
  //print newline to stdout
  printf("\n");
	close(socketFD); // Close the socket
	return 0;
}
