/**
 **********************************************************************************
 * Copyright (c) 2026 Your Name
 *
 * SPDX-License-Identifier: MIT
 **********************************************************************************
 * @file    ringbuf.c
 * @author  2026 Your Name
 * @date    2026-10-11
 * @version 1.0.0
 * @brief   本文件用于环形缓冲区的实现基于STM32HAL库，仅涉及读写相关的函数以及一个寻找
 *          不定长数据帧的函数，并预留了一个数据帧处理函数供用户自定义。用户可自行编辑
 *          处理函数，在需要的地方调用。
 * @note    数据帧格式强制为：
 *              [HEAD] + [LEN] + [DATA] + [CRC] + [TAIL]
 *               必有     1或2    可变     可选     必有
 *               帧头   DATA长度  数据     校验     帧尾
 * @note    使用方法：
 *              首先需要定义缓冲区，为ringbuf_t类型，然后调用初始化函数ringbuf_init，然后
 *          定义帧描述符，来描述你的数据帧格式，具体见头文件frame_desc_t类型的结构体，寻帧
 *          函数会根据这个描述符寻找数据帧。用户需要在需要写数据的地方调用ringbuf_write函数
 *          ，在读数据的地方调用ringbuf_read函数，当然用户可以直接调用寻帧函数。文件还提供
 *          了三种数据校验方式。
 *
 *
 **********************************************************************************
 */

#include "ringbuf.h"

/**
 * ========================================================================================= 
 * ================================= 环形缓冲区核心函数 =====================================
 * ========================================================================================= 
 */
 
 
/**
 * @brief 用于初始化缓冲区
 * @param[in] rb 缓冲区地址
 */
