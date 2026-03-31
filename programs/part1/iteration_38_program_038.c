/* test_auto_inc_dec.c
 * Program designed to trigger GCC's auto-inc-dec pass
 * Specifically targets the zero-offset memory access pattern
 * in find_inc() function (lines 1352-1358)
 */

#include <stdio.h>
#include <string.h>

/* Prevent inlining to ensure loops remain intact */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ - both have zero offset */
    while (n--) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src - both have zero offset */
    while (n--) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ - zero offset access */
    while (n--) {
        sum += *p++;  /* Memory access with zero offset */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Pattern: swap using *start++ and *end-- */
    while (start < end) {
        int temp = *start;    /* Zero offset */
        *start++ = *end;      /* Zero offset */
        *end-- = temp;        /* Zero offset */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char value, int n) {
    char *p = buf;
    
    /* Pattern: *p++ = value - simple zero-offset store */
    while (n--) {
        *p++ = value;  /* Zero offset store */
    }
}

/* Test function that combines multiple patterns */
static void __attribute__((noinline))
test_combined_patterns(void) {
    const int SIZE = 100;
    int src[SIZE], dst[SIZE];
    int i;
    
    /* Initialize source array */
    for (i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Test 1: Post-increment copy */
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    for (i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("Error in copy_with_post_increment at index %d\n", i);
            return;
        }
    }
    
    /* Test 2: Pre-decrement copy (reverse copy) */
    int src2[SIZE], dst2[SIZE];
    for (i = 0; i < SIZE; i++) {
        src2[i] = i * 2;
    }
    
    copy_with_pre_decrement(dst2, src2, SIZE);
    
    /* Verify reverse copy */
    for (i = 0; i < SIZE; i++) {
        if (dst2[i] != src2[SIZE - 1 - i]) {
            printf("Error in copy_with_pre_decrement at index %d\n", i);
            return;
        }
    }
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, SIZE);
    int expected_sum = 0;
    for (i = 0; i < SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (sum != expected_sum) {
        printf("Error in sum_with_post_increment: %d != %d\n", 
               sum, expected_sum);
        return;
    }
    
    /* Test 4: Reverse in-place with dual pointers */
    int arr[SIZE];
    for (i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    reverse_with_dual_pointers(arr, SIZE);
    
    /* Verify reversal */
    for (i = 0; i < SIZE; i++) {
        if (arr[i] != SIZE - 1 - i) {
            printf("Error in reverse_with_dual_pointers at index %d\n", i);
            return;
        }
    }
    
    /* Test 5: Fill with post-increment */
    char buffer[SIZE];
    fill_with_post_increment(buffer, 'A', SIZE);
    
    /* Verify fill */
    for (i = 0; i < SIZE; i++) {
        if (buffer[i] != 'A') {
            printf("Error in fill_with_post_increment at index %d\n", i);
            return;
        }
    }
    
    printf("All tests passed! Auto-inc-dec patterns triggered successfully.\n");
}

int main(void) {
    /* Call test function multiple times to ensure optimization */
    test_combined_patterns();
    
    /* Additional simple loop to increase chance of pattern matching */
    {
        int small_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        int small_copy[10];
        int *src = small_array;
        int *dst = small_copy;
        int count = 10;
        
        /* Very clear zero-offset pattern */
        while (count--) {
            *dst++ = *src++;
        }
        
        /* Use result to prevent dead code elimination */
        int verify = 0;
        for (int i = 0; i < 10; i++) {
            verify += small_copy[i];
        }
        
        if (verify != 55) {  /* Sum of 1..10 */
            printf("Verification failed: %d\n", verify);
        }
    }
    
    return 0;
}
