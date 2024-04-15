#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

int main(void)
{
    CURL *curl;
    CURLcode res;

    // 初始化 libcurl
    curl = curl_easy_init();
    if (curl)
    {
        // 设置要访问的 URL
        curl_easy_setopt(curl, CURLOPT_URL, "http://example.com");

        // 发送 HTTP GET 请求
        res = curl_easy_perform(curl);

        // 检查是否发生错误
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // 清理 libcurl 资源
        curl_easy_cleanup(curl);
    }

    return 0;
}
