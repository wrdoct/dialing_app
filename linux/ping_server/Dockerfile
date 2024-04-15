# 使用基础的 Ubuntu 镜像作为基础
FROM ubuntu:18.04

# 设置时区
ENV TZ=Asia/Shanghai

# 设置工作目录
WORKDIR /usr/src/app

# 将应用程序复制到容器中
COPY ping /usr/src/app

# 为所有文件添加可执行权限
RUN chmod +x ./*

# # 设置容器启动时执行的默认命令，使用 bash shell
# CMD ["sh", "-c", "./run.sh"]