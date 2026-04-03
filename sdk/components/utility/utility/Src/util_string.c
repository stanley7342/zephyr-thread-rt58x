/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/** @file util_string.c
 *
 * @license
 * @description
 */

#include <stddef.h>
#include <stdint.h>

#include "util_string.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static const char* _utility_trim_spaces(const char* str) {
    while (*str == ' ')
        ++str;
    return str;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
long utility_strtol(const char* str, char** ep) {
    if (str == NULL) {
        if (ep)
            *ep = NULL;
        return 0;
    }

    long v = 0;
    long sign = 1;
    str = _utility_trim_spaces(str);

    if (*str == '-') {
        sign = -1;
        ++str;
    } else if (*str == '+')
        ++str;

    while (*str >= '0' && *str <= '9')
        v = v * 10 + (long)(*str++ - '0');

    if (ep)
        *ep = (char*)str;
    return sign * v;
}

// str to unsigned long
unsigned long utility_strtoul(const char* str, char** ep) {
    if (str == NULL) {
        if (ep)
            *ep = NULL;
        return 0;
    }

    unsigned long v = 0;
    int neg_v = 0;

    str = _utility_trim_spaces(str);

    if (*str == '-') {
        neg_v = 1;
        ++str;
    } else if (*str == '+')
        ++str;

    while (*str >= '0' && *str <= '9')
        v = v * 10 + (unsigned long)(*str++ - '0');

    if (ep)
        *ep = (char*)str;

    return (neg_v) ? (~v + 1) : v;
}

// str to hex
unsigned long utility_strtox(const char* str, char** ep, uint8_t dig) {
    if (str == NULL) {
        if (ep)
            *ep = NULL;
        return 0;
    }

    unsigned long v = 0;
    str = _utility_trim_spaces(str);
    if (*str == '0' && *(str + 1) == 'x')
        str += 2;

    for (uint8_t nr_digits = 0; nr_digits < dig; ++nr_digits) {
        if (*str >= '0' && *str <= '9')
            v = v * 16 + (unsigned long)(*str++ - '0');
        else if (*str >= 'a' && *str <= 'f')
            v = v * 16 + (unsigned long)(*str++ - 'a' + 10);
        else if (*str >= 'A' && *str <= 'F')
            v = v * 16 + (unsigned long)(*str++ - 'A' + 10);
        else
            break;
    }

    if (ep)
        *ep = (char*)str;
    return v;
}

// long to string
void utility_ltoa(char* a, unsigned long* len, long l, bool w_sign) {
    if (a == NULL || len == NULL || *len == 0)
        return;

    char* iter_b = a;
    char* iter_e = a + *len;
    char* iter = iter_e - 1;

    if (iter_b < iter_e) {
        if (l < 0) {
            l = -l;
            *iter_b++ = '-';
        } else if (w_sign)
            *iter_b++ = '+';
    }

    do {
        *iter-- = '0' + (l % 10);
        l /= 10;
    } while (l > 0 && iter >= iter_b);

    while (++iter < iter_e)
        *iter_b++ = *iter;

    if (iter_b < iter_e)
        *iter_b = '\0';

    *len = (unsigned long)(iter_b - a);
}

// long to string
void utility_ultoa(char* a, unsigned long* len, unsigned long u) {
    if (a == NULL || len == NULL || *len == 0)
        return;

    char* iter_b = a;
    char* iter_e = a + *len;
    char* iter = iter_e - 1;

    do {
        *iter-- = '0' + (u % 10);
        u /= 10;
    } while (u > 0 && iter >= iter_b);

    while (++iter < iter_e)
        *iter_b++ = *iter;

    if (iter_b < iter_e)
        *iter_b = '\0';

    *len = (unsigned long)(iter_b - a);
}

void utility_ftoa(char* a, unsigned long* len, double f,
                  unsigned long frc_precision) {
    if (a == NULL || len == NULL || *len == 0)
        return;

    char* iter = a;
    char* iter_e = iter + *len;
    double round_val = 0.5;
    double frc;
    long fint;

    if (frc_precision > 0) {
        double div = 1.0;
        for (unsigned long pp = frc_precision; pp > 0; --pp)
            div *= 10.0;
        round_val /= div;
    }

    f = (f >= 0) ? (f + round_val) : (f - round_val);

    fint = (long)f;
    frc = f - (double)fint;

    if (f < 0 && fint == 0) {
        frc = -frc;
        if (iter < iter_e)
            *iter++ = '-';
    }

    unsigned long int_len = iter_e - iter;
    utility_ltoa(iter, &int_len, fint, false);
    iter += int_len;

    if (iter >= iter_e || frc_precision == 0) {
        *len = (unsigned long)(iter - a);
        return;
    }

    *iter++ = '.';

    while (iter < iter_e && frc_precision-- > 0) {
        frc *= 10.0;
        fint = (long)frc;
        frc -= fint;
        *iter++ = '0' + fint;
    }

    if (iter < iter_e)
        *iter = '\0';

    *len = (unsigned long)(iter - a);
}

// hex to string
void utility_xtoa(char* a, unsigned long* len, unsigned long x,
                  bool capitalized) {
    if (a == NULL || len == NULL || *len == 0)
        return; // Return if input is invalid

    char* iter = a;
    char* iter_e = iter + *len;
    int skip_leading_zero = 1;

    // Handle zero case
    if (x == 0) {
        if (iter < iter_e)
            *iter++ = '0';
        if (iter < iter_e)
            *iter = '\0';
        *len = 1;
        return;
    }

    // Convert each nibble to hex character
    for (long i = 8 * sizeof(unsigned long) - 4; i >= 0 && iter < iter_e;
         i -= 4) {
        uint8_t xx = (uint8_t)(x >> i) & 0x0F;

        if (skip_leading_zero && xx == 0)
            continue; // Skip leading zeros
        skip_leading_zero = 0;

        if (xx < 10)
            *iter++ = '0' + xx;
        else if (capitalized)
            *iter++ = 'A' + (xx - 10);
        else
            *iter++ = 'a' + (xx - 10);
    }

    // Null-terminate the string if there's space
    if (iter < iter_e)
        *iter = '\0';

    // Update length to exclude null-terminating character
    *len = (unsigned long)(iter - a);
}

long utility_strlen(const char* s) {
    if (s == NULL)
        return 0; // Return 0 if input is NULL
    const char* sc = s;
    while (*sc != '\0')
        sc++;
    return (long)(sc - s); // Return the length of the string
}