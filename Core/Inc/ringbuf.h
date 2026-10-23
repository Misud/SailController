/**
 **********************************************************************************
 * Copyright (c) 2026 Your Name
 *
 * SPDX-License-Identifier: MIT
 **********************************************************************************
 * @file    ringbuf.h
 * @author  2026 Your Name
 * @date    2026-10-11
 * @version 1.0.0
 **********************************************************************************
 */

#ifndef __RINGBUF_H
#define __RINGBUF_H

#include <stdint.h>
#include <stdbool.h>

/* ===================== 配置 ===================== */

//缓冲区大小，必须是 2 的幂
#define RINGBUF_SIZE         128

//校验类型开关：只编译开启的校验
#define RINGBUF_USE_XOR      0
#define RINGBUF_USE_SUM      0
#define RINGBUF_USE_CRC8     0

//是否启用半帧超时保护：防止 LEN 出错导致死等
#define RINGBUF_USE_TIMEOUT  1

//帧长保护
#define FRAME_MIN_LEN        6       //最小帧：@@ + LEN + 0数据 + CRC + ##
#define FRAME_MAX_LEN        128    //最大帧保护，超此值丢弃
#define FRAME_STUCK_MS       100     //半帧停留超时阈值（调用周期数）

/* ===================== 环形缓冲区 ===================== */

typedef struct {
    uint8_t  ringbuf[RINGBUF_SIZE];
    volatile uint32_t head;   //写指针，中断改
    volatile uint32_t tail;   //读指针，主循环改
} ringbuf_t;

void     ringbuf_init   (ringbuf_t *rb);
uint32_t ringbuf_used   (ringbuf_t *rb);
uint32_t ringbuf_free   (ringbuf_t *rb);
bool     ringbuf_is_full(ringbuf_t *rb);
bool     ringbuf_is_empty(ringbuf_t *rb);

uint32_t ringbuf_write  (ringbuf_t *rb, const uint8_t *data, uint32_t len);
uint32_t ringbuf_read   (ringbuf_t *rb, uint8_t *out, uint32_t len);
void     ringbuf_droponebyte(ringbuf_t *rb);

uint32_t ringbuf_peek   (ringbuf_t *rb, uint32_t offset, uint8_t *out, uint32_t len);
bool     ringbuf_frame_verify(ringbuf_t *rb, uint32_t offset,
                              const uint8_t *pattern, uint32_t len);

/* ===================== 校验 ===================== */

typedef enum {
    CK_NONE = 0,
#if RINGBUF_USE_XOR
    CK_XOR,
#endif
#if RINGBUF_USE_SUM
    CK_SUM,
#endif
#if RINGBUF_USE_CRC8
    CK_CRC8,
#endif
} checksum_type_t;

uint8_t checksum_calc(checksum_type_t type, const uint8_t *data, uint32_t len);

/* ===================== 帧描述符 ===================== */
/* 数据帧格式强制为：
 *          [HEAD] + [LEN] + [DATA] + [CRC] + [TAIL]
 *           必有     1或2    可变     可选     必有
 *           帧头   DATA长度  数据     校验     帧尾
 */
typedef struct {
    const uint8_t  *head;          //帧头内容，如 "@@"
    uint32_t        head_len;      //帧头长度
    const uint8_t  *tail;          //帧尾内容，如 "##"
    uint32_t        tail_len;      //帧尾长度
    uint32_t        len_size;      //LEN 字段字节数：1 或 2
    bool            has_crc;       //是否带校验
    checksum_type_t ck_type;       //校验类型（has_crc=true 时有效）
} frame_desc_t;

/* ===================== 帧解析 ===================== */

/* 从 rb 中找一帧写入 frame_buf
 * 返回 true = 找到一帧，*out_len 输出实际帧长
 * 返回 false = 数据不够或无完整帧（等下次调用）
 */
bool ringbuf_frame_find(ringbuf_t *rb, const frame_desc_t *desc,
                        uint8_t *frame_buf, uint32_t *out_len);


#endif
