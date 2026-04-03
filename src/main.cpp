/**
 * @file main.cpp
 * @brief 主程序入口文件
 * 
 * 本文件是多传感器实时数据同步与调度系统的主程序入口，
 * 负责初始化系统、启动传感器和调度器、运行测试并输出性能指标。
 */

#include "../include/scheduler.hpp"
#include "../include/metrics.hpp"
#include "../include/logger.hpp"
#include "camera_sensor.cpp"
#include "imu_sensor.cpp"
#include "lidar_sensor.cpp"
#include <thread>
#include <chrono>

/**
 * @brief 主函数
 * @return 程序退出码，0表示成功
 * 
 * 程序执行流程：
 * 1. 初始化日志系统
 * 2. 创建调度器（缓冲区大小1000）
 * 3. 创建三种传感器实例
 * 4. 启动调度器
 * 5. 启动所有传感器
 * 6. 运行10秒钟
 * 7. 停止传感器和调度器
 * 8. 打印性能指标
 */
int main() {
    // 初始化日志系统
    Logger::getInstance().setLogLevel(LogLevel::INFO);  // 设置日志级别为INFO
    Logger::getInstance().setLogFile("sensor_scheduler.log");  // 设置日志文件
    
    LOG_INFO("Starting multi-sensor scheduler system");  // 记录系统启动
    
    // 创建调度器，缓冲区大小为1000
    // 这个大小足够容纳高频传感器产生的数据
    Scheduler scheduler(1000);
    
    // 创建传感器实例，传入调度器引用
    CameraSensor camera(scheduler);  // Camera传感器，30Hz
    ImuSensor imu(scheduler);        // IMU传感器，200Hz
    LidarSensor lidar(scheduler);    // LiDAR传感器，10Hz
    
    // 启动调度器，开始数据处理线程
    scheduler.start();
    
    // 启动所有传感器，开始数据生成线程
    camera.start();  // 启动Camera传感器
    imu.start();     // 启动IMU传感器
    lidar.start();   // 启动LiDAR传感器
    
    // 运行10秒钟，收集性能数据
    LOG_INFO("System running for 10 seconds...");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // 停止传感器，按逆序停止（先启动的后停止）
    lidar.stop();   // 停止LiDAR传感器
    imu.stop();     // 停止IMU传感器
    camera.stop();  // 停止Camera传感器
    
    // 停止调度器，等待数据处理线程结束
    scheduler.stop();
    
    // 打印性能指标报告
    LOG_INFO("Printing performance metrics...");
    Metrics::getInstance().printMetrics();
    
    LOG_INFO("Multi-sensor scheduler system stopped");  // 记录系统停止
    
    return 0;  // 程序正常退出
}
