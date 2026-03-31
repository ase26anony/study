/* test_auto_inc_dec.c
 * Program designed to trigger GCC's auto-increment/decrement optimization
 * Specifically targets the zero-offset memory access pattern in find_inc()
 */

#include <stdio.h>
#include <string.h>

/* Prevent inlining to ensure loops remain intact for the pass */
static void __attribute__((noinline)) 
copy_forward(int *dst, const int *src, int n) {
    /* Pattern 1: Post-increment with zero offset at memory access */
    while (n--) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_backward(int *dst, const int *src, int n) {
    /* Pattern 2: Pre-decrement with zero offset at memory access */
    dst += n;
    src += n;
    while (n--) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_array(const int *arr, int n) {
    /* Pattern 3: Post-increment in read operation */
    int sum = 0;
    while (n--) {
        sum += *arr++;  /* Memory read with zero offset */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_array(int *arr, int n) {
    /* Pattern 4: Both increment and decrement in same loop */
    int *start = arr;
    int *end = arr + n - 1;
    
    while (start < end) {
        /* Two memory accesses with zero offset each */
        int temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

static void __attribute__((noinline))
fill_pattern(char *buf, int size) {
    /* Pattern 5: Byte-wise operations with char pointer */
    char pattern = 'A';
    while (size--) {
        *buf++ = pattern++;  /* Byte access with zero offset */
    }
}

/* Verification functions */
static int verify_copy(const int *a, const int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

static int verify_reverse(const int *orig, const int *rev, int n) {
    for (int i = 0; i < n; i++) {
        if (orig[i] != rev[n - 1 - i]) return 0;
    }
    return 1;
}

int main(void) {
    const int N = 256;
    int src[N], dst[N], src_copy[N];
    char buffer[100];
    
    /* Initialize source array with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i * 3 + 1;
        src_copy[i] = src[i];
    }
    
    printf("Testing auto-increment/decrement patterns...\n");
    
    /* Test 1: Forward copy with post-increment */
    memset(dst, 0, sizeof(dst));
    copy_forward(dst, src, N);
    if (verify_copy(src, dst, N)) {
        printf("✓ Forward copy passed\n");
    } else {
        printf("✗ Forward copy failed\n");
        return 1;
    }
    
    /* Test 2: Backward copy with pre-decrement */
    memset(dst, 0, sizeof(dst));
    copy_backward(dst, src, N);
    if (verify_copy(src, dst, N)) {
        printf("✓ Backward copy passed\n");
    } else {
        printf("✗ Backward copy failed\n");
        return 1;
    }
    
    /* Test 3: Array sum with post-increment */
    int sum = sum_array(src, N);
    int expected_sum = 0;
    for (int i = 0; i < N; i++) {
        expected_sum += src[i];
    }
    if (sum == expected_sum) {
        printf("✓ Array sum passed: %d\n", sum);
    } else {
        printf("✗ Array sum failed: got %d, expected %d\n", sum, expected_sum);
        return 1;
    }
    
    /* Test 4: Array reversal with both increment and decrement */
    memcpy(dst, src, sizeof(src));
    reverse_array(dst, N);
    if (verify_reverse(src, dst, N)) {
        printf("✓ Array reversal passed\n");
    } else {
        printf("✗ Array reversal failed\n");
        return 1;
    }
    
    /* Test 5: Byte-wise fill with char pointer */
    memset(buffer, 0, sizeof(buffer));
    fill_pattern(buffer, 50);
    
    int valid = 1;
    for (int i = 0; i < 50; i++) {
        if (buffer[i] != 'A' + i) {
            valid = 0;
            break;
        }
    }
    
    if (valid) {
        printf("✓ Byte fill passed\n");
    } else {
        printf("✗ Byte fill failed\n");
        return 1;
    }
    
    printf("\nAll tests passed! The auto-inc-dec pass should have processed the patterns.\n");
    
    /* Use result to prevent dead code elimination */
    volatile int checksum = sum + buffer[0] + dst[0];
    (void)checksum;  /* Prevent unused variable warning */
    
    return 0;
}
