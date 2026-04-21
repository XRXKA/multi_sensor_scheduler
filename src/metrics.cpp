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
#include <fstream>
#include <sstream>

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
    
    // 初始化每种传感器的成功处理数据包计数
    processedPackets[SensorType::CAMERA] = 0;
    processedPackets[SensorType::IMU] = 0;
    processedPackets[SensorType::LIDAR] = 0;
    
    // 初始化每种传感器的丢包计数
    lostPackets[SensorType::CAMERA] = 0;
    lostPackets[SensorType::IMU] = 0;
    lostPackets[SensorType::LIDAR] = 0;
    
    // 初始化CPU使用率数据存储
    cpuUsageData = std::vector<double>();
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
 * @brief 记录成功处理的数据包
 * @param type 传感器类型
 * 
 * 线程安全的成功处理数据包记录方法
 */
void Metrics::recordProcessedPacket(SensorType type) {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    processedPackets[type]++;  // 增加成功处理计数
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
 * @brief 记录CPU占用情况
 * 
 * 记录当前系统CPU使用率（Linux版本）
 */
void Metrics::recordCPUUsage() {
    #ifdef __linux__
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    
    // 读取/proc/stat文件获取CPU使用情况
    std::ifstream statFile("/proc/stat");
    std::string line;
    if (std::getline(statFile, line)) {
        std::istringstream iss(line);
        std::string cpu;
        unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
        
        if (iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice) {
            // 计算总CPU时间
            unsigned long total = user + nice + system + idle + iowait + irq + softirq + steal;
            // 计算CPU使用率
            double usage = 100.0 * (1.0 - static_cast<double>(idle) / total);
            cpuUsageData.push_back(usage);
        }
    }
    #endif
}

/**
 * @brief 获取平均CPU使用率
 * @return 平均CPU使用率（0.0-100.0）
 * 
 * 线程安全的查询方法，计算CPU使用率的平均值
 */
double Metrics::getAverageCPUUsage() const {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    if (cpuUsageData.empty()) {
        return 0.0;  // 如果没有数据，返回0
    }
    // 计算平均值
    double sum = 0.0;
    for (double usage : cpuUsageData) {
        sum += usage;
    }
    return sum / cpuUsageData.size();  // 返回平均值
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
    auto it = latencyData.find(type);  // 使用find方法查找
    if (it == latencyData.end() || it->second.empty()) {
        return 0.0;  // 如果没有数据，返回0
    }
    const auto& data = it->second;  // 获取该类型的延迟数据
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
    auto totalIt = totalPackets.find(type);  // 使用find方法查找总数据包数
    auto lostIt = lostPackets.find(type);    // 使用find方法查找丢包数
    if (totalIt == totalPackets.end() || totalIt->second == 0) {
        return 0.0;  // 如果没有数据包，返回0
    }
    size_t total = totalIt->second;  // 获取总数据包数
    size_t lost = (lostIt != lostPackets.end()) ? lostIt->second : 0;  // 获取丢包数
    // 计算丢包率
    return static_cast<double>(lost) / total;
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
 * @brief 获取指定传感器的成功处理数据包数量
 * @param type 传感器类型
 * @return 成功处理的数据包数量
 * 
 * 线程安全的查询方法
 */
size_t Metrics::getProcessedPackets(SensorType type) const {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    auto it = processedPackets.find(type);  // 使用find方法查找
    if (it == processedPackets.end()) {
        return 0;  // 如果没有数据，返回0
    }
    return it->second;  // 返回成功处理的数据包数量
}

/**
 * @brief 获取指定传感器的总数据包数量
 * @param type 传感器类型
 * @return 总数据包数量
 * 
 * 线程安全的查询方法
 */
size_t Metrics::getTotalPackets(SensorType type) const {
    std::lock_guard<std::mutex> lock(metricsMutex);  // 获取互斥锁
    auto it = totalPackets.find(type);  // 使用find方法查找
    if (it == totalPackets.end()) {
        return 0;  // 如果没有数据，返回0
    }
    return it->second;  // 返回总数据包数量
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
    // 重置成功处理数据包计数
    for (auto& pair : processedPackets) {
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
    // 清空CPU使用率数据
    cpuUsageData.clear();
}

/**
 * @brief 打印性能指标报告
 * 
 * 线程安全的打印方法，输出所有性能指标的统计结果到日志
 */
void Metrics::printMetrics() const {
    // 在锁外获取所有需要的数据，避免递归锁问题
    size_t cameraTotal = getTotalPackets(SensorType::CAMERA);
    size_t cameraProcessed = getProcessedPackets(SensorType::CAMERA);
    double cameraPassRate = (cameraTotal > 0) ? static_cast<double>(cameraProcessed) / cameraTotal * 100 : 0;
    
    size_t imuTotal = getTotalPackets(SensorType::IMU);
    size_t imuProcessed = getProcessedPackets(SensorType::IMU);
    double imuPassRate = (imuTotal > 0) ? static_cast<double>(imuProcessed) / imuTotal * 100 : 0;
    
    size_t lidarTotal = getTotalPackets(SensorType::LIDAR);
    size_t lidarProcessed = getProcessedPackets(SensorType::LIDAR);
    double lidarPassRate = (lidarTotal > 0) ? static_cast<double>(lidarProcessed) / lidarTotal * 100 : 0;
    
    double cameraLatency = getAverageLatency(SensorType::CAMERA) * 1000;
    double imuLatency = getAverageLatency(SensorType::IMU) * 1000;
    double lidarLatency = getAverageLatency(SensorType::LIDAR) * 1000;
    
    double cameraLossRate = getPacketLossRate(SensorType::CAMERA) * 100;
    double imuLossRate = getPacketLossRate(SensorType::IMU) * 100;
    double lidarLossRate = getPacketLossRate(SensorType::LIDAR) * 100;
    
    double syncSuccessRate = getSyncSuccessRate() * 100;
    size_t avgQueueSize = getAverageQueueSize();
    double avgCPUUsage = getAverageCPUUsage();
    
    // 现在一次性输出所有数据，不需要再持有锁
    LOG_INFO("=== Performance Metrics ===");  // 报告标题
    
    // 打印每种传感器的发包数和通过率
    LOG_INFO("Camera packets sent: " + std::to_string(cameraTotal));
    LOG_INFO("Camera packets processed: " + std::to_string(cameraProcessed));
    LOG_INFO("Camera pass rate: " + std::to_string(cameraPassRate) + "%");
    
    LOG_INFO("IMU packets sent: " + std::to_string(imuTotal));
    LOG_INFO("IMU packets processed: " + std::to_string(imuProcessed));
    LOG_INFO("IMU pass rate: " + std::to_string(imuPassRate) + "%");
    
    LOG_INFO("LiDAR packets sent: " + std::to_string(lidarTotal));
    LOG_INFO("LiDAR packets processed: " + std::to_string(lidarProcessed));
    LOG_INFO("LiDAR pass rate: " + std::to_string(lidarPassRate) + "%");
    
    // 打印每种传感器的平均延迟（转换为毫秒显示）
    LOG_INFO("Camera average latency: " + std::to_string(cameraLatency) + " ms");
    LOG_INFO("IMU average latency: " + std::to_string(imuLatency) + " ms");
    LOG_INFO("LiDAR average latency: " + std::to_string(lidarLatency) + " ms");
    
    // 打印丢包率（转换为百分比显示）
    LOG_INFO("Camera packet loss rate: " + std::to_string(cameraLossRate) + "%");
    LOG_INFO("IMU packet loss rate: " + std::to_string(imuLossRate) + "%");
    LOG_INFO("LiDAR packet loss rate: " + std::to_string(lidarLossRate) + "%");
    
    // 打印同步成功率（转换为百分比显示）
    LOG_INFO("Sync success rate: " + std::to_string(syncSuccessRate) + "%");
    
    // 打印平均队列大小
    LOG_INFO("Average queue size: " + std::to_string(avgQueueSize));
    
    // 打印CPU使用率
    LOG_INFO("Average CPU usage: " + std::to_string(avgCPUUsage) + "%");
    
    LOG_INFO("==========================");  // 报告结束
}