void ringbuf_init(ringbuf_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

/**
 * @brief 用于计算已经使用的缓冲区大小
 * @param[in] rb 缓冲区地址
 * @return 返回缓冲区已经使用的缓冲区字节数
 */
uint32_t ringbuf_used(ringbuf_t *rb)
{
    return (rb->head - rb->tail) & (RINGBUF_SIZE - 1);
}

/**
 * @brief 用于计算空闲缓冲区的大小
 * @param[in] rb 缓冲区地址
 * @return 返回缓冲区空闲的字节数
 * @note 需要留一个字节的区域判断缓冲区状态
 */
uint32_t ringbuf_free(ringbuf_t *rb)
{
    return RINGBUF_SIZE - 1 - ringbuf_used(rb);   //留1字节区分空满
}

/**
 * @brief 用于确认缓冲区是否已满
 * @param[in] rb 缓冲区地址
 * @return 返回bool类型
 */
bool ringbuf_is_full(ringbuf_t *rb)
{
    return ((rb->head + 1) & (RINGBUF_SIZE - 1)) == rb->tail;
}

/**
 * @brief 用于确认缓冲区是否是空的
 * @param[in] rb 缓冲区地址
 * @return 返回bool类型
 */
bool ringbuf_is_empty(ringbuf_t *rb)
{
    return rb->head == rb->tail;
}

/**
 * @brief 用于向缓冲区写数据
 * @details 使用掩码限制读写指针，实现循环
 * @param[in] rb 缓冲区地址
 * @param[in] data 要写入的数据的地址
 * @param[in] len 要写入的字节数
 * @return 返回实际写入的字节数。
 * @note 函数会自动判断空闲空间是否够写入len长度的数据，最后返回实际写入长度
 */
uint32_t ringbuf_write(ringbuf_t *rb, const uint8_t *data, uint32_t len)
{
    uint32_t free = ringbuf_free(rb);
    if (len > free) len = free;

    for (uint32_t i = 0; i < len; i++) {
        rb->ringbuf[rb->head] = data[i];
        rb->head = (rb->head + 1) & (RINGBUF_SIZE - 1);
    }
    return len;
}

/**
 * @brief 用于从缓冲区读数据
 * @details 使用掩码限制读写指针，实现循环
 * @param[in] rb 缓冲区地址
 * @param[out] out 将数据读取到的空间的地址
 * @param[in] len 要读取的字节数
 * @return 返回实际读取的字节数。
 * @note 函数会自动判断已使用的缓冲区数据长度是否足够读取，最后返回实际读取长度
 */
uint32_t ringbuf_read(ringbuf_t *rb, uint8_t *out, uint32_t len)
{
    uint32_t used = ringbuf_used(rb);
    if (len > used) len = used;

    for (uint32_t i = 0; i < len; i++) {
        out[i] = rb->ringbuf[rb->tail];
        rb->tail = (rb->tail + 1) & (RINGBUF_SIZE - 1);
    }
    return len;
}

/**
 * @brief 用于丢弃一字节的缓冲区数据
 * @param[in] rb 缓冲区地址
 */
void ringbuf_droponebyte(ringbuf_t *rb)
{
    uint8_t dummy;
    ringbuf_read(rb, &dummy, 1);
}

/**
 * @brief 用于查看读指针后面的数据，并不是真的把数据读出来
 * @param[in] rb 缓冲区地址
 * @param[in] offset 偏移量，即从读指针后面第几个字节开始读取
 * @param[out] out 读出来的字节存放地址
 * @param[in] len 要查看的数据字节数
 * @return 返回0代表读取失败
 */
uint32_t ringbuf_peek(ringbuf_t *rb, uint32_t offset, uint8_t *out, uint32_t len)
{
    uint32_t used = ringbuf_used(rb);
    if (offset + len > used) return 0;

    uint32_t idx = (rb->tail + offset) & (RINGBUF_SIZE - 1);
    for (uint32_t i = 0; i < len; i++) {
        out[i] = rb->ringbuf[idx];
        idx = (idx + 1) & (RINGBUF_SIZE - 1);
    }
    return len;
}

/**
 * @brief 用于验证帧头或者帧尾，用户也可用来验证特定位是否是特定数据
 * @param[in] rb 缓冲区地址
 * @param[in] offset 偏移量，即从读指针后面第几个字节开始验证
 * @param[in] pattern 需要验证的数据，例如帧头"@@"
 * @param[in] len 需要验证的数据个数
 * @return bool类型，表示验证成功或失败
 * @note 失败也可能是因为偏移和要读取的数据个数之和超出了缓冲区已有数据的大小
 */
bool ringbuf_frame_verify(ringbuf_t *rb, uint32_t offset, const uint8_t *pattern, uint32_t len)
{
    uint32_t used = ringbuf_used(rb);
    if (offset + len > used) return false;

    uint32_t idx = (rb->tail + offset) & (RINGBUF_SIZE - 1);
    for (uint32_t i = 0; i < len; i++) {
        if (rb->ringbuf[idx] != pattern[i]) return false;
        idx = (idx + 1) & (RINGBUF_SIZE - 1);
    }
    return true;
}

/**
 * ========================================================================================= 
 * ================================= 校验函数 ==============================================
 * ========================================================================================= 
 */

/**
 * @brief 用于进行异或校验
 * @param[in] d 要校验的数据首地址
 * @param[in] n 要校验的数据个数
 * @return 返回八位计算后的校验值
 * @note 需要在头文件中将宏定义置1来启用
 */
#if RINGBUF_USE_XOR
static uint8_t xor_checksum(const uint8_t *d, uint32_t n)
{
    uint8_t x = 0;
    for (uint32_t i = 0; i < n; i++) x ^= d[i];
    return x;
}
#endif

/**
 * @brief 用于进行求和校验
 * @param[in] d 要校验的数据首地址
 * @param[in] n 要校验的数据个数
 * @return 返回八位计算后的校验值
 * @note 需要在头文件中将宏定义置1来启用
 */
#if RINGBUF_USE_SUM
static uint8_t sum_checksum(const uint8_t *d, uint32_t n)
{
    uint8_t s = 0;
    for (uint32_t i = 0; i < n; i++) s += d[i];
    return s;
}
#endif

/**
 * @brief 用于进行CRC8校验
 * @param[in] d 要校验的数据首地址
 * @param[in] n 要校验的数据个数
 * @return 返回八位计算后的校验值
 * @note 需要在头文件中将宏定义置1来启用
 */
#if RINGBUF_USE_CRC8
static uint8_t crc8(const uint8_t *d, uint32_t n)
{
    uint8_t c = 0x00;
    for (uint32_t i = 0; i < n; i++) {
        c ^= d[i];
        for (uint8_t b = 0; b < 8; b++)
            c = (c & 0x80) ? (uint8_t)((c << 1) ^ 0x07)
                           : (uint8_t)(c << 1);
    }
    return c;
}
#endif

/**
 * @brief 校验函数，方便选择哪种校验方式
 * @param[in] type 选择校验方式
 * @param[in] data 要校验的数据首地址
 * @param[in] len 要校验的数据个数
 * @return 返回八位计算后的校验值
 */
uint8_t checksum_calc(checksum_type_t type, const uint8_t *data, uint32_t len)
{
    switch (type) {
#if RINGBUF_USE_XOR
        case CK_XOR:  return xor_checksum(data, len);
#endif
#if RINGBUF_USE_SUM
        case CK_SUM:  return sum_checksum(data, len);
#endif
#if RINGBUF_USE_CRC8
        case CK_CRC8: return crc8(data, len);
#endif
        default:      return 0;
    }
}

/**
 * ========================================================================================= 
 * ================================= 帧解析函数 ============================================
 * ========================================================================================= 
 */


/**
 * @brief 从数据帧中获取数据段的字节个数
 * @param[in] rb 缓冲区地址
 * @param[in] d 帧描述符
 * @return 返回从数据帧中读取到的len字段表示的字节个数
 * @note 小端顺序，即如果len字段是两位，低位在前
 */
static uint32_t read_data_len(ringbuf_t *rb, const frame_desc_t *d)
{
    uint8_t lb[2] = {0};
    ringbuf_peek(rb, d->head_len, lb, d->len_size);
    if (d->len_size == 1) {
        return lb[0];
    } else {
        return (uint32_t)lb[0] | ((uint32_t)lb[1] << 8);
    }
}

/**
 * @brief 寻帧函数
 * @param[in] rb 缓冲区地址
 * @param[in] d 帧描述符
 * @param[out] frame_buf 读取到的完整一帧数据存放的地址，包含帧头帧尾等
 * @param[in] out_len 实际读取到的数据帧长度
 * @return bool类型，表示是否找到了完整的一帧
 */
bool ringbuf_frame_find(ringbuf_t *rb, const frame_desc_t *d, uint8_t *frame_buf, uint32_t *out_len)
{
#if RINGBUF_USE_TIMEOUT
    /* 注意：此处静态变量假设同一时刻只用一种描述符解析一条流。
     * 如需并行多描述符，把这两个变量挪到 ringbuf_t 或调用者侧。 */
    static uint32_t last_used = 0;
    static uint32_t stuck_cnt = 0;
#endif

    /* 最小帧长（数据长度=0）*/
    uint32_t min_len = d->head_len + d->len_size
                     + (d->has_crc ? 1 : 0) + d->tail_len;
    if (min_len < FRAME_MIN_LEN) min_len = FRAME_MIN_LEN;

    while (ringbuf_used(rb) >= min_len) {
        /* 1. 验证帧头 */
        if (!ringbuf_frame_verify(rb, 0, d->head, d->head_len)) {
            ringbuf_droponebyte(rb);
            continue;
        }

        /* 2. 读 LEN 字段，算整帧长度 */
        uint32_t data_len = read_data_len(rb, d);
        uint32_t total    = d->head_len + d->len_size + data_len
                          + (d->has_crc ? 1 : 0) + d->tail_len;

        /* 3. 帧长保护：LEN 被干扰导致过大，滑动重找 */
        if (total > FRAME_MAX_LEN) {
            ringbuf_droponebyte(rb);
            continue;
        }

        /* 4. 数据不够整帧，等下次 */
        if (ringbuf_used(rb) < total) {
#if RINGBUF_USE_TIMEOUT
            if (ringbuf_used(rb) == last_used) {
                if (++stuck_cnt > FRAME_STUCK_MS) {
                    ringbuf_droponebyte(rb);   /* 半帧停留太久，强制滑动 */
                    stuck_cnt = 0;
                }
            } else {
                stuck_cnt = 0;
            }
            last_used = ringbuf_used(rb);
#endif
            return false;
        }
#if RINGBUF_USE_TIMEOUT
        stuck_cnt = 0;
        last_used = ringbuf_used(rb);
#endif

        /* 5. 验证帧尾（按位置，不盲搜）*/
        if (!ringbuf_frame_verify(rb, total - d->tail_len,
                                  d->tail, d->tail_len)) {
            ringbuf_droponebyte(rb);
            continue;
        }

        /* 6. 读走整帧 */
        ringbuf_read(rb, frame_buf, total);

        /* 7. 校验（如有）：覆盖 LEN + DATA */
        if (d->has_crc) {
            uint32_t crc_pos = total - d->tail_len - 1;
            uint8_t  recv    = frame_buf[crc_pos];
            uint8_t  calc    = checksum_calc(d->ck_type,
                                             &frame_buf[d->head_len],
                                             crc_pos - d->head_len);
            if (recv != calc) {
                continue;   /* 校验失败，丢帧继续找 */
            }
        }

        *out_len = total;
        return true;
    }
    return false;
}

/**
 * ========================================================================================= 
 * ================================= 用户处理函数 ============================================
 * ========================================================================================= 
 */

/*
void ringbuf_process_frame(uint8_t *frame_buf, uint32_t len)
{
    
    
}


#define PROCESS_BUFSIZE 64//大小建议一定要大于最长数据帧的长度
void Frame_Process()
{
    uint32_t len;
    uint8_t frame[PROCESS_BUFSIZE];
    if(ringbuf_frame_find(&ringbuffer,&description,frame,&len)){
        ringbuf_process_frame(frame,len);
    }
}
*/


