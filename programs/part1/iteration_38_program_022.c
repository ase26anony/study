/* test_auto_inc_dec.c
 * Program designed to trigger GCC's auto-inc-dec pass
 * Specifically targets the zero-offset memory access pattern
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 1024
#define CHECKSUM_SEED 0x12345678

/* Prevent inlining to ensure loops remain distinct */
static void __attribute__((noinline)) 
copy_forward_with_postinc(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ - should generate base+0 offset */
    while (n--) {
        *dst++ = *src++;
    }
}

static void __attribute__((noinline))
copy_backward_with_predec(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src - should generate base+0 offset */
    dst += n;
    src += n;
    while (n--) {
        *--dst = *--src;
    }
}

static int __attribute__((noinline))
sum_with_postinc(const int *arr, int n) {
    /* Pattern: sum += *ptr++ - read with post-increment */
    int sum = 0;
    const int *ptr = arr;
    while (n--) {
        sum += *ptr++;
    }
    return sum;
}

static void __attribute__((noinline))
fill_with_preinc(int *arr, int n, int value) {
    /* Pattern: *++ptr = value - write with pre-increment */
    int *ptr = arr - 1; /* Start one before */
    while (n--) {
        *++ptr = value;
    }
}

static void __attribute__((noinline))
reverse_with_dual_incdec(int *arr, int n) {
    /* Pattern: Dual pointer with inc/dec */
    int *start = arr;
    int *end = arr + n - 1;
    while (start < end) {
        /* Both accesses should have zero offset */
        int temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

static int __attribute__((noinline))
process_char_buffer(char *dst, const char *src, int n) {
    /* Char type to test different data widths */
    char *d = dst;
    const char *s = src;
    int checksum = CHECKSUM_SEED;
    
    while (n--) {
        *d++ = *s++;
        checksum ^= (int)*s; /* Use result to prevent elimination */
    }
    return checksum;
}

/* Volatile global to ensure side effects are visible */
volatile int global_checksum = 0;

int main(void) {
    /* Test data setup */
    int src[SIZE];
    int dst[SIZE];
    char char_src[SIZE];
    char char_dst[SIZE];
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
        char_src[i] = (char)(i & 0xFF);
    }
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Test 1: Forward copy with post-increment */
    memset(dst, 0, sizeof(dst));
    copy_forward_with_postinc(dst, src, SIZE);
    
    /* Verify */
    int ok1 = memcmp(src, dst, sizeof(src)) == 0;
    printf("Test 1 (forward postinc): %s\n", ok1 ? "PASS" : "FAIL");
    
    /* Test 2: Backward copy with pre-decrement */
    memset(dst, 0, sizeof(dst));
    copy_backward_with_predec(dst, src, SIZE);
    
    int ok2 = memcmp(src, dst, sizeof(src)) == 0;
    printf("Test 2 (backward predec): %s\n", ok2 ? "PASS" : "FAIL");
    
    /* Test 3: Sum calculation with post-increment */
    int sum = sum_with_postinc(src, SIZE);
    int expected_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        expected_sum += src[i];
    }
    printf("Test 3 (sum postinc): %s (sum=%d, expected=%d)\n", 
           sum == expected_sum ? "PASS" : "FAIL", sum, expected_sum);
    
    /* Test 4: Fill with pre-increment */
    memset(dst, 0, sizeof(dst));
    fill_with_preinc(dst, SIZE, 42);
    
    int ok4 = 1;
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != 42) {
            ok4 = 0;
            break;
        }
    }
    printf("Test 4 (fill preinc): %s\n", ok4 ? "PASS" : "FAIL");
    
    /* Test 5: Reverse with dual pointers */
    memcpy(dst, src, sizeof(src));
    reverse_with_dual_incdec(dst, SIZE);
    
    int ok5 = 1;
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[SIZE - 1 - i]) {
            ok5 = 0;
            break;
        }
    }
    printf("Test 5 (reverse dual): %s\n", ok5 ? "PASS" : "FAIL");
    
    /* Test 6: Char buffer with mixed operations */
    int checksum = process_char_buffer(char_dst, char_src, SIZE);
    global_checksum = checksum; /* Ensure side effect */
    
    int ok6 = memcmp(char_src, char_dst, SIZE) == 0;
    printf("Test 6 (char buffer): %s (checksum=0x%x)\n", 
           ok6 ? "PASS" : "FAIL", checksum);
    
    /* Additional test: Nested pointer access */
    {
        int data[100];
        int *p = data;
        int *end = data + 100;
        
        /* Initialize */
        for (int *q = data; q < end; q++) {
            *q = (int)(q - data);
        }
        
        /* Process with simple pointer arithmetic */
        int local_sum = 0;
        p = data;
        while (p < end) {
            local_sum += *p++;
        }
        printf("Additional test (local pointer): sum=%d\n", local_sum);
    }
    
    return 0;
}
