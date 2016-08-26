#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <pthread.h>
#include "linked_list.h"


int flag = 0, key = -1, time_counter = 0, count = 0;  //Flag, and global variables
double number, max = -10000000.0, min = 10000000.0, sum = 0.0, ave;   
pthread_mutex_t lock;
pthread_mutex_t key_lock;
pthread_mutex_t ll_lock;
pthread_mutex_t proximity_lock;
pthread_mutex_t disconnect_lock;
pthread_mutex_t standby_lock;
list* head_node = NULL;         //Linked list storing temperature / distance data
double arr[3600];               //An array that stores temperature in the recent 3600s
char message[1024];             //Buffer for storing message that is sent back to the phone
int DisconnectedFromArduino = 0;  //Flag to indicate whether serial connection is broken
int Standby = 0;                  //Flag to indicate whether Standby mode is on
int proximity = 0;                //Flag to indicate whether we are using Proximity Sensor
void* user_input(void* argument); //Thread
void* serial(void* argument);     //Thread


//Run the server
int start_server(int PORT_NUMBER) {
  struct sockaddr_in server_addr, client_addr;
  int sock = 0; 
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket");
    exit(1);
  }
  int temp = 0;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int)) == -1) {
    perror("Setsockopt");
    exit(1);
  }
  server_addr.sin_port = htons(PORT_NUMBER); 
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(server_addr.sin_zero), 8);
  if (bind(sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr))
      == -1) {
    perror("Unable to bind");
    exit(1);
  }
  if (listen(sock, 5) == -1) {
    perror("Listen");
    exit(1);
  }
  printf("\nServer configured to listen on port %d\n", PORT_NUMBER);
  //fflush(stdout);
  int sin_size = sizeof(struct sockaddr_in);



  //Enterng the Big Loop
  while (1) {
    if (flag == 1)  //if Q is hit, server will be down
      break;

    int fd = accept(sock, (struct sockaddr *) &client_addr,
        (socklen_t *) &sin_size);
    if (fd != -1) {
      fprintf(stderr, "Server got a connection from (%s, %d)\n",
          inet_ntoa(client_addr.sin_addr),
          ntohs(client_addr.sin_port));
      char request[1024];                               //A buffer to store http request
      int bytes_received = recv(fd, request, 1024, 0);
      request[bytes_received] = '\0';                   //Null terminate the request
      char c = request[5];                              //Find the key sent from the phone
      if (c == 'f')
        continue;
      int num = c - '0';                                //Convert the key into a number

      printf("%s\n", request);

      if (DisconnectedFromArduino == 1) {               //If serial connection is down, send error message back
        char* reply = malloc(256);
        strcpy(reply,
            "{\n\"Disconnected_from_Arduino\": \"Disconnected_from_Arduino");
        strcat(reply, "\"\n}\n");
        printf("reply: %s\n", reply);
        send(fd, reply, strlen(reply), 0);
      } else {                                          //If serial connection is good, do some work
        if (num == 1) {                                 //If key is 1, write key 1 to Arduino
          char *reply = malloc(256);
          strcpy(reply, "{\n\"timer\": \"");
          strcat(reply, "timer running");
          strcat(reply, "\"\n}\n");
          key = num;
          send(fd, reply, strlen(reply), 0);
          free(reply);
        }

        if (num == 0) {                                 //If key is 0, write key 0 to Arduino, send the 
          key = num;
          pthread_mutex_lock(&proximity_lock);          //temperature back in the reply string
          proximity = 0;
          pthread_mutex_unlock(&proximity_lock);
          char *reply = malloc(256);
          strcat(reply, "{\n\"Temp\": \"");

          if (head_node != NULL) {
            strcat(reply, head_node->name);
            reply[strlen(reply) - 1] = '\0';
            strcat(reply, "\"\n}\n");
            send(fd, reply, strlen(reply), 0);
          }
          printf("The reply is %s\n", reply);
          free(reply);
        }

        if (num == 2) {                                 //If key is 2, write key 2 to Arduino, send the distance
          key = num; 
          pthread_mutex_lock(&proximity_lock);          //back in the reply string 
          proximity = 1;
          pthread_mutex_unlock(&proximity_lock);
          char *reply = malloc(256);
          strcat(reply, "{\n\"Proximity\": \"");
          if (head_node != NULL && strlen(head_node->name) > 1) {
            strcat(reply, head_node->name);
            reply[strlen(reply) - 1] = '\0';
            strcat(reply, "\"\n}\n");
            send(fd, reply, strlen(reply), 0);
          }
          free(reply);
        }

        if (num == 3) {                                 //If key is 3, write key 3 to Arduino, send the statistics
          if (count == 0) {                             //back. Statistics are stored as global variables. 
            char *reply = malloc(256);
            strcat(reply, "{\n\"Statistics\": \"");
            strcat(reply, "No Stat!");
            reply[strlen(reply)] = '\0';
            strcat(reply, "\"\n}\n");
            send(fd, reply, strlen(reply), 0);
            free(reply);
          } else {
            char *reply = malloc(256);
            char str[64];
            strcat(reply, "{\n\"Statistics\": \"");
            sprintf(str, "%.2f" " %.2f" " %.2f", min, max, ave);
            strcat(reply, str);
            strcat(reply, "\"\n}\n");
            send(fd, reply, strlen(reply), 0);
            free(reply);
          }

        }

        if (num == 4) {                                 //If key is 4, write key 4 to Arduino, in Standby Mode
          char *reply = malloc(256);
          strcpy(reply, "{\n\"Standby\": \"");
          strcat(reply, "Standby sent!");
          strcat(reply, "\"\n}\n");
          key = num;
          send(fd, reply, strlen(reply), 0);
          free(reply);
        }

        if (num == 5) {                                 //If key is 5, write key 5 to Arduino, in Action Mode
          char *reply = malloc(256);
          strcpy(reply, "{\n\"Action\": \"");
          strcat(reply, "Action sent!");
          strcat(reply, "\"\n}\n");
          key = num;
          send(fd, reply, strlen(reply), 0);
          free(reply);
        }

        if (num == 6) {                                 //If key is 6, write key 6 to Arduino, stay using Celsius
          char *reply = malloc(256);
          strcpy(reply, "{\n\"Celsius\": \"");
          strcat(reply, "Celsius sent!");
          strcat(reply, "\"\n}\n");
          key = num;
          send(fd, reply, strlen(reply), 0);
          free(reply);
        }

        if (num == 7) {                                 //If key is 7, write key 7 to Arduino, use Fahrenheit
          char *reply = malloc(256);
          strcpy(reply, "{\n\"Fahrenheit\": \"");
          strcat(reply, "Fahrenheit sent!");
          strcat(reply, "\"\n}\n");
          key = num;
          send(fd, reply, strlen(reply), 0);
          free(reply);
        }

      }

      close(fd);
      printf("Server closed connection\n");
    }
  }

  printf("%s\n", "server shutting down");              //Shutting down
  close(sock);
  delete_list(head_node);
  return 0;
}

