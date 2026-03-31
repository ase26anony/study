/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass with zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>

#define SIZE 256

/* Prevent inlining to ensure loops remain distinct for analysis */
static void __attribute__((noinline)) 
copy_forward(int *dst, const int *src, int n) {
    /* Pattern: post-increment with zero offset at memory access */
    while (n--) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline)) 
copy_backward(int *dst, const int *src, int n) {
    /* Move pointers to end for reverse copy */
    dst += n;
    src += n;
    
    /* Pattern: pre-decrement with zero offset at memory access */
    while (n--) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline)) 
sum_array(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: post-increment read with zero offset */
    while (n--) {
        sum += *p++;  /* Memory read with (mem (reg)) pattern */
    }
    return sum;
}

static void __attribute__((noinline))
fill_pattern(int *arr, int n, int start) {
    int *p = arr;
    int value = start;
    
    /* Pattern: post-increment write with zero offset */
    while (n--) {
        *p++ = value++;  /* Memory write with (mem (reg)) pattern */
    }
}

static void __attribute__((noinline))
reverse_in_place(int *arr, int n) {
    int *left = arr;
    int *right = arr + n - 1;
    
    /* Pattern: mixed pre/post operations */
    while (left < right) {
        int temp = *left;     /* Post-increment read */
        *left++ = *right;     /* Post-increment write */
        *right-- = temp;      /* Pre-decrement write */
    }
}

/* Global volatile to prevent dead code elimination */
volatile int checksum = 0;

int main(void) {
    int src[SIZE];
    int dst[SIZE];
    int i, result;
    
    /* Initialize source with known pattern */
    for (i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Clear destination */
    memset(dst, 0, sizeof(dst));
    
    /* Test 1: Forward copy with post-increment */
    copy_forward(dst, src, SIZE);
    
    /* Verify copy */
    for (i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Forward copy mismatch at index %d\n", i);
            return 1;
        }
    }
    
    /* Test 2: Backward copy with pre-decrement */
    int src2[SIZE];
    int dst2[SIZE];
    fill_pattern(src2, SIZE, 100);
    copy_backward(dst2, src2, SIZE);
    
    /* Verify backward copy */
    for (i = 0; i < SIZE; i++) {
        if (dst2[i] != src2[i]) {
            printf("FAIL: Backward copy mismatch at index %d\n", i);
            return 1;
        }
    }
    
    /* Test 3: Sum calculation with post-increment */
    result = sum_array(src, SIZE);
    
    /* Calculate expected sum: arithmetic series */
    int expected_sum = 0;
    for (i = 0; i < SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (result != expected_sum) {
        printf("FAIL: Sum calculation error: got %d, expected %d\n", 
               result, expected_sum);
        return 1;
    }
    
    /* Test 4: Reverse in place with mixed increments */
    int arr[SIZE];
    fill_pattern(arr, SIZE, 0);
    
    /* Keep copy for verification */
    int arr_copy[SIZE];
    memcpy(arr_copy, arr, sizeof(arr));
    
    reverse_in_place(arr, SIZE);
    
    /* Verify reversal */
    for (i = 0; i < SIZE; i++) {
        if (arr[i] != arr_copy[SIZE - 1 - i]) {
            printf("FAIL: Reverse mismatch at index %d\n", i);
            return 1;
        }
    }
    
    /* Store result in volatile to prevent optimization */
    checksum = result;
    
    printf("SUCCESS: All auto-inc-dec patterns executed correctly\n");
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
