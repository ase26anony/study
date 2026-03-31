/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256

/* Use noinline to prevent inlining and preserve loop structure */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset accesses */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
reverse_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src creates zero-offset accesses */
    dst += n;  /* Point to one past the end */
    src += n;  /* Point to one past the end */
    
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    /* Pattern: sum += *ptr++ creates zero-offset read */
    int sum = 0;
    const int *ptr = arr;
    
    while (n-- > 0) {
        sum += *ptr++;  /* Should generate (mem (reg)) with offset 0 */
    }
    return sum;
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char value, int n) {
    /* Pattern: *ptr++ = value creates zero-offset write */
    char *ptr = buf;
    
    while (n-- > 0) {
        *ptr++ = value;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_words_with_mixed_increment(uint32_t *dst, const uint32_t *src, int n) {
    /* Mixed pattern with different data types */
    uint32_t *d = dst;
    const uint32_t *s = src;
    
    while (n-- > 0) {
        *d++ = *s++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

/* Test function that combines multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    int reversed[ARRAY_SIZE];
    char buffer[ARRAY_SIZE];
    uint32_t words_src[ARRAY_SIZE/4];
    uint32_t words_dst[ARRAY_SIZE/4];
    
    /* Initialize source arrays with predictable values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        words_src[i] = i * 5 + 2;
    }
    
    /* Test 1: Post-increment copy */
    copy_with_post_increment(dst, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            return 1;  /* Failure */
        }
    }
    
    /* Test 2: Pre-decrement reverse */
    reverse_with_pre_decrement(reversed, src, ARRAY_SIZE);
    
    /* Verify reverse */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (reversed[i] != src[ARRAY_SIZE - 1 - i]) {
            return 2;  /* Failure */
        }
    }
    
    /* Test 3: Post-increment sum */
    int expected_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    
    int actual_sum = sum_with_post_increment(src, ARRAY_SIZE);
    if (actual_sum != expected_sum) {
        return 3;  /* Failure */
    }
    
    /* Test 4: Post-increment fill */
    fill_with_post_increment(buffer, 'A', ARRAY_SIZE);
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (buffer[i] != 'A') {
            return 4;  /* Failure */
        }
    }
    
    /* Test 5: Word copy with post-increment */
    copy_words_with_mixed_increment(words_dst, words_src, ARRAY_SIZE/4);
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        if (words_dst[i] != words_src[i]) {
            return 5;  /* Failure */
        }
    }
    
    return 0;  /* All tests passed */
}

/* Additional test with nested loops to explore different patterns */
static void __attribute__((noinline))
matrix_copy(int dst[][8], const int src[][8], int rows) {
    /* Process each row with pointer arithmetic */
    for (int i = 0; i < rows; i++) {
        int *d = dst[i];
        const int *s = src[i];
        int cols = 8;
        
        /* Inner loop with post-increment */
        while (cols-- > 0) {
            *d++ = *s++;  /* Should generate (mem (reg)) with offset 0 */
        }
    }
}

int main(void) {
    printf("Testing auto-inc-dec patterns...\n");
    
    int result = test_combined_patterns();
    
    if (result == 0) {
        printf("All tests passed successfully!\n");
        
        /* Additional test with matrix */
        int src_mat[4][8];
        int dst_mat[4][8];
        
        /* Initialize matrix */
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 8; j++) {
                src_mat[i][j] = i * 8 + j;
            }
        }
        
        matrix_copy(dst_mat, src_mat, 4);
        
        /* Verify matrix copy */
        int ok = 1;
        for (int i = 0; i < 4 && ok; i++) {
            for (int j = 0; j < 8; j++) {
                if (dst_mat[i][j] != src_mat[i][j]) {
                    ok = 0;
                    break;
                }
            }
        }
        
        if (ok) {
            printf("Matrix copy test also passed!\n");
        } else {
            printf("Matrix copy test failed.\n");
        }
    } else {
        printf("Test %d failed!\n", result);
    }
    
    return result;
}
