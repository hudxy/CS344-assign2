#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>

//function protoypes
void * writeTime();
void readTime(pthread_mutex_t *myMutex, int count);

//Room struct
struct Room {
  char name[10];
  char connection[7][10];
  char roomType[12];
  int connectionCount;
};

//finds most recent directory
void  findDirectory(struct Room* rooms) {
  //open current directory
  DIR *dirp = opendir("./");
  struct stat dStat;
  time_t last = 0;
  struct dirent *dp;
  char dName[50];
  memset(dName, '\0', sizeof(dName));
  //skip current directory
  dp =  readdir(dirp);
  while(dp = readdir(dirp)) {
    memset(&dStat, 0, sizeof(dStat));
    //Read directory information
    if (stat(dp->d_name, &dStat) < 0) {
      printf("Error cannot get file info...\n");
      continue;
    }
    //If its a file, skip it
    if ((dStat.st_mode & S_IFDIR) != S_IFDIR) {
      continue;
    }
    //Check for the last timestamp
    if (dStat.st_mtime > last) {
      //Switch last timestamp to
      strcpy(dName, dp->d_name);
      last = dStat.st_mtime;
    }
  }
  closedir(dirp);

  char buffer[50];
  memset(buffer, '\0', sizeof(buffer));
  sprintf(buffer, "./%s", dName);
  dirp = opendir(buffer);
  //skip first 2 directories
  dp = readdir(dirp);
  dp = readdir(dirp);

  char tmpstr[25];
  int i=0;
  //while reading files
  while(dp = readdir(dirp)) {

    memset(tmpstr, '\0', sizeof(tmpstr));
    sprintf(tmpstr, "%s/%s", dName, dp->d_name);


    //Open files in most recent directory
    FILE* fp = fopen(tmpstr, "r");
    if (fp == NULL) {
      perror("Failed: ");
      exit(1);
    }

    //Look for Room Name
    fseek(fp, 11, SEEK_SET);
    memset(tmpstr, '\0', sizeof(tmpstr));
    fscanf(fp, "%s", tmpstr);
    sprintf(rooms[i].name, "%s", tmpstr);

    //Look for connections and number of connections
    memset(tmpstr, '\0', sizeof(tmpstr));
    fseek(fp, 0, SEEK_SET);

    int connectCount = 0;
    while(fscanf(fp, "%s", tmpstr) != EOF) {
      if(strcmp(tmpstr, "CONNECTION") == 0) {
        fseek(fp, 3, SEEK_CUR);
        memset(tmpstr, '\0', sizeof(tmpstr));
        fscanf(fp, "%s", tmpstr);
        sprintf(rooms[i].connection[connectCount], "%s", tmpstr);
        connectCount++;
      }
      memset(tmpstr, '\0', sizeof(tmpstr));
    }
    rooms[i].connectionCount = connectCount;
    memset(tmpstr, '\0', sizeof(tmpstr));



    //Look for room Type
    fseek(fp, -10, SEEK_END);
    memset(tmpstr, '\0', sizeof(tmpstr));
    fscanf(fp, "%s\n", tmpstr);
    if(strcmp(tmpstr, "TART_ROOM") == 0) {
      memset(tmpstr, '\0', sizeof(tmpstr));
      sprintf(tmpstr, "START_ROOM");
    }
    sprintf(rooms[i].roomType, "%s", tmpstr);

    i++;
    //close current file
    fclose(fp);
  }


}

//Function to search for desired room and return that room
struct Room* searchRoom(struct Room rooms[], struct Room room, char* input) {
  int i;
  for(i = 0; i < 7; i++) {
    if(strcmp(input, room.connection[i]) == 0) {
      int j;
      for(j=0; j<7; j++) {
        if(strcmp(input, rooms[j].name) == 0) {
          return &rooms[j];
        }
      }
    }
  }
  //if room not found, print error message to user
  printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
  return NULL;
}

//Function checks if game is over
int isGameOver(struct Room * room, int steps, char* path[]) {
  if(strcmp(room->roomType, "END_ROOM") == 0) {
    int i=0;
    //print congrats message if at END_ROOM
    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!!\n");
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
    //print path user took
    for(i=0; i<steps; i++) {
      printf("%s\n", path[i]);
    }
    return 1;
  }
  return 0;
}

//Function prints room info to console
void roomInfo(struct Room * room) {
  printf("CURRENT LOCATION: %s\n", room->name);
  printf("POSSIBLE CONNECTIONS: ");
  int i=0;
  while(strcmp(room->connection[i], "\0") != 0) {
    if(room->connectionCount-1 == i) {
      printf("%s. ", room->connection[i]);
      i++;
    } else {
      printf("%s, ", room->connection[i]);
      i++;
    }
  }
  printf("\nWHERE TO? >");
}


