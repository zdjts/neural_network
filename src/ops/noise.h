
#ifndef NOISE_H
#define NOISE_H
// 由于MNIST数据集无法适应人工手写数字识别，因此，准备为MNIST的矩阵添加噪声以及变化。

#include "../core/Matrix.h"

// ================== 常见数据增强与噪声函数声明 ==================

// 随机缩放（中心缩放，空白区域填0）
void random_scale(Matrix& m, float min_scale = 0.7f, float max_scale = 1.0f);

// 添加高斯噪声
void add_gaussian_noise(Matrix& m, float mean = 0.0f, float stddev = 0.1f);

// 随机平移（上下左右填0）
void random_shift(Matrix& m, int max_shift = 3);

// 随机旋转（空白区域填0）
void random_rotate(Matrix& m, float max_angle = 20.0f);

// 随机反转颜色（黑白翻转）
void random_invert(Matrix& m, float prob = 0.5f);

// 随机擦除（随机区域置0）
void random_erasing(Matrix& m, float erase_prob = 0.3f, float min_area = 0.02f, float max_area = 0.2f);

#endif