import matplotlib.pyplot as plt
import numpy as np
import sys

# 模拟数据
camera_latency = np.random.normal(1.0, 0.2, 1000)
imu_latency = np.random.normal(0.5, 0.1, 1000)
lidar_latency = np.random.normal(2.0, 0.3, 1000)

# 创建图表
plt.figure(figsize=(12, 6))

# 绘制延迟分布
plt.subplot(1, 2, 1)
plt.hist(camera_latency, bins=50, alpha=0.5, label='Camera')
plt.hist(imu_latency, bins=50, alpha=0.5, label='IMU')
plt.hist(lidar_latency, bins=50, alpha=0.5, label='LiDAR')
plt.xlabel('Latency (ms)')
plt.ylabel('Frequency')
plt.title('Latency Distribution')
plt.legend()

# 绘制延迟时间序列
plt.subplot(1, 2, 2)
time = np.arange(1000)
plt.plot(time, camera_latency, label='Camera')
plt.plot(time, imu_latency, label='IMU')
plt.plot(time, lidar_latency, label='LiDAR')
plt.xlabel('Sample')
plt.ylabel('Latency (ms)')
plt.title('Latency Time Series')
plt.legend()

plt.tight_layout()
plt.savefig('latency_plot.png')
print('Latency plot saved as latency_plot.png')
