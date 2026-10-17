/*
    utils 
    用于管理限幅函数
*/

#include "Utils.h"

float ConstrainFloat(float val, float min_val, float max_val)
{
    if(val < min_val) val = min_val;
    if(val > max_val) val = max_val;

    return val;
}

int32_t ConstrainInt(int32_t val, int32_t min_val, int32_t max_val)
{
    if(val < min_val) val = min_val;
    if(val > max_val) val = max_val;

    return val;
}