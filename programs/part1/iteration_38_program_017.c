/* test_auto_inc_dec.c
 * Program designed to trigger GCC's auto-increment/decrement optimization
 * Specifically targets the zero-offset memory access pattern in find_inc()
 */

#include <stdio.h>
#include <string.h>

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Function with post-increment pattern - should trigger auto-inc */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    while (n-- > 0) {
        /* Critical pattern: memory access with pointer, then increment
         * Should generate RTL: (mem (reg)) with offset 0 */
        *dst++ = *src++;
    }
}

/* Function with pre-decrement pattern - should trigger auto-dec */
static void __attribute__((noinline)) 
reverse_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for reverse copy */
    dst += n;
    src += n;
    
    while (n-- > 0) {
        /* Critical pattern: decrement pointer, then memory access
         * Should generate RTL: (mem (reg)) with offset 0 */
        *--dst = *--src;
    }
}

/* Function with mixed operations to test pattern recognition */
static void __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    while (n-- > 0) {
        /* Memory read with post-increment */
        sum += *p++;
    }
    
    /* Store result to prevent optimization */
    global_checksum = sum;
}

/* Function with char pointers - different data type */
static void __attribute__((noinline))
copy_chars_with_inc(char *dst, const char *src, int n) {
    while (n-- > 0) {
        /* Char version of the pattern */
        *dst++ = *src++;
    }
}

/* Test case 1: Simple array copy with post-increment */
void test_post_increment_copy(void) {
    const int N = 256;
    int src[N], dst[N];
    
    /* Initialize source with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Clear destination */
    memset(dst, 0, sizeof(dst));
    
    /* Call function with critical pattern */
    copy_with_post_increment(dst, src, N);
    
    /* Verify copy */
    for (int i = 0; i < N; i++) {
        if (dst[i] != src[i]) {
            printf("Error in post-increment copy at index %d\n", i);
            return;
        }
    }
    printf("Post-increment copy test passed\n");
}

/* Test case 2: Reverse copy with pre-decrement */
void test_pre_decrement_reverse(void) {
    const int N = 128;
    int src[N], dst[N];
    
    /* Initialize source */
    for (int i = 0; i < N; i++) {
        src[i] = i * 2;
    }
    
    /* Clear destination */
    memset(dst, 0, sizeof(dst));
    
    /* Call function with pre-decrement pattern */
    reverse_with_pre_decrement(dst, src, N);
    
    /* Verify reverse copy */
    for (int i = 0; i < N; i++) {
        if (dst[i] != src[N - 1 - i]) {
            printf("Error in pre-decrement reverse at index %d\n", i);
            return;
        }
    }
    printf("Pre-decrement reverse test passed\n");
}

/* Test case 3: Sum calculation with post-increment */
void test_sum_with_increment(void) {
    const int N = 512;
    int arr[N];
    int expected_sum = 0;
    
    /* Initialize array and calculate expected sum */
    for (int i = 0; i < N; i++) {
        arr[i] = i + 1;
        expected_sum += i + 1;
    }
    
    /* Call function that uses post-increment in loop */
    sum_with_post_increment(arr, N);
    
    /* Verify result */
    if (global_checksum != expected_sum) {
        printf("Error in sum calculation: got %d, expected %d\n", 
               global_checksum, expected_sum);
        return;
    }
    printf("Sum with increment test passed\n");
}

/* Test case 4: Character buffer copy */
void test_char_copy(void) {
    const int N = 1024;
    char src[N], dst[N];
    
    /* Initialize source string */
    for (int i = 0; i < N - 1; i++) {
        src[i] = 'A' + (i % 26);
    }
    src[N - 1] = '\0';
    
    /* Clear destination */
    memset(dst, 0, sizeof(dst));
    
    /* Call char copy function */
    copy_chars_with_inc(dst, src, N);
    
    /* Verify copy */
    if (memcmp(src, dst, N) != 0) {
        printf("Error in char copy\n");
        return;
    }
    printf("Char copy test passed\n");
}

/* Test case 5: Nested loops with pointer arithmetic */
static void __attribute__((noinline))
matrix_copy(int *dst, const int *src, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int *d = dst + i * cols;
        const int *s = src + i * cols;
        
        /* Inner loop with simple pointer increment */
        for (int j = 0; j < cols; j++) {
            *d++ = *s++;
        }
    }
}

void test_matrix_copy(void) {
    const int ROWS = 32;
    const int COLS = 32;
    int src[ROWS * COLS];
    int dst[ROWS * COLS];
    
    /* Initialize matrix */
    for (int i = 0; i < ROWS * COLS; i++) {
        src[i] = i * 7;
    }
    
    /* Clear destination */
    memset(dst, 0, sizeof(dst));
    
    /* Copy matrix */
    matrix_copy(dst, src, ROWS, COLS);
    
    /* Verify */
    for (int i = 0; i < ROWS * COLS; i++) {
        if (dst[i] != src[i]) {
            printf("Error in matrix copy at index %d\n", i);
            return;
        }
    }
    printf("Matrix copy test passed\n");
}

int main(void) {
    printf("Testing auto-increment/decrement patterns...\n\n");
    
    test_post_increment_copy();
    test_pre_decrement_reverse();
    test_sum_with_increment();
    test_char_copy();
    test_matrix_copy();
    
    printf("\nAll tests completed.\n");
    
    /* Return success only if global_checksum is non-zero 
     * (prevents dead code elimination of sum function) */
    return (global_checksum != 0) ? 0 : 1;
}
