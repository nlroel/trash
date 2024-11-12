#include <iostream>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

const int PORT = 4098;
const int BUFFER_SIZE = 65536;

void processPacket(char *buffer, ssize_t size) {
    // 解析 IP 头部
    struct iphdr *ipHeader = (struct iphdr*) buffer;
    std::cout << "Received packet from: " << inet_ntoa(*(in_addr*)&ipHeader->saddr) << "\n";

    // 检查协议类型是否为 UDP（协议号 17）
    if (ipHeader->protocol == IPPROTO_UDP) {
        struct udphdr *udpHeader = (struct udphdr*) (buffer + ipHeader->ihl * 4);
        std::cout << "Source Port: " << ntohs(udpHeader->source) << "\n";
        std::cout << "Destination Port: " << ntohs(udpHeader->dest) << "\n";

        // 获取 UDP 数据负载
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
