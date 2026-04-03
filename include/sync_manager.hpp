/**
 * @file sync_manager.hpp
 * @brief 时间戳同步管理器头文件
 * 
 * 本文件定义了时间戳同步管理器，用于检查传感器数据的时间戳是否符合预期的时间间隔，
 * 从而判断数据是否同步正常。通过比较相邻数据的时间间隔与预期频率来判断同步状态。
 */

#ifndef SYNC_MANAGER_HPP
#define SYNC_MANAGER_HPP

#include <map>
#include <chrono>
#include "sensor_data.hpp"

/**
 * @class SyncManager
 * @brief 时间戳同步管理器类
 * 
 * 负责检查传感器数据的时间同步状态，主要功能：
 * - 记录每种传感器最后一次数据的时间戳
 * - 检查数据时间间隔是否符合预期频率
 * - 判断数据是否同步成功
 * 
 * 同步判断逻辑：
 * 根据传感器的预期频率（Camera: 30Hz, IMU: 200Hz, LiDAR: 10Hz），
 * 计算预期的时间间隔，如果实际时间间隔与预期间隔的差值超过阈值，
 * 则认为同步失败。
 * 
 * 使用单例模式，通过 getInstance() 获取全局唯一实例
 */
class SyncManager {
public:
    /**
     * @brief 获取同步管理器单例实例
     * @return SyncManager实例的引用
     * 
     * 线程安全的单例获取方法
     */
    static SyncManager& getInstance();
    
    /**
     * @brief 同步传感器数据
     * @param data 传感器数据
     * @return 同步成功返回true，失败返回false
     * 
     * 检查数据的时间戳是否符合预期的时间间隔：
     * 1. 获取该类型传感器上次数据的时间戳
     * 2. 计算时间间隔
     * 3. 与预期间隔比较（根据传感器频率计算）
     * 4. 如果差值超过阈值，返回false
     * 5. 更新最后时间戳记录
     */
    bool syncData(const SensorData& data);
    
    /**
     * @brief 设置同步阈值
     * @param threshold 时间阈值（秒）
     * 
     * 设置允许的时间间隔偏差阈值，默认为0.01秒（10毫秒）
     */
    void setSyncThreshold(double threshold);
    
private:
    SyncManager();  // 私有构造函数，确保单例模式
    
    // 记录每种传感器最后一次数据的时间戳
    std::map<SensorType, std::chrono::steady_clock::time_point> lastTimestamps;
    
    double syncThreshold;  // 同步阈值（秒），允许的时间间隔偏差
};

#endif // SYNC_MANAGER_HPP
