/*
    KeyScan
    用于检测按键
*/
#ifndef __KEYSCAN_H
#define __KEYSCAN_H

// 定义按键枚举
typedef enum{
    Key_None = 0,
    Key_1,
    Key_2,
    Key_3,
    Key_4
}KeyValue_t;

// 用于单次按键扫描
KeyValue_t Key_Scan(void);


#endif