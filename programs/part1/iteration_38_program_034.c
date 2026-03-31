/* test_auto_inc_dec.c
 * This program creates memory access patterns that should trigger
 * GCC's auto-increment/decrement optimization pass, specifically
 * targeting the zero-offset pattern in find_inc().
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 256
#define CHECKSUM_SEED 0x55AA

/* Global volatile to prevent dead code elimination */
volatile int g_checksum = 0;

/* Noinline to ensure functions aren't inlined and their loops remain distinct */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ - should generate base+0 addressing */
    while (n-- > 0) {
        *dst++ = *src++;
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src - should generate base+0 addressing */
    dst += n;
    src += n;
    while (n-- > 0) {
        *--dst = *--src;
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *data, int n) {
    /* Pattern: sum += *ptr++ - read with post-increment */
    int sum = 0;
    const int *ptr = data;
    while (n-- > 0) {
        sum += *ptr++;
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(char *dst, const char *src, int n) {
    /* Pattern: *dst-- = *src++ - mixed increment/decrement */
    char *d = dst + n - 1;
    const char *s = src;
    while (n-- > 0) {
        *d-- = *s++;
    }
}

static void __attribute__((noinline))
fill_with_pre_increment(short *arr, int n, short value) {
    /* Pattern: *--ptr = value - write with pre-decrement */
    short *ptr = arr + n;
    while (n-- > 0) {
        *--ptr = value;
    }
}

/* Test function that combines multiple patterns */
static int __attribute__((noinline))
test_auto_inc_patterns(void) {
    int src[SIZE];
    int dst[SIZE];
    char src_str[SIZE];
    char dst_str[SIZE];
    short short_arr[SIZE];
    
    /* Initialize source data */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
        src_str[i] = 'A' + (i % 26);
        short_arr[i] = (short)(i * 2);
    }
    
    /* Clear destination arrays */
    memset(dst, 0, sizeof(dst));
    memset(dst_str, 0, sizeof(dst_str));
    
    /* Test 1: Post-increment copy */
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            return -1;
        }
    }
    
    /* Test 2: Pre-decrement copy (reverse copy) */
    int temp[SIZE];
    for (int i = 0; i < SIZE; i++) {
        temp[i] = src[i];
    }
    copy_with_pre_decrement(dst, temp, SIZE);
    
    /* Verify reversed copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != temp[SIZE - 1 - i]) {
            return -2;
        }
    }
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, SIZE);
    int expected_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        expected_sum += src[i];
    }
    if (sum != expected_sum) {
        return -3;
    }
    
    /* Test 4: Reverse string with mixed pointers */
    reverse_with_dual_pointers(dst_str, src_str, SIZE);
    
    /* Verify reversed string */
    for (int i = 0; i < SIZE; i++) {
        if (dst_str[i] != src_str[SIZE - 1 - i]) {
            return -4;
        }
    }
    
    /* Test 5: Fill with pre-increment */
    fill_with_pre_increment(short_arr, SIZE / 2, 0x55AA);
    
    /* Verify fill */
    for (int i = SIZE / 2; i < SIZE; i++) {
        if (short_arr[i] != 0x55AA) {
            return -5;
        }
    }
    
    /* Store checksum to volatile global to prevent optimization */
    g_checksum = sum;
    
    return 0; /* All tests passed */
}

/* Additional test with pointer arithmetic in loop */
static void __attribute__((noinline))
process_buffer(uint8_t *buf, int len, uint8_t key) {
    /* Pattern: *p++ ^= key - modify with post-increment */
    uint8_t *p = buf;
    int n = len;
    while (n-- > 0) {
        *p++ ^= key;
    }
}

/* Test with different data types */
static int __attribute__((noinline))
test_mixed_types(void) {
    struct {
        int a;
        char b;
        short c;
    } data[100];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        data[i].a = i;
        data[i].b = i & 0xFF;
        data[i].c = i * 2;
    }
    
    /* Copy just the 'a' members using pointer arithmetic */
    int dest[100];
    int *d = dest;
    const typeof(data[0]) *s = data;
    
    /* Pattern: *d++ = s++->a - structure member access */
    for (int i = 0; i < 100; i++) {
        *d++ = s++->a;
    }
    
    /* Verify */
    for (int i = 0; i < 100; i++) {
        if (dest[i] != i) {
            return -10;
        }
    }
    
    return 0;
}

int main(void) {
    printf("Testing auto-increment/decrement patterns...\n");
    
    /* Run main pattern tests */
    int result = test_auto_inc_patterns();
    if (result != 0) {
        printf("Test failed with error: %d\n", result);
        return 1;
    }
    
    /* Test mixed types */
    result = test_mixed_types();
    if (result != 0) {
        printf("Mixed types test failed: %d\n", result);
        return 1;
    }
    
    /* Test buffer processing */
    uint8_t buffer[128];
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i;
    }
    
    process_buffer(buffer, sizeof(buffer), 0xAA);
    
    /* Verify XOR was applied */
    for (int i = 0; i < sizeof(buffer); i++) {
        if (buffer[i] != (i ^ 0xAA)) {
            printf("Buffer test failed at index %d\n", i);
            return 1;
        }
    }
    
    printf("All tests passed! (Checksum: %d)\n", g_checksum);
    printf("Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c test.c\n");
    printf("Check for auto-inc-dec pass in the dump files.\n");
    
    return 0;
}
