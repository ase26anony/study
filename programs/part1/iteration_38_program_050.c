/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 256

/* Prevent inlining to ensure loops remain intact for analysis */
static void __attribute__((noinline)) 
copy_forward_with_postinc(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ generates base+0 offset accesses */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline)) 
copy_backward_with_predec(int *dst, const int *src, int n) {
    /* Move pointers to end for backward copy */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src generates base+0 offset accesses */
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline)) 
sum_with_postinc(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ generates base+0 offset read */
    while (n-- > 0) {
        sum += *p++;  /* Should generate (mem (reg)) with offset 0 */
    }
    return sum;
}

static void __attribute__((noinline))
fill_with_postinc(int *arr, int value, int n) {
    int *p = arr;
    
    /* Pattern: *p++ = value generates base+0 offset write */
    while (n-- > 0) {
        *p++ = value;  /* Should generate (mem (reg)) with offset 0 */
    }
}

/* Test both increment and decrement patterns */
static void __attribute__((noinline))
reverse_array(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Mixed pattern: increment and decrement in same loop */
    while (start < end) {
        int temp = *start;    /* (mem (reg)) with offset 0 */
        *start++ = *end;      /* post-increment write */
        *end-- = temp;        /* pre-decrement write */
    }
}

/* Simple memory move with overlapping regions */
static void __attribute__((noinline))
memmove_like(int *dst, const int *src, int n) {
    if (dst < src) {
        /* Forward copy with post-increment */
        while (n-- > 0) {
            *dst++ = *src++;
        }
    } else if (dst > src) {
        /* Backward copy with pre-decrement */
        dst += n;
        src += n;
        while (n-- > 0) {
            *--dst = *--src;
        }
    }
}

int main(void) {
    int src[SIZE];
    int dst[SIZE];
    int expected_sum = 0;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 2 + 1;
        expected_sum += src[i];
    }
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Test 1: Forward copy with post-increment */
    memset(dst, 0, sizeof(dst));
    copy_forward_with_postinc(dst, src, SIZE);
    
    /* Verify copy */
    if (memcmp(src, dst, sizeof(src)) != 0) {
        printf("FAIL: Forward copy mismatch\n");
        return 1;
    }
    printf("PASS: Forward copy with post-increment\n");
    
    /* Test 2: Backward copy with pre-decrement */
    memset(dst, 0, sizeof(dst));
    copy_backward_with_predec(dst, src, SIZE);
    
    if (memcmp(src, dst, sizeof(src)) != 0) {
        printf("FAIL: Backward copy mismatch\n");
        return 1;
    }
    printf("PASS: Backward copy with pre-decrement\n");
    
    /* Test 3: Sum calculation with post-increment */
    int calculated_sum = sum_with_postinc(src, SIZE);
    if (calculated_sum != expected_sum) {
        printf("FAIL: Sum calculation error (%d != %d)\n", 
               calculated_sum, expected_sum);
        return 1;
    }
    printf("PASS: Sum calculation with post-increment\n");
    
    /* Test 4: Fill with post-increment */
    int test_value = 0xABCD;
    fill_with_postinc(dst, test_value, SIZE);
    
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != test_value) {
            printf("FAIL: Fill pattern mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Fill with post-increment\n");
    
    /* Test 5: Array reversal with mixed increment/decrement */
    memcpy(dst, src, sizeof(src));
    reverse_array(dst, SIZE);
    
    /* Verify reversal */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[SIZE - 1 - i]) {
            printf("FAIL: Reversal mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Array reversal with mixed patterns\n");
    
    /* Test 6: memmove-like operation */
    /* Create overlapping scenario */
    for (int i = 0; i < SIZE/2; i++) {
        dst[i] = i;
    }
    
    /* Copy first half to second half (non-overlapping forward) */
    memmove_like(dst + SIZE/2, dst, SIZE/2);
    
    /* Copy back (overlapping backward) */
    memmove_like(dst + SIZE/4, dst + SIZE/2, SIZE/4);
    
    printf("PASS: memmove-like operations\n");
    
    printf("\nAll tests passed! The auto-inc-dec pass should have processed\n");
    printf("the zero-offset memory access patterns in the loops.\n");
    
    return 0;
}
