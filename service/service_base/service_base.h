/**
 * @file      service_base.h
 * @author:   Shubhendu B B
 * @date:     02/08/2026
 * @brief
 * @details   Distributed globally for free under the MIT License terms.
 *
 * @copyright Copyright (c) 2025 er-shubhendu-ee
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

#ifndef __SERVICE_BASE_H__
#define __SERVICE_BASE_H__

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Define boolean types
#ifndef BOOL
#define BOOL bool
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE !FALSE
#endif

// Define carriage return and line feed constants
#ifndef CR
#define CR 0x0D
#endif

#ifndef LF
#define LF 0x0A
#endif

// Bit manipulation macros
#ifndef BIT_SHIFT
#define BIT_SHIFT(DWORD, BITPOSITION) ((DWORD) |= (1 << (BITPOSITION)))
#endif

// Type definitions for byte and word sizes
#ifndef BYTE
#define BYTE uint8_t
#endif

#ifndef WORD
#define WORD uint16_t
#endif

// String macros
#ifndef STRING
#define STRING(x) #x
#define STR(x) STRING(x)
#endif

// Define unsigned char type
#ifndef uchar_t
typedef unsigned char uchar_t;
#endif

// Type definitions for various sizes
#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

// Define HIGH and LOW constants
#ifndef HIGH
#define HIGH 1u
#endif

#ifndef LOW
#define LOW 0u
#endif

// Define ON and OFF constants
#ifndef ON
#define ON 1u
#endif

#ifndef OFF
#define OFF 0u
#endif

// Define pass and fail constants
#ifndef PASS
#define PASS 1u
#endif

#ifndef FAIL
#define FAIL 0u
#endif

/**
 * @}
 *
 */

#ifndef __weak__
#define __weak__ __attribute__((weak))
#endif /* __weak */

// Enumeration for base data types
typedef enum {
    CHAR8_T,
    UCHAR8_T,
    INTEGER,
    INT8_T,
    UINT8_T,
    INT16_T,
    UINT16_T,
    INT32_T,
    UINT32_T,
    INT64_T,
    UINT64_T,
    FLOAT_T,
    FLOAT64_T,
    LONG_T,
    LONG_LONG_T,
    DOUBLE_T,
    STRING_T
} service_base_type_t;

// Enumeration for bit positions
typedef enum { BIT0, BIT1, BIT2, BIT3, BIT4, BIT5, BIT6, BIT7 } service_base_bit_t;

// Type definition for boolean
#ifndef bool
#define bool _Bool
#endif

// Union for byte representation
typedef union {
    unsigned char value;
    struct {
        unsigned B0 : 1;
        unsigned B1 : 1;
        unsigned B2 : 1;
        unsigned B3 : 1;
        unsigned B4 : 1;
        unsigned B5 : 1;
        unsigned B6 : 1;
        unsigned B7 : 1;
    };
} service_base_byte_t;

// Union for word representation
typedef union {
    uint16_t value;
    struct {
        service_base_byte_t byte_0;
        service_base_byte_t byte_1;
    };
    struct {
        unsigned B0 : 1;
        unsigned B1 : 1;
        unsigned B2 : 1;
        unsigned B3 : 1;
        unsigned B4 : 1;
        unsigned B5 : 1;
        unsigned B6 : 1;
        unsigned B7 : 1;
        unsigned B8 : 1;
        unsigned B9 : 1;
        unsigned B10 : 1;
        unsigned B11 : 1;
        unsigned B12 : 1;
        unsigned B13 : 1;
        unsigned B14 : 1;
        unsigned B15 : 1;
    };
} service_base_word_t;

// Union for double word representation
typedef union {
    uint32_t value;
    struct {
        service_base_byte_t byte_0;
        service_base_byte_t byte_1;
        service_base_byte_t byte_2;
        service_base_byte_t byte_3;
    };
    struct {
        service_base_word_t word_0;
        service_base_word_t word_1;
    };
    struct {
        unsigned B0 : 1;
        unsigned B1 : 1;
        unsigned B2 : 1;
        unsigned B3 : 1;
        unsigned B4 : 1;
        unsigned B5 : 1;
        unsigned B6 : 1;
        unsigned B7 : 1;
        unsigned B8 : 1;
        unsigned B9 : 1;
        unsigned B10 : 1;
        unsigned B11 : 1;
        unsigned B12 : 1;
        unsigned B13 : 1;
        unsigned B14 : 1;
        unsigned B15 : 1;
        unsigned B16 : 1;
        unsigned B17 : 1;
        unsigned B18 : 1;
        unsigned B19 : 1;
        unsigned B20 : 1;
        unsigned B21 : 1;
        unsigned B22 : 1;
        unsigned B23 : 1;
        unsigned B24 : 1;
        unsigned B25 : 1;
        unsigned B26 : 1;
        unsigned B27 : 1;
        unsigned B28 : 1;
        unsigned B29 : 1;
        unsigned B30 : 1;
        unsigned B31 : 1;
    };
} service_base_dword_t;

