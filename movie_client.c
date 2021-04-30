#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>

#include <stdint.h>
#include <unistd.h>

#define MAX 80
#define PORT 8888
#define SA struct sockaddr

typedef enum
{
    MOVIE_ADD = 0x01,
    MOVIE_DEL = 0x02,
    MOVIE_LIST = 0x03,
    MOVIE_GET = 0x04
} movie_cmd;

typedef enum
{
    SERR_SUCCESS = 0x00,
    SERR_ADDERROR = 0x01,
    SERR_GETERROR = 0x02
} server_ret_code;

typedef struct
{
    movie_cmd cmd;
    uint32_t packet_len;
} __attribute__((__packed__)) movie_cmd_hdr;

typedef struct
{
    uint8_t ret_status;
    uint32_t packet_len;
} __attribute__((__packed__)) movie_ret_hdr;

static char *genre_str = "horror";
static char *movie_str = "willy's wonderland";

void func_add(int sockfd)
{
	movie_cmd_hdr cmd;
    uint32_t component_size;
    cmd.cmd = MOVIE_ADD;
    cmd.packet_len = sizeof(movie_cmd_hdr) + (2 * sizeof(uint32_t)) + strlen(genre_str) + strlen(movie_str);
    write(sockfd, &cmd, sizeof(movie_cmd_hdr));
    component_size = strlen(genre_str);
    write(sockfd, &component_size, sizeof(uint32_t));
    write(sockfd, genre_str, component_size);
    component_size = strlen(movie_str);
    write(sockfd, &component_size, sizeof(uint32_t));
    write(sockfd, movie_str, component_size);
}

void func_get(int sockfd)
{
    movie_cmd_hdr cmd;
    movie_ret_hdr ret_hdr;
    uint32_t component_size, bytes;
    uint8_t recv_buf[1024];
    cmd.cmd = MOVIE_GET;
    cmd.packet_len = sizeof(movie_cmd_hdr) + sizeof(uint32_t) + strlen(genre_str);
    write(sockfd, &cmd, sizeof(movie_cmd_hdr));
    component_size = strlen(genre_str);
    write(sockfd, &component_size, sizeof(uint32_t));
    write(sockfd, genre_str, component_size);

    // recv movie
    bytes = read(sockfd, &ret_hdr, sizeof(movie_ret_hdr));
    if (bytes != sizeof(movie_ret_hdr))
    {
        printf("Could not receive from server\n");
        exit(-1);
    }
    if (ret_hdr.ret_status == SERR_GETERROR)
    {
        printf("Get movie error from server\n");
        exit(-1);
    }
    bytes = read(sockfd, &component_size, sizeof(uint32_t));
    if (bytes != sizeof(uint32_t))
    {
        printf("Could not get entry size\n");
        exit(-1);
    }
    bytes = read(sockfd, &recv_buf, component_size);
    if (bytes != component_size)
    {
        printf("Could not get entry\n");
        exit(-1);
    }
    recv_buf[component_size] = '\0';

    printf("Found random movie %s\n", recv_buf);
}

int main()
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	// socket create and varification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	// function for chat
	func_add(sockfd);
    //func_get(sockfd);

	// close the socket
	close(sockfd);
}
