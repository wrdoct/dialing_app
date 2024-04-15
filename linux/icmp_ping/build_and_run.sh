#!/bin/bash

# 构建 Docker 镜像
docker build -t icmp-ping-image .

# 从脚本参数中获取目标 IP 地址
target_ip="$1"

# 运行容器并指定目标 IP 地址
docker run --rm --privileged icmp-ping-image "$target_ip"  # docker run -it icmp-ping-image bash