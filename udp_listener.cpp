#include <iostream>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <iomanip>
#include <sys/socket.h>
#include <fcntl.h>

const char *IP_ADDRESS = "0.0.0.0";  // 监听所有网卡
const int PORT = 4098;
const int BUFFER_SIZE = 1508;

void parsePacket(const std::vector<uint8_t> &data) {
    std::cout << "Received packet with size: " << data.size() << " bytes\n";
    for (size_t i = 0; i < std::min(data.size(), static_cast<size_t>(32)); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
    }
    std::cout << std::dec << "\n";
}

void udpListener() {
    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    // 创建 UDP 套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Socket creation failed\n";
        return;
    }

    // 设置接收超时
    struct timeval tv;
    tv.tv_sec = 5; // 5秒超时
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    // 绑定套接字到指定地址和端口
    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Socket bind failed: " << strerror(errno) << "\n";
        close(sockfd);
        return;
    }

    std::cout << "Listening for UDP packets on " << IP_ADDRESS << ":" << PORT << "\n";

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t len = sizeof(clientAddr);
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &len);

        if (n < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                std::cout << "No data received within timeout.\n";
                continue;
            } else {
                std::cerr << "Failed to receive data: " << strerror(errno) << "\n";
                continue;
            }
        }

        std::vector<uint8_t> data(buffer, buffer + n);
        parsePacket(data);
    }

    close(sockfd);
}

int main() {
    std::thread listenerThread(udpListener);
    listenerThread.join();
    return 0;
}
