/* test_auto_inc_dec.c
 * Program to trigger GCC's auto-increment/decrement pass
 * Specifically targets the zero-offset memory access pattern
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Global volatile to prevent dead code elimination */
volatile int g_checksum = 0;

/* Function 1: Post-increment pattern (forward copy) */
static void __attribute__((noinline)) 
copy_forward(int *dst, const int *src, int n) {
    /* Simple loop with post-increment - should generate base+0 pattern */
    while (n--) {
        *dst++ = *src++;  /* Memory access with zero offset */
    }
}

/* Function 2: Pre-decrement pattern (reverse copy) */
static void __attribute__((noinline))
copy_reverse(int *dst, const int *src, int n) {
    /* Move pointers to end and use pre-decrement */
    dst += n;
    src += n;
    
    while (n--) {
        *--dst = *--src;  /* Memory access with zero offset */
    }
}

/* Function 3: Post-increment with computation (sum calculation) */
static int __attribute__((noinline))
sum_array(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    while (n--) {
        sum += *p++;  /* Read with post-increment */
    }
    return sum;
}

/* Function 4: Mixed read/write with auto-increment */
static void __attribute__((noinline))
transform_array(int *dst, const int *src, int n) {
    /* Process array with separate read/write pointers */
    const int *read_ptr = src;
    int *write_ptr = dst;
    
    while (n--) {
        /* Both memory accesses should have zero offset */
        int val = *read_ptr++;
        *write_ptr++ = val * 2 + 1;
    }
}

/* Function 5: Char buffer copy (smaller data type) */
static void __attribute__((noinline))
copy_chars(char *dst, const char *src, int n) {
    /* Character copy often generates simple addressing */
    while (n--) {
        *dst++ = *src++;
    }
}

/* Function 6: Search with auto-increment */
static int __attribute__((noinline))
find_value(const int *arr, int n, int target) {
    const int *p = arr;
    int i = 0;
    
    while (i < n) {
        if (*p++ == target) {  /* Read with post-increment */
            return i;
        }
        i++;
    }
    return -1;
}

/* Main test driver */
int main(void) {
    const int ARRAY_SIZE = 100;
    int src[ARRAY_SIZE];
    int dst1[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    int dst3[ARRAY_SIZE];
    char src_chars[ARRAY_SIZE];
    char dst_chars[ARRAY_SIZE];
    
    /* Initialize source arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
        src_chars[i] = (char)(i % 26 + 'A');
    }
    
    /* Clear destination arrays */
    memset(dst1, 0, sizeof(dst1));
    memset(dst2, 0, sizeof(dst2));
    memset(dst3, 0, sizeof(dst3));
    memset(dst_chars, 0, sizeof(dst_chars));
    
    /* Test 1: Forward copy with post-increment */
    copy_forward(dst1, src, ARRAY_SIZE);
    
    /* Test 2: Reverse copy with pre-decrement */
    copy_reverse(dst2, src, ARRAY_SIZE);
    
    /* Test 3: Sum calculation */
    int sum = sum_array(src, ARRAY_SIZE);
    g_checksum = sum;  /* Use volatile global */
    
    /* Test 4: Transform with auto-increment */
    transform_array(dst3, src, ARRAY_SIZE);
    
    /* Test 5: Char buffer copy */
    copy_chars(dst_chars, src_chars, ARRAY_SIZE);
    
    /* Test 6: Search with auto-increment */
    int found_idx = find_value(src, ARRAY_SIZE, 100);
    
    /* Verify results */
    int errors = 0;
    
    /* Check forward copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != src[i]) errors++;
        if (dst2[ARRAY_SIZE - 1 - i] != src[i]) errors++;
        if (dst3[i] != src[i] * 2 + 1) errors++;
        if (dst_chars[i] != src_chars[i]) errors++;
    }
    
    /* Calculate expected sum */
    int expected_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (sum != expected_sum) errors++;
    
    printf("Auto-inc/dec test completed with %d errors\n", errors);
    printf("Checksum: %d (global: %d)\n", sum, g_checksum);
    printf("Search result: %s\n", found_idx >= 0 ? "found" : "not found");
    
    return errors > 0 ? 1 : 0;
}
