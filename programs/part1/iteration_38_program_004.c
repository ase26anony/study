/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass with zero-offset memory access
 * patterns, specifically targeting lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 1024

/* Prevent inlining to ensure loops remain distinct for analysis */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset memory accesses */
    while (n--) {
        *dst = *src;  /* Base + 0 offset pattern */
        dst++;        /* Post-increment */
        src++;        /* Post-increment */
    }
}

static void __attribute__((noinline))
reverse_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src creates zero-offset memory accesses */
    dst += n;  /* Point to end of destination */
    src += n;  /* Point to end of source */
    
    while (n--) {
        --dst;        /* Pre-decrement */
        --src;        /* Pre-decrement */
        *dst = *src;  /* Base + 0 offset pattern */
    }
}

static int __attribute__((noinline))
sum_with_mixed_operations(int *arr, int n) {
    /* Mixed pattern to test various zero-offset cases */
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* Pattern: sum += *p++ */
    while (p < end) {
        sum += *p;  /* Base + 0 offset pattern */
        p++;        /* Post-increment */
    }
    
    return sum;
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char val, int n) {
    /* Simple char fill with post-increment */
    while (n--) {
        *buf = val;  /* Base + 0 offset pattern */
        buf++;       /* Post-increment */
    }
}

static void __attribute__((noinline))
process_buffer_with_pointers(int16_t *dst, const int16_t *src, int n) {
    /* Using different data type to test generalization */
    while (n--) {
        *dst = *src;  /* Base + 0 offset pattern */
        dst++;
        src++;
    }
}

/* Volatile global to prevent dead code elimination */
volatile int checksum = 0;

int main(void) {
    /* Test data initialization */
    int src_array[ARRAY_SIZE];
    int dst_array[ARRAY_SIZE];
    char char_buffer[ARRAY_SIZE];
    int16_t src_short[ARRAY_SIZE];
    int16_t dst_short[ARRAY_SIZE];
    
    /* Initialize source arrays with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src_array[i] = i * 3 + 1;
        char_buffer[i] = (i % 26) + 'A';
        src_short[i] = (int16_t)(i * 2);
    }
    
    /* Clear destination arrays */
    memset(dst_array, 0, sizeof(dst_array));
    memset(dst_short, 0, sizeof(dst_short));
    
    /* Test 1: Simple copy with post-increment */
    copy_with_post_increment(dst_array, src_array, ARRAY_SIZE);
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst_array[i] != src_array[i]) {
            printf("Error: Copy mismatch at index %d\n", i);
            return 1;
        }
    }
    
    /* Test 2: Reverse copy with pre-decrement */
    int reversed[ARRAY_SIZE];
    reverse_with_pre_decrement(reversed, src_array, ARRAY_SIZE);
    
    /* Verify reverse */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (reversed[i] != src_array[ARRAY_SIZE - 1 - i]) {
            printf("Error: Reverse mismatch at index %d\n", i);
            return 1;
        }
    }
    
    /* Test 3: Sum calculation with pointer arithmetic */
    int sum = sum_with_mixed_operations(src_array, ARRAY_SIZE);
    checksum = sum;  /* Use volatile to prevent optimization */
    
    /* Test 4: Char buffer fill */
    char test_buffer[ARRAY_SIZE];
    fill_with_post_increment(test_buffer, 'X', ARRAY_SIZE);
    
    /* Verify fill */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (test_buffer[i] != 'X') {
            printf("Error: Fill mismatch at index %d\n", i);
            return 1;
        }
    }
    
    /* Test 5: Different data type (int16_t) */
    process_buffer_with_pointers(dst_short, src_short, ARRAY_SIZE);
    
    /* Verify short copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst_short[i] != src_short[i]) {
            printf("Error: Short copy mismatch at index %d\n", i);
            return 1;
        }
    }
    
    /* Additional test: Nested pointer operations */
    {
        int temp[100];
        int *p1 = temp;
        int *p2 = temp + 50;
        int count = 50;
        
        /* Pattern that should generate zero-offset accesses */
        while (count--) {
            *p1++ = *p2++;  /* Combined operation - may generate different RTL */
        }
    }
    
    printf("All tests passed successfully!\n");
    printf("Checksum (volatile): %d\n", checksum);
    
    return 0;
}
