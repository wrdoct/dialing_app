# 使用基础镜像
FROM ubuntu:latest

# 更新软件包列表并安装必要的软件
RUN apt-get update && apt-get install -y gcc && apt-get install -y cmake && apt-get install -y make

#在容器中创建了一个目录 /app，其中 -p 选项确保在创建过程中递归地创建所需的父目录。
RUN mkdir /app -p

# 将工作目录切换到 /app
WORKDIR /app

# 复制源代码到容器中的 /app 目录
COPY . /app

# 使用 CMake 构建项目
RUN cmake -Bbuild -H. && cmake --build build

# 使用 make 生成可执行文件
RUN make

# # 设置容器启动时执行的命令
# CMD ["./ping"]