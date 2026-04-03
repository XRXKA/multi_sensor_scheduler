/**
 * @file ring_buffer.cpp
 * @brief 环形缓冲区实现文件
 * 
 * 本文件实现了线程安全的环形缓冲区，用于在多线程环境中高效地缓存和传递传感器数据。
 */

#include "../include/ring_buffer.hpp"

/**
 * @brief 构造函数实现
 * @param capacity 缓冲区容量
 * 
 * 初始化缓冲区：
 * - 初始化数据计数为0
 * - 设置缓冲区容量
 */
RingBuffer::RingBuffer(size_t capacity)
    : buffer(), count(0), cap(capacity) {}

/**
 * @brief 向缓冲区写入数据
 * @param data 要写入的传感器数据
 * @return 写入成功返回true，缓冲区已满返回false
 * 
 * 实现细节：
 * 1. 获取互斥锁，保证线程安全
 * 2. 检查缓冲区是否已满
 * 3. 将数据添加到缓冲区末尾
 * 4. 增加数据计数
 * 5. 通知等待的读取线程
 */
bool RingBuffer::push(const SensorData& data) {
    std::unique_lock<std::mutex> lock(bufferMutex);  // 获取互斥锁
    
    // 检查缓冲区是否已满
    if (isFull()) {
        return false;  // 缓冲区已满，写入失败
    }
    
    // 将数据添加到缓冲区末尾
    buffer.push_back(data);
    // 增加数据计数
    count++;
    
    // 通知一个等待的读取线程，缓冲区不为空
    notEmpty.notify_one();
    return true;  // 写入成功
}

/**
 * @brief 从缓冲区读取数据
 * @return 成功返回传感器数据
 * 
 * 实现细节：
 * 1. 获取互斥锁，保证线程安全
 * 2. 等待缓冲区不为空（阻塞式）
 * 3. 从缓冲区前端读取数据
 * 4. 从缓冲区中移除该数据
 * 5. 减少数据计数
 * 6. 通知等待的写入线程
 */
std::optional<SensorData> RingBuffer::pop() {
    std::unique_lock<std::mutex> lock(bufferMutex);  // 获取互斥锁
    
    // 等待缓冲区不为空，使用lambda表达式检查条件
    notEmpty.wait(lock, [this] { return !isEmpty(); });
    
    // 从缓冲区前端读取数据
    SensorData data = buffer.front();
    // 从缓冲区中移除该数据
    buffer.pop_front();
    // 减少数据计数
    count--;
    
    // 通知一个等待的写入线程，缓冲区未满
    notFull.notify_one();
    return data;  // 返回读取的数据
}

/**
 * @brief 获取缓冲区当前数据量
 * @return 当前缓冲区中的数据数量
 * 
 * 线程安全的查询操作
 */
size_t RingBuffer::size() const {
    std::lock_guard<std::mutex> lock(bufferMutex);  // 获取互斥锁
    return count;  // 返回当前数据计数
}

/**
 * @brief 获取缓冲区容量
 * @return 缓冲区的总容量
 * 
 * 容量是固定的，不需要加锁
 */
size_t RingBuffer::capacity() const {
    return cap;  // 返回缓冲区容量
}

/**
 * @brief 检查缓冲区是否为空
 * @return 为空返回true，否则返回false
 * 
 * 内部方法，调用前需要确保已持有锁
 */
bool RingBuffer::isEmpty() const {
    return count == 0;  // 数据计数为0表示为空
}

/**
 * @brief 检查缓冲区是否已满
 * @return 已满返回true，否则返回false
 * 
 * 内部方法，调用前需要确保已持有锁
 */
bool RingBuffer::isFull() const {
    return count == cap;  // 数据计数等于容量表示已满
}

/**
 * @brief 清空缓冲区
 * 
 * 实现细节：
 * 1. 获取互斥锁，保证线程安全
 * 2. 清空缓冲区
 * 3. 重置数据计数为0
 */
void RingBuffer::clear() {
    std::lock_guard<std::mutex> lock(bufferMutex);  // 获取互斥锁
    buffer.clear();  // 清空缓冲区
    count = 0;       // 重置数据计数
}
