#include "http_server.h"
#include "icmp_ping.h"

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // 创建套接字
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 设置套接字选项避免地址使用错误
    int on = 1; // 允许地址重用, 0禁止地址重用
    if ((setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // 将套接字绑定到指定端口
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // 监听连接请求
    if (listen(server_socket, MAX_CONNECTIONS) < 0)
    {
        perror("Failed to listen for connections");
        exit(EXIT_FAILURE);
    }

    // 接受连接并处理请求
    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("Failed to accept connection");
            continue;
        }

        handle_client(client_socket);

        close(client_socket);
    }

    close(server_socket);

    return 0;
}