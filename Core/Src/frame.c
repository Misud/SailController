#include "frame.h"


float readpid(uint8_t *data,int index1,int index2,int index3,int index4,int index5)      //读取指定5位数，用于读取pid参数值，整数一位，小数4位
{
    uint8_t a = data[index1] - '0';
    uint8_t b = data[index2] - '0';
    uint8_t c = data[index3] - '0';
    uint8_t d = data[index4] - '0';
    uint8_t e = data[index5] - '0';
    float number = a+b*0.1f+c*0.01f+d*0.001f+e*0.0001f;
    
    return number;
}

float readangle(uint8_t *data,int index1,int index2,int index3)
{
    uint8_t a = data[index1] - '0';
    uint8_t b = data[index2] - '0';
    uint8_t c = data[index3] - '0';
    uint16_t number = a*100+b*10+c;
    
    return number;
}