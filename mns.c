#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

int sock;

struct clientinfo {
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    char packet[1024];
};

void *sendfile(void *args) {
    struct clientinfo *inf = (struct clientinfo *)args;
    const int maxtries = 10;
    int tries = maxtries;
    int ok = 1;
    int i = 0;
    uint32_t packetindex = 0;
    char outpacket[2052] = {0};
    char readbuffer[2048] = {0};
    char *filenameparse = inf->packet;
    char *filename = strchr(filenameparse, ',');
    if (filename != NULL) {
        filename++;
    } else {
        filename = "test.txt";
    }
    char header[256];
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file.");
        free(inf);
        pthread_exit(NULL); // skip this request
    }
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);
    snprintf(header, sizeof(header), "%ld, Prepared to send", filesize);
    sendto(sock, header, strlen(header), 0, (struct sockaddr*)&inf->client_addr, inf->addr_len);
    i = 0;
    ok = 1;
    tries = maxtries;
    while (ok > 0) {
        fseek(file, 2048 * i, SEEK_SET);
        ok = fread(readbuffer, 1, sizeof(readbuffer), file);
        packetindex = htonl(i);
        memcpy(outpacket, &packetindex, 4);
        memset(outpacket + 4, 0, 2048);
        memcpy(outpacket + 4, readbuffer, ok);
        sendto(sock, outpacket, ok + 4, 0, (struct sockaddr*)&inf->client_addr, inf->addr_len);
        printf("%i bytes read\n", ok);
        // read wether the client is ok or if he needs mental help
        struct sockaddr_in tmpa = inf->client_addr;
        socklen_t tmpl = inf->addr_len;
        char tmpp[1024] = {0};
        ssize_t recv_len = recvfrom(sock, tmpp, sizeof(tmpp) -1, 0, (struct sockaddr*)&tmpa, &tmpl);

        if (recv_len >= 0) {
            if (tmpa.sin_port == inf->client_addr.sin_port) {
                memcpy(inf->packet, tmpp, sizeof(tmpp));
            } else {
                continue;
            }
            inf->packet[recv_len] = '\0';
        }
        if (recv_len > 0 && atoi(inf->packet) != -1) {
            ok = 1;
            i = atoi(inf->packet) - 1;
            printf("Rewinding to %i due to packet drop\n", i);
        } else if (recv_len < 0) {
            if (ok > 0) {
                if (tries > 0) {
                    tries--;
                    ok = 1;
                    printf("Resending packet %i due to timeout (%i tries left)\n", i, tries);
                } else {
                    break;
                }
            }
            else {
                free(inf);
                pthread_exit(NULL);
            }
        } else if (atoi(inf->packet) == -1) {
            printf("Client said OK\n");
            tries = maxtries;
            i++;
        } else {
            printf("tf\n");
            break;
        }
    }
free(inf);
fclose(file);
pthread_exit(NULL);
}

void goodbye(int sig) {
    close(sock);
    write(1, "Goodbye!\n", 10);
    _exit(0);
}
int main() {
    int version = 0;
    int release = 1;
    int subrelease = 1;
    struct sigaction sa;
    sa.sa_handler = goodbye;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    printf("Starting up MeshNet Server version %i.%i.%i...\n", version, release, subrelease);
    printf("Opening socket...\n");
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("Error %i creating socket\n", sock);
        return sock;
    }
    struct timeval tv;
    tv.tv_sec = 1;       // 1 second timeout
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(5960);
    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind to port! its bc this");
        return 1;
    }
    printf("Listening on UDP port 5960\n");
    while (1) {
        char packet[1024];
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        //its time for the big mamaaa
        ssize_t recv_len = recvfrom(sock, packet, sizeof(packet) -1, 0, (struct sockaddr*)&client_addr, &addr_len);
        if (recv_len < 0) {
            continue;
        }
        packet[recv_len] = '\0';
        printf("recieved %zd bytes from %s:%d: %s\n", recv_len, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), packet);
        const char *connectionmessage = "MeshNet Connection Established\n";
        sendto(sock, connectionmessage, strlen(connectionmessage), 0, (struct sockaddr*)&client_addr, addr_len);
        if (strncmp(packet, "GET", 3) == 0) {
            struct clientinfo *args = malloc(sizeof(*args));
            args->client_addr=client_addr;
            args->addr_len=addr_len;
            strncpy(args->packet, packet, strlen(packet));
            pthread_t tid;
            pthread_create(&tid, NULL, sendfile, args);
            pthread_detach(tid);
        }
    }
    return(0);
}