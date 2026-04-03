/**
 * @file logger.cpp
 * @brief 日志系统实现文件
 * 
 * 本文件实现了线程安全的日志系统，支持多级别日志输出，
 * 可同时输出到控制台和文件。
 */

#include "../include/logger.hpp"
#include <iostream>
#include <iomanip>

/**
 * @brief 构造函数实现
 * 
 * 初始化日志级别为INFO
 */
Logger::Logger() : currentLevel(LogLevel::INFO) {}

/**
 * @brief 析构函数实现
 * 
 * 如果日志文件已打开，则关闭它
 */
Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();  // 关闭日志文件
    }
}

/**
 * @brief 获取日志记录器单例实例
 * @return Logger实例的引用
 * 
 * 使用静态局部变量实现线程安全的单例模式（C++11保证）
 */
Logger& Logger::getInstance() {
    static Logger instance;  // 静态局部变量，只初始化一次
    return instance;  // 返回单例实例的引用
}

/**
 * @brief 设置日志输出级别
 * @param level 日志级别
 * 
 * 只有大于等于此级别的日志才会被输出
 */
void Logger::setLogLevel(LogLevel level) {
    currentLevel = level;  // 更新当前日志级别
}

/**
 * @brief 设置日志输出文件
 * @param filename 日志文件路径
 * 
 * 如果已有日志文件打开，先关闭再打开新文件
 * 使用追加模式打开，保留之前的日志内容
 */
void Logger::setLogFile(const std::string& filename) {
    if (logFile.is_open()) {
        logFile.close();  // 关闭已打开的文件
    }
    // 以追加模式打开日志文件
    logFile.open(filename, std::ios::out | std::ios::app);
}

/**
 * @brief 输出DEBUG级别日志
 * @param message 日志消息内容
 */
void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

/**
 * @brief 输出INFO级别日志
 * @param message 日志消息内容
 */
void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

/**
 * @brief 输出WARNING级别日志
 * @param message 日志消息内容
 */
void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

/**
 * @brief 输出ERROR级别日志
 * @param message 日志消息内容
 */
void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

/**
 * @brief 获取当前时间戳字符串
 * @return 格式化的时间戳字符串
 * 
 * 格式：YYYY-MM-DD HH:MM:SS.mmm
 * 包含毫秒级精度
 */
std::string Logger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();  // 获取当前时间
    auto time_t_now = std::chrono::system_clock::to_time_t(now);  // 转换为time_t
    // 计算毫秒部分
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::tm tm_now;
    #ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);  // Windows平台的线程安全本地时间转换
    #else
    localtime_r(&time_t_now, &tm_now);  // Linux/Unix平台的线程安全本地时间转换
    #endif
    
    std::stringstream ss;
    // 格式化时间：年-月-日 时:分:秒.毫秒
    ss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S") << "." << std::setw(3) << std::setfill('0') << ms.count();
    return ss.str();  // 返回格式化的时间字符串
}

/**
 * @brief 将日志级别转换为字符串
 * @param level 日志级别
 * @return 日志级别的字符串表示
 */
std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

/**
 * @brief 核心日志输出方法
 * @param level 日志级别
 * @param message 日志消息内容
 * 
 * 实现细节：
 * 1. 检查日志级别是否满足输出条件
 * 2. 获取互斥锁，保证线程安全
 * 3. 构造完整的日志消息（时间戳 + 级别 + 消息）
 * 4. 输出到控制台
 * 5. 如果文件已打开，输出到文件并刷新缓冲区
 */
void Logger::log(LogLevel level, const std::string& message) {
    // 检查日志级别，低于当前级别的日志不输出
    if (level < currentLevel) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex);  // 获取互斥锁，保证线程安全
    
    // 构造完整的日志消息：[时间戳] [级别] 消息
    std::string logMessage = "[" + getTimestamp() + "] [" + levelToString(level) + "] " + message;
    
    // 输出到控制台
    std::cout << logMessage << std::endl;
    
    // 输出到文件
    if (logFile.is_open()) {
        logFile << logMessage << std::endl;  // 写入文件
        logFile.flush();  // 刷新缓冲区，确保立即写入
    }
}
