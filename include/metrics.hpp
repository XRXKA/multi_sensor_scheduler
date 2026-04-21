/**
 * @file metrics.hpp
 * @brief 性能指标收集系统头文件
 * 
 * 本文件定义了性能指标收集系统，用于统计和监控传感器数据处理的各项性能指标，
 * 包括延迟、丢包率、同步成功率和队列积压情况等。
 */

#ifndef METRICS_HPP
#define METRICS_HPP

#include <map>
#include <vector>
#include <chrono>
#include <string>
#include <mutex>
#include "sensor_data.hpp"

/**
 * @class Metrics
 * @brief 性能指标收集器类
 * 
 * 提供全面的性能指标收集和统计功能，支持：
 * - 各类传感器的延迟统计
 * - 丢包率统计
 * - 同步成功率统计
 * - 队列积压情况统计
 * 
 * 使用单例模式，通过 getInstance() 获取全局唯一实例
 * 所有方法都是线程安全的
 */
class Metrics {
public:
    /**
     * @brief 获取指标收集器单例实例
     * @return Metrics实例的引用
     * 
     * 线程安全的单例获取方法
     */
    static Metrics& getInstance();
    
    /**
     * @brief 记录传感器数据延迟
     * @param type 传感器类型
     * @param latency 延迟时间（秒）
     * 
     * 将延迟数据添加到对应传感器的统计中
     */
    void recordLatency(SensorType type, double latency);
    
    /**
     * @brief 记录丢包事件
     * @param type 传感器类型
     * 
     * 增加对应传感器的丢包计数
     */
    void recordPacketLoss(SensorType type);
    
    /**
     * @brief 记录成功处理的数据包
     * @param type 传感器类型
     * 
     * 增加对应传感器的成功处理计数
     */
    void recordProcessedPacket(SensorType type);
    
    /**
     * @brief 记录同步尝试结果
     * @param success 同步是否成功
     * 
     * 用于统计同步成功率
     */
    void recordSyncSuccess(bool success);
    
    /**
     * @brief 记录队列大小
     * @param size 当前队列大小
     * 
     * 用于统计队列积压情况
     */
    void recordQueueSize(size_t size);
    
    /**
     * @brief 记录CPU占用情况
     * 
     * 记录当前系统CPU使用率
     */
    void recordCPUUsage();
    
    /**
     * @brief 获取平均CPU使用率
     * @return 平均CPU使用率（0.0-100.0）
     */
    double getAverageCPUUsage() const;
    
    /**
     * @brief 获取指定传感器的平均延迟
     * @param type 传感器类型
     * @return 平均延迟时间（秒）
     */
    double getAverageLatency(SensorType type) const;
    
    /**
     * @brief 获取指定传感器的丢包率
     * @param type 传感器类型
     * @return 丢包率（0.0-1.0）
     */
    double getPacketLossRate(SensorType type) const;
    
    /**
     * @brief 获取指定传感器的成功处理数据包数量
     * @param type 传感器类型
     * @return 成功处理的数据包数量
     */
    size_t getProcessedPackets(SensorType type) const;
    
    /**
     * @brief 获取指定传感器的总数据包数量
     * @param type 传感器类型
     * @return 总数据包数量
     */
    size_t getTotalPackets(SensorType type) const;
    
    /**
     * @brief 获取同步成功率
     * @return 同步成功率（0.0-1.0）
     */
    double getSyncSuccessRate() const;
    
    /**
     * @brief 获取平均队列大小
     * @return 平均队列大小
     */
    size_t getAverageQueueSize() const;
    
    /**
     * @brief 重置所有统计数据
     * 
     * 清空所有已收集的指标数据
     */
    void reset();
    
    /**
     * @brief 打印性能指标报告
     * 
     * 输出所有性能指标的统计结果到日志
     */
    void printMetrics() const;
    
private:
    Metrics();  // 私有构造函数，确保单例模式
    
    mutable std::mutex metricsMutex;  // 互斥锁，保证线程安全
    
    // 延迟数据存储，按传感器类型分类
    std::map<SensorType, std::vector<double>> latencyData;
    
    // 总数据包计数，按传感器类型分类
    std::map<SensorType, size_t> totalPackets;
    
    // 成功处理的数据包计数，按传感器类型分类
    std::map<SensorType, size_t> processedPackets;
    
    // 丢包计数，按传感器类型分类
    std::map<SensorType, size_t> lostPackets;
    
    // 同步尝试总次数
    size_t totalSyncAttempts;
    
    // 同步成功次数
    size_t successfulSyncs;
    
    // 队列大小历史记录
    std::vector<size_t> queueSizes;
    
    // CPU使用率历史记录
    std::vector<double> cpuUsageData;
};

#endif // METRICS_HPP
