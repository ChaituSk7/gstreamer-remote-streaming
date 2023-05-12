#include "header.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 8090

using namespace std;
/* send file extension to client using sockets */
void
sendExtenstionToClient (std::string extenstion)
{

  int sockfd, new_socket;
  struct sockaddr_in servaddr, cliaddr;
  char *buf = NULL;
  buf = &extenstion[0];

  /* Creating socket file descriptor */
  if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == 0) {
    perror ("socket failed");
    exit (EXIT_FAILURE);
  }
  /* for reusing the address */
  int optval = 1;
  if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &optval,
          sizeof (optval)) == -1) {
    perror ("");
    exit (EXIT_FAILURE);
  }
  /* Assign IP, PORT */
  memset (&servaddr, 0, sizeof (servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr ("10.1.137.49");
  servaddr.sin_port = htons (PORT);

  /* Bind the socket with the server address */
  if (bind (sockfd, (const struct sockaddr *) &servaddr, sizeof (servaddr)) < 0) {
    perror ("bind failed");
    exit (EXIT_FAILURE);
  }
  /* Listen for connections */
  if (listen (sockfd, 5) < 0) {
    perror ("listen");
    exit (EXIT_FAILURE);
  }
  /* Accept the connection */
  socklen_t len = sizeof (cliaddr);
  g_print ("Waiting for clinet to connect....\n");
  if ((new_socket = accept (sockfd, (struct sockaddr *) &cliaddr, &len)) < 0) {
    perror ("accept");
    exit (EXIT_FAILURE);
  }

  /* Send message to client */
  send (new_socket, buf, sizeof (buf), 0);

  /* Close the socket */
  close (sockfd);
}

int
main (int argc, char *argv[])
{

  DIR *dir;
  struct dirent *ent;
  char *uri = argv[1];
  uri = realpath (uri, NULL);
  cout << "uri: " << uri << endl;
  /* open directory */
  if ((dir = opendir (uri)) != NULL) {
    /* Read file from directory */
    while ((ent = readdir (dir)) != NULL) {
      if (ent->d_type != DT_REG) {
        continue;
      }
      string filename = ent->d_name;
      char *val = ent->d_name;
      cout << "file: " << val << endl;
      size_t pos = filename.find_last_of (".");
      if (pos != std::string::npos) {
        std::string extension = filename.substr (pos + 1);
        std::cout << "File extension is : " << extension << std::endl;
        string path = string (uri) + "/" + string (val);
        char *file_path = (char *) path.c_str ();
        /* Function to send extension to client */
        sendExtenstionToClient (extension);
        if (extension == "mp4") {
          directory_set (path);
          hostmp4_pipeline (file_path);
          host_thumbnail ();
        } else if (extension == "avi") {
          directory_set (path.c_str ());
          hostavi_pipeline (file_path);
          host_thumbnail ();
        } else if (extension == "mp3") {
          hostmp3_pipeline (file_path);
        } else if (extension == "webm") {
          directory_set (path.c_str ());
          hostwebm_pipeline (file_path);
          host_thumbnail ();
        } else {
          g_printerr ("Unsuported format.\n");
        }
      }
    }
  }
  return 0;
}
