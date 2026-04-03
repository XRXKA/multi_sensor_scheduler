/**
 * @file metrics.cpp
 * @brief 性能指标收集系统实现文件
 * 
 * 本文件实现了性能指标收集系统，用于统计和监控传感器数据处理的各项性能指标。
 */

#include "../include/metrics.hpp"
#include "../include/logger.hpp"
#include <iostream>
#include <algorithm>

/**
 * @brief 构造函数实现
 * 
 * 初始化所有统计数据结构：
 * - 为每种传感器创建空的延迟数据向量
 * - 初始化总数据包计数为0
 * - 初始化丢包计数为0
 * - 初始化同步统计为0
 */
Metrics::Metrics() : totalSyncAttempts(0), successfulSyncs(0) {
    // 初始化每种传感器的延迟数据存储
    latencyData[SensorType::CAMERA] = std::vector<double>();
    latencyData[SensorType::IMU] = std::vector<double>();
    latencyData[SensorType::LIDAR] = std::vector<double>();
    
    // 初始化每种传感器的总数据包计数
    totalPackets[SensorType::CAMERA] = 0;
    totalPackets[SensorType::IMU] = 0;
    totalPackets[SensorType::LIDAR] = 0;
    
    // 初始化每种传感器的丢包计数
    lostPackets[SensorType::CAMERA] = 0;
    lostPackets[SensorType::IMU] = 0;
    lostPackets[SensorType::LIDAR] = 0;
}

/**
 * @brief 获取指标收集器单例实例
 * @return Metrics实例的引用
 * 
 * 使用静态局部变量实现线程安全的单例模式（C++11保证）
 */
Metrics& Metrics::getInstance() {
    static Metrics instance;  // 静态局部变量，只初始化一次
    return instance;  // 返回单例实例的引用
}

/**
 * @brief 记录传感器数据延迟
 * @param type 传感器类型
 * @param latency 延迟时间（秒）
 * 
 * 线程安全的延迟记录方法
 */
void Metrics::recordLatency(SensorType type, double latency) {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    latencyData[type].push_back(latency);  // 添加延迟数据
    totalPackets[type]++;  // 增加总数据包计数
}

/**
 * @brief 记录丢包事件
 * @param type 传感器类型
 * 
 * 线程安全的丢包记录方法
 */
void Metrics::recordPacketLoss(SensorType type) {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    lostPackets[type]++;  // 增加丢包计数
    totalPackets[type]++;  // 增加总数据包计数
}

/**
 * @brief 记录同步尝试结果
 * @param success 同步是否成功
 * 
 * 线程安全的同步结果记录方法
 */
void Metrics::recordSyncSuccess(bool success) {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    totalSyncAttempts++;  // 增加同步尝试计数
    if (success) {
        successfulSyncs++;  // 如果成功，增加成功计数
    }
}

/**
 * @brief 记录队列大小
 * @param size 当前队列大小
 * 
 * 线程安全的队列大小记录方法
 */
void Metrics::recordQueueSize(size_t size) {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    queueSizes.push_back(size);  // 添加队列大小数据
}

/**
 * @brief 获取指定传感器的平均延迟
 * @param type 传感器类型
 * @return 平均延迟时间（秒）
 * 
 * 线程安全的查询方法，计算所有延迟数据的平均值
 */
double Metrics::getAverageLatency(SensorType type) const {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    const auto& data = latencyData[type];  // 获取该类型的延迟数据
    if (data.empty()) {
        return 0.0;  // 如果没有数据，返回0
    }
    // 计算平均值
    double sum = 0.0;
    for (double lat : data) {
        sum += lat;
    }
    return sum / data.size();  // 返回平均值
}

/**
 * @brief 获取指定传感器的丢包率
 * @param type 传感器类型
 * @return 丢包率（0.0-1.0）
 * 
 * 线程安全的查询方法，计算丢包率
 */
double Metrics::getPacketLossRate(SensorType type) const {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    size_t total = totalPackets[type];  // 获取总数据包数
    if (total == 0) {
        return 0.0;  // 如果没有数据包，返回0
    }
    // 计算丢包率
    return static_cast<double>(lostPackets[type]) / total;
}

/**
 * @brief 获取同步成功率
 * @return 同步成功率（0.0-1.0）
 * 
 * 线程安全的查询方法，计算同步成功率
 */
double Metrics::getSyncSuccessRate() const {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    if (totalSyncAttempts == 0) {
        return 0.0;  // 如果没有同步尝试，返回0
    }
    // 计算成功率
    return static_cast<double>(successfulSyncs) / totalSyncAttempts;
}

/**
 * @brief 获取平均队列大小
 * @return 平均队列大小
 * 
 * 线程安全的查询方法，计算队列大小的平均值
 */
size_t Metrics::getAverageQueueSize() const {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    if (queueSizes.empty()) {
        return 0;  // 如果没有数据，返回0
    }
    // 计算平均值
    size_t sum = 0;
    for (size_t size : queueSizes) {
        sum += size;
    }
    return sum / queueSizes.size();  // 返回平均值
}

/**
 * @brief 重置所有统计数据
 * 
 * 线程安全的重置方法，清空所有已收集的指标数据
 */
void Metrics::reset() {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    // 清空延迟数据
    for (auto& pair : latencyData) {
        pair.second.clear();
    }
    // 重置总数据包计数
    for (auto& pair : totalPackets) {
        pair.second = 0;
    }
    // 重置丢包计数
    for (auto& pair : lostPackets) {
        pair.second = 0;
    }
    // 重置同步统计
    totalSyncAttempts = 0;
    successfulSyncs = 0;
    // 清空队列大小数据
    queueSizes.clear();
}

/**
 * @brief 打印性能指标报告
 * 
 * 线程安全的打印方法，输出所有性能指标的统计结果到日志
 */
void Metrics::printMetrics() const {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    
    LOG_INFO("=== Performance Metrics ===");  // 报告标题
    
    // 打印每种传感器的平均延迟（转换为毫秒显示）
    LOG_INFO("Camera average latency: " + std::to_string(getAverageLatency(SensorType::CAMERA) * 1000) + " ms");
    LOG_INFO("IMU average latency: " + std::to_string(getAverageLatency(SensorType::IMU) * 1000) + " ms");
    LOG_INFO("LiDAR average latency: " + std::to_string(getAverageLatency(SensorType::LIDAR) * 1000) + " ms");
    
    // 打印丢包率（转换为百分比显示）
    LOG_INFO("Camera packet loss rate: " + std::to_string(getPacketLossRate(SensorType::CAMERA) * 100) + "%");
    LOG_INFO("IMU packet loss rate: " + std::to_string(getPacketLossRate(SensorType::IMU) * 100) + "%");
    LOG_INFO("LiDAR packet loss rate: " + std::to_string(getPacketLossRate(SensorType::LIDAR) * 100) + "%");
    
    // 打印同步成功率（转换为百分比显示）
    LOG_INFO("Sync success rate: " + std::to_string(getSyncSuccessRate() * 100) + "%");
    
    // 打印平均队列大小
    LOG_INFO("Average queue size: " + std::to_string(getAverageQueueSize()));
    
    LOG_INFO("==========================");  // 报告结束
}
