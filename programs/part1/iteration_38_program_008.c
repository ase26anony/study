/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 256
#define CHECKSUM_SEED 0x55AA

/* Prevent inlining to ensure loops remain intact for analysis */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ generates base+0 addressing */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should create (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src generates base+0 addressing */
    while (n-- > 0) {
        *--dst = *--src;  /* Should create (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ generates base+0 addressing */
    while (n-- > 0) {
        sum += *p++;  /* Should create (mem (reg)) with offset 0 */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_decrement(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Pattern: *start++ = *end-- generates base+0 addressing */
    while (start < end) {
        int temp = *end;    /* (mem (reg)) with offset 0 */
        *end-- = *start;    /* (mem (reg)) with offset 0 */
        *start++ = temp;    /* (mem (reg)) with offset 0 */
    }
}

/* Another variant with char pointers (often generates simpler addressing) */
static void __attribute__((noinline))
memfill_with_increment(char *dst, char val, int n) {
    char *p = dst;
    
    /* Pattern: *p++ = val generates base+0 addressing */
    while (n-- > 0) {
        *p++ = val;  /* Should create (mem (reg)) with offset 0 */
    }
}

/* Mixed pattern test */
static int __attribute__((noinline))
process_buffer(int *buf, int n) {
    int *read_ptr = buf;
    int *write_ptr = buf;
    int count = 0;
    
    /* Pattern: read with post-inc, conditional write with post-inc */
    while (n-- > 0) {
        int val = *read_ptr++;  /* (mem (reg)) with offset 0 */
        if (val > 0) {
            *write_ptr++ = val;  /* (mem (reg)) with offset 0 */
            count++;
        }
    }
    return count;
}

int main(void) {
    int src[SIZE];
    int dst[SIZE];
    char char_buf[SIZE];
    int checksum = 0;
    
    /* Initialize source with pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Test 1: Simple post-increment copy */
    memset(dst, 0, sizeof(dst));
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Post-increment copy mismatch at %d\n", i);
            return 1;
        }
    }
    printf("PASS: Post-increment copy\n");
    
    /* Test 2: Pre-decrement copy */
    memset(dst, 0, sizeof(dst));
    copy_with_pre_decrement(dst, src, SIZE);
    
    /* Verify copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Pre-decrement copy mismatch at %d\n", i);
            return 1;
        }
    }
    printf("PASS: Pre-decrement copy\n");
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, SIZE);
    int expected_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        expected_sum += src[i];
    }
    if (sum != expected_sum) {
        printf("FAIL: Sum mismatch: got %d, expected %d\n", sum, expected_sum);
        return 1;
    }
    printf("PASS: Sum with post-increment: %d\n", sum);
    
    /* Test 4: Reverse with dual pointers */
    memcpy(dst, src, sizeof(src));
    reverse_with_dual_decrement(dst, SIZE);
    
    /* Verify reverse */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[SIZE - 1 - i]) {
            printf("FAIL: Reverse mismatch at %d\n", i);
            return 1;
        }
    }
    printf("PASS: Reverse with dual decrement\n");
    
    /* Test 5: Char buffer fill */
    memset(char_buf, 0, sizeof(char_buf));
    memfill_with_increment(char_buf, 0xAB, SIZE);
    
    /* Verify fill */
    for (int i = 0; i < SIZE; i++) {
        if (char_buf[i] != (char)0xAB) {
            printf("FAIL: Char fill mismatch at %d\n", i);
            return 1;
        }
    }
    printf("PASS: Char fill with increment\n");
    
    /* Test 6: Mixed read/write pattern */
    int count = process_buffer(src, SIZE);
    printf("PASS: Processed %d positive elements\n", count);
    
    printf("\nAll tests passed! The auto-inc-dec pass should have seen\n");
    printf("multiple (mem (reg)) patterns with offset 0.\n");
    
    return 0;
}
