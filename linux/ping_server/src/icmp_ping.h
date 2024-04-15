#ifndef ICMP_PING_H
#define ICMP_PING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/socket.h>

#define ICMP_ECHO 8      /* Echo Request			*/
#define ICMP_ECHOREPLY 0 /* Echo Reply			*/

#define ICMP_DEST_UNREACH 3    /* Destination Unreachable	*/
#define ICMP_SOURCE_QUENCH 4   /* Source Quench		*/
#define ICMP_REDIRECT 5        /* Redirect (change route)	*/
#define ICMP_TIME_EXCEEDED 11  /* Time Exceeded		*/
#define ICMP_PARAMETERPROB 12  /* Parameter Problem		*/
#define ICMP_TIMESTAMP 13      /* Timestamp Request		*/
#define ICMP_TIMESTAMPREPLY 14 /* Timestamp Reply		*/
#define ICMP_INFO_REQUEST 15   /* Information Request		*/
#define ICMP_INFO_REPLY 16     /* Information Reply		*/
#define ICMP_ADDRESS 17        /* Address Mask Request		*/
#define ICMP_ADDRESSREPLY 18   /* Address Mask Reply		*/
#define NR_ICMP_TYPES 18

#define MAGIC "1234567890"       /**< ICMP Echo 请求的魔术字符串 */
#define MAGIC_LEN 11             /**< 魔术字符串长度 */
#define IP_BUFFER_SIZE 65536     /**< 接收缓冲区大小 */
#define RECV_TIMEOUT_USEC 100000 /**< 接收超时时间（微秒） */

#define IPV4_LEN 16
#define IPV6_LEN 40

/**
 * @brief ICMP Echo 请求数据结构  __attribute__((__packed__)) 属性告诉编译器不要对结构体中的字段进行任何填充,确保了结构体的大小正好是这些字段大小的总和
 */
struct __attribute__((__packed__)) icmp_echo
{
    uint8_t type;          /**< 类型 */
    uint8_t code;          /**< 代码 */
    uint16_t checksum;     /**< 校验和 */
    uint16_t ident;        /**< 标识符 */
    uint16_t seq;          /**< 序列号 */
    double sending_ts;     /**< 发送时间戳 */
    char magic[MAGIC_LEN]; /**< 魔术字符串 */
};

/**
 * @brief ping_result 返回的ping结果数据结构
 */
struct ping_result
{
    char ipv4[IPV4_LEN]; // 目标主机ip IPv4
    char ipv6[IPV6_LEN]; // 目标主机ip IPv6
    uint16_t seq;        // 发送的包的序列号
    double time;         // ms
};

/**
 * @brief 获取当前时间戳（秒为单位，包含微秒部分）
 * @return 当前时间戳
 */
double get_timestamp();

/**
 * @brief 计算校验和
 * @param buffer 数据缓冲区
 * @param bytes 数据长度
 * @return 计算得到的校验和
 */
uint16_t calculate_checksum(unsigned char *buffer, int bytes);

/**
 * @brief 发送 ICMP Echo 请求
 * @param sock 套接字描述符
 * @param addr 目标地址信息
 * @param ident 标识符
 * @param seq 序列号
 * @return 发送是否成功的状态码，成功返回0，失败返回-1
 */
int send_echo_request(int sock, struct sockaddr_in *addr, int ident, int seq);

/**
 * @brief 接收 ICMP Echo 应答
 * @param sock 套接字描述符
 * @param ident 标识符
 * @return 接收ping结果
 */
struct ping_result recv_echo_reply(int sock, int ident);

/**
 * @brief 发送 ICMP Ping 请求
 * @param ip 目标主机的 IP 地址字符串
 * @param icmp_num 向目标主机发送的icmp包个数
 * @param result 发送 ICMP Ping 请求的结果 (传入struct ping_result results[icmp_num];)
 * @return 发送 ICMP Ping 请求的状态码，成功返回0，失败返回-1
 */
int ping(const char *ip, int icmp_num, struct ping_result *result);

#endif /* ICMP_PING_H */
