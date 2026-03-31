/* test_auto_inc_dec.c
 * This program creates memory access patterns that should trigger
 * GCC's auto-increment/decrement optimization pass, specifically
 * the zero-offset case in find_inc().
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

/* Use noinline to prevent inlining and preserve loop structure */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset accesses */
    while (n-- > 0) {
        *dst = *src;  /* Base + 0 offset for both src and dst */
        dst++;        /* Post-increment */
        src++;        /* Post-increment */
    }
}

static void __attribute__((noinline))
reverse_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src creates zero-offset accesses */
    dst += n;  /* Point to one past the end */
    src += n;  /* Point to one past the end */
    
    while (n-- > 0) {
        --dst;  /* Pre-decrement */
        --src;  /* Pre-decrement */
        *dst = *src;  /* Base + 0 offset for both */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    /* Pattern: sum += *ptr++ creates zero-offset read */
    int sum = 0;
    const int *ptr = arr;
    
    while (n-- > 0) {
        sum += *ptr;  /* Base + 0 offset */
        ptr++;        /* Post-increment */
    }
    return sum;
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char value, int n) {
    /* Pattern: *ptr++ = value creates zero-offset write */
    char *ptr = buf;
    
    while (n-- > 0) {
        *ptr = value;  /* Base + 0 offset */
        ptr++;         /* Post-increment */
        value ^= 0x55; /* Modify value to prevent dead code elimination */
    }
}

/* Another variant with pointer arithmetic in the loop */
static void __attribute__((noinline))
copy_alternate_pattern(int *dst, const int *src, int n) {
    /* More explicit pointer arithmetic that should still generate
       zero-offset memory accesses */
    int *d = dst;
    const int *s = src;
    int i;
    
    for (i = 0; i < n; i++) {
        *d = *s;  /* Both are base + 0 */
        d = d + 1;
        s = s + 1;
    }
}

/* Test function that combines multiple patterns */
static int test_auto_inc_dec(void) {
    int src[ARRAY_SIZE];
    int dst1[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    int i, result = 0;
    
    /* Initialize source array with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Test 1: Post-increment copy */
    memset(dst1, 0, sizeof(dst1));
    copy_with_post_increment(dst1, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != src[i]) {
            printf("Error: Post-increment copy failed at index %d\n", i);
            result = 1;
            break;
        }
    }
    
    /* Test 2: Pre-decrement reverse */
    memset(dst2, 0, sizeof(dst2));
    reverse_with_pre_decrement(dst2, src, ARRAY_SIZE);
    
    /* Verify reverse */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst2[i] != src[ARRAY_SIZE - 1 - i]) {
            printf("Error: Pre-decrement reverse failed at index %d\n", i);
            result = 1;
            break;
        }
    }
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    int expected_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (sum != expected_sum) {
        printf("Error: Sum calculation failed: got %d, expected %d\n", 
               sum, expected_sum);
        result = 1;
    }
    
    /* Test 4: Fill with post-increment */
    char buffer[ARRAY_SIZE];
    fill_with_post_increment(buffer, 'A', ARRAY_SIZE);
    
    /* Test 5: Alternate pattern */
    int dst3[ARRAY_SIZE];
    copy_alternate_pattern(dst3, src, ARRAY_SIZE);
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst3[i] != src[i]) {
            printf("Error: Alternate pattern copy failed at index %d\n", i);
            result = 1;
            break;
        }
    }
    
    return result;
}

int main(void) {
    printf("Testing auto-inc-dec patterns...\n");
    
    int result = test_auto_inc_dec();
    
    if (result == 0) {
        printf("All tests passed successfully.\n");
        printf("Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c test_auto_inc_dec.c\n");
        printf("Then check the .dump file for auto-inc-dec pass activity.\n");
    } else {
        printf("Some tests failed.\n");
    }
    
    return result;
}
