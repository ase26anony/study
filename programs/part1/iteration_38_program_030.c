/* test_auto_inc_dec.c
 * Program designed to trigger GCC's auto-increment/decrement optimization
 * Specifically targets the zero-offset memory access pattern in find_inc()
 */

#include <stdio.h>
#include <string.h>

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
    /* Move pointers to end for reverse copy */
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
fill_with_post_increment(char *buf, char val, int n) {
    char *p = buf;
    
    /* Pattern: *p++ = val generates base+0 addressing */
    while (n-- > 0) {
        *p++ = val;  /* Should create (mem (reg)) with offset 0 */
    }
}

/* Test different data types and access patterns */
static void __attribute__((noinline))
mixed_operations(short *dst, const short *src, int n) {
    short *d = dst;
    const short *s = src;
    
    /* Mixed read/write with post-increment */
    while (n-- > 0) {
        *d++ = *s++ + 1;  /* Both reads and writes with base+0 */
    }
}

int main(void) {
    const int N = 256;
    int src[N], dst[N];
    char buffer[N];
    short short_src[N], short_dst[N];
    
    /* Initialize source arrays with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i * 2 + 1;
        short_src[i] = i % 100;
    }
    
    /* Test 1: Post-increment copy */
    memset(dst, 0, sizeof(dst));
    copy_with_post_increment(dst, src, N);
    
    /* Verify copy */
    int ok1 = 1;
    for (int i = 0; i < N; i++) {
        if (dst[i] != src[i]) {
            ok1 = 0;
            break;
        }
    }
    
    /* Test 2: Pre-decrement reverse copy */
    memset(dst, 0, sizeof(dst));
    copy_with_pre_decrement(dst, src, N);
    
    /* Verify reverse copy */
    int ok2 = 1;
    for (int i = 0; i < N; i++) {
        if (dst[i] != src[N - 1 - i]) {
            ok2 = 0;
            break;
        }
    }
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, N);
    int expected_sum = 0;
    for (int i = 0; i < N; i++) {
        expected_sum += src[i];
    }
    int ok3 = (sum == expected_sum);
    
    /* Test 4: Char fill with post-increment */
    memset(buffer, 0, sizeof(buffer));
    fill_with_post_increment(buffer, 'A', N);
    
    int ok4 = 1;
    for (int i = 0; i < N; i++) {
        if (buffer[i] != 'A') {
            ok4 = 0;
            break;
        }
    }
    
    /* Test 5: Mixed operations with shorts */
    memset(short_dst, 0, sizeof(short_dst));
    mixed_operations(short_dst, short_src, N);
    
    int ok5 = 1;
    for (int i = 0; i < N; i++) {
        if (short_dst[i] != short_src[i] + 1) {
            ok5 = 0;
            break;
        }
    }
    
    /* Report results */
    printf("Test 1 (post-increment copy): %s\n", ok1 ? "PASS" : "FAIL");
    printf("Test 2 (pre-decrement reverse): %s\n", ok2 ? "PASS" : "FAIL");
    printf("Test 3 (sum with increment): %s\n", ok3 ? "PASS" : "FAIL");
    printf("Test 4 (char fill): %s\n", ok4 ? "PASS" : "FAIL");
    printf("Test 5 (mixed short ops): %s\n", ok5 ? "PASS" : "FAIL");
    
    return !(ok1 && ok2 && ok3 && ok4 && ok5);
}
