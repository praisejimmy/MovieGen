#include <stdio.h>
#include <stdint.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#define LIST_PATH_MAX 256

#define MOVIE_LIST_LOC   "/home/ryan/Documents/Projects/MovieGen/MovieServer/MovieLists"
#define ACTION_LIST_FILE "action_movies.list"
#define COMEDY_LIST_FILE "comedy_movies.list"
#define HORROR_LIST_FILE "horror_movies.list"
#define SCIFI_LIST_FILE  "scifi_movies.list"
#define ROMCOM_LIST_FILE "romcom_movies.list"
#define TEMP_LIST_FILE   "temp.list"

typedef enum
{
    invalid_genre = 0x00,
    action = 0x01,
    comedy = 0x02,
    horror = 0x03,
    scifi = 0x04,
    romcom = 0x05
} movie_genre;

#define MAX 80
#define PORT 8888
#define RECV_BUF_LEN 1025
#define SEND_BUF_LEN 512
#define ENTRY_MAX_SIZE 256

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
    uint32_t cmd;
    uint32_t packet_len;
} __attribute__((__packed__)) movie_cmd_hdr;

typedef struct
{
    uint32_t ret_status;
    uint32_t packet_len;
} __attribute__((__packed__)) movie_ret_hdr;

void cmd_handler(int sockfd);

movie_genre get_genre_from_name(const char *genre_name)
{
    if (!strcmp(genre_name, "action"))
    {
        return action;
    }
    else if (!strcmp(genre_name, "comedy"))
    {
        return comedy;
    }
    else if (!strcmp(genre_name, "horror"))
    {
        return horror;
    }
    else if (!strcmp(genre_name, "scifi"))
    {
        return scifi;
    }
    else if (!strcmp(genre_name, "romcom"))
    {
        return romcom;
    }
    else
    {
        return invalid_genre;
    }
}

int remove_movie(int list_fd, uint8_t *entry_name, movie_genre genre)
{
    int temp_fd, i;
    uint32_t num_entries, num_entries_update, bytes, entry_size;
    char list_path[LIST_PATH_MAX];
    char temp_path[LIST_PATH_MAX];
    uint8_t entry_buf[ENTRY_MAX_SIZE];
    lseek(list_fd, 0, SEEK_SET);
    switch (genre)
    {
        case action:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, ACTION_LIST_FILE);
            break;
        case comedy:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, COMEDY_LIST_FILE);
            break;
        case horror:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, HORROR_LIST_FILE);
            break;
        case scifi:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, SCIFI_LIST_FILE);
            break;
        case romcom:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, ROMCOM_LIST_FILE);
            break;
        default:
            printf("Received invalid genre in removal\n");
            return -1;
    }

    bytes = read(list_fd, &num_entries, sizeof(uint32_t));
    if (bytes != sizeof(uint32_t))
    {
        printf("Could not read num entries in remove\n");
        return -1;
    }

    sprintf(temp_path, "%s/%s", MOVIE_LIST_LOC, TEMP_LIST_FILE);

    if ((temp_fd = open(temp_path, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH)) < 0)
    {
        printf("Could not open temp for removal\n");
        return -1;
    }

    num_entries_update = num_entries;
    write(temp_fd, &num_entries_update, sizeof(uint32_t));

    for (i = 0; i < num_entries; i++)
    {
        bytes = read(list_fd, &entry_size, sizeof(uint32_t));
        if (bytes != sizeof(uint32_t))
        {
            printf("Could not read entry length in removal\n");
            return -1;
        }
        bytes = read(list_fd, entry_buf, entry_size);
        if (bytes != entry_size)
        {
            printf("Could not read entry for removal\n");
            return -1;
        }
        entry_buf[entry_size] = '\0';
        if (strcmp(entry_buf, entry_name) != 0)
        {
            write(temp_fd, &entry_size, sizeof(uint32_t));
            write(temp_fd, &entry_buf, entry_size);
        }
        else
        {
            num_entries_update--;
        }
    }

    unlink(list_path);

    if (num_entries_update == 0)
    {
        close(temp_fd);
        unlink(temp_path);
        return 0;
    }

    lseek(temp_fd, 0, SEEK_SET);
    write(temp_fd, &num_entries_update, sizeof(uint32_t));
    close(temp_fd);
    rename(temp_path, list_path);
    return 0;
}

