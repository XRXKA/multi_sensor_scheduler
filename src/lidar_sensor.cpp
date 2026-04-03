/**
 * @file lidar_sensor.cpp
 * @brief LiDAR传感器模拟器实现文件
 * 
 * 本文件实现了LiDAR（激光雷达）传感器的模拟器，以10Hz的频率生成模拟的LiDAR数据，
 * 并将数据提交给调度器进行处理。
 */

#include "../include/sensor_data.hpp"
#include "../include/scheduler.hpp"
#include "../include/logger.hpp"
#include <thread>
#include <chrono>

/**
 * @class LidarSensor
 * @brief LiDAR传感器模拟器类
 * 
 * 模拟激光雷达的工作，主要功能：
 * - 以10Hz频率生成模拟数据
 * - 为数据分配低优先级（优先级1）
 * - 将数据提交给调度器
 * - 支持启动和停止操作
 * 
 * LiDAR传感器特点：
 * - 频率：10Hz（每100毫秒生成一次数据）
 * - 数据大小：4096字节（较大的点云数据）
 * - 优先级：1（低优先级，因为LiDAR数据量最大但实时性要求相对较低）
 */
class LidarSensor {
public:
    /**
     * @brief 构造函数
     * @param scheduler 调度器引用
     * 
     * 初始化传感器，关联调度器，设置初始序列号为0
     */
    LidarSensor(Scheduler& scheduler)
        : scheduler(scheduler), running(false), sequence(0) {}
    
    /**
     * @brief 启动传感器
     * 
     * 启动数据生成线程，开始以10Hz频率生成数据
     */
    void start() {
        running = true;  // 设置运行标志
        sensorThread = std::thread(&LidarSensor::generateData, this);  // 启动数据生成线程
        LOG_INFO("LiDAR sensor started at 10Hz");  // 记录启动日志
    }
    
    /**
     * @brief 停止传感器
     * 
     * 停止数据生成线程，等待线程结束
     */
    void stop() {
        running = false;  // 清除运行标志
        if (sensorThread.joinable()) {
            sensorThread.join();  // 等待线程结束
        }
        LOG_INFO("LiDAR sensor stopped");  // 记录停止日志
    }
    
private:
    Scheduler& scheduler;           // 调度器引用，用于提交数据
    std::thread sensorThread;       // 数据生成线程
    std::atomic<bool> running;      // 运行标志，原子变量保证线程安全
    uint64_t sequence;              // 数据序列号，用于丢包检测
    
    /**
     * @brief 数据生成主循环
     * 
     * 线程的主函数，循环执行以下操作：
     * 1. 记录开始时间
     * 2. 创建传感器数据（类型：LIDAR，序列号递增，大小：4096字节，优先级：1）
     * 3. 将数据提交给调度器
     * 4. 计算已用时间，休眠剩余时间以保持10Hz频率
     */
    void generateData() {
        const auto interval = std::chrono::milliseconds(100); // 10Hz，每100毫秒一次
        
        while (running) {
            auto start = std::chrono::steady_clock::now();  // 记录开始时间
            
            // 生成模拟数据
            // 参数：传感器类型、序列号（递增）、数据大小、优先级（1为低）
            SensorData data(SensorType::LIDAR, sequence++, 4096, 1);
            
            // 添加到调度器
            scheduler.addSensorData(data);
            
            // 控制频率：计算已用时间，休眠剩余时间
            auto end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            if (elapsed < interval) {
                std::this_thread::sleep_for(interval - elapsed);
            }
        }
    }
};
