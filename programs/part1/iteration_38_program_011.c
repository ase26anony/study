/* test_auto_inc_dec.c
 * Program designed to trigger GCC's auto-inc-dec pass
 * Specifically targets the zero-offset memory access pattern
 * in find_inc() function (lines 1352-1358)
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256

/* Prevent inlining to ensure loops remain distinct */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset accesses */
    while (n-- > 0) {
        *dst = *src;  /* Base + 0 offset pattern */
        dst++;        /* Post-increment */
        src++;        /* Post-increment */
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src creates zero-offset accesses */
    while (n-- > 0) {
        dst--;        /* Pre-decrement */
        src--;        /* Pre-decrement */
        *dst = *src;  /* Base + 0 offset pattern */
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ creates zero-offset read */
    while (n-- > 0) {
        sum += *p;  /* Base + 0 offset pattern */
        p++;        /* Post-increment */
    }
    return sum;
}

static void __attribute__((noinline)) 
reverse_with_pointers(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src + n - 1;
    
    /* Pattern: *d++ = *s-- mixes increment and decrement */
    while (n-- > 0) {
        *d = *s;  /* Base + 0 offset pattern */
        d++;      /* Post-increment */
        s--;      /* Post-decrement */
    }
}

static void __attribute__((noinline))
fill_with_pointer_arithmetic(int *arr, int n, int value) {
    int *p = arr;
    int *end = arr + n;
    
    /* Pattern: *p = value; p += stride (stride=1) */
    while (p < end) {
        *p = value;  /* Base + 0 offset pattern */
        p += 1;      /* Simple increment */
    }
}

/* Test function that combines multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst1[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    char src_str[ARRAY_SIZE];
    char dst_str[ARRAY_SIZE];
    int i, checksum = 0;
    
    /* Initialize source arrays */
    for (i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
        src_str[i] = 'A' + (i % 26);
    }
    
    /* Test 1: Post-increment copy */
    memset(dst1, 0, sizeof(dst1));
    copy_with_post_increment(dst1, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != src[i]) {
            printf("Error: Post-increment copy failed at index %d\n", i);
            return -1;
        }
    }
    checksum += dst1[ARRAY_SIZE/2];
    
    /* Test 2: Pre-decrement copy */
    memset(dst2, 0, sizeof(dst2));
    copy_with_pre_decrement(dst2, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst2[i] != src[i]) {
            printf("Error: Pre-decrement copy failed at index %d\n", i);
            return -1;
        }
    }
    checksum += dst2[ARRAY_SIZE/2];
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    checksum += sum;
    
    /* Test 4: Reverse with mixed pointer ops */
    memset(dst_str, 0, sizeof(dst_str));
    reverse_with_pointers(dst_str, src_str, ARRAY_SIZE);
    
    /* Verify reverse */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst_str[i] != src_str[ARRAY_SIZE - 1 - i]) {
            printf("Error: Reverse failed at index %d\n", i);
            return -1;
        }
    }
    checksum += dst_str[ARRAY_SIZE/2];
    
    /* Test 5: Fill with pointer arithmetic */
    fill_with_pointer_arithmetic(dst1, ARRAY_SIZE, 0x55AA55AA);
    checksum += dst1[ARRAY_SIZE/4];
    
    return checksum;
}

int main(void) {
    int result = test_combined_patterns();
    
    if (result >= 0) {
        printf("All auto-inc-dec patterns executed successfully\n");
        printf("Checksum: %d\n", result);
        return 0;
    } else {
        printf("Test failed\n");
        return 1;
    }
}