//Main
int main(int argc, char *argv[]) {
  pthread_mutex_init(&lock, NULL);
  pthread_mutex_init(&ll_lock, NULL);
  pthread_mutex_init(&key_lock, NULL);
  pthread_mutex_init(&standby_lock, NULL);
  pthread_mutex_init(&proximity_lock, NULL);
  pthread_mutex_init(&disconnect_lock, NULL);
  if (argc != 2) {
    printf("\n %s Please specify a port number!\n", argv[0]);   //Handling wrong number of arguments
    return -1;
  }

  int PORT_NUMBER = atoi(argv[1]);
  if (PORT_NUMBER <= 1024) {
    perror("Please use a port number greater than 1024!");
    return -1;
  }
  pthread_t thread2, thread3, thread4;                          //Create threads
  void* ret = NULL;

  int ret_val = pthread_create(&thread2, NULL, &user_input, NULL);
  printf("%s\n", "thread2 created!");
  if (ret_val) {
    fprintf(stderr, "%s\n", "Creating thread2 failed!");
    return -1;
  }

  ret_val = pthread_create(&thread3, NULL, &serial, NULL);
  printf("%s\n", "thread3 created!");
  if (ret_val) {
    fprintf(stderr, "%s\n", "Creating thread3 failed!");
    return -1;
  }

  start_server(PORT_NUMBER);                                    //Run the server

  ret_val = pthread_join(thread2, ret);                         //Join the threads
  if (ret_val != 0) {
    fprintf(stderr, "%s\n", "Joining thread2 failed!");
    return -1;
  }

  ret_val = pthread_join(thread3, ret);
  if (ret_val != 0) {
    fprintf(stderr, "%s\n", "Joining thread3 failed!");
    return -1;
  }

  return 0;
}