// Union for quad word representation
typedef union {
    uint64_t value;
    struct {
        service_base_byte_t byte_0;
        service_base_byte_t byte_1;
        service_base_byte_t byte_2;
        service_base_byte_t byte_3;
        service_base_byte_t byte_4;
        service_base_byte_t byte_5;
        service_base_byte_t byte_6;
        service_base_byte_t byte_7;
    };
    struct {
        service_base_word_t word_0;
        service_base_word_t word_1;
        service_base_word_t word_2;
        service_base_word_t word_3;
    };
    struct {
        service_base_dword_t dword_0;
        service_base_dword_t dword_1;
    };
    struct {
        unsigned B0 : 1;
        unsigned B1 : 1;
        unsigned B2 : 1;
        unsigned B3 : 1;
        unsigned B4 : 1;
        unsigned B5 : 1;
        unsigned B6 : 1;
        unsigned B7 : 1;
        unsigned B8 : 1;
        unsigned B9 : 1;
        unsigned B10 : 1;
        unsigned B11 : 1;
        unsigned B12 : 1;
        unsigned B13 : 1;
        unsigned B14 : 1;
        unsigned B15 : 1;
        unsigned B16 : 1;
        unsigned B17 : 1;
        unsigned B18 : 1;
        unsigned B19 : 1;
        unsigned B20 : 1;
        unsigned B21 : 1;
        unsigned B22 : 1;
        unsigned B23 : 1;
        unsigned B24 : 1;
        unsigned B25 : 1;
        unsigned B26 : 1;
        unsigned B27 : 1;
        unsigned B28 : 1;
        unsigned B29 : 1;
        unsigned B30 : 1;
        unsigned B31 : 1;
        unsigned B32 : 1;
        unsigned B33 : 1;
        unsigned B34 : 1;
        unsigned B35 : 1;
        unsigned B36 : 1;
        unsigned B37 : 1;
        unsigned B38 : 1;
        unsigned B39 : 1;
        unsigned B40 : 1;
        unsigned B41 : 1;
        unsigned B42 : 1;
        unsigned B43 : 1;
        unsigned B44 : 1;
        unsigned B45 : 1;
        unsigned B46 : 1;
        unsigned B47 : 1;
        unsigned B48 : 1;
        unsigned B49 : 1;
        unsigned B50 : 1;
        unsigned B51 : 1;
        unsigned B52 : 1;
        unsigned B53 : 1;
        unsigned B54 : 1;
        unsigned B55 : 1;
        unsigned B56 : 1;
        unsigned B57 : 1;
        unsigned B58 : 1;
        unsigned B59 : 1;
        unsigned B60 : 1;
        unsigned B61 : 1;
        unsigned B62 : 1;
        unsigned B63 : 1;
    };
} service_base_ddword_t;

// Enumeration for date formats
typedef enum {
    DDMMYY,
    DDMMYYYY,
    DDMMMYY,
    DDMMMYYYY,
    MMDDYY,
    MMDDYYYY,
    MMMDDYY,
    MMMDDYYYY,
} service_base_date_format_t;

// Structure for time of day representation
typedef struct {
    uint8_t dd;                                 // Day
    uint8_t mm;                                 // Month
    uint8_t yy;                                 // Year
    uint8_t day;                                // Day of the week
    uint8_t hr;                                 // Hour
    uint8_t min;                                // Minute
    uint8_t sec;                                // Second
    uint8_t ms;                                 // Millisecond
    unsigned timeFormat : 1;                    // 0: 24-hour, 1: AM/PM
    unsigned meridianIndicator : 1;             // 0: AM, 1: PM
    service_base_date_format_t dateFormatType;  // Date format type
} service_base_tod_t;

// Structure for ADC data representation
typedef struct {
    uint8_t channelNumber;  // ADC channel number
    float channelValue;     // ADC channel value
} service_base_adcData_t;

// Structure for edge processing buffer
typedef struct {
    unsigned lastState : 1;     // Last state
    unsigned currentState : 1;  // Current state
    unsigned positiveEdge : 1;  // Positive edge detected
    unsigned negativeEdge : 1;  // Negative edge detected
    unsigned level : 1;         // Current level
    unsigned highLevel : 1;     // High level detected
    unsigned lowLevel : 1;      // Low level detected
} service_base_edgeProcessBuffer_t;

// Enumeration for status codes
typedef enum {
    SERVICE_BASE_STATUS_OK = 0,
    SERVICE_BASE_STATUS_BUSY,

    SERVICE_BASE_STATUS_ERROR = -1,
    SERVICE_BASE_STATUS_ERROR_QUEUE_EMPTY = -2
} service_base_status_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* @end __SERVICE_BASE_H__ */
