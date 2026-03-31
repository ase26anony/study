/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 * Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c test_auto_inc_dec.c
 */

#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Use noinline to prevent inlining and preserve loop structure */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates base+0 offset memory accesses */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
reverse_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src creates base+0 offset memory accesses */
    dst += n;  /* Point to one past the end */
    src += n;  /* Point to one past the end */
    
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    /* Pattern: sum += *arr++ creates base+0 offset memory read */
    int sum = 0;
    while (n-- > 0) {
        sum += *arr++;  /* Should generate (mem (reg)) with offset 0 */
    }
    return sum;
}

static void __attribute__((noinline))
fill_with_post_increment(int *arr, int value, int n) {
    /* Pattern: *arr++ = value creates base+0 offset memory write */
    while (n-- > 0) {
        *arr++ = value;  /* Should generate (mem (reg)) with offset 0 */
    }
}

/* Test function with multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst1[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    int i;
    
    /* Initialize source array with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Clear destination arrays */
    memset(dst1, 0, sizeof(dst1));
    memset(dst2, 0, sizeof(dst2));
    
    /* Test 1: Post-increment copy */
    copy_with_post_increment(dst1, src, ARRAY_SIZE);
    
    /* Test 2: Pre-decrement reverse copy */
    reverse_with_pre_decrement(dst2, src, ARRAY_SIZE);
    
    /* Test 3: Sum calculation with post-increment */
    int sum1 = sum_with_post_increment(src, ARRAY_SIZE);
    
    /* Test 4: Fill with post-increment */
    int dst3[ARRAY_SIZE];
    fill_with_post_increment(dst3, 42, ARRAY_SIZE);
    int sum2 = sum_with_post_increment(dst3, ARRAY_SIZE);
    
    /* Verify results to prevent dead code elimination */
    int errors = 0;
    
    /* Check forward copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != src[i]) {
            errors++;
            break;
        }
    }
    
    /* Check reverse copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst2[i] != src[ARRAY_SIZE - 1 - i]) {
            errors++;
            break;
        }
    }
    
    /* Check sum calculation */
    int expected_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    if (sum1 != expected_sum) {
        errors++;
    }
    
    /* Check fill operation */
    if (sum2 != 42 * ARRAY_SIZE) {
        errors++;
    }
    
    return errors;
}

/* Alternative: Simple pointer arithmetic in loop */
static void __attribute__((noinline))
pointer_arithmetic_example(char *dst, const char *src, int n) {
    /* Direct pointer arithmetic - very likely to generate base+0 pattern */
    char *d = dst;
    const char *s = src;
    
    while (n--) {
        *d = *s;        /* Base+0 offset access */
        d++;            /* Increment after use */
        s++;            /* Increment after use */
    }
}

/* Test with char type for different access size */
static int __attribute__((noinline))
test_char_pointers(void) {
    char src[256];
    char dst[256];
    int i;
    
    for (i = 0; i < 256; i++) {
        src[i] = (char)i;
    }
    
    pointer_arithmetic_example(dst, src, 256);
    
    /* Verify */
    for (i = 0; i < 256; i++) {
        if (dst[i] != src[i]) {
            return 1;
        }
    }
    return 0;
}

int main(void) {
    int errors = 0;
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Test integer patterns */
    errors += test_combined_patterns();
    
    /* Test char patterns */
    errors += test_char_pointers();
    
    if (errors == 0) {
        printf("All tests passed successfully.\n");
        printf("Check auto-inc-dec pass output with: gcc -O2 -fdump-rtl-auto-inc-dec -c test_auto_inc_dec.c\n");
    } else {
        printf("Found %d errors\n", errors);
    }
    
    return errors;
}
