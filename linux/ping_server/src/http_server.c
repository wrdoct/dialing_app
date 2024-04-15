#include "http_server.h"

int parse_request(const char *request, char *ip, int *icmp_num)
{
    // 解析请求，假设只处理 GET 请求，比较前3个字符是否相等
    if (strncmp(request, "GET", 3) != 0)
    {
        return 501; // Not Implemented
    }

    // 提取请求中的 IP 地址和 ICMP 数量
    const char *ip_start = strstr(request, "ip=");
    const char *icmp_num_start = strstr(request, "icmp_num=");
    if (ip_start == NULL || icmp_num_start == NULL)
    {
        return 400; // Bad Request
    }
    ip_start += 3;       // 跳过 "ip="
    icmp_num_start += 9; // 跳过 "icmp_num="

    int i;
    for (i = 0; i < 15; i++)
    {
        if (ip_start[i] == '&' || ip_start[i] == ' ' || ip_start[i] == '\r' || ip_start[i] == '\n' || ip_start[i] == '\0')
        {
            break;
        }
        ip[i] = ip_start[i];
    }
    ip[i] = '\0';

    *icmp_num = atoi(icmp_num_start);

    return 200; // OK
}

void handle_client(int client_socket)
{
    char request[1024];
    int bytes_received = recv(client_socket, request, sizeof(request), 0);
    if (bytes_received < 0)
    {
        perror("Failed to receive data from client");
        close(client_socket);
        return;
    }

    char ip[16]; // IPv4
    int icmp_num;

    int status = parse_request(request, ip, &icmp_num);
    if (status != 200)
    {
        char response[1024];
        snprintf(response, sizeof(response), "HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n", status, (status == 501 ? "Not Implemented" : "Bad Request"));
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }

    if (icmp_num <= 0 || icmp_num > MAX_RESULTS)
    {
        char response[] = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }

    // 调用 ping 函数获取结果
    struct ping_result results[MAX_RESULTS];
    int ping_result_flag = ping(ip, icmp_num, results);
    if (ping_result_flag != 0)
    {
        char response[] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }

    // 构建响应头部
    char response[BUFFER_SIZE];
    int offset = snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\n");
    offset += snprintf(response + offset, sizeof(response) - offset, "Content-Type: text/plain\r\n");

    // 构建响应正文
    char response_body[BUFFER_SIZE - offset]; // 剩余空间用于响应正文
    int response_body_offset = 0;
    for (int i = 0; i < icmp_num; i++)
    {
        response_body_offset += snprintf(response_body + response_body_offset, sizeof(response_body) - response_body_offset, "ipv4:%s,ipv6:%s,seq:%d,time:%.2f\n", results[i].ipv4, results[i].ipv6, results[i].seq, results[i].time);
    }

    // 构建 Content-Length 头部
    int content_length = response_body_offset;
    offset += snprintf(response + offset, sizeof(response) - offset, "Content-Length: %d\r\n\r\n", content_length);

    // 将响应头部和响应正文拼接在一起
    memcpy(response + offset, response_body, response_body_offset);
    offset += response_body_offset;

    // 发送响应给客户端
    send(client_socket, response, offset, 0);
    close(client_socket);
}
