/**
 * @file ring_buffer.hpp
 * @brief 环形缓冲区头文件
 * 
 * 本文件定义了一个线程安全的环形缓冲区（Ring Buffer），
 * 用于在多线程环境中高效地缓存和传递传感器数据。
 * 支持阻塞式的读写操作，当缓冲区满时写入阻塞，空时读取阻塞。
 */

#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <deque>
#include <mutex>
#include <condition_variable>
#include <optional>
#include "sensor_data.hpp"

/**
 * @class RingBuffer
 * @brief 线程安全的环形缓冲区类
 * 
 * 实现了一个固定大小的环形缓冲区，特点：
 * - 线程安全，支持多生产者多消费者模式
 * - 使用条件变量实现阻塞式读写
 * - 高效的内存使用，无需频繁分配释放
 * - 支持查询当前状态（大小、是否为空、是否已满）
 * 
 * 环形缓冲区通过维护头尾指针实现循环利用存储空间，
 * 当缓冲区满时，push操作会阻塞直到有空间可用；
 * 当缓冲区空时，pop操作会阻塞直到有数据可读。
 */
class RingBuffer {
public:
    /**
     * @brief 构造函数
     * @param capacity 缓冲区容量
     * 
     * 创建指定容量的环形缓冲区
     */
    explicit RingBuffer(size_t capacity);
    
    /**
     * @brief 向缓冲区写入数据
     * @param data 要写入的传感器数据
     * @return 写入成功返回true，缓冲区已满返回false
     * 
     * 线程安全的写入操作，如果缓冲区已满则立即返回false
     */
    bool push(const SensorData& data);
    
    /**
     * @brief 从缓冲区读取数据
     * @return 成功返回传感器数据，缓冲区为空时阻塞等待
     * 
     * 线程安全的读取操作，如果缓冲区为空则阻塞等待数据
     */
    std::optional<SensorData> pop();
    
    /**
     * @brief 获取缓冲区当前数据量
     * @return 当前缓冲区中的数据数量
     */
    size_t size() const;
    
    /**
     * @brief 获取缓冲区容量
     * @return 缓冲区的总容量
     */
    size_t capacity() const;
    
    /**
     * @brief 检查缓冲区是否为空
     * @return 为空返回true，否则返回false
     */
    bool isEmpty() const;
    
    /**
     * @brief 检查缓冲区是否已满
     * @return 已满返回true，否则返回false
     */
    bool isFull() const;
    
    /**
     * @brief 清空缓冲区
     * 
     * 清除缓冲区中的所有数据，重置头尾指针
     */
    void clear();
    
private:
    std::deque<SensorData> buffer;  // 数据存储容器
    size_t count;                     // 当前数据计数
    const size_t cap;                 // 缓冲区容量
    
    mutable std::mutex bufferMutex;              // 互斥锁，保证线程安全
    std::condition_variable notEmpty;            // 条件变量，缓冲区非空通知
    std::condition_variable notFull;             // 条件变量，缓冲区未满通知
};

#endif // RING_BUFFER_HPP
