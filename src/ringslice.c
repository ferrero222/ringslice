/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Nikita Maltsev (aleph-five)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "ringslice_util.h"
#include "ringslice.h"
#include <string.h>

DBC_MODULE_NAME(RINGSLICE_MODULE)

/*!
* Searches for substring in ringslice instance with lenght
* @param[in] me ringslice instance where substring is searched for
* @param[in] substr searched substring
* @param[in] substr_len lenght of byte to compare in substring
*
* @return subslice of me slice containing substring, otherwise empty ringslice
*
* @note if substr is empty string, then copy of me slice will be returned
*
*/
ringslice_t ringslice_strnstr(ringslice_t const *const me, char const *substr, const ringslice_cnt_t substr_len) {
    ringslice_t substr_slice = ringslice_initializer(me->buf, me->buf_size, 0, 0);  // initialize with empty slice
    if(substr[0] == '\0') return substr_slice;
    uint8_t const *const buf_end = &(me->buf[me->buf_size]);
    uint8_t const *const buf_start = &(me->buf[0]);
    uint8_t const *first_ptr = &(me->buf[me->first]);
    
    if(ringslice_is_empty(me)) return substr_slice;
        
    ringslice_cnt_t chars_checked = 0;
    ringslice_cnt_t cmp_pos = 0;
    
    while(chars_checked < ringslice_len(me)) 
    {
        cmp_pos = 0;
        
        while(cmp_pos < substr_len && substr[cmp_pos] != '\0') 
        {
            if (chars_checked + cmp_pos >= ringslice_len(me)) {
                cmp_pos = 0;
                break;
            }
            uint8_t const *check_ptr = ringslice_ptr_increment_wrap_around(first_ptr, cmp_pos, buf_start, buf_end);
            if (*check_ptr != substr[cmp_pos]) {
                cmp_pos = 0;
                break;
            }
            cmp_pos++;
        }
        if(cmp_pos > 0 && (cmp_pos == substr_len || substr[cmp_pos] == '\0')) {
            ringslice_cnt_t start_idx = (ringslice_cnt_t)(first_ptr - buf_start);
            uint8_t const *end_ptr = ringslice_ptr_increment_wrap_around(first_ptr, cmp_pos, buf_start, buf_end);
            ringslice_cnt_t end_idx = (ringslice_cnt_t)(end_ptr - buf_start);
            
            substr_slice.first = start_idx;
            substr_slice.last = end_idx;
            break;
        } 
        first_ptr = ringslice_ptr_increment_wrap_around(first_ptr, 1, buf_start, buf_end);
        chars_checked++;
    }
    return substr_slice;
}

/*!
* Searches for substring in ringslice instance
* @param[in] me ringslice instance where substring is searched for
* @param[in] substr searched substring
*
* @return subslice of me slice containing substring, otherwise empty ringslice
*
* @note if substr is empty string, then copy of me slice will be returned
*
*/
ringslice_t ringslice_strstr(ringslice_t const *const me, char const *substr) {
    return ringslice_strnstr(me, substr, strlen(substr));
}

/*!
* Compares ringslice instance with string lexicographically
* @param[in] me ringslice instance which is compared with string
* @param[in] str string for compare
*
* @return 0 if are equal,
*   negative value if ringslice appears before str in lexicographical order,
*   positive value if ringslice appears after str in lexicographical order
*
*/
int ringslice_strcmp(ringslice_t const *const me, char const *str) {
    uint8_t const *first_ptr = &(me->buf[me->first]);
    uint8_t const *last_ptr = &(me->buf[me->last]);
    uint8_t const *const buf_end = &(me->buf[me->buf_size]);
    uint8_t const *const buf_start = &(me->buf[0]);
    uint8_t const *chr = (uint8_t const *)str;

    while (first_ptr != last_ptr) {
        int diff = (int)*first_ptr - (int)*chr;
        if (diff) {
            return diff;
        }

        chr++;
        first_ptr = ringslice_ptr_increment_wrap_around(first_ptr, 1, buf_start, buf_end);
    }

    return -(int)*chr;
}

/*!
* Compares ringslice instance with string lexicographically and lenght check
* @param[in] me ringslice instance which is compared with string
* @param[in] str string for compare
* @param[in] n lenght
*
* @return 0 if are equal,
*   negative value if ringslice appears before str in lexicographical order,
*   positive value if ringslice appears after str in lexicographical order
*
*/
int ringslice_strncmp(ringslice_t const *const me, char const *str, int n) {
    if (n <= 0) {
        return 0;
    }
    
    uint8_t const *first_ptr = &(me->buf[me->first]);
    uint8_t const *last_ptr = &(me->buf[me->last]);
    uint8_t const *const buf_end = &(me->buf[me->buf_size]);
    uint8_t const *const buf_start = &(me->buf[0]);
    uint8_t const *chr = (uint8_t const *)str;

    int count = 0;
    while (first_ptr != last_ptr && count < n) {
        int diff = (int)*first_ptr - (int)*chr;
        if (diff) {
            return diff;
        }

        chr++;
        count++;
        first_ptr = ringslice_ptr_increment_wrap_around(first_ptr, 1, buf_start, buf_end);
    }

    if (count == n) {
        return 0; 
    }
    
    return -(int)*chr;
}