//Function to start game
void game(struct Room rooms[]) {
  //Variable declarations
  struct Room* curRoom = &rooms[4];
  char* path[50];
  int steps = 0;
  int count = 0;
  pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

  while(!isGameOver(curRoom, steps, path)) {
    char* input = NULL;
    size_t inputSize = 0;
    int numCharIn = -5;

    //get info of current room
    struct Room* tmpRoom;
    roomInfo(curRoom);

    //get input from user
    numCharIn = getline(&input, &inputSize, stdin);
    input[numCharIn - 1] = '\0';
    printf("\n");

    //check if user enters time
    while(strcmp(input, "time")==0) {
      free(input);
      count++;
      int resultInt;
      //start pthread and lock mutex while writing time to file
      pthread_t myThread;
      pthread_mutex_lock(&myMutex);
      resultInt = pthread_create(&myThread, NULL, writeTime, (void*)&myMutex);

      //read time from file
      readTime(&myMutex, count);


      //get input from user again
      input = NULL;
      inputSize = 0;
      numCharIn = -5;
      numCharIn = getline(&input, &inputSize, stdin);
      input[numCharIn - 1] = '\0';
      printf("\n");
    }

    //check room is valid, if not ask user for another input
    tmpRoom = searchRoom(rooms, *curRoom, input);
    free(input);
    while(tmpRoom == NULL) {
      input = NULL;
      inputSize = 0;
      numCharIn = -5;

      //current room info
      roomInfo(curRoom);

      //get user input
      numCharIn = getline(&input, &inputSize, stdin);
      input[numCharIn - 1] = '\0';
      printf("\n");

      //check if user enters time
      while(strcmp(input, "time")==0) {
        free(input);
        count++;
        int resultInt;
        //start pthread and lock mutex while writing time to file
        pthread_t myThread;
        //lock mutex for second thread
        pthread_mutex_lock(&myMutex);
        resultInt = pthread_create(&myThread, NULL, writeTime, (void*)&myMutex);

        //read time from file
        readTime(&myMutex, count);

        //get input from user again
        input = NULL;
        inputSize = 0;
        numCharIn = -5;
        numCharIn = getline(&input, &inputSize, stdin);
        input[numCharIn - 1] = '\0';
        printf("\n");
      }
      //check if valid room was provided
      tmpRoom = searchRoom(rooms, *curRoom, input);
      free(input);
    }
    //set validated room to current room
    curRoom = tmpRoom;
    //get room name for the path taken
    path[steps] = tmpRoom->name;
    //increment steps counter
    steps++;
  }
  return;
}

//Function writes the current time to a file named "currentTime.txt"
void * writeTime(void * mutex) {
  //cast mutex from pthread arg
  pthread_mutex_t* myMutex = (pthread_mutex_t *) mutex;
  char buffer[50];
  memset(buffer, '\0', sizeof(buffer));
  //open file for appending
  FILE* fp = fopen("currentTime.txt", "a");
  if (fp == NULL) {
    perror("Failed: ");
    exit(1);
  }
  //get time information
  time_t rawTime;
  time(&rawTime);
  struct tm * timeinfo = localtime(&rawTime);
  //format time to assignment specification
  if (strftime(buffer, sizeof(buffer), "%l:%M%p, %A, %B %d, 20%y\n", timeinfo) == 0) {
    fprintf(stderr, "strftime returned 0");
    exit(EXIT_FAILURE);
  }
  //write to file
  fwrite(buffer, 1, strlen(buffer), fp);
  //close file
  fclose(fp);
  //unlock mutex for reading (second thread)
  pthread_mutex_unlock(myMutex);
  return NULL;
}

//Function reads file from "currentTime.txt"
void readTime(pthread_mutex_t *myMutex, int count) {
  char buffer[50];
  memset(buffer, '\0', sizeof(buffer));
  //lock mutex from writing (main thread)
  pthread_mutex_lock(myMutex);
  //open file
  FILE* fp = fopen("currentTime.txt", "r");
  fseek(fp, 0, SEEK_SET);
  int i;

  //count how many lines from file needed to get most recent time
  for(i=0; i<count+1; i++) {
    fgets(buffer, 50, fp);
  }
  printf("%s\n", buffer);
  //close file and unlock mutex for writing
  fclose(fp);
  //unlock mutex from main thread
  pthread_mutex_unlock(myMutex);
  printf("WHERE TO? >");
}

int main() {
  //Create file to write time to
  FILE * fp = fopen("currentTime.txt", "w");
  fclose(fp);
  //Hold rooms in struct
  struct Room rooms[7];
  //Set rooms to be blank
  int i,j;
  for(i=0; i<7; i++) {
    rooms[i].connectionCount = 0;
    memset(rooms[i].name, '\0', sizeof(rooms[i].name));
    memset(rooms[i].roomType, '\0', sizeof(rooms[i].roomType));
    for(j =0; j< 7; j++) {
      memset(rooms[i].connection[j], '\0', sizeof(rooms[i].connection[j]));
    }
  }
  //Populate rooms from files
  findDirectory(rooms);
  //run game
  game(rooms);
  //exit
  return 0;
}
