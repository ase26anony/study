/* test_auto_inc_dec.c
 * Program to trigger zero-offset memory access patterns for GCC's auto-inc-dec pass
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Global volatile to prevent dead code elimination */
volatile int g_checksum = 0;

/* Function 1: Post-increment pattern */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ generates base+0 offset */
    while (n--) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

/* Function 2: Pre-decrement pattern */
static void __attribute__((noinline)) 
reverse_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end and use pre-decrement */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src generates base+0 offset */
    while (n--) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

/* Function 3: Mixed read/write with post-increment */
static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ generates base+0 offset for read */
    while (n--) {
        sum += *p++;  /* Memory read with zero offset */
    }
    return sum;
}

/* Function 4: Pointer arithmetic in loop with zero offset */
static void __attribute__((noinline))
fill_with_post_increment(char *buf, char val, int n) {
    char *p = buf;
    
    /* Pattern: *p++ = val generates base+0 offset for write */
    while (n--) {
        *p++ = val;  /* Memory write with zero offset */
    }
}

/* Function 5: Array copy with char pointers (smaller unit) */
static void __attribute__((noinline))
copy_chars_with_inc(char *dst, const char *src, int n) {
    /* Simple pointer increment pattern */
    while (n--) {
        *dst++ = *src++;
    }
}

/* Function 6: Search loop with post-increment */
static int __attribute__((noinline))
find_with_post_increment(const int *arr, int n, int target) {
    const int *p = arr;
    int count = 0;
    
    while (n--) {
        if (*p++ == target) {  /* Read with zero offset */
            count++;
        }
    }
    return count;
}

/* Main test driver */
int main(void) {
    const int ARRAY_SIZE = 256;
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    char char_buf1[ARRAY_SIZE];
    char char_buf2[ARRAY_SIZE];
    
    /* Initialize source arrays with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
        char_buf1[i] = (char)(i & 0xFF);
    }
    
    /* Test 1: Post-increment copy */
    memset(dst, 0, sizeof(dst));
    copy_with_post_increment(dst, src, ARRAY_SIZE);
    
    /* Verify copy */
    int copy_ok = 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            copy_ok = 0;
            break;
        }
    }
    printf("Test 1 (post-increment copy): %s\n", copy_ok ? "PASS" : "FAIL");
    
    /* Test 2: Pre-decrement reverse */
    int src_rev[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src_rev[i] = src[ARRAY_SIZE - 1 - i];
    }
    
    memset(dst, 0, sizeof(dst));
    reverse_with_pre_decrement(dst, src, ARRAY_SIZE);
    
    int reverse_ok = 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src_rev[i]) {
            reverse_ok = 0;
            break;
        }
    }
    printf("Test 2 (pre-decrement reverse): %s\n", reverse_ok ? "PASS" : "FAIL");
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    int expected_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    printf("Test 3 (sum with post-increment): %s (sum=%d, expected=%d)\n",
           sum == expected_sum ? "PASS" : "FAIL", sum, expected_sum);
    
    /* Test 4: Fill with post-increment */
    memset(char_buf2, 0, sizeof(char_buf2));
    fill_with_post_increment(char_buf2, 'A', ARRAY_SIZE);
    
    int fill_ok = 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (char_buf2[i] != 'A') {
            fill_ok = 0;
            break;
        }
    }
    printf("Test 4 (fill with post-increment): %s\n", fill_ok ? "PASS" : "FAIL");
    
    /* Test 5: Char copy with increment */
    memset(char_buf2, 0, sizeof(char_buf2));
    copy_chars_with_inc(char_buf2, char_buf1, ARRAY_SIZE);
    
    int char_copy_ok = 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (char_buf2[i] != char_buf1[i]) {
            char_copy_ok = 0;
            break;
        }
    }
    printf("Test 5 (char copy with increment): %s\n", char_copy_ok ? "PASS" : "FAIL");
    
    /* Test 6: Search with post-increment */
    int target = 100;
    int count = find_with_post_increment(src, ARRAY_SIZE, target);
    int expected_count = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (src[i] == target) {
            expected_count++;
        }
    }
    printf("Test 6 (search with post-increment): %s (found=%d, expected=%d)\n",
           count == expected_count ? "PASS" : "FAIL", count, expected_count);
    
    /* Store checksum to volatile global to ensure all computations are kept */
    g_checksum = sum + count + copy_ok + reverse_ok + fill_ok + char_copy_ok;
    
    return 0;
}
