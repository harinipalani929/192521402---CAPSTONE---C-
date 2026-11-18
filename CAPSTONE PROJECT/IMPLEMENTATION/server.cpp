#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

std::string loadHtml(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return "<h1>404 - index.html not found</h1><p>Place index.html in the same folder as server.exe.</p>";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(serverSock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Port 8080 is busy. Close any running server or change 8080 to 8081.\n";
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    listen(serverSock, SOMAXCONN);
    std::cout << "==================================================\n";
    std::cout << "  QuantCPP Local Server Running!\n";
    std::cout << "  Open in browser: http://localhost:8080\n";
    std::cout << "==================================================\n";

    while (true) {
        SOCKET clientSock = accept(serverSock, nullptr, nullptr);
        if (clientSock == INVALID_SOCKET) continue;

        char buffer[4096] = {0};
        recv(clientSock, buffer, sizeof(buffer) - 1, 0);

        // Checks current folder first, then web/index.html
        std::ifstream test("index.html");
        std::string html = test.good() ? loadHtml("index.html") : loadHtml("web/index.html");

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Content-Length: " + std::to_string(html.size()) + "\r\n"
            "Connection: close\r\n\r\n" + html;

        send(clientSock, response.c_str(), static_cast<int>(response.size()), 0);
        closesocket(clientSock);
    }

    closesocket(serverSock);
    WSACleanup();
    return 0;
}