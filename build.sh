#!/bin/bash

# 构建脚本：清除旧的cmake产物并重新构建项目
# 同时将终端输出同步记录到log文件

echo "开始构建多传感器调度系统..."

# 定义log文件路径
LOG_FILE="build.log"

# 清除旧的build目录
if [ -d "build" ]; then
    echo "清除旧的build目录..."
    rm -rf build
fi

# 创建新的build目录
mkdir -p build

# 进入build目录
cd build

# 执行cmake和make，并将输出重定向到log文件
echo "执行cmake构建..."
echo "构建过程输出将记录到 $LOG_FILE"
echo "====================================="

# 执行cmake命令
cmake .. 2>&1 | tee ../$LOG_FILE

# 检查cmake是否成功
if [ $? -ne 0 ]; then
    echo "cmake失败，查看 $LOG_FILE 获取详细信息"
    exit 1
fi

echo "====================================="
echo "执行make构建..."

# 执行make命令
make 2>&1 | tee -a ../$LOG_FILE

# 检查make是否成功
if [ $? -ne 0 ]; then
    echo "make失败，查看 $LOG_FILE 获取详细信息"
    exit 1
fi

echo "====================================="
echo "构建完成！可执行文件位置: $(pwd)/sensor_scheduler"
echo "构建日志已保存到: $(pwd)/../$LOG_FILE"

# 返回到原始目录
cd ..
