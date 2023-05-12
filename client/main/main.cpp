#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/* Defining port number and size of buffer */
#define PORT 8090
#define MAXLINE 1024

void receive_extention(){
    int sockfd;
    char buffer[MAXLINE] = {0};
    struct sockaddr_in servaddr;

    /* Creating socket file descriptor */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* Assign IP, PORT */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);

    /* Convert IPv4 and IPv6 addresses from text to binary form */
    if (inet_pton(AF_INET, "10.1.137.49", &servaddr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    /* Connect to server */
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    /* Receive string from server and print it */
	recv(sockfd, &buffer, sizeof(buffer), 0);
	printf("\nFile: %s\n", buffer);

    int argc;
    char *argv[] = {0};

    /* Checking the buffer and playing the appropriate pipeline */
    if(strcmp(buffer, "mp3") == 0){
        remotehost_Mp3_pipeline(argc, argv);
    }else if(strcmp(buffer, "webm") == 0){
        remotehost_WebM_pipeline(argc, argv);
        thumbnail_pipeline(argc, argv);
    }else if(strcmp(buffer, "mp4") == 0){
        remotehost_Mp4_pipeline(argc, argv);
        thumbnail_pipeline(argc, argv);
    }else if(strcmp(buffer, "avi") == 0){
        remotehost_Avi_pipeline(argc, argv);
        thumbnail_pipeline(argc, argv);
    }else{  
        g_printerr("Unsupported format!");
    }

    /* Close the socket */
    close(sockfd);
}

int main (int argc, char *argv[]) {
        
    while(1){
        receive_extention();
    }

    return 0;
} 



