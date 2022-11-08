// websocket 协议的实现
/* Server based on EPOLL Reactor Model
 * compile command: gcc -I /usr/local/openssl/include -L /usr/local/openssl/lib websocket_server.c -o websocket_server -lcrypto
 * if the environment variable has been added. just add the '-lcrypto' for compile.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include <openssl/sha.h> // import openssl for sha-1
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#define BUFFER_LENGTH 1024
#define EVENT_SIZE 1024
#define GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

// define the state machine for web socket protocol

enum WS_STATUS
{
    WS_INIT = 0,
    WS_HANDSHAKE = 1,
    WS_DATATRANSFORM = 2,
    WS_DATAEND
};

struct sockitem
{
    int sockfd;
    int (*callback)(int events, void *arg);
    char recvbuffer[BUFFER_LENGTH];
    char sendbuffer[BUFFER_LENGTH];

    int rlength; // length of the received buffer
    int slength; // length of the send buffer.

    int status;
};

struct reactor
{
    int epollfd;
    struct epoll_event events[EVENT_SIZE];
};

// store for big-endian. // for FIN RSV1 RSV2 RSV3 opcode
struct _websocket_ophdr
{
    unsigned char opcode : 4,
        rsv3 : 1,
        rsv2 : 1,
        rsv1 : 1,
        fin : 1;
    unsigned char payload_length : 7,
        mask : 1;
} __attribute__((packed));

// struct for RFC6455 Websocket data frame.
struct _websocket_head_126
{
    // length:1 0-->125
    // length:2 126 --> 2^23
    // length:3 127 --> 2^71
    unsigned short payload_length;
    char mask_key[4];
    unsigned char data[8];
} __attribute__((packed));

struct _websocket_head_127
{
    unsigned long long payload_length;
    char mask_key[4];
    unsigned char data[8];
} __attribute__((packed));

typedef struct _websocket_head_127 websocket_head_127;
typedef struct _websocket_head_126 websocket_head_126;
typedef struct _websocket_ophdr ophdr;

struct reactor *g_eventloop = NULL;

//==============function declaration==============
int recv_cb(int events, void *arg);
char *decode_packet(char *stream, char *mask, int length, int *ret);
int encode_packet(char *buffer, char *mask, char *stream, int length);
//===============end declaration==================

int send_cb(int events, void *arg)
{
    if (!(events & EPOLLOUT) || arg == NULL)
        return -1;
    struct sockitem *si = (struct sockitem *)arg;

    int clientfd = si->sockfd;
    int epoll_fd = g_eventloop->epollfd;
    if (send(clientfd, si->sendbuffer, si->slength, 0) == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // the send buffer is full.
            // add send into EPOLLOUT event.
            return 0;
        }
        // send data error maybe some network issue.
        perror("send error");
        close(clientfd);
        return -1;
    }
    // change state into EPOLLIN
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    // WS_DATATRANSFORM; // once we have finished the transfer of header. change the state.
    if (si->status == WS_HANDSHAKE)
        si->status++;
    si->callback = recv_cb;
    ev.data.ptr = si;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, clientfd, &ev);
    return 0;
}

int base64_encode(char *in_str, int in_len, char *out_str)
{
    BIO *b64, *bio;
    BUF_MEM *bptr = NULL;
    size_t size = 0;

    if (in_str == NULL || out_str == NULL)
        return -1;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, in_str, in_len);
    BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bptr);
    memcpy(out_str, bptr->data, bptr->length);
    out_str[bptr->length - 1] = '\0';
    size = bptr->length;

    BIO_free_all(bio);
    return size;
}

int readline(char *allbuf, int level, char *linebuf)
{
    int len = strlen(allbuf);

    for (; level < len; ++level)
    {
        if (allbuf[level] == '\r' && allbuf[level + 1] == '\n')
            return level + 2;
        else
            *(linebuf++) = allbuf[level];
    }
    return -1;
}

// implement for state handshake of the websocket protocol.
int handshake(struct sockitem *si, struct reactor *mainloop)
{
    char linebuf[BUFSIZ] = {0};
    char sec_accept[32] = {0};
    char sha1_data[SHA_DIGEST_LENGTH + 1] = {0};
    char head[BUFFER_LENGTH] = {0};
    int level = 0;
    // si->recvbuffer, si->rlength;
    do
    {
        memset(linebuf, 0, sizeof(linebuf));
        level = readline(si->recvbuffer, level, linebuf);

        // if this line container the Sec-WebSocket-Key.
        if (strstr(linebuf, "Sec-WebSocket-Key") != NULL)
        {
            strcat(linebuf, GUID); // add the GUID
            // encode by SHA1 alrogithm from openSSL
            SHA1((unsigned char *)&linebuf + 19, strlen(linebuf + 19), (unsigned char *)&sha1_data);
            base64_encode(sha1_data, strlen(sha1_data), sec_accept);
            sprintf(head, "HTTP/1.1 101 Switching Protocols\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Accept: %s\r\n"
                          "\r\n",
                    sec_accept);

            printf("response\n");
            printf("%s\n\n\n", head);

            memset(si->sendbuffer, 0, BUFFER_LENGTH);
            memset(si->recvbuffer, 0, BUFFER_LENGTH);
            // copy head to sendbuffer
            si->slength = strlen(head);
            memcpy(si->sendbuffer, head, si->slength);
            si->rlength = 0;

            struct epoll_event ev;
            ev.events = EPOLLOUT | EPOLLET;
            // si->sockfd = si->sockfd;
            si->callback = send_cb;
            ev.data.ptr = si;
            epoll_ctl(mainloop->epollfd, EPOLL_CTL_MOD, si->sockfd, &ev);
            break;
        }

    } while ((si->recvbuffer[level] != 'r' || si->recvbuffer[level + 1] != '\n') && level != -1);
    return 0;
}

// implementation for data transform for WS_DATATRANSFORM state.
int datatransform(struct sockitem *si, struct reactor *mainloop)
{
    // si->status --> WS_DATAFRANSFORM
    // si->recvbuffer
    int ret = 0;
    char mask[4] = {0};
    char *data = decode_packet(si->recvbuffer, mask, si->rlength, &ret);

    // process the data frame from the client.
    printf("data : %s , length : %d\n", data, ret);

    ret = encode_packet(si->sendbuffer, mask, data, ret);
    si->slength = ret;

    memset(si->recvbuffer, 0, BUFFER_LENGTH);

    struct epoll_event ev;
    ev.events = EPOLLOUT | EPOLLET;
    // si->sockfd = si->sockfd;
    si->callback = send_cb;
    si->status = WS_DATATRANSFORM;
    ev.data.ptr = si;

    epoll_ctl(mainloop->epollfd, EPOLL_CTL_MOD, si->sockfd, &ev);
}

void umask(char *data, int len, char *mask)
{
    int i;
    for (i = 0; i < len; i++)
        *(data + i) ^= *(mask + (i % 4));
}

char *decode_packet(char *stream, char *mask, int length, int *ret)
{

    ophdr *hdr = (ophdr *)stream;
    char *data = stream + sizeof(ophdr);
    int size = 0;
    int start = 0;
    // char mask[4] = {0};
    int i = 0;

    // if (hdr->fin == 1) return NULL;

    if ((hdr->mask & 0x7F) == 126)
    {

        websocket_head_126 *hdr126 = (websocket_head_126 *)data;
        size = hdr126->payload_length;

        for (i = 0; i < 4; i++)
        {
            mask[i] = hdr126->mask_key[i];
        }
        start = 8;
    }
    else if ((hdr->mask & 0x7F) == 127)
    {
        websocket_head_127 *hdr127 = (websocket_head_127 *)data;
        size = hdr127->payload_length;

        for (i = 0; i < 4; i++)
        {
            mask[i] = hdr127->mask_key[i];
        }
        start = 14;
    }
    else
    {
        size = hdr->payload_length;
        memcpy(mask, data, 4);
        start = 6;
    }

    *ret = size;
    umask(stream + start, size, mask);
    return stream + start;
}

int encode_packet(char *buffer, char *mask, char *stream, int length)
{

    ophdr head = {0};
    head.fin = 1;
    head.opcode = 1;
    int size = 0;

    if (length < 126)
    {
        head.payload_length = length;
        memcpy(buffer, &head, sizeof(ophdr));
        size = 2;
    }
    else if (length < 0xffff)
    {
        websocket_head_126 hdr = {0};
        hdr.payload_length = length;
        memcpy(hdr.mask_key, mask, 4);

        memcpy(buffer, &head, sizeof(ophdr));
        memcpy(buffer + sizeof(ophdr), &hdr, sizeof(websocket_head_126));
        size = sizeof(websocket_head_126);
    }
    else
    {
        websocket_head_127 hdr = {0};
        hdr.payload_length = length;
        memcpy(hdr.mask_key, mask, 4);

        memcpy(buffer, &head, sizeof(ophdr));
        memcpy(buffer + sizeof(ophdr), &hdr, sizeof(websocket_head_127));

        size = sizeof(websocket_head_127);
    }

    memcpy(buffer + 2, stream, length);
    return length + 2;
}

int recv_cb(int events, void *arg)
{
    if (!(events & EPOLLIN) || arg == NULL)
        return -1;

    struct sockitem *si = (struct sockitem *)arg;

    int clientfd = si->sockfd;
    int epoll_fd = g_eventloop->epollfd;

    // char buffer[BUFFER_LENGTH] = { 0 };
    struct epoll_event ev;
    int ret = recv(clientfd, si->recvbuffer, BUFFER_LENGTH, 0);
    if (ret < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            printf("read all data\n");
        }
        close(clientfd);
        ev.events = EPOLLIN | EPOLLET;
        // ev->data.fd = clientfd;
        ev.data.ptr = NULL;
        free(si);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, clientfd, &ev);
    }
    else if (ret == 0)
    {
        printf(" disconnect clientfd:%d\n", clientfd);

        close(clientfd);
        ev.events = EPOLLIN | EPOLLET;
        // ev->data.fd = clientfd;
        ev.data.ptr = NULL;
        free(si);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, clientfd, &ev);
        return 0;
    }
    else
    {
        printf("Recv: \n%.*s\nTotal: %d Bytes\n", ret, si->recvbuffer, ret);
#if 0
        si->rlength = ret;
        memcpy(si->sendbuffer, si->recvbuffer, si->rlength);
        si->slength = si->rlength;
#else
        // state machine.
        if (si->status == WS_HANDSHAKE)
        {
            handshake(si, g_eventloop);
        }
        else if (si->status == WS_DATATRANSFORM)
        {
            datatransform(si, g_eventloop);
        }
        else if (si->status == WS_DATAEND)
        {
        }
        else
        {
            assert(0);
        }

#endif
    }
}

// callback handler for accept events of the sockfd.
int accept_cb(int events, void *arg)
{
    if (!(events & EPOLLIN) || arg == NULL)
        return -1;

    struct sockitem *psi = (struct sockitem *)arg;
    int epoll_fd = g_eventloop->epollfd;
    int sockfd = psi->sockfd;

    struct sockaddr_in client_addr;
    struct epoll_event ev;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_len = sizeof(client_addr);

    int clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);

    if (clientfd <= 0)
        return -1; // do nothing.

    char str[INET_ADDRSTRLEN] = {0};

    struct sockitem *si = (struct sockitem *)malloc(sizeof(struct sockitem));
    si->sockfd = clientfd;
    si->callback = recv_cb;
    si->status = WS_HANDSHAKE;

    printf("received from %s at port:%d, sockfd:%d, clientfd:%d\n",
           inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)),
           ntohs(client_addr.sin_port), sockfd, clientfd);

    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = si;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientfd, &ev);
}

// ./epoll 8080.
int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("parameter error!\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port); // convert network bytes
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
        exit(EXIT_FAILURE);
    }

    printf("start server and wait for connection...\n");
    if (listen(sockfd, 5) < 0)
    {
        exit(EXIT_FAILURE);
    }

    g_eventloop = (struct reactor *)malloc(sizeof(struct reactor));

    // epoll coding.
    g_eventloop->epollfd = epoll_create(1); // create the epoll root node for epoll object.
    struct epoll_event ev;
    ev.events = EPOLLIN;
    // ev.data.fd = sockfd;

    // define socketitem
    struct sockitem *si = (struct sockitem *)malloc(sizeof(struct sockitem));
    si->sockfd = sockfd;
    si->status = WS_INIT;
    si->callback = accept_cb;
    ev.data.ptr = si;

    epoll_ctl(g_eventloop->epollfd, EPOLL_CTL_ADD, sockfd, &ev);

    while (1)
    {
        // condition wait.
        // max amount of events could be caught by 1 epoll_wait call.
        int nready = epoll_wait(g_eventloop->epollfd, g_eventloop->events, EVENT_SIZE, -1);
        if (nready < -1)
        {
            break;
        }

        int i = 0;
        for (i = 0; i < nready; i++)
        {

            if (g_eventloop->events[i].events & EPOLLIN)
            {
                struct sockitem *si = (struct sockitem *)g_eventloop->events[i].data.ptr;
                if (si && si->callback)
                    si->callback(g_eventloop->events[i].events, si);
            }

            if (g_eventloop->events[i].events & EPOLLOUT)
            {
                struct sockitem *si = (struct sockitem *)g_eventloop->events[i].data.ptr;
                if (si && si->callback)
                    si->callback(g_eventloop->events[i].events, si);
            }
        }
    }
    close(sockfd);
    exit(EXIT_SUCCESS);
}