int add_movie(int sockfd)
{
    int i, list_fd;
    movie_ret_hdr ret_hdr;
    uint32_t component_size, bytes, movie_count;
    uint8_t recv_buf[RECV_BUF_LEN];
    char list_path[LIST_PATH_MAX];
    movie_genre genre;

    bytes = read(sockfd, &component_size, sizeof(uint32_t));

    if (component_size > (RECV_BUF_LEN - 1))
    {
        printf("Size of genre too large\n");
        return -1;
    }

    bytes = read(sockfd, &recv_buf, component_size);

    if (bytes != component_size)
    {
        printf("Unable to read genre\n");
        exit(-1);
    }

    for (i = 0; i < component_size; i++){
        recv_buf[i] = tolower(recv_buf[i]);
    }
    recv_buf[component_size] = '\0';
    printf("Recv genre: %s\n", recv_buf);
    // compare genre to name
    genre = get_genre_from_name(recv_buf);
    switch (genre)
    {
        case action:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, ACTION_LIST_FILE);
            break;
        case comedy:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, COMEDY_LIST_FILE);
            break;
        case horror:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, HORROR_LIST_FILE);
            break;
        case scifi:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, SCIFI_LIST_FILE);
            break;
        case romcom:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, ROMCOM_LIST_FILE);
            break;
        default:
            printf("Received invalid genre\n");
            exit(-1);
    }

    bytes = read(sockfd, &component_size, sizeof(uint32_t));
    printf("Movie size: %u\n", component_size);
    if (bytes > (RECV_BUF_LEN - 1))
    {
        printf("Size of movie too large\n");
        exit(-1);
    }

    bytes = read(sockfd, &recv_buf, component_size);
    printf("Read bytes: %u\n", bytes);
    for (i = 0; i < bytes; i++)
    {
        printf("%c ", recv_buf[i]);
    }
    printf("\n");
    if (bytes != component_size)
    {
        printf("Unable to read movie\n");
        exit(-1);
    }

    for (i = 0; i < component_size; i++){
        recv_buf[i] = tolower(recv_buf[i]);
    }
    recv_buf[component_size] = '\0';

    if ((list_fd = open(list_path, O_RDWR)) >= 0)
    {
        bytes = read(list_fd, &movie_count, sizeof(uint32_t));
        if (bytes != sizeof(uint32_t))
        {
            printf("Invalid file configuration");
            exit(-1);
        }
        movie_count++;
        lseek(list_fd, 0, SEEK_SET);
        write(list_fd, &movie_count, sizeof(uint32_t));
        lseek(list_fd, 0, SEEK_END);
        write(list_fd, &component_size, sizeof(uint32_t));
        write(list_fd, recv_buf, component_size);
    }
    else
    {
        list_fd = open(list_path, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
        if (list_fd < 0)
        {
            printf("Unable to create list");
            exit(-1);
        }
        movie_count = 1;
        write(list_fd, &movie_count, sizeof(uint32_t));
        write(list_fd, &component_size, sizeof(uint32_t));
        write(list_fd, recv_buf, component_size);
    }
    ret_hdr.ret_status = SERR_SUCCESS;
    ret_hdr.packet_len = sizeof(movie_ret_hdr);
    write(sockfd, &ret_hdr, sizeof(movie_ret_hdr));
    close(list_fd);
    return 0;
}