/*!
* Searches for subslice with suffix in ringslice instance
* @param[in] me ringslice instance where suffix is searched for
* @param[in] from_idx start index for searching; if 0, then search from beginning of the slice
* @param[in] suffix suffix that is searched for
* @param[in] suffix_len lenght of suffix to search
*
* @return subslice of me slice with suffix, otherwise empty ringslice
*
* @note if suffix is empty string, then copy of me slice will be returned
*
*/
ringslice_t ringslice_subslice_with_nsuffix(ringslice_t const *const me, ringslice_cnt_t from_idx, char const *suffix, const ringslice_cnt_t suffix_len) {
    ringslice_cnt_t const rs_len = ringslice_len(me);
    DBC_ASSERT(204, from_idx <= ringslice_len(me));

    ringslice_t resp_slice = ringslice_initializer(me->buf, me->buf_size, me->first, me->first);  // initialize with empty slice
    ringslice_t search_slice = ringslice_subslice(me, from_idx, rs_len);
    ringslice_t suffix_slice = ringslice_strnstr(&search_slice, suffix, suffix_len);

    if (ringslice_is_empty(&suffix_slice)) {
        // resp_slice already initialized with empty slice
    } else {
        resp_slice.last = suffix_slice.last;
    }

    return resp_slice;
}

/*!
* Searches for subslice with suffix in ringslice instance
* @param[in] me ringslice instance where suffix is searched for
* @param[in] from_idx start index for searching; if 0, then search from beginning of the slice
* @param[in] suffix suffix that is searched for
*
* @return subslice of me slice with suffix, otherwise empty ringslice
*
* @note if suffix is empty string, then copy of me slice will be returned
*
*/
ringslice_t ringslice_subslice_with_suffix(ringslice_t const *const me, ringslice_cnt_t from_idx, char const *suffix) {
  return ringslice_subslice_with_nsuffix(me, from_idx, suffix, strlen(suffix));
}

/**
* @brief Gets gap slice between two subslices from one buffer. slice1 should be before slice2.
*        Slices shouldnt be overlaped.
* @param[in] slice1 first ringslice instance
* @param[in] slice2 second ringslice instance
* @return gap slice instance
*/
ringslice_t ringslice_subslice_gap(ringslice_t const *const slice1, ringslice_t const *const slice2) { 
    DBC_ASSERT(705, (slice1->buf == slice2->buf) || (slice1->buf_size == slice2->buf_size)); //different buffers

    uint8_t* const rs_buff = slice1->buf;
    ringslice_cnt_t const rs_buff_len = slice1->buf_size;

    DBC_ASSERT(706, slice1->first < rs_buff_len && slice1->last < rs_buff_len && // within bounds
                    slice2->first < rs_buff_len && slice2->last < rs_buff_len);
    
    ringslice_cnt_t a_first = slice1->first;
    ringslice_cnt_t a_last = (slice1->last < slice1->first) ? slice1->last + rs_buff_len : slice1->last;
    ringslice_cnt_t b_first = slice2->first;
    ringslice_cnt_t b_last = (slice2->last < slice2->first) ? slice2->last + rs_buff_len : slice2->last;

    DBC_ASSERT(707, !((a_first <= b_first && b_first < a_last) || // slices should NOT overlap
                     (a_first < b_last  && b_last  <= a_last)  ||
                     (b_first <= a_first && a_first < b_last)  ||
                     (b_first < a_last  && a_last  <= b_last)));

    DBC_ASSERT(708, (a_last <= b_first) || (b_last <= a_first));

    return ringslice_initializer(rs_buff, rs_buff_len, slice1->last, slice2->first);
}

/**
* @brief Checks if slice1 is located later in buffer than slice2
* @param[in] slice1 first ringslice instance
* @param[in] slice2 second ringslice instance
* @return true if slice1 ends after slice2, false otherwise
*
* @note Both slices must be from the same parent buffer
*/
bool ringslice_is_later_than(ringslice_t const *const slice1, ringslice_t const *const slice2) {
    DBC_MODULE_REQUIRE(RINGSLICE_MODULE, 901, slice1 != NULL);
    DBC_MODULE_REQUIRE(RINGSLICE_MODULE, 902, slice2 != NULL);
    DBC_MODULE_REQUIRE(RINGSLICE_MODULE, 903, slice1->buf == slice2->buf);
    DBC_MODULE_REQUIRE(RINGSLICE_MODULE, 904, slice1->buf_size == slice2->buf_size);
    
    if(slice1->last >= slice1->first && slice2->last >= slice2->first) return slice1->last > slice2->last;
    if(slice1->last < slice1->first && slice2->last >= slice2->first)  return true;
    if(slice2->last < slice2->first && slice1->last >= slice1->first)  return false;
    if(slice1->last < slice1->first && slice2->last < slice2->first)   return slice1->last < slice2->last;    
    
    return false; 
}

int ringslice_prefixcmp(ringslice_t const *const me, char const *str) {
    uint8_t const *buf = me->buf;
    ringslice_cnt_t idx = me->first;
    ringslice_cnt_t last = me->last;
    ringslice_cnt_t size = me->buf_size;

    while (*str && idx != last) {
        int diff = (int)buf[idx] - (int)*str;
        if (diff) {
            return diff;
        }

        str++;
        idx = ringslice_index_shift_wrap_around(idx, 1, size);
    }

    return -(int)*str;
}
