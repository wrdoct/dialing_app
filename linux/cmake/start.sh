#!/usr/bin/env bash

service_name=$1

# 检查是否提供了服务名称作为参数
if [ -z "$service_name" ]; then
    echo "用法: $0 <service_name>"
    exit 1
fi

# 检查服务名称是否为 'icmp_ping'
if [[ $service_name != 'icmp_ping' ]]; then
    echo '无效的服务'
    exit 1
fi

# 构建 Docker 镜像
docker build --build-arg gcc_file=ping -t iot-icmp_ping:v1.0 . || {
    echo "无法构建 Docker 镜像。"
    exit 1
}

# 删除无用的镜像，如果没有则捕获错误并继续执行
if ! docker images | grep -q none; then
    echo "没有发现无用的镜像，无需执行 docker rmi 命令。"
else
    docker images | grep none | awk '{print $3}' | xargs docker rmi --force || {
        echo "无法删除无用的镜像。"
        exit 1
    }
fi

# 停止并删除现有容器
docker_id=$(docker ps -a --filter "name=${service_name}" --format "{{.ID}}")
if [ -n "$docker_id" ]; then
    if docker stop "$docker_id" && docker rm -f "$docker_id"; then
        echo "已成功停止和删除容器 $service_name。"
    else
        echo "无法停止或删除容器 $service_name。"
        exit 1
    fi
fi

# 运行 Docker 容器
if docker run --privileged=true \
              -v /dev/dri/renderD129:/dev/dri/renderD129 \
              -v /proc/device-tree/compatible:/proc/device-tree/compatible \
              -p 5268:5252 --name iot-icmp_ping -d iot-icmp_ping:v1.0; then
    echo '已成功启动 Docker 容器。'
else
    echo "无法启动 Docker 容器。"
    exit 1
fi

echo '重启服务完成'