int get_movie(int sockfd)
{
    uint8_t send_buf[SEND_BUF_LEN];
    uint8_t recv_buf[RECV_BUF_LEN];
    char list_path[LIST_PATH_MAX];
    int list_fd, i;
    uint32_t rand_movie, bytes, num_movies, entry_size, component_size;
    movie_ret_hdr ret_hdr;
    movie_genre genre;

    bytes = read(sockfd, &component_size, sizeof(uint32_t));

    if (component_size > (RECV_BUF_LEN - 1))
    {
        printf("Size of genre too large\n");
        exit(-1);
    }

    bytes = read(sockfd, &recv_buf, component_size);

    if (bytes != component_size)
    {
        printf("Unable to read genre\n");
        exit(-1);
    }

    for (i = 0; i < component_size; i++){
        recv_buf[i] = tolower(recv_buf[i]);
    }
    recv_buf[component_size] = '\0';

    // compare genre to name
    genre = get_genre_from_name(recv_buf);
    switch (genre)
    {
        case action:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, ACTION_LIST_FILE);
            break;
        case comedy:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, COMEDY_LIST_FILE);
            break;
        case horror:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, HORROR_LIST_FILE);
            break;
        case scifi:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, SCIFI_LIST_FILE);
            break;
        case romcom:
            sprintf(list_path, "%s/%s", MOVIE_LIST_LOC, ROMCOM_LIST_FILE);
            break;
        default:
            printf("Received invalid genre\n");
            exit(-1);
    }

    if ((list_fd = open(list_path, O_RDWR)) < 0)
    {
        printf("Could not open get file\n");
        ret_hdr.ret_status = SERR_GETERROR;
        ret_hdr.packet_len = sizeof(movie_ret_hdr);
        write(sockfd, &ret_hdr, ret_hdr.packet_len);
        return -1;
    }
    bytes = read(list_fd, &num_movies, sizeof(uint32_t));
    if (bytes != sizeof(uint32_t))
    {
        printf("Could not read number movies\n");
        ret_hdr.ret_status = SERR_GETERROR;
        ret_hdr.packet_len = sizeof(movie_ret_hdr);
        write(sockfd, &ret_hdr, ret_hdr.packet_len);
        return -1;
    }
    rand_movie = rand() % num_movies;
    for (i = 0; i < rand_movie; i++)
    {
        bytes = read(list_fd, &entry_size, sizeof(uint32_t));
        if (bytes != sizeof(uint32_t))
        {
            printf("Could not read entry size\n");
            ret_hdr.ret_status = SERR_GETERROR;
            ret_hdr.packet_len = sizeof(movie_ret_hdr);
            write(sockfd, &ret_hdr, ret_hdr.packet_len);
            return -1;
        }
        lseek(list_fd, entry_size, SEEK_CUR);
    }
    bytes = read(list_fd, &entry_size, sizeof(uint32_t));
    if (bytes != sizeof(uint32_t))
    {
        printf("Could not read final entry size\n");
        ret_hdr.ret_status = SERR_GETERROR;
        ret_hdr.packet_len = sizeof(movie_ret_hdr);
        write(sockfd, &ret_hdr, ret_hdr.packet_len);
        return -1;
    }
    bytes = read(list_fd, send_buf, entry_size);
    if (bytes != entry_size)
    {
        printf("Could not read final entry\n");
        ret_hdr.ret_status = SERR_GETERROR;
        ret_hdr.packet_len = sizeof(movie_ret_hdr);
        write(sockfd, &ret_hdr, ret_hdr.packet_len);
        return -1;
    }
    ret_hdr.ret_status = SERR_SUCCESS;
    ret_hdr.packet_len = sizeof(movie_ret_hdr) + sizeof(uint32_t) + entry_size;
    write(sockfd, &ret_hdr, sizeof(movie_ret_hdr));
    write(sockfd, &entry_size, sizeof(uint32_t));
    write(sockfd, send_buf, entry_size);

    // remove entry from list
    send_buf[entry_size] = '\0';
    remove_movie(list_fd, send_buf, genre);
    close(list_fd);
    return 0;
}

// Function designed for chat between client and server.
void cmd_handler(int sockfd)
{
    uint32_t bytes;
    movie_cmd_hdr cmd_hdr;
    int i;
    uint8_t test_buf[1024];
    bytes = read(sockfd, &cmd_hdr, sizeof(movie_cmd_hdr));

    if (bytes != sizeof(movie_cmd_hdr))
    {
       printf("Invalid command header received with byte length %d\n", bytes);
       exit(-1);
    }

    printf("Recieved packet with len %d\n", cmd_hdr.packet_len);
    printf("%u, %u\n", cmd_hdr.cmd, cmd_hdr.packet_len);
    switch (cmd_hdr.cmd)
    {
        case MOVIE_ADD:
            printf("Add command received\n");
            add_movie(sockfd);
            break;
        case MOVIE_DEL:
            printf("Del command received\n");
            break;
        case MOVIE_LIST:
            printf("List command received\n");
            break;
        case MOVIE_GET:
            printf("Get command recieved");
            get_movie(sockfd);
            break;
        default:
            printf("Invalid command received 0x%02x\n", cmd_hdr.cmd);
            exit(-1);
    }
}

// Driver function
int main()
{
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	// socket create and verification
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
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);

    while (1)
    {
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
        if (connfd < 0) {
            printf("server acccept failed...\n");
            exit(0);
        }
        else
            printf("server acccept the client...\n");

        // Function for chatting between client and server
        cmd_handler(connfd);

        // After chatting close the socket
    }
    close(sockfd);
}
