/**
 * @file scheduler.cpp
 * @brief 数据调度器实现文件
 * 
 * 本文件实现了传感器数据调度器，负责接收、缓冲和处理来自不同传感器的数据，
 * 并提供优先级调度和丢包检测功能。
 */

#include "../include/scheduler.hpp"
#include "../include/sync_manager.hpp"
#include "../include/metrics.hpp"
#include "../include/logger.hpp"
#include <chrono>

/**
 * @brief 构造函数实现
 * @param bufferSize 环形缓冲区大小
 * 
 * 初始化调度器：
 * - 创建指定大小的环形缓冲区
 * - 设置运行标志为false
 */
Scheduler::Scheduler(size_t bufferSize)
    : buffer(bufferSize), running(false) {}

/**
 * @brief 析构函数实现
 * 
 * 确保调度器停止运行，释放资源
 */
Scheduler::~Scheduler() {
    stop();  // 停止调度器
}

/**
 * @brief 启动调度器
 * 
 * 启动数据处理线程，开始接收和处理数据
 */
void Scheduler::start() {
    running = true;  // 设置运行标志
    processingThread = std::thread(&Scheduler::processData, this);  // 启动处理线程
    LOG_INFO("Scheduler started");  // 记录启动日志
}

/**
 * @brief 停止调度器
 * 
 * 停止数据处理线程，等待线程结束
 */
void Scheduler::stop() {
    running = false;  // 清除运行标志
    buffer.shutdown();  // 关闭缓冲区，通知所有阻塞的线程
    if (processingThread.joinable()) {
        processingThread.join();  // 等待线程结束
    }
    LOG_INFO("Scheduler stopped");  // 记录停止日志
}

/**
 * @brief 添加传感器数据到调度器
 * @param data 传感器数据
 * @return 成功返回true，缓冲区已满返回false
 * 
 * 线程安全的数据提交方法，将数据推入环形缓冲区
 */
bool Scheduler::addSensorData(const SensorData& data) {
    return buffer.push(data);  // 将数据推入缓冲区
}

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
void Scheduler::processData() {
    while (running) {
        // 记录CPU使用率
        Metrics::getInstance().recordCPUUsage();
        
        // 从环形缓冲区读取数据（阻塞式）
        auto dataOpt = buffer.pop();
        if (!dataOpt) {
            continue;  // 没有数据，继续循环
        }
        
        SensorData data = *dataOpt;  // 解包数据
        
        // 计算延迟：当前时间 - 数据生成时间
        auto now = std::chrono::steady_clock::now();
        data.latency = now - data.timestamp;
        
        // 记录延迟到性能指标
        Metrics::getInstance().recordLatency(data.type, data.latency.count());
        
        // 记录队列大小到性能指标
        Metrics::getInstance().recordQueueSize(buffer.size());
        
        // 检查是否有丢包（通过检测序列号）
        // 使用static变量保存每种传感器的最后序列号
        static std::map<SensorType, uint64_t> lastSequence;
        auto it = lastSequence.find(data.type);
        if (it != lastSequence.end() && data.sequence > it->second + 1) {
            // 检测到丢包：当前序列号 > 上次序列号 + 1
            size_t lostCount = data.sequence - it->second - 1;  // 计算丢包数量
            LOG_WARNING("Packet loss detected for " + data.typeToString() + ": " + std::to_string(lostCount) + " packets");
            
            // 记录每个丢包事件
            for (size_t i = 0; i < lostCount; i++) {
                Metrics::getInstance().recordPacketLoss(data.type);
            }
        }
        // 更新该类型传感器的最后序列号
        lastSequence[data.type] = data.sequence;
        
        // 添加到优先级队列
        { 
            std::lock_guard<std::mutex> lock(queueMutex);  // 获取队列锁
            priorityQueue.push({data});  // 将数据包装后推入优先级队列
        }
        queueCond.notify_one();  // 通知等待的线程
        
        // 处理优先级队列中的数据
        while (!priorityQueue.empty()) {
            SensorData prioritizedData;
            {
                std::lock_guard<std::mutex> lock(queueMutex);  // 获取队列锁
                if (priorityQueue.empty()) {
                    break;  // 队列为空，退出循环
                }
                // 获取优先级最高的数据
                prioritizedData = priorityQueue.top().data;
                priorityQueue.pop();  // 移除已处理的数据
            }
            
            // 同步数据：检查时间戳是否符合预期
            bool syncSuccess = SyncManager::getInstance().syncData(prioritizedData);
            Metrics::getInstance().recordSyncSuccess(syncSuccess);  // 记录同步结果
            
            // 处理数据（这里只是模拟处理）
            LOG_DEBUG("Processing " + prioritizedData.typeToString() + " data, sequence: " + std::to_string(prioritizedData.sequence) + ", latency: " + std::to_string(prioritizedData.latency.count() * 1000) + "ms, priority: " + std::to_string(prioritizedData.priority));
            
            // 记录成功处理的数据包
            Metrics::getInstance().recordProcessedPacket(prioritizedData.type);
            
            // 模拟处理时间
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}
