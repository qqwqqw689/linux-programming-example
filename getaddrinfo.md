```c
   Server program

       #include <netdb.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <string.h>
       #include <sys/socket.h>
       #include <sys/types.h>
       #include <unistd.h>

       #define BUF_SIZE 500

       int
       main(int argc, char *argv[])
       {
           int                      sfd, s;
           char                     buf[BUF_SIZE];
           ssize_t                  nread;
           socklen_t                peer_addrlen;
           struct addrinfo          hints;
           struct addrinfo          *result, *rp;
           struct sockaddr_storage  peer_addr;

           if (argc != 2) {
               fprintf(stderr, "Usage: %s port\n", argv[0]);
               exit(EXIT_FAILURE);
           }

        /* If the AI_PASSIVE flag is specified in hints.ai_flags, and node
           is NULL, then the returned socket addresses will be suitable for
           bind(2)ing a socket that will accept(2) connections. 
        */
           memset(&hints, 0, sizeof(hints));
           hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
           hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
           hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
           hints.ai_protocol = 0;          /* Any protocol */
           hints.ai_canonname = NULL;
           hints.ai_addr = NULL;
           hints.ai_next = NULL;           /*A pointer to the socket address is placed in the ai_addr field*/

           s = getaddrinfo(NULL, argv[1], &hints, &result);
           if (s != 0) {
               fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
               exit(EXIT_FAILURE);
           }

           /* getaddrinfo() returns a list of address structures.
              Try each address until we successfully bind(2).
              If socket(2) (or bind(2)) fails, we (close the socket
              and) try the next address.The items in the linked list
              are linked by the ai_next field. */

           for (rp = result; rp != NULL; rp = rp->ai_next) {
               sfd = socket(rp->ai_family, rp->ai_socktype,
                            rp->ai_protocol);
               if (sfd == -1)
                   continue;

               if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
                   break;                  /* Success */

               close(sfd);
           }

           freeaddrinfo(result);           /* No longer needed */

           if (rp == NULL) {               /* No address succeeded */
               fprintf(stderr, "Could not bind\n");
               exit(EXIT_FAILURE);
           }

           /* Read datagrams and echo them back to sender. */

           for (;;) {
               char host[NI_MAXHOST], service[NI_MAXSERV];

               peer_addrlen = sizeof(peer_addr);
               nread = recvfrom(sfd, buf, BUF_SIZE, 0,
                                (struct sockaddr *) &peer_addr, &peer_addrlen);
               if (nread == -1)
                   continue;               /* Ignore failed request */
                /* getnameinfo - address-to-name translation in protocol-independent manner*/
               s = getnameinfo((struct sockaddr *) &peer_addr,
                               peer_addrlen, host, NI_MAXHOST,
                               service, NI_MAXSERV, NI_NUMERICSERV);
                               // #define NI_MAXHOST      1025
                               // #define NI_MAXSERV      32
                               // NI_NUMERICSERV : If set, then the numeric form of the service address is returned.
               if (s == 0)
                   printf("Received %zd bytes from %s:%s\n",
                          nread, host, service);
               else
                   fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

               if (sendto(sfd, buf, nread, 0, (struct sockaddr *) &peer_addr,
                          peer_addrlen) != nread)
               {
                   fprintf(stderr, "Error sending response\n");
               }
           }
       }

   Client program

       #include <netdb.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <string.h>
       #include <sys/socket.h>
       #include <sys/types.h>
       #include <unistd.h>

       #define BUF_SIZE 500

       int
       main(int argc, char *argv[])
       {
           int              sfd, s;
           char             buf[BUF_SIZE];
           size_t           len;
           ssize_t          nread;
           struct addrinfo  hints;
           struct addrinfo  *result, *rp;

           if (argc < 3) {
               fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
               exit(EXIT_FAILURE);
           }

           /* Obtain address(es) matching host/port. */

           memset(&hints, 0, sizeof(hints));
           hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
           hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
           hints.ai_flags = 0;
           hints.ai_protocol = 0;          /* Any protocol */

           s = getaddrinfo(argv[1], argv[2], &hints, &result);
           if (s != 0) {
               fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
               exit(EXIT_FAILURE);
           }

           /* getaddrinfo() returns a list of address structures.
              Try each address until we successfully connect(2).
              If socket(2) (or connect(2)) fails, we (close the socket
              and) try the next address. */

           for (rp = result; rp != NULL; rp = rp->ai_next) {
               sfd = socket(rp->ai_family, rp->ai_socktype,
                            rp->ai_protocol);
               if (sfd == -1)
                   continue;

               if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
                   break;                  /* Success */

               close(sfd);
           }

           freeaddrinfo(result);           /* No longer needed */

           if (rp == NULL) {               /* No address succeeded */
               fprintf(stderr, "Could not connect\n");
               exit(EXIT_FAILURE);
           }

           /* Send remaining command-line arguments as separate
              datagrams, and read responses from server. */

           for (size_t j = 3; j < argc; j++) {
               len = strlen(argv[j]) + 1;
                       /* +1 for terminating null byte */

               if (len > BUF_SIZE) {
                   fprintf(stderr,
                           "Ignoring long message in argument %zu\n", j);
                   continue;
               }

               if (write(sfd, argv[j], len) != len) {
                   fprintf(stderr, "partial/failed write\n");
                   exit(EXIT_FAILURE);
               }

               nread = read(sfd, buf, BUF_SIZE);
               if (nread == -1) {
                   perror("read");
                   exit(EXIT_FAILURE);
               }

               printf("Received %zd bytes: %s\n", nread, buf);
           }

           exit(EXIT_SUCCESS);
       }
```
