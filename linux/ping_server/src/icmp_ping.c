#include "icmp_ping.h"

double get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL); // 系统调用，用于获取当前时间
    return tv.tv_sec + ((double)tv.tv_usec) / 1000000;
}

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

struct ping_result recv_echo_reply(int sock, int ident)
{
    struct ping_result default_result = {"", "", 0, 0.00};
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
            return default_result;
        }
        perror("Failed to receive ICMP echo reply");
        return default_result;
    }

    // IP头部长度
    int ip_header_len = (buffer[0] & 0xf) << 2;
    // 从 IP 报文中取出 ICMP 报文
    struct icmp_echo *icmp = (struct icmp_echo *)(buffer + ip_header_len); // ICMP 报文紧随 IP 头部之后
    if (icmp->type != ICMP_ECHOREPLY || icmp->code != 0)
    {
        return default_result;
    }

    // ntohs()是一个函数名，作用是将一个16位数由网络字节顺序转换为主机字节顺序
    if (ntohs(icmp->ident) != ident)
    {
        return default_result;
    }

    double time_ms = (get_timestamp() - icmp->sending_ts) * 1000;
    printf("%s seq=%-5d %8.2fms\n",
           inet_ntoa(peer_addr.sin_addr),
           ntohs(icmp->seq),
           time_ms);

    struct ping_result ping_result;
    bzero(&ping_result, sizeof(ping_result));
    strcpy(ping_result.ipv4, inet_ntoa(peer_addr.sin_addr));
    ping_result.seq = ntohs(icmp->seq);
    ping_result.time = time_ms;

    return ping_result;
}

int ping(const char *ip, int icmp_num, struct ping_result *result)
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
        close(sock);
        return -1;
    }

    double next_ts = get_timestamp();
    int ident = getpid(); // 取得进程识别码
    int seq = 1;
    int exit_flag = 0;

    for (;;)
    {
        double current_ts = get_timestamp();
        if (current_ts >= next_ts)
        {
            ret = send_echo_request(sock, &addr, ident, seq);
            if (ret == -1)
            {
                perror("Send failed");
                close(sock);
                return -1;
            }
            next_ts = current_ts + 1;
            seq++;
        }

        struct ping_result ping_result = recv_echo_reply(sock, ident);
        if (ping_result.seq != 0)
        {
            result[exit_flag] = ping_result;
            exit_flag++;
        }
        if (exit_flag >= icmp_num)
            break;
    }

    return 0;
}
