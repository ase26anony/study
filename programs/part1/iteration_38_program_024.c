/* test_auto_inc_dec.c
 * Program designed to trigger GCC's auto-inc-dec pass
 * Specifically targets the zero-offset memory access pattern
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256

/* Global volatile to prevent dead code elimination */
volatile int g_checksum = 0;

/* __attribute__((noinline)) prevents inlining which could obscure patterns */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates base+0 addressing */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src creates base+0 addressing */
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *src, int n) {
    int sum = 0;
    const int *p = src;
    
    /* Pattern: sum += *p++ creates base+0 addressing */
    while (n-- > 0) {
        sum += *p++;  /* Memory access with pointer increment */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Pattern: *start++ = *end-- creates two base+0 accesses */
    while (start < end) {
        int temp = *end;    /* Memory read with base+0 */
        *end-- = *start;    /* Memory write with base+0 */
        *start++ = temp;    /* Memory write with base+0 */
    }
}

static void __attribute__((noinline))
byte_copy_with_increment(char *dst, const char *src, int n) {
    /* Using char pointers for simpler addressing */
    while (n-- > 0) {
        *dst++ = *src++;  /* Simple byte copy with post-increment */
    }
}

/* Test function that combines multiple patterns */
static void __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst1[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    char src_bytes[ARRAY_SIZE];
    char dst_bytes[ARRAY_SIZE];
    
    /* Initialize source arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
        src_bytes[i] = (char)(i & 0xFF);
    }
    
    /* Test 1: Post-increment copy */
    copy_with_post_increment(dst1, src, ARRAY_SIZE);
    
    /* Test 2: Pre-decrement copy (reverse copy) */
    copy_with_pre_decrement(dst2, src, ARRAY_SIZE);
    
    /* Test 3: Sum calculation with post-increment */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    g_checksum = sum;  /* Use global volatile to preserve computation */
    
    /* Test 4: Array reversal using dual pointers */
    int temp_arr[10];
    for (int i = 0; i < 10; i++) temp_arr[i] = i;
    reverse_with_dual_pointers(temp_arr, 10);
    
    /* Test 5: Byte copy with increment */
    byte_copy_with_increment(dst_bytes, src_bytes, ARRAY_SIZE);
    
    /* Verify results to prevent optimization */
    int verify_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        verify_sum += dst1[i];
    }
    
    if (verify_sum != sum) {
        printf("Verification failed!\n");
    }
}

/* Main driver with multiple test cases */
int main(void) {
    printf("Testing auto-increment/decrement patterns...\n");
    
    /* Run multiple times to increase chance of pattern recognition */
    for (int iteration = 0; iteration < 3; iteration++) {
        test_combined_patterns();
        
        /* Additional simple direct test */
        int small_src[16];
        int small_dst[16];
        
        for (int i = 0; i < 16; i++) {
            small_src[i] = i + iteration;
        }
        
        /* Direct loop that should generate ideal pattern */
        {
            int *dst = small_dst;
            const int *src = small_src;
            int count = 16;
            
            /* This is the most direct pattern for auto-inc-dec */
            while (count--) {
                *dst++ = *src++;  /* Should create (mem (reg)) with offset 0 */
            }
        }
        
        /* Test decrement pattern */
        {
            int *dst = small_dst + 16;
            const int *src = small_src + 16;
            int count = 16;
            
            while (count--) {
                *--dst = *--src;  /* Pre-decrement pattern */
            }
        }
    }
    
    printf("Test completed. Checksum: %d\n", g_checksum);
    return 0;
}
