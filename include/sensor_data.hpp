/**
 * @file sensor_data.hpp
 * @brief 传感器数据结构定义文件
 * 
 * 本文件定义了传感器数据的基本结构和枚举类型，
 * 用于在多传感器系统中统一管理不同类型的传感器数据。
 */

#ifndef SENSOR_DATA_HPP
#define SENSOR_DATA_HPP

#include <chrono>
#include <string>

/**
 * @enum SensorType
 * @brief 传感器类型枚举
 * 
 * 定义系统中支持的三种传感器类型：
 * - CAMERA: 摄像头传感器，频率30Hz
 * - IMU: 惯性测量单元，频率200Hz
 * - LIDAR: 激光雷达传感器，频率10Hz
 */
enum class SensorType {
    CAMERA,  // 摄像头传感器
    IMU,     // 惯性测量单元
    LIDAR    // 激光雷达传感器
};

/**
 * @struct SensorData
 * @brief 传感器数据结构体
 * 
 * 封装了传感器数据的所有相关信息，包括：
 * - 传感器类型
 * - 时间戳
 * - 序列号
 * - 数据负载大小
 * - 优先级
 * - 延迟信息
 */
struct SensorData {
    SensorType type;                              // 传感器类型
    std::chrono::steady_clock::time_point timestamp;  // 数据生成时间戳
    uint64_t sequence;                            // 数据序列号，用于丢包检测
    size_t payload_size;                          // 数据负载大小（字节）
    int priority;                                 // 数据处理优先级（数值越大优先级越高）
    std::chrono::duration<double> latency;        // 数据处理延迟
    
    /**
     * @brief 构造函数
     * @param t 传感器类型
     * @param seq 序列号
     * @param size 数据负载大小
     * @param prio 优先级
     * 
     * 构造函数会自动记录当前时间作为数据生成时间戳
     */
    SensorData(SensorType t, uint64_t seq, size_t size, int prio) 
        : type(t), sequence(seq), payload_size(size), priority(prio) {
        timestamp = std::chrono::steady_clock::now();  // 记录数据生成时刻
    }
    
    /**
     * @brief 将传感器类型转换为字符串
     * @return 传感器类型的字符串表示
     * 
     * 用于日志输出和调试信息显示
     */
    std::string typeToString() const {
        switch (type) {
            case SensorType::CAMERA: return "CAMERA";
            case SensorType::IMU: return "IMU";
            case SensorType::LIDAR: return "LIDAR";
            default: return "UNKNOWN";
        }
    }
};

#endif // SENSOR_DATA_HPP
