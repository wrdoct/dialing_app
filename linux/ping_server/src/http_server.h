#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "icmp_ping.h"

#define PORT 8080
#define MAX_CONNECTIONS 10
#define MAX_RESULTS 100

#define BUFFER_SIZE 1024
#define HTTP_VERSION "HTTP/1.1"
#define RESPONSE_OK "HTTP/1.1 200 OK\r\nContent-Type: text/csv\r\nContent-Length: %d\r\n\r\n"
// #define RESPONSE_TEMPLATE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s"

/*
 * 解析 HTTP 请求，检查请求的有效性
 * 参数:
 *   request: HTTP 请求字符串
 *   ip: 存储提取的 IP 地址
 *   icmp_num: 存储提取的 ICMP 数量
 * 返回值:
 *   200: 请求有效
 *   400: 错误的请求
 *   501: 不支持的请求方法
 */
int parse_request(const char *request, char *ip, int *icmp_num);

/*
 * 处理客户端请求
 * 参数:
 *   client_socket: 客户端套接字
 */
void handle_client(int client_socket);

#endif /* HTTP_SERVER_H */
