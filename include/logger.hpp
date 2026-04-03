/**
 * @file logger.hpp
 * @brief 日志系统头文件
 * 
 * 本文件定义了一个线程安全的日志系统，支持多级别日志输出，
 * 可同时输出到控制台和文件。采用单例模式设计，全局唯一实例。
 */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>

/**
 * @enum LogLevel
 * @brief 日志级别枚举
 * 
 * 定义了四个日志级别，按严重程度递增：
 * - DEBUG: 调试信息，用于开发阶段的详细跟踪
 * - INFO: 一般信息，用于记录正常运行状态
 * - WARNING: 警告信息，用于记录潜在问题
 * - ERROR: 错误信息，用于记录严重错误
 */
enum class LogLevel {
    DEBUG,    // 调试级别
    INFO,     // 信息级别
    WARNING,  // 警告级别
    ERROR     // 错误级别
};

/**
 * @class Logger
 * @brief 日志记录器类
 * 
 * 提供线程安全的日志记录功能，支持：
 * - 多级别日志输出
 * - 同时输出到控制台和文件
 * - 时间戳标记
 * - 日志级别过滤
 * 
 * 使用单例模式，通过 getInstance() 获取全局唯一实例
 */
class Logger {
public:
    /**
     * @brief 获取日志记录器单例实例
     * @return Logger实例的引用
     * 
     * 线程安全的单例获取方法
     */
    static Logger& getInstance();
    
    /**
     * @brief 设置日志输出级别
     * @param level 日志级别
     * 
     * 只有大于等于此级别的日志才会被输出
     */
    void setLogLevel(LogLevel level);
    
    /**
     * @brief 设置日志输出文件
     * @param filename 日志文件路径
     * 
     * 日志将同时输出到控制台和指定文件
     */
    void setLogFile(const std::string& filename);
    
    /**
     * @brief 输出DEBUG级别日志
     * @param message 日志消息内容
     */
    void debug(const std::string& message);
    
    /**
     * @brief 输出INFO级别日志
     * @param message 日志消息内容
     */
    void info(const std::string& message);
    
    /**
     * @brief 输出WARNING级别日志
     * @param message 日志消息内容
     */
    void warning(const std::string& message);
    
    /**
     * @brief 输出ERROR级别日志
     * @param message 日志消息内容
     */
    void error(const std::string& message);
    
private:
    Logger();   // 私有构造函数，确保单例模式
    ~Logger();  // 析构函数，关闭日志文件
    
    LogLevel currentLevel;      // 当前日志级别
    std::ofstream logFile;      // 日志文件输出流
    std::mutex logMutex;        // 互斥锁，保证线程安全
    
    /**
     * @brief 获取当前时间戳字符串
     * @return 格式化的时间戳字符串
     * 
     * 格式：YYYY-MM-DD HH:MM:SS.mmm
     */
    std::string getTimestamp() const;
    
    /**
     * @brief 将日志级别转换为字符串
     * @param level 日志级别
     * @return 日志级别的字符串表示
     */
    std::string levelToString(LogLevel level) const;
    
    /**
     * @brief 核心日志输出方法
     * @param level 日志级别
     * @param message 日志消息内容
     * 
     * 内部方法，负责实际的日志输出操作
     */
    void log(LogLevel level, const std::string& message);
};

// 日志输出宏定义，简化调用
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)      // DEBUG日志宏
#define LOG_INFO(msg) Logger::getInstance().info(msg)        // INFO日志宏
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)  // WARNING日志宏
#define LOG_ERROR(msg) Logger::getInstance().error(msg)      // ERROR日志宏

#endif // LOGGER_HPP