//Thread for user input
void* user_input(void* argument) {
  while (1) {
    char user[20];
    scanf("%s", user);
    printf("The user input is %s\n", user);
    if (strcmp(user, "q") == 0 || strcmp(user, "Q") == 0) {     //if input is Q or q, set flag to 1
      pthread_mutex_lock(&lock);
      flag = 1;
      pthread_mutex_unlock(&lock);
      break;
    }
  }
  pthread_exit(NULL);
}

//Thread for Arduino USB connection
void* serial(void* argument) {
  int fd = open("/dev/cu.usbmodem1411", O_RDWR);                //Open the connection, if failed, set DisconnectedFromArduino to 1
  if (fd == -1) {
    perror("cannot open serial connection oh no!");
    pthread_mutex_lock(&disconnect_lock);
    DisconnectedFromArduino = 1;
    pthread_mutex_unlock(&disconnect_lock);
    return 0;
  }
  printf("serial fd is: %d\n", fd);

  char buf[100];
  printf("serial buf: %s\n", buf);
  struct termios options; // struct to hold options
  tcgetattr(fd, &options); // associate with this fd
  cfsetispeed(&options, 9600); // set input baud rate
  cfsetospeed(&options, 9600); // set output baud rate
  tcsetattr(fd, TCSANOW, &options);

  //Enter the Big Loop
  while (1) {
    if (flag == 1)                                              //If flag is 1, quit
      break;

    if (key != -1) {
      pthread_mutex_lock(&key_lock);                                            //Check and write byte to Arduino
      write(fd, &key, 1);
      pthread_mutex_unlock(&key_lock);  
      if (key == 4) {
        pthread_mutex_lock(&standby_lock);  
        Standby = 1;
        pthread_mutex_unlock(&standby_lock);  
      }
      if (key == 5) {
        pthread_mutex_lock(&standby_lock);  
        Standby = 0;
        pthread_mutex_unlock(&standby_lock);  
      }
      pthread_mutex_lock(&key_lock);
      key = -1;                                                 //Reset the key to default
      pthread_mutex_unlock(&key_lock);
    }

    int bytes_read = read(fd, buf, 100);                        //Read the bytes
    printf("bytes read %d\n", bytes_read);
    if (bytes_read == -1) {                                     //If negative, serial connection is broken
      pthread_mutex_lock(&disconnect_lock);
      DisconnectedFromArduino = 1;
      pthread_mutex_unlock(&disconnect_lock);
      return 0;
      //continue;
    }

    printf("%s\n", "before bytes_read = \0");                   //Waiting until \n character is read, and        
    if (bytes_read != 0) {                                      //put the data in our linked list 

      buf[bytes_read] = '\0';
      strcat(message, buf);
      //printf("msg %s\n", message);
      if (buf[bytes_read - 1] == '\n') {
        pthread_mutex_lock(&ll_lock);
        int len = strlen(message);
        message[len] = '\0';
        head_node = add_to_top(head_node, message);
        printf("head_node %s\n", head_node->name);
        pthread_mutex_unlock(&ll_lock);
        printf("%s %f %f %f %d %f\n", "The stats: ", max, min, sum,
            count, ave);

        if (proximity == 1) {                                   //If the user is using Proximity Sensor
          strcpy(message, "");                                  //, then do not record data as temperature statistics
          continue;
        }

        char num[64];                                          //Record current temperature, max, min, avg
        char* ptr;
        char* temperature = head_node->name;                   //Convert string to double 
        while (*temperature != ':') {
          temperature++;
        }
        temperature++;
        strncpy(num, temperature, 6);                   
        number = strtod(num, &ptr);                             
        if (number != 0 && number < 100 && proximity != 1) {   //Compare a new temperature with max, min,  
          fprintf(stderr, "num is: %s\n", num);                //, increment count, increment total, and compute average 
          fprintf(stderr, "number is: %s\n", num);
          count++;
          if (number > max)
            max = number;
          if (number < min)
            min = number;
          double save = arr[(count - 1) % 3600];
          arr[(count - 1) % 3600] = number;
          if (count <= 3600) {
            sum += number;
            ave = sum / count;
          } else {
            sum = sum - save + number;
            ave = sum / 3600;
          }
        }
        strcpy(message, "");                                   //Clear the buffer 

      }
    }

  }

  pthread_exit(NULL);
}

