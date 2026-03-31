/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

/* Global volatile to prevent dead code elimination */
volatile int checksum = 0;

/* noinline to ensure function isn't inlined and loop structure is preserved */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset accesses */
    while (n--) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src creates zero-offset accesses */
    dst += n;
    src += n;
    while (n--) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ creates zero-offset read */
    while (n--) {
        sum += *p++;  /* Memory read with zero offset */
    }
    
    checksum = sum;  /* Store to volatile to prevent elimination */
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Pattern: *start++ = *end-- creates zero-offset accesses */
    while (start < end) {
        int temp = *end;
        *end-- = *start;
        *start++ = temp;  /* Both writes have zero offset */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char value, int n) {
    char *p = buf;
    
    /* Pattern: *p++ = value creates zero-offset write */
    while (n--) {
        *p++ = value;  /* Memory write with zero offset */
    }
}

/* Test function that combines multiple patterns */
static void __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    
    /* Initialize source array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Test 1: Simple copy with post-increment */
    copy_with_post_increment(dst, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("Error in copy_with_post_increment at index %d\n", i);
            return;
        }
    }
    
    /* Test 2: Copy with pre-decrement (reverse copy) */
    copy_with_pre_decrement(dst2, src, ARRAY_SIZE);
    
    /* Verify reverse copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst2[i] != src[ARRAY_SIZE - 1 - i]) {
            printf("Error in copy_with_pre_decrement at index %d\n", i);
            return;
        }
    }
    
    /* Test 3: Sum calculation with post-increment */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    int expected_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (sum != expected_sum) {
        printf("Error in sum_with_post_increment: %d != %d\n", sum, expected_sum);
        return;
    }
    
    /* Test 4: In-place reversal with dual pointers */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    reverse_with_dual_pointers(arr, 100);
    
    for (int i = 0; i < 100; i++) {
        if (arr[i] != 99 - i) {
            printf("Error in reverse_with_dual_pointers at index %d\n", i);
            return;
        }
    }
    
    /* Test 5: Character buffer fill */
    char buffer[256];
    fill_with_post_increment(buffer, 'A', 256);
    
    for (int i = 0; i < 256; i++) {
        if (buffer[i] != 'A') {
            printf("Error in fill_with_post_increment at index %d\n", i);
            return;
        }
    }
    
    printf("All tests passed! Auto-inc-dec patterns should have been triggered.\n");
}

/* Main function with different loop patterns */
int main(void) {
    printf("Testing auto-inc-dec pass patterns...\n");
    
    /* Run combined tests */
    test_combined_patterns();
    
    /* Additional simple direct test */
    {
        int a[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int b[10];
        int *src_ptr = a;
        int *dst_ptr = b;
        
        /* Very simple pattern that should generate clean RTL */
        for (int i = 0; i < 10; i++) {
            *dst_ptr++ = *src_ptr++;  /* Zero-offset memory access */
        }
        
        /* Verify */
        for (int i = 0; i < 10; i++) {
            if (a[i] != b[i]) {
                printf("Simple copy failed at index %d\n", i);
                return 1;
            }
        }
    }
    
    /* Test with different data types */
    {
        short s_src[20];
        short s_dst[20];
        short *sp_src = s_src;
        short *sp_dst = s_dst;
        
        for (int i = 0; i < 20; i++) {
            s_src[i] = i * 2;
        }
        
        /* Pointer arithmetic with different type */
        for (int i = 0; i < 20; i++) {
            *sp_dst++ = *sp_src++;  /* Zero-offset with 16-bit data */
        }
    }
    
    printf("All patterns executed successfully.\n");
    printf("Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c test_auto_inc_dec.c\n");
    printf("Check the .auto-inc-dec dump file for pass activity.\n");
    
    return 0;
}
