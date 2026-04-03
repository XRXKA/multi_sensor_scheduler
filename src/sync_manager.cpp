/**
 * @file sync_manager.cpp
 * @brief 时间戳同步管理器实现文件
 * 
 * 本文件实现了时间戳同步管理器，用于检查传感器数据的时间戳是否符合预期的时间间隔，
 * 从而判断数据是否同步正常。
 */

#include "../include/sync_manager.hpp"

/**
 * @brief 构造函数实现
 * 
 * 初始化同步阈值为0.01秒（10毫秒）
 * 这个阈值用于判断时间间隔是否在合理范围内
 */
SyncManager::SyncManager() : syncThreshold(0.01) {} // 10ms threshold

/**
 * @brief 获取同步管理器单例实例
 * @return SyncManager实例的引用
 * 
 * 使用静态局部变量实现线程安全的单例模式（C++11保证）
 */
SyncManager& SyncManager::getInstance() {
    static SyncManager instance;  // 静态局部变量，只初始化一次
    return instance;  // 返回单例实例的引用
}

/**
 * @brief 同步传感器数据
 * @param data 传感器数据
 * @return 同步成功返回true，失败返回false
 * 
 * 检查数据的时间戳是否符合预期的时间间隔：
 * 1. 获取该类型传感器上次数据的时间戳
 * 2. 计算时间间隔
 * 3. 根据传感器类型确定预期时间间隔
 * 4. 比较实际间隔与预期间隔
 * 5. 如果差值超过阈值，返回false
 * 6. 更新最后时间戳记录
 */
bool SyncManager::syncData(const SensorData& data) {
    auto now = data.timestamp;  // 获取当前数据的时间戳
    
    // 检查与上次数据的时间差
    auto it = lastTimestamps.find(data.type);
    if (it != lastTimestamps.end()) {
        // 计算与上次数据的时间间隔（秒）
        auto timeDiff = std::chrono::duration<double>(now - it->second).count();
        
        // 根据传感器类型确定预期时间间隔
        double expectedInterval = 0.0;
        switch (data.type) {
            case SensorType::CAMERA:
                expectedInterval = 1.0 / 30.0; // 30Hz，预期间隔约33.3ms
                break;
            case SensorType::IMU:
                expectedInterval = 1.0 / 200.0; // 200Hz，预期间隔5ms
                break;
            case SensorType::LIDAR:
                expectedInterval = 1.0 / 10.0; // 10Hz，预期间隔100ms
                break;
        }
        
        // 检查时间间隔是否在合理范围内
        // 如果实际间隔与预期间隔的差值超过阈值，认为同步失败
        if (std::abs(timeDiff - expectedInterval) > syncThreshold) {
            return false;  // 同步失败
        }
    }
    
    // 更新该类型传感器的最后时间戳
    lastTimestamps[data.type] = now;
    return true;  // 同步成功
}

/**
 * @brief 设置同步阈值
 * @param threshold 时间阈值（秒）
 * 
 * 设置允许的时间间隔偏差阈值
 * 默认为0.01秒（10毫秒），可根据实际需求调整
 */
void SyncManager::setSyncThreshold(double threshold) {
    syncThreshold = threshold;  // 更新同步阈值
}
