#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <sys/un.h>
#include <signal.h>
#include <stdarg.h>

// Prototypes for internal functions and utilities
void error(const char *fmt, ...);
int runClient(char *clientName, int numMessages, char **messages);
int runServer(int port);
void serverReady(int signal);

bool serverIsReady = false;

// Prototypes for functions to be implemented by students
void server();
void client(char *clientName, int numMessages, char *messages[]);

void verror(const char *fmt, va_list argp)
{
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, argp);
    fprintf(stderr, "\n");
}

void error(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    verror(fmt, argp);
    va_end(argp);
    exit(1);
}

int runServer(int port) {
    int forkPID = fork();
    if (forkPID < 0)
        error("ERROR forking server");
    else if (forkPID == 0) {
        server();
        exit(0);
    } else {
        printf("Main: Server(%d) launched...\n", forkPID);
    }
    return forkPID;
}

int runClient(char *clientName, int numMessages, char **messages) {
    fflush(stdout);
    printf("Launching client %s...\n", clientName);
    int forkPID = fork();
    if (forkPID < 0)

        error("ERROR forking client %s", clientName);
    else if (forkPID == 0) {
        client(clientName, numMessages, messages);
        exit(0);
    }
    return forkPID;
}

void serverReady(int signal) {
    serverIsReady = true;
}

#define NUM_CLIENTS 2
#define MAX_MESSAGES 5
#define MAX_CLIENT_NAME_LENGTH 10

struct client {
    char name[MAX_CLIENT_NAME_LENGTH];
    int num_messages;
    char *messages[MAX_MESSAGES];
    int PID;
};

// Modify these to implement different scenarios
struct client clients[] = {
        {"Uno", 3, {"Mensaje 1-1", "Mensaje 1-2", "Mensaje 1-3"}},
        {"Dos", 3, {"Mensaje 2-1", "Mensaje 2-2", "Mensaje 2-3"}}
};

int main() {
//    signal(SIGUSR1, serverReady);               /* Map ready signal (SIGUSR1) to the serverReady function */
//    int serverPID = runServer(getpid());        /* Fork the server process and store pid */
//
//    /* Wait for server to signal it's ready */
//    while(!serverIsReady) {
//        sleep(1);
//    }
//    printf("Main: Server(%d) signaled ready to receive messages\n", serverPID);
//
//    /* Fork the client processes, and assign a PID to each client in "clients" array */
//    for (int client = 0 ; client < NUM_CLIENTS ; client++) {
//        clients[client].PID = runClient(clients[client].name, clients[client].num_messages,
//                                        clients[client].messages);
//    }
//
//    /* Wait for each client 'child' to finish */
//    for (int client = 0 ; client < NUM_CLIENTS ; client++) {
//        int clientStatus;
//        printf("Main: Waiting for client %s(%d) to complete...\n", clients[client].name, clients[client].PID);
//        waitpid(clients[client].PID, &clientStatus, 0);
//        printf("Main: Client %s(%d) terminated with status: %d\n",
//               clients[client].name, clients[client].PID, clientStatus);
//    }
//
//    /* Kill server and wait for it to finish */
//    printf("Main: Killing server (%d)\n", serverPID);
//    kill(serverPID, SIGINT);
//    int statusServer;
//    pid_t wait_result = waitpid(serverPID, &statusServer, 0);
//    printf("Main: Server(%d) terminated with status: %d\n", serverPID, statusServer);
//    return 0;

//    Part 4 of LAB!!!!!!!!!!!!!!!!
//    int pid;
//    pid = fork();
//    if (pid < 0) {
//        fprintf(stderr, "Fork failed!\n");
//        exit(-1);
//    }
//    else if (pid==0) {
//        printf("Child Process (%d) forked from process: %d\n", getpid(), getppid());
//        fflush(stdout);
//        execlp("/bin/ps", "ps", NULL);
//        printf("Still around...\n");
//    }
//    else {
//        int status;
//        pid_t child_pid = wait(&status);
//        printf("Child Process (%d) terminated with status code: %d\n", child_pid, status);
//        exit(0);
//    }

    char command[300];
    //copy the command to download the file
    strcpy(command, "wget --quiet https://www.c-programming-simple-steps.com/support-files/return-statement.zip");
    //execute the command
    system(command);
    //copy the commmand to list the files
    strcpy(command, "ls | grep return");
    //execute the command
    system(command);
    return(0);
}



//*********************************************************************************
//**************************** EDIT FROM HERE *************************************
//#you can create the global variables and functions that you consider necessary***
//*********************************************************************************

#define PORT_NUMBER 23220
bool serverShutdown = false;
int serverPIDs[NUM_CLIENTS];

void shutdownServer(int signal) {
    serverShutdown = true; //Indicate that the server has to shutdown

    //Wait for the children to finish
    for(int i=0; i<NUM_CLIENTS; i++) {
        waitpid(serverPIDs[i], &serverPIDs[i], 0);
    }

    exit(0);
}

void client(char *clientName, int numMessages, char *messages[])
{
    char buffer[256];
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //Open the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    //Connect to the server
    server = gethostbyname("localhost");
    if (server == NULL)
    {
        fprintf(stderr,"ERROR, no such host");
        exit(0);
    }

    /* Initialize server address to connect to */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(PORT_NUMBER);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) error("ERROR connecting");

    //For each message, write to the server and read the response
    for (int message = 0; message < numMessages; message++) {
        //Accept connection and create a child proccess to read and write
        n = write(sockfd,messages[message],strlen(messages[message]));
        if (n < 0) error("ERROR writing to socket");
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");

        printf("Client %s(%d): Return message: %s\n", clientName, getpid(), buffer);
        fflush(stdout);
        sleep(1);
    }
    //Close socket
    close(sockfd);
    exit(0);
}

void server()
{
    char buffer[256];
    int sockfd, newsockfd, n;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    //Handle SIGINT so the server stops when the main process kills it
    signal(SIGINT, shutdownServer);

    //Open the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    // Initialize serv_address to bind
    bzero((char *) &serv_addr, sizeof(serv_addr)); //Set all values in serv_addr buffer to 0.
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUMBER);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    //Bind the socket to serv_address
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");

    //Listen for connections on socket
    listen(sockfd,5);

    //Signal server is ready
    kill (getppid(), SIGUSR1);

    for (int i = 0; i < NUM_CLIENTS; i++) {
        //Accept connection and create a child proccess to read and write
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");

        serverPIDs[i] = fork();
        if (serverPIDs[i] < 0) error("ERROR forking server(%d)", serverPIDs[i]);
        else if (serverPIDs[i] == 0) {
            for (int i=0; i<MAX_MESSAGES; i++) {
                //Read Message
                bzero(buffer, 256);
                n = read(newsockfd, buffer, 255);
                if (n < 0) error("ERROR receiving from socket");
                else if (n==0) { //Client closed the connection. (Therefore no more messages to be received)
                    break;
                }

                printf("Server child(%d): got message: %s\n", getpid(), buffer); //expected output

                //Send message to client
                n = write(newsockfd, buffer, n);
                if (n < 0) error("ERROR writing to socket");
            }
            close(newsockfd);
            exit(0);

        }
        else {
            //close(newsockfd);
        }
    }
}