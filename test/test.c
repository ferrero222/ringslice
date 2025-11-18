//============================================================================
// ET: embedded test; very simple test example
//============================================================================
#include <string.h>
#include <stdio.h>
#include "et.h"  // ET: embedded test
#include "ringslice.h"

void setup(void) {
    // executed before *every* non-skipped test
}

void teardown(void) {
    // executed after *every* non-skipped and non-failing test
}

// test group ----------------------------------------------------------------
TEST_GROUP("Basic") {
    TEST("Testing ringslice_len(), continuous ring buffer") {
        char test_buf[] = "abcdefghij";  // note that 'j' character not in ring buffer
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf, strlen(test_buf), 0, 9);
        VERIFY(ringslice_len(&rs) == strlen("abcdefghi"));
    }

    TEST("Testing ringslice_len(), discontinuous ring buffer") {
        char test_buf[] = "abcdefghij";  // note that 'j' character not in ring buffer
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf, strlen(test_buf), 9, 8);
        VERIFY(ringslice_len(&rs) == strlen("jabcdefgh"));
    }

    TEST("Testing ringslice_strcmp(), simple test") {
        char const slice_str_beg[] = "Hell";
        char const slice_str_end[] = "o World!";

        char buf[sizeof("Hello World!") + 10];

        ringslice_cnt_t buffer_size = (ringslice_cnt_t)ARRAY_NELEM(buf);

        ringslice_cnt_t last = (ringslice_cnt_t)strlen(slice_str_end);
        ringslice_cnt_t first = (buffer_size - (ringslice_cnt_t)strlen(slice_str_beg)) % buffer_size;

        memcpy(buf, slice_str_end, strlen(slice_str_end));            // copy the end part of string into beginning of buffer
        memcpy(&(buf[first]), slice_str_beg, strlen(slice_str_beg));  // copy the beginning of string into part of buffer

        ringslice_t rs = ringslice_initializer((uint8_t *)buf, buffer_size, first, last);

        VERIFY(ringslice_strcmp(&rs, "Hello World!") == 0);
        VERIFY(ringslice_strcmp(&rs, "Hello!") < 0);
        VERIFY(ringslice_strcmp(&rs, "Hello there") < 0);
        VERIFY(ringslice_strcmp(&rs, "Hello") > 0);
        VERIFY(ringslice_strcmp(&rs, "Hello Nick") > 0);
        VERIFY(ringslice_strcmp(&rs, "Hello World! ") < 0);
    }

    TEST("Testing ringslice_strcmp(), buffer and positioning variation") {
        char const slice_str[] = "Hello World!";
        int const slice_str_len = strlen(slice_str);

        char buf[sizeof(slice_str) + 20];

        for (int i = 0; i < slice_str_len; i++) {
            for (int buffer_size = (int)ARRAY_NELEM(slice_str); buffer_size < (int)ARRAY_NELEM(buf); buffer_size++) {
                ringslice_cnt_t last = slice_str_len - (ringslice_cnt_t)i;
                ringslice_cnt_t first = (buffer_size - i) % buffer_size;
                memcpy(buf, &(slice_str[i]), slice_str_len - i);  // copy the end part of string into beginning of buffer
                memcpy(&(buf[first]), slice_str, i);              // copy the beginning of string into part of buffer

                ringslice_t rs = ringslice_initializer((uint8_t *)buf, buffer_size, first, last);

                VERIFY(ringslice_strcmp(&rs, slice_str) == 0);
                VERIFY(ringslice_strcmp(&rs, "Hello!") < 0);
                VERIFY(ringslice_strcmp(&rs, "Hello there") < 0);
                VERIFY(ringslice_strcmp(&rs, "Hello") > 0);
                VERIFY(ringslice_strcmp(&rs, "Hello Nick") > 0);
                VERIFY(ringslice_strcmp(&rs, "Hello World! ") < 0);
            }
        }
    }

    TEST("Testing ringslice_subslice(), continuous ring buffer") {
        char const test_buf[] = "abcdefghij";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf, strlen(test_buf), 0, 9);  // note that 'j' character not in ring buffer

        ringslice_t subrs = ringslice_subslice(&rs, 0, ringslice_len(&rs));
        VERIFY(ringslice_strcmp(&subrs, "abcdefghi") == 0);

        subrs = ringslice_subslice(&rs, 1, ringslice_len(&rs));
        VERIFY(ringslice_strcmp(&subrs, "bcdefghi") == 0);

        subrs = ringslice_subslice(&rs, 1, ringslice_len(&rs) - 2);
        VERIFY(ringslice_strcmp(&subrs, "bcdefg") == 0);

        for (ringslice_cnt_t i = 0; i < ringslice_len(&rs); i++) {
            subrs = ringslice_subslice(&rs, i, i);
            VERIFY(ringslice_is_empty(&subrs));  // because there is no such substring, the slice must be empty
        }
    }

    TEST("Testing ringslice_subslice(), discontinuous ring buffer") {
        char const test_buf[] = "efghijabcd";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf, strlen(test_buf), 6, 5);  // note that 'j' character not in ring buffer

        ringslice_t subrs = ringslice_subslice(&rs, 0, ringslice_len(&rs));
        VERIFY(ringslice_strcmp(&subrs, "abcdefghi") == 0);

        subrs = ringslice_subslice(&rs, 1, ringslice_len(&rs));
        VERIFY(ringslice_strcmp(&subrs, "bcdefghi") == 0);

        subrs = ringslice_subslice(&rs, 1, ringslice_len(&rs) - 2);
        VERIFY(ringslice_strcmp(&subrs, "bcdefg") == 0);

        for (ringslice_cnt_t i = 0; i < ringslice_len(&rs); i++) {
            subrs = ringslice_subslice(&rs, i, i);
            VERIFY(ringslice_is_empty(&subrs));  // because there is no such substring, the slice must be empty
        }
    }

    TEST("Testing ringslice_strstr(), continuous ring buffer") {
        char const test_buf[] = "abcdefghij";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf, strlen(test_buf), 0, 9);  // note that 'j' character not in ring buffer
        ringslice_t subrs = ringslice_strstr(&rs, "abc");
        VERIFY(ringslice_strcmp(&subrs, "abc") == 0);

        subrs = ringslice_strstr(&rs, "cd");
        VERIFY(ringslice_strcmp(&subrs, "cd") == 0);

        subrs = ringslice_strstr(&rs, "defg");
        VERIFY(ringslice_strcmp(&subrs, "defg") == 0);

        subrs = ringslice_strstr(&rs, "fghi");
        VERIFY(ringslice_strcmp(&subrs, "fghi") == 0);

        subrs = ringslice_strstr(&rs, "cdfgh");
        VERIFY(ringslice_is_empty(&subrs));             // because there is no such substring, the slice must be empty
        VERIFY(ringslice_strcmp(&subrs, "cdfgh") < 0);  // any empty slice is considered as empty string and must be less than any string
    }

    TEST("Testing ringslice_strstr(), discontinuous ring buffer") {
        char const test_buf[] = "efghijabccd";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf, strlen(test_buf), 6, 5);  // note that 'j' character not in ring buffer

        ringslice_t subrs = ringslice_strstr(&rs, "abc");
        VERIFY(ringslice_strcmp(&subrs, "abc") == 0);

        subrs = ringslice_strstr(&rs, "cd");
        VERIFY(ringslice_strcmp(&subrs, "cd") == 0);

        subrs = ringslice_strstr(&rs, "defg");
        VERIFY(ringslice_strcmp(&subrs, "defg") == 0);

        subrs = ringslice_strstr(&rs, "fghi");
        VERIFY(ringslice_strcmp(&subrs, "fghi") == 0);

        subrs = ringslice_strstr(&rs, "cdfgh");
        VERIFY(ringslice_is_empty(&subrs));             // because there is no such substring, the slice must be empty
        VERIFY(ringslice_strcmp(&subrs, "cdfgh") < 0);  // any empty slice is considered as empty string and must be less than any string
    }

    TEST("Testing ringslice_strstr(), discontinuous ring buffer, part of substring is adjacent to substring") {
        char const test_buf[] = "fghijabfgh";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf, strlen(test_buf), 6, 5);  // note that 'a' character not in ring buffer

        ringslice_t subrs = ringslice_strstr(&rs, "fghi");
        VERIFY(ringslice_strcmp(&subrs, "fghi") == 0);
    }

    TEST("Testing ringslice_subslice_with_suffix(), continuous ring buffer") {
        char const test_buf[] = "abcdefghij";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf, strlen(test_buf), 0, 9);

        ringslice_t subrs = ringslice_subslice_with_suffix(&rs, 0, "hi");
        VERIFY(ringslice_strcmp(&subrs, "abcdefghi") == 0);
    }

    TEST("Testing ringslice_subslice_with_suffix(), discontinuous ring buffer") {
        char const test_buf[] = "efghijabcd";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf, strlen(test_buf), 6, 5);

        ringslice_t subrs = ringslice_subslice_with_suffix(&rs, 0, "hi");
        VERIFY(ringslice_strcmp(&subrs, "abcdefghi") == 0);
    }

    TEST("Testing ringslice_subslice_equals()") {
        char const test_buf[] = "efghijabcdsdadsaaasd";
        ringslice_t rs1 = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                5,
                                                2);
        ringslice_t rs2 = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                5,
                                                2);
        bool res = ringslice_subslice_equals(&rs1, &rs2);
        VERIFY(res == true);
        rs2.first = 6;
        res = ringslice_subslice_equals(&rs1, &rs2);
        VERIFY(res == false);
    }

    TEST("Testing ringslice_subslice_get_content()") {
        char const test_buf[] = "0123456789";
        char dest_buf[50] = {'F'};
        memset(dest_buf, 'F', 50);
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                0,
                                                3);
        ringslice_subslice_get_content(&rs, (uint8_t*)dest_buf, 50);
        VERIFY(memcmp((uint8_t*)dest_buf, "012FFF", 6) == 0);
        memset(dest_buf, 'F', 50);
        rs = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                7,
                                                3);
        ringslice_subslice_get_content(&rs, (uint8_t*)dest_buf, 50);
        VERIFY(memcmp((uint8_t*)dest_buf, "789012FF", 8) == 0);
        memset((uint8_t*)dest_buf, 'F', 50);
        ringslice_subslice_get_content(&rs, (uint8_t*)dest_buf, 4);
        VERIFY(memcmp((uint8_t*)dest_buf, "7890FF", 6) == 0);
    }

    TEST("Testing ringslice_subslice_gap()") {
        char const test_buf[] = "qwer1234rewq";
        ringslice_t rs1 = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                0,
                                                4);
        ringslice_t rs2 = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                8,
                                                0);
        ringslice_t rs_res = ringslice_subslice_gap(&rs1, &rs2);
        VERIFY(ringslice_strcmp(&rs_res, "1234") == 0);
        
        rs1 = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                6,
                                                10);
        rs2 = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                2,
                                                6);
        rs_res = ringslice_subslice_gap(&rs1, &rs2);
        
        VERIFY(ringslice_strcmp(&rs_res, "wqqw") == 0);   
    }

    TEST("Testing ringslice_subslice_after()") {
        char const test_buf[] = "qwer1234rewq";
        ringslice_t rs1 = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                0,
                                                11);
        ringslice_t rs2 = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                0,
                                                4);
        ringslice_t rs_res = ringslice_subslice_after(&rs1, &rs2, 4);
        VERIFY(ringslice_strcmp(&rs_res, "1234") == 0);
        rs_res = ringslice_subslice_after(&rs1, &rs2, 0);
        VERIFY(ringslice_strcmp(&rs_res, "1234rew") == 0);   
    }


    TEST("Testing ringslice_strncmp(), simple test") {
        char const slice_str_beg[] = "Hell";
        char const slice_str_end[] = "o World!";

        char buf[sizeof("Hello World!") + 10];

        ringslice_cnt_t buffer_size = (ringslice_cnt_t)ARRAY_NELEM(buf);

        ringslice_cnt_t last = (ringslice_cnt_t)strlen(slice_str_end);
        ringslice_cnt_t first = (buffer_size - (ringslice_cnt_t)strlen(slice_str_beg)) % buffer_size;

        memcpy(buf, slice_str_end, strlen(slice_str_end));            // copy the end part of string into beginning of buffer
        memcpy(&(buf[first]), slice_str_beg, strlen(slice_str_beg));  // copy the beginning of string into part of buffer

        ringslice_t rs = ringslice_initializer((uint8_t *)buf, buffer_size, first, last);

        VERIFY(ringslice_strncmp(&rs, "Hello World!", buffer_size) == 0);
        VERIFY(ringslice_strncmp(&rs, "Hello!", buffer_size) < 0);
        VERIFY(ringslice_strncmp(&rs, "Hello there", buffer_size) < 0);
        VERIFY(ringslice_strncmp(&rs, "Hello", buffer_size) > 0);
        VERIFY(ringslice_strncmp(&rs, "Hello Nick", buffer_size) > 0);
        VERIFY(ringslice_strncmp(&rs, "Hello World! ", buffer_size) < 0);
    }

    TEST("Testing ringslice_strncmp(), buffer and positioning variation") {
        char const slice_str[] = "Hello World!";
        int const slice_str_len = strlen(slice_str);

        char buf[sizeof(slice_str) + 20];

        for (int i = 0; i < slice_str_len; i++) {
            for (int buffer_size = (int)ARRAY_NELEM(slice_str); buffer_size < (int)ARRAY_NELEM(buf); buffer_size++) {
                ringslice_cnt_t last = slice_str_len - (ringslice_cnt_t)i;
                ringslice_cnt_t first = (buffer_size - i) % buffer_size;
                memcpy(buf, &(slice_str[i]), slice_str_len - i);  // copy the end part of string into beginning of buffer
                memcpy(&(buf[first]), slice_str, i);              // copy the beginning of string into part of buffer

                ringslice_t rs = ringslice_initializer((uint8_t *)buf, buffer_size, first, last);

                VERIFY(ringslice_strncmp(&rs, slice_str, buffer_size) == 0);
                VERIFY(ringslice_strncmp(&rs, "Hello!", buffer_size) < 0);
                VERIFY(ringslice_strncmp(&rs, "Hello there", buffer_size) < 0);
                VERIFY(ringslice_strncmp(&rs, "Hello", buffer_size) > 0);
                VERIFY(ringslice_strncmp(&rs, "Hello Nick", buffer_size) > 0);
                VERIFY(ringslice_strncmp(&rs, "Hello World! ", buffer_size) < 0);
            }
        }
    }


    TEST("Testing ringslice_sscanf(), discontinuous ring buffer, integers") {
        char const test_buf[] = "G: 1, 2, 0xFFEF +CRE";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                strlen(test_buf) - strlen("+CRE"),
                                                strlen(test_buf) - strlen("+CRE") - 1);
        long a, b, c;
        int argc = ringslice_scanf(&rs, "+CREG:%d,%d,%x\n", &a, &b, &c);
        VERIFY(argc == 3);
        VERIFY(a == 1);
        VERIFY(b == 2);
        VERIFY(c == 0xFFEF);
    }

    TEST("Testing ringslice_sscanf(), discontinuous ring buffer, excluding scanset") {
        char const test_buf[] = "R:\"REC UNREAD\"   +CMG";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                strlen(test_buf) - strlen("+CMG"),
                                                strlen(test_buf) - strlen("+CMG") - 1);
        char string_buf[20] = {0};
        int argc = ringslice_scanf(&rs, "+CMGR: \"%15[^\"]\"", string_buf);
        VERIFY(argc == 1);
        VERIFY(strcmp("REC UNREAD", string_buf) == 0);
    }

    TEST("Testing ringslice_sscanf(), discontinuous ring buffer, including scanset") {
        char const test_buf[] = "R:\"REC-UNREAD\"   +CMG";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                strlen(test_buf) - strlen("+CMG"),
                                                strlen(test_buf) - strlen("+CMG") - 1);
        char string_buf[20] = {0};
        int argc = ringslice_scanf(&rs, "+CMGR: \"%15[-A-Z]\"", string_buf);
        VERIFY(argc == 1);
        VERIFY(strcmp("REC-UNREAD", string_buf) == 0);
    }
    
    TEST("Testing ringslice_sscanf(), discontinuous ring buffer, integer before and after scanset") {
        char const test_buf[] = "C UNREAD\", 15  +CMGR: 42 \"RE";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                strlen(test_buf) - strlen("+CMGR: 42 \"RE"),
                                                strlen(test_buf) - strlen("+CMGR: 42 \"RE") - 1);
        char string_buf[20] = {0};
        int intval1 = 0, intval2 = 0;
        int argc = ringslice_scanf(&rs, "+CMGR: %d \"%15[A-Z ]\",%d", &intval1, string_buf, &intval2);
        VERIFY(argc == 3);
        VERIFY(strcmp("REC UNREAD", string_buf) == 0);
        VERIFY(intval1 == 42);
        VERIFY(intval2 == 15);
    }

    TEST("Testing ringslice_sscanf(), discontinuous ring buffer, buffer overflow during scanset") {
        char const test_buf[] = "C UNREAD\", 15  +CMGR: 42 \"RE";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                strlen(test_buf) - strlen("+CMGR: 42 \"RE"),
                                                strlen(test_buf) - strlen("+CMGR: 42 \"RE") - 1);
        char string_buf[20] = {0};
        int intval1 = 0, intval2 = 0;
        int argc = ringslice_scanf(&rs, "+CMGR: %d \"%5[A-Z ]\",%d", &intval1, string_buf, &intval2);
        VERIFY(argc == 3);
        VERIFY(strcmp("REC U", string_buf) == 0);
        VERIFY(intval1 == 42);
    }

    TEST("Testing ringslice_sscanf(), discontinuous ring buffer, scanning brackets") {
        char const test_buf[] = "[]\" 42  +CMGR: \"";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                strlen(test_buf) - strlen("+CMGR: \""),
                                                strlen(test_buf) - strlen("+CMGR: \"") - 1);
        char string_buf[20] = {0};
        int val = 0;
        int argc = ringslice_scanf(&rs, "+CMGR: \"%15[][]\" %d", string_buf, &val);
        VERIFY(argc == 2);
        VERIFY(val == 42);
        VERIFY(strcmp("[]", string_buf) == 0);
    }
    TEST("Testing ringslice_sscanf(), discontinuous ring buffer, scip arg *") {
        char const test_buf[] = "+CMGR: \"REC UNREAD\",\"+79031234567\",56,\"23/10/15,14:30:25+12\"";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                0,
                                                strlen(test_buf) - 1);
        char string_buf[20] = {0};
        int val = 0;
        int argc = ringslice_scanf(&rs, "+CMGR: \"%*[^\"]\",\"%[^\"]\",%d,", string_buf, &val);
        VERIFY(argc == 2);
        VERIFY(val == 56);
        VERIFY(strcmp("+79031234567", string_buf) == 0);
    }
	
	  TEST("Testing ringslice_sscanf(), continuous ring buffer, new arg after scanset with witdh") {
        char const test_buf[] = "+CMGR: \"REC UNREAD\",\"+79031234567\",56,\"23/10/15,14:30:25+12\"";
        ringslice_t rs = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                0,
                                                strlen(test_buf) - 1);
        char string_buf[20] = {0};
        int val = 0;
        int argc = ringslice_scanf(&rs, "+CMGR: \"%*[^\"]\",\"%4[^\"]\",%d,", string_buf, &val);
        VERIFY(argc == 2);
        VERIFY(val == 56);
        VERIFY(strcmp("+790", string_buf) == 0);
    }

    	
    TEST("Testing ringslice_is_later_than()") {
        char const test_buf[] = "qwer1234rewq";
        ringslice_t rs1 = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                0,
                                                4);
        ringslice_t rs2 = ringslice_initializer((uint8_t *)test_buf,
                                                strlen(test_buf),
                                                8,
                                                0);
        bool res = ringslice_is_later_than(&rs2, &rs1);
        VERIFY(res);
        res = ringslice_is_later_than(&rs1, &rs2);
        VERIFY(!res);
    }
	
}  // TEST_GROUP()