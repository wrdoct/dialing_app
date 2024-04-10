/**
 * @file icmp_ping.c
 * @brief 使用原始套接字实现的简单 ICMP Ping 工具
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

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
 * @brief 获取当前时间戳（秒为单位，包含微秒部分）
 * @return 当前时间戳
 */
double get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL); // 系统调用，用于获取当前时间
    return tv.tv_sec + ((double)tv.tv_usec) / 1000000;
}

/**
 * @brief 计算校验和
 * @param buffer 数据缓冲区
 * @param bytes 数据长度
 * @return 计算得到的校验和
 */
uint16_t calculate_checksum(unsigned char *buffer, int bytes)
{
    uint32_t checksum = 0;
    unsigned char *end = buffer + bytes;

    // 奇数字节添加最后一个字节并重置end
    if (bytes % 2 == 1)
    {
        end = buffer + bytes - 1;
        checksum += (*end) << 8; // 左移8位的目的是将8位的值转换为16位
    }

    while (buffer < end)
    {
        checksum += (buffer[0] << 8) + buffer[1];
        uint32_t carry = checksum >> 16; // 进位
        if (carry != 0)
        {
            checksum = (checksum & 0xffff) + carry;
        }
        buffer += 2;
    }

    checksum = ~checksum;
    return checksum & 0xffff;
}

/**
 * @brief 发送 ICMP Echo 请求
 * @param sock 套接字描述符
 * @param addr 目标地址信息
 * @param ident 标识符
 * @param seq 序列号
 * @return 发送是否成功的状态码，成功返回0，失败返回-1
 */
int send_echo_request(int sock, struct sockaddr_in *addr, int ident, int seq)
{
    struct icmp_echo icmp;
    bzero(&icmp, sizeof(icmp));
    icmp.type = ICMP_ECHO;
    icmp.code = 0;
    icmp.ident = htons(ident); // htons函数将进程识别码转换为网络字节序
    icmp.seq = htons(seq);
    strncpy(icmp.magic, MAGIC, MAGIC_LEN); // 用于在ICMP Echo请求消息中填充一些特定信息
    icmp.sending_ts = get_timestamp();
    icmp.checksum = htons(calculate_checksum((unsigned char *)&icmp, sizeof(icmp)));

    /*
        sendto() 用来将数据由指定的 socket 传给对方主机
        参数1：socket文件描述符
        参数2：发送数据的首地址
        参数3：数据长度
        参数4：0表示默认方式发送
        参数5：存放目的主机的IP和端口信息
        参数6: 参数5的长度
    */
    int bytes = sendto(sock, &icmp, sizeof(icmp), 0, (struct sockaddr *)addr, sizeof(*addr));
    if (bytes == -1)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief 接收 ICMP Echo 应答
 * @param sock 套接字描述符
 * @param ident 标识符
 * @return 接收是否成功的状态码，成功返回0，失败返回-1
 */
int recv_echo_reply(int sock, int ident)
{
    // 定义缓冲区
    unsigned char buffer[IP_BUFFER_SIZE];
    struct sockaddr_in peer_addr;
    int addr_len = sizeof(peer_addr);
    /*
        recvfrom()本函数用于从（已连接）套接口上接收数据，并捕获数据发送源的地址
        s：标识一个已连接套接口的描述字。
        buf：接收数据缓冲区。
        len：缓冲区长度。
        flags：调用操作方式。
        from：（可选）指针，指向装有源地址的缓冲区。
        fromlen：（可选）指针，指向from缓冲区长度值。
    */
    int bytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&peer_addr, &addr_len);
    if (bytes == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return 0;
        }
        return -1;
    }

    // IP头部长度
    int ip_header_len = (buffer[0] & 0xf) << 2;
    // 从 IP 报文中取出 ICMP 报文
    struct icmp_echo *icmp = (struct icmp_echo *)(buffer + ip_header_len); // ICMP 报文紧随 IP 头部之后
    if (icmp->type != ICMP_ECHOREPLY || icmp->code != 0)
    {
        return 0;
    }

    // ntohs()是一个函数名，作用是将一个16位数由网络字节顺序转换为主机字节顺序
    if (ntohs(icmp->ident) != ident)
    {
        return 0;
    }

    printf("%s seq=%-5d %8.2fms\n",
           inet_ntoa(peer_addr.sin_addr),
           ntohs(icmp->seq),
           (get_timestamp() - icmp->sending_ts) * 1000);

    return 0;
}

/**
 * @brief 发送 ICMP Ping 请求
 * @param ip 目标主机的 IP 地址字符串
 * @return 发送 ICMP Ping 请求的状态码，成功返回0，失败返回-1
 */
int ping(const char *ip)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET; // IPv4
    addr.sin_port = 0;
    // inet_aton是一个计算机函数，功能是将一个字符串IP地址转换为一个32位的网络序列IP地址。
    if (inet_aton(ip, (struct in_addr *)&addr.sin_addr.s_addr) == 0)
    {
        fprintf(stderr, "bad ip address: %s\n", ip);
        return -1;
    }

    // 创建一个原始套接字，协议类型为 IPPROTO_ICMP
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1)
    {
        perror("create raw socket");
        return -1;
    }

    // 设置接收超时时间
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = RECV_TIMEOUT_USEC;
    int ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (ret == -1)
    {
        perror("set socket option");
        return -1;
    }

    double next_ts = get_timestamp();
    int ident = getpid(); // 取得进程识别码
    int seq = 1;

    for (;;)
    {
        double current_ts = get_timestamp();
        if (current_ts >= next_ts)
        {
            ret = send_echo_request(sock, &addr, ident, seq);
            if (ret == -1)
            {
                perror("Send failed");
            }
            next_ts = current_ts + 1;
            seq += 1;
        }

        ret = recv_echo_reply(sock, ident);
        if (ret == -1)
        {
            perror("Receive failed");
        }
    }

    return 0;
}

/**
 * @brief 主函数，用于解析命令行参数并调用 ICMP Ping 函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数列表
 * @return 程序执行结果的状态码，成功返回0，失败返回-1
 */
int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "no host specified");
        return -1;
    }
    return ping(argv[1]);
}
