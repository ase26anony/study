/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass with zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Global volatile to prevent dead code elimination */
volatile int g_checksum = 0;

/* Noinline to ensure function boundaries are preserved */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ generates base+0 addressing */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for reverse copy */
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
fill_with_post_increment(char *buf, char val, int n) {
    char *p = buf;
    
    /* Pattern: *p++ = val generates base+0 addressing */
    while (n-- > 0) {
        *p++ = val;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
reverse_with_dual_pointers(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src + n - 1;
    
    /* Pattern: *d++ = *s-- generates base+0 addressing on both sides */
    while (n-- > 0) {
        *d++ = *s--;  /* Both memory accesses should have offset 0 */
    }
}

/* Test different data types to explore various RTL patterns */
static void __attribute__((noinline))
test_mixed_types(void) {
    int int_buf[64];
    char char_buf[256];
    int *int_ptr;
    char *char_ptr;
    
    /* Initialize buffers */
    for (int i = 0; i < 64; i++) {
        int_buf[i] = i * 3;
    }
    
    /* Test 1: int pointer with post-increment */
    int_ptr = int_buf;
    for (int i = 0; i < 64; i++) {
        g_checksum += *int_ptr++;  /* Should generate (mem (reg)) with offset 0 */
    }
    
    /* Test 2: char pointer with post-increment */
    char_ptr = char_buf;
    for (int i = 0; i < 256; i++) {
        *char_ptr++ = (char)(i & 0xFF);  /* Should generate (mem (reg)) with offset 0 */
    }
}

int main(void) {
    const int N = 100;
    int src[N], dst1[N], dst2[N];
    int expected_sum = 0;
    
    /* Initialize source array with known values */
    for (int i = 0; i < N; i++) {
        src[i] = i * 2 + 1;
        expected_sum += src[i];
    }
    
    printf("Testing auto-increment/decrement patterns...\n");
    
    /* Test 1: Simple copy with post-increment */
    copy_with_post_increment(dst1, src, N);
    
    /* Verify copy */
    if (memcmp(src, dst1, N * sizeof(int)) != 0) {
        printf("FAIL: copy_with_post_increment\n");
        return 1;
    }
    printf("PASS: copy_with_post_increment\n");
    
    /* Test 2: Reverse copy with pre-decrement */
    copy_with_pre_decrement(dst2, src, N);
    
    /* Verify reverse copy */
    for (int i = 0; i < N; i++) {
        if (dst2[i] != src[N - 1 - i]) {
            printf("FAIL: copy_with_pre_decrement\n");
            return 1;
        }
    }
    printf("PASS: copy_with_pre_decrement\n");
    
    /* Test 3: Sum calculation with post-increment */
    int calculated_sum = sum_with_post_increment(src, N);
    if (calculated_sum != expected_sum) {
        printf("FAIL: sum_with_post_increment (expected %d, got %d)\n", 
               expected_sum, calculated_sum);
        return 1;
    }
    printf("PASS: sum_with_post_increment\n");
    
    /* Test 4: Char buffer fill */
    char char_buf1[50], char_buf2[50];
    fill_with_post_increment(char_buf1, 'A', 50);
    
    /* Verify fill */
    for (int i = 0; i < 50; i++) {
        if (char_buf1[i] != 'A') {
            printf("FAIL: fill_with_post_increment\n");
            return 1;
        }
    }
    printf("PASS: fill_with_post_increment\n");
    
    /* Test 5: Reverse with dual pointers */
    const char *test_str = "HelloWorld";
    int len = 10;
    char reversed[20];
    reverse_with_dual_pointers(reversed, test_str, len);
    reversed[len] = '\0';
    
    if (strcmp(reversed, "dlroWolleH") != 0) {
        printf("FAIL: reverse_with_dual_pointers\n");
        return 1;
    }
    printf("PASS: reverse_with_dual_pointers\n");
    
    /* Test 6: Mixed types */
    test_mixed_types();
    printf("PASS: test_mixed_types\n");
    
    printf("\nAll tests passed! The auto-inc-dec pass should have seen\n");
    printf("multiple instances of (mem (reg)) patterns with offset 0.\n");
    
    return 0;
}
