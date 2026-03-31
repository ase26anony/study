/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>

#define SIZE 256

/* Use noinline to prevent inlining and preserve loop structure */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ generates base+0 addressing */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src generates base+0 addressing */
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ generates base+0 addressing */
    while (n-- > 0) {
        sum += *p++;  /* Should generate (mem (reg)) with offset 0 */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Pattern: *start++ = *end-- generates base+0 addressing */
    while (start < end) {
        int temp = *end;
        *end-- = *start;
        *start++ = temp;  /* Both accesses should be base+0 */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char value, int n) {
    /* Pattern: *buf++ = value generates base+0 addressing */
    while (n-- > 0) {
        *buf++ = value;  /* Should generate (mem (reg)) with offset 0 */
    }
}

/* Main function to exercise all patterns */
int main() {
    int src[SIZE];
    int dst[SIZE];
    char buffer[SIZE];
    int expected_sum = 0;
    
    /* Initialize source array with known values */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 2;
        expected_sum += i * 2;
        dst[i] = 0;
    }
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Test 1: Post-increment copy */
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    if (memcmp(src, dst, SIZE * sizeof(int)) != 0) {
        printf("FAIL: Post-increment copy mismatch\n");
        return 1;
    }
    printf("PASS: Post-increment copy\n");
    
    /* Test 2: Pre-decrement copy */
    memset(dst, 0, SIZE * sizeof(int));
    copy_with_pre_decrement(dst, src, SIZE);
    
    if (memcmp(src, dst, SIZE * sizeof(int)) != 0) {
        printf("FAIL: Pre-decrement copy mismatch\n");
        return 1;
    }
    printf("PASS: Pre-decrement copy\n");
    
    /* Test 3: Post-increment sum */
    int calculated_sum = sum_with_post_increment(src, SIZE);
    
    if (calculated_sum != expected_sum) {
        printf("FAIL: Sum calculation error: expected %d, got %d\n", 
               expected_sum, calculated_sum);
        return 1;
    }
    printf("PASS: Post-increment sum = %d\n", calculated_sum);
    
    /* Test 4: Reverse with dual pointers */
    memcpy(dst, src, SIZE * sizeof(int));
    reverse_with_dual_pointers(dst, SIZE);
    
    /* Verify reverse */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[SIZE - 1 - i]) {
            printf("FAIL: Reverse mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Reverse with dual pointers\n");
    
    /* Test 5: Char buffer fill */
    fill_with_post_increment(buffer, 'A', SIZE);
    
    /* Verify fill */
    for (int i = 0; i < SIZE; i++) {
        if (buffer[i] != 'A') {
            printf("FAIL: Buffer fill mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Char buffer fill\n");
    
    printf("\nAll tests passed! Auto-inc-dec patterns exercised.\n");
    return 0;
}
