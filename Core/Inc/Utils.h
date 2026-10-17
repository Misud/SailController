/*
    utils 
    用于管理限幅函数
*/

#ifndef __UTILS_H
#define __UTILS_H

#include <main.h>

// 浮点数限幅
float ConstrianFloat(float val, float min_val, float max_val);

// 整数限幅
int32_t ConstrainInt(int32_t val, int32_t min_val, int32_t max_val);

#endif