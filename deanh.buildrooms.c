#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

//Prototype definitions
struct Room* GetRandomRoom(struct Room * rooms);
int IsGraphFull(struct Room *rooms);
void AddRandomConnection(char* roomNames[], struct Room * rooms);
int CanAddConnectionFrom(struct Room* x);
int ConnectionAlreadyExists(struct Room* x, struct Room* y, char* roomNames[]);
int IsSameRoom(struct Room* x, struct Room* y);
void ConnectRoom(struct Room* x, struct Room* y, char* roomNames[]);

//Room struct
struct Room {
 char* name;
 int connection[7];
 char* roomType;
};


// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
int IsGraphFull(struct Room *rooms)
{
  int i, j;

  int k = 1;
  for(i = 0; i < 7; i++) {
    int count = 0;
    for(j=0; j < 7; j++) {
      if(rooms[i].connection[j] == 1) {
        count++;
      }
    }
    if(count < 3 || count > 6) {
      k=0;
    }
  }
  return k;
}

// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection(char* roomNames[], struct Room * rooms)
{
  struct Room *a;  // Maybe a struct, maybe global arrays of ints
  struct Room *b;

  while(1)
  {
    a = GetRandomRoom(rooms);

    if (CanAddConnectionFrom(a) == 1)
      break;
  }

  do
  {
    b = GetRandomRoom(rooms);
  }
  while(CanAddConnectionFrom(b) == 0 || IsSameRoom(a, b) == 1 || ConnectionAlreadyExists(a, b, roomNames) == 1);

  ConnectRoom(a, b, roomNames);  // TODO: Add this connection to the real variables,
  ConnectRoom(b, a, roomNames);  //  because this A and B will be destroyed when this function terminates
}

// Returns a random Room, does NOT validate if connection can be added
struct Room* GetRandomRoom(struct Room * rooms)
{
  int n = rand() % 7;
  return &rooms[n];
}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
int CanAddConnectionFrom(struct Room* x)
{
  int i;
  int count = 0;
  for(i=0; i < 7; i++) {
    if(x->connection[i] == 1) {
      count++;
    }
  }
  if(count < 6) {
    return 1;
  }
  return 0;
}
// Returns true if a connection from Room x to Room y already exists, false otherwise
int ConnectionAlreadyExists(struct Room* x, struct Room* y, char* roomNames[])
{
  int j=0, k=0;
  int i;
  for(i=0; i<7; i++) {
    if(strcmp(roomNames[i], y->name) == 0) {
      if(x->connection[i] == 1) {
        j=1;
      }
    } else if (strcmp(roomNames[i], x->name) == 0) {
      if(y->connection[i] == 1) {
        k=1;
      }
    }
  }
  if(j == 1 && k == 1) {
    return 1;
  } else {
    return 0;
  }
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(struct Room* x, struct Room* y, char* roomNames[])
{
  int i;
  for(i = 0; i < 7; i++) {
    if(strcmp(roomNames[i], y->name) == 0) {
      x->connection[i] = 1;
    } else if (strcmp(roomNames[i], x->name) == 0) {
      y->connection[i] = 1;
    }
  }
}

// Returns true if Rooms x and y are the same Room, false otherwise
int IsSameRoom(struct Room* x, struct Room* y)
{
  if(strcmp(x->name, y->name) == 0) {
    return 1;
  }
  return 0;
}




int main() {
  srand(time(0));

  //Create directory for files
  char dirname[25];
  memset(dirname, '\0', sizeof(dirname));
  //Set dirname with process id
  sprintf(dirname, "deanh.rooms.%ld", (long)getpid());
  //Check if directory was successful
  if(mkdir(dirname, 0777) < 0) {
    printf("ERROR: Unable to create directory..\n");
  }
  //Array for file descriptors
  char* fileName[7] = {"goku", "vegeta", "piccolo", "krillin", "yamcha", "tien", "gohan"};
  char buffer[50];
  char* roomName[10] = {"Dungeon", "School", "House", "Hospital", "Lucid", "Dream", "Ethereal", "Realm", "Tinker", "Loft"};
  char* selectedRoomName[7];
  char* roomType[3] = {"START_ROOM", "MID_ROOM", "END_ROOM"};
  int i, j;

  struct Room rooms[7];
  for(i = 0; i < 7; i++) {
    //generate randome number
    int n = rand() % 10;
    //Check random room name for duplicates
    int end = 1;
    while(end) {
      int j;
      int stop=0;
      for(j = 0; j<i; j++) {
        if(strcmp(roomName[n], selectedRoomName[j]) == 0) {
          stop = 1;
        }
      }
      if(stop) {
        n = rand() % 10;
      } else {
        selectedRoomName[i] = roomName[n];
        rooms[i].name = roomName[n];
        for(j=0; j<7; j++) {
          rooms[i].connection[j] = 0;
        }
        end = 0;
      }
    }
    //Set room type
    if(i == 0) {
      rooms[i].roomType = roomType[0];
      } else if(i == 6) {
        rooms[i].roomType = roomType[2];
      } else {
        rooms[i].roomType = roomType[1];
      }
  }

  // Create all connections in graph
  while (IsGraphFull(rooms) == 0)
  {
    AddRandomConnection(selectedRoomName, rooms);
  }

  //For each file
   for(i=0; i < 7; i++) {
     memset(buffer, '\0', sizeof(buffer));
     sprintf(buffer, "%s/%s", dirname, fileName[i]);
     FILE* fp = fopen(buffer, "w");
     if (fp == NULL) {
       perror("Failed: ");
       return 1;
    }
    //Wrtie Room Name
    memset(buffer, '\0', sizeof(buffer));
    sprintf(buffer, "ROOM NAME: %s\n", rooms[i].name);
    fwrite(buffer, 1, strlen(buffer), fp);

    //Wrtie connections
    int count =1;
    for(j=0; j < 7; j++) {
      if(rooms[i].connection[j] == 1) {
        memset(buffer, '\0', sizeof(buffer));
        sprintf(buffer, "CONNECTION %d: %s\n", count, selectedRoomName[j]);
        count++;
        fwrite(buffer, 1, strlen(buffer), fp);
      }
    }

    //Write Room Type
    memset(buffer, '\0', sizeof(buffer));
    sprintf(buffer, "ROOM TYPE: %s\n", rooms[i].roomType);
    fwrite(buffer, 1, strlen(buffer), fp);

    //close file
    fclose(fp);
  }
  return 0;
}
