#include <iostream>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

const int BUFFER_SIZE = 65536;
const char *INTERFACE = "eth0";  // 要绑定的网卡接口

void processPacket(char *buffer, ssize_t size) {
    struct iphdr *ipHeader = (struct iphdr*) buffer;
    std::cout << "Received packet from: " << inet_ntoa(*(in_addr*)&ipHeader->saddr) << "\n";

    if (ipHeader->protocol == IPPROTO_UDP) {
        struct udphdr *udpHeader = (struct udphdr*) (buffer + ipHeader->ihl * 4);
        std::cout << "Source Port: " << ntohs(udpHeader->source) << "\n";
        std::cout << "Destination Port: " << ntohs(udpHeader->dest) << "\n";

        char *data = buffer + ipHeader->ihl * 4 + sizeof(struct udphdr);
        ssize_t dataSize = size - (ipHeader->ihl * 4 + sizeof(struct udphdr));

        std::cout << "Data (" << dataSize << " bytes): ";
        for (ssize_t i = 0; i < dataSize; ++i) {
            printf("%02x ", (unsigned char)data[i]);
        }
        std::cout << std::endl;
    } else {
        std::cout << "Non-UDP packet received, protocol: " << (int)ipHeader->protocol << "\n";
    }
}

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];

    // 创建原始套接字，使用 IP 协议
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        std::cerr << "Socket creation failed. Run as root.\n";
        return -1;
    }

    // 绑定到指定网卡接口 eth0
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, INTERFACE, strlen(INTERFACE)) < 0) {
        std::cerr << "Binding to device failed: " << strerror(errno) << "\n";
        close(sockfd);
        return -1;
    }

    while (true) {
        struct sockaddr_in sourceAddr;
        socklen_t addrLen = sizeof(sourceAddr);
        ssize_t dataSize = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&sourceAddr, &addrLen);
        
        if (dataSize < 0) {
            std::cerr << "Failed to receive data\n";
            continue;
        }

        processPacket(buffer, dataSize);
    }

    close(sockfd);
    return 0;
}
