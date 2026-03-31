/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 * Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c test_auto_inc_dec.c
 */

#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Prevent inlining to ensure loops remain distinct for analysis */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates base+0 addressing */
    while (n--) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src creates base+0 addressing */
    while (n--) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ creates base+0 addressing */
    while (n--) {
        sum += *p++;  /* Memory access with pointer increment */
    }
    return sum;
}

static void __attribute__((noinline)) 
reverse_with_dual_pointers(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Pattern: *start++ = *end-- creates two base+0 accesses */
    while (start < end) {
        int temp = *end;      /* (mem (reg)) with offset 0 */
        *end-- = *start;      /* (mem (reg)) with offset 0 */
        *start++ = temp;      /* (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char value, int n) {
    /* Simple byte fill with post-increment */
    while (n--) {
        *buf++ = value;  /* Base+0 store operation */
    }
}

int main(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    char buffer[ARRAY_SIZE];
    
    /* Initialize source array with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Test 1: Post-increment copy */
    memset(dst, 0, sizeof(dst));
    copy_with_post_increment(dst, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Post-increment copy mismatch at index %d\n", i);
            return 1;
        }
    }
    
    /* Test 2: Pre-decrement copy */
    memset(dst, 0, sizeof(dst));
    copy_with_pre_decrement(dst, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Pre-decrement copy mismatch at index %d\n", i);
            return 1;
        }
    }
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    int expected_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (sum != expected_sum) {
        printf("FAIL: Sum mismatch: got %d, expected %d\n", sum, expected_sum);
        return 1;
    }
    
    /* Test 4: Reverse with dual pointers */
    memcpy(dst, src, sizeof(src));
    reverse_with_dual_pointers(dst, ARRAY_SIZE);
    
    /* Verify reverse */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[ARRAY_SIZE - 1 - i]) {
            printf("FAIL: Reverse mismatch at index %d\n", i);
            return 1;
        }
    }
    
    /* Test 5: Byte fill with post-increment */
    memset(buffer, 0, sizeof(buffer));
    fill_with_post_increment(buffer, 0xAB, ARRAY_SIZE);
    
    /* Verify fill */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (buffer[i] != 0xAB) {
            printf("FAIL: Fill mismatch at index %d\n", i);
            return 1;
        }
    }
    
    printf("SUCCESS: All auto-inc-dec patterns executed correctly\n");
    return 0;
}
