/**
 * @file scheduler.hpp
 * @brief 数据调度器头文件
 * 
 * 本文件定义了传感器数据调度器，负责接收、缓冲和处理来自不同传感器的数据。
 * 调度器使用优先级队列确保高优先级数据优先处理，同时提供丢包检测功能。
 */

#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "sensor_data.hpp"
#include "ring_buffer.hpp"

/**
 * @class Scheduler
 * @brief 传感器数据调度器类
 * 
 * 核心调度模块，负责：
 * - 接收来自各传感器的数据
 * - 使用环形缓冲区暂存数据
 * - 基于优先级调度数据处理
 * - 检测丢包情况
 * - 统计延迟和同步成功率
 * 
 * 工作流程：
 * 1. 传感器线程通过 addSensorData() 提交数据
 * 2. 数据首先进入环形缓冲区
 * 3. 处理线程从缓冲区读取数据
 * 4. 数据按优先级排序后处理
 * 5. 处理过程中记录各项性能指标
 */
class Scheduler {
public:
    /**
     * @brief 构造函数
     * @param bufferSize 环形缓冲区大小
     * 
     * 创建调度器并初始化指定大小的缓冲区
     */
    Scheduler(size_t bufferSize);
    
    /**
     * @brief 析构函数
     * 
     * 停止调度器并清理资源
     */
    ~Scheduler();
    
    /**
     * @brief 启动调度器
     * 
     * 启动数据处理线程，开始接收和处理数据
     */
    void start();
    
    /**
     * @brief 停止调度器
     * 
     * 停止数据处理线程，等待线程结束
     */
    void stop();
    
    /**
     * @brief 添加传感器数据到调度器
     * @param data 传感器数据
     * @return 成功返回true，缓冲区已满返回false
     * 
     * 线程安全的数据提交方法，由传感器线程调用
     */
    bool addSensorData(const SensorData& data);
    
private:
    /**
     * @struct PriorityData
     * @brief 优先级数据包装结构
     * 
     * 用于优先级队列的数据包装，重载比较运算符实现优先级排序
     */
    struct PriorityData {
        SensorData data;  // 传感器数据
        
        /**
         * @brief 比较运算符重载
         * @param other 另一个PriorityData对象
         * @return 当前对象优先级较低时返回true
         * 
         * 注意：优先级队列是最大堆，返回true表示当前对象优先级较低
         */
        bool operator<(const PriorityData& other) const {
            return data.priority < other.data.priority;
        }
    };
    
    RingBuffer buffer;                           // 环形缓冲区，暂存传感器数据
    std::priority_queue<PriorityData> priorityQueue;  // 优先级队列，按优先级处理数据
    
    std::thread processingThread;                // 数据处理线程
    std::atomic<bool> running;                   // 运行标志，原子变量保证线程安全
    
    std::mutex queueMutex;                       // 优先级队列互斥锁
    std::condition_variable queueCond;           // 条件变量，通知队列状态变化
    
    /**
     * @brief 数据处理主循环
     * 
     * 处理线程的主函数，循环执行以下操作：
     * 1. 从环形缓冲区读取数据
     * 2. 计算数据延迟
     * 3. 检测丢包情况
     * 4. 将数据加入优先级队列
     * 5. 按优先级处理数据
     * 6. 记录性能指标
     */
    void processData();
};

#endif // SCHEDULER_HPP
