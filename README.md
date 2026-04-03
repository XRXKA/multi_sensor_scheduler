# 基于 C++/Linux 的多传感器实时数据同步与调度系统

## 项目概述

本项目模拟机器人中常见的三类传感器（Camera、IMU、LiDAR）的数据生成和处理，实现了一个实时数据同步与调度系统。系统包含以下核心功能：

- 多线程传感器数据生成
- RingBuffer 缓冲
- 条件变量 + 多线程调度
- 时间戳同步
- 优先级队列
- 异常丢包检测
- 延迟统计
- 日志与监控

## 传感器规格

| 传感器 | 频率 | 优先级 | 数据大小 |
|--------|------|--------|----------|
| Camera | 30Hz | 2 (中等) | 1024 bytes |
| IMU    | 200Hz | 3 (高) | 128 bytes |
| LiDAR  | 10Hz | 1 (低) | 4096 bytes |

## 目录结构

```
multi_sensor_scheduler/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── sensor_data.hpp
│   ├── ring_buffer.hpp
│   ├── scheduler.hpp
│   ├── sync_manager.hpp
│   ├── metrics.hpp
│   └── logger.hpp
├── src/
│   ├── main.cpp
│   ├── camera_sensor.cpp
│   ├── imu_sensor.cpp
│   ├── lidar_sensor.cpp
│   ├── scheduler.cpp
│   ├── sync_manager.cpp
│   ├── metrics.cpp
│   └── logger.cpp
├── scripts/
│   └── plot_latency.py
└── docs/
    ├── architecture.png
    └── performance_report.md
```

## 构建与运行

### 构建

```bash
mkdir build
cd build
cmake ..
make
```

### 运行

```bash
./sensor_scheduler
```

系统会运行10秒钟，然后输出性能指标。

## 性能指标

系统会输出以下性能指标：
- 各类传感器数据的平均延迟
- 丢包率
- 同步成功率
- 队列积压情况
- CPU占用情况

## 依赖

- C++11 或更高版本
- CMake 3.10 或更高版本
- pthread 库
- Python 3 (用于绘制延迟图表)
- matplotlib (用于绘制延迟图表)

## 扩展与定制

- 可以修改传感器的频率、优先级和数据大小
- 可以调整缓冲区大小
- 可以修改同步阈值
- 可以扩展支持更多类型的传感器
