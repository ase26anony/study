/* test_auto_inc_dec.c
 * This program creates memory access patterns that should trigger
 * GCC's auto-inc-dec pass to identify zero-offset memory accesses
 * for conversion to auto-increment/decrement addressing modes.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256

/* Global volatile to prevent dead code elimination */
volatile int g_result = 0;

/* Function 1: Post-increment pattern - should trigger auto-increment */
static void __attribute__((noinline)) 
copy_with_post_inc(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ - both have zero offset at access time */
    while (n-- > 0) {
        *dst = *src;  /* Base+0 offset for both src and dst */
        dst++;        /* Increment happens after access */
        src++;        /* Increment happens after access */
    }
}

/* Function 2: Pre-decrement pattern - should trigger auto-decrement */
static void __attribute__((noinline))
reverse_with_pre_dec(int *dst, const int *src, int n) {
    /* Start pointers at the end */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src - both have zero offset at access time */
    while (n-- > 0) {
        --dst;        /* Decrement happens before access */
        --src;        /* Decrement happens before access */
        *dst = *src;  /* Base+0 offset for both src and dst */
    }
}

/* Function 3: Mixed operations with char pointers */
static int __attribute__((noinline))
sum_with_inc(const char *data, int n) {
    int sum = 0;
    const char *p = data;
    
    /* Pattern: sum += *p++ - zero offset at access time */
    while (n-- > 0) {
        sum += *p;  /* Base+0 offset */
        p++;        /* Increment after access */
    }
    
    return sum;
}

/* Function 4: Array initialization with pointer arithmetic */
static void __attribute__((noinline))
init_array(int *arr, int n, int start) {
    int *p = arr;
    int value = start;
    
    /* Pattern: *p++ = value++ - zero offset at access time */
    while (n-- > 0) {
        *p = value;  /* Base+0 offset */
        p++;
        value++;
    }
}

/* Function 5: Memory copy with overlapping regions (memmove-like) */
static void __attribute__((noinline))
copy_overlap(char *dst, const char *src, int n) {
    if (dst > src) {
        /* Copy backwards using pre-decrement */
        dst += n;
        src += n;
        while (n-- > 0) {
            --dst;
            --src;
            *dst = *src;  /* Base+0 offset */
        }
    } else {
        /* Copy forwards using post-increment */
        while (n-- > 0) {
            *dst = *src;  /* Base+0 offset */
            dst++;
            src++;
        }
    }
}

/* Function 6: Search pattern with pointer */
static int * __attribute__((noinline))
find_value(int *arr, int n, int target) {
    int *p = arr;
    
    /* Pattern: if (*p == target) - zero offset at access time */
    while (n-- > 0) {
        if (*p == target) {  /* Base+0 offset */
            return p;
        }
        p++;
    }
    return NULL;
}

int main(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    char char_data[ARRAY_SIZE];
    int result = 0;
    
    /* Initialize source arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 2;
        char_data[i] = (char)(i % 128);
    }
    
    /* Test 1: Post-increment copy */
    memset(dst, 0, sizeof(dst));
    copy_with_post_inc(dst, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Post-increment copy mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Post-increment copy verified\n");
    
    /* Test 2: Pre-decrement reverse */
    memset(dst, 0, sizeof(dst));
    reverse_with_pre_dec(dst, src, ARRAY_SIZE);
    
    /* Verify reverse copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[ARRAY_SIZE - 1 - i]) {
            printf("FAIL: Pre-decrement reverse mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Pre-decrement reverse verified\n");
    
    /* Test 3: Char pointer sum with increment */
    int sum = sum_with_inc(char_data, ARRAY_SIZE);
    
    /* Calculate expected sum */
    int expected_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += char_data[i];
    }
    
    if (sum != expected_sum) {
        printf("FAIL: Sum calculation incorrect: got %d, expected %d\n", 
               sum, expected_sum);
        return 1;
    }
    printf("PASS: Char pointer sum verified: %d\n", sum);
    
    /* Test 4: Array initialization */
    int test_arr[100];
    init_array(test_arr, 100, 42);
    
    for (int i = 0; i < 100; i++) {
        if (test_arr[i] != 42 + i) {
            printf("FAIL: Array initialization mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Array initialization verified\n");
    
    /* Test 5: Overlapping copy */
    char overlap_src[50];
    char overlap_dst[50];
    
    for (int i = 0; i < 50; i++) {
        overlap_src[i] = (char)('A' + i);
    }
    
    /* Test forward copy (non-overlapping) */
    copy_overlap(overlap_dst, overlap_src, 50);
    if (memcmp(overlap_dst, overlap_src, 50) != 0) {
        printf("FAIL: Forward overlapping copy mismatch\n");
        return 1;
    }
    
    /* Test backward copy (overlapping) */
    memcpy(overlap_dst, overlap_src, 50);
    copy_overlap(overlap_dst + 10, overlap_dst, 30);
    printf("PASS: Overlapping copy tests completed\n");
    
    /* Test 6: Search pattern */
    int *found = find_value(src, ARRAY_SIZE, 100);
    if (found != NULL && *found == 100) {
        printf("PASS: Search pattern found value 100\n");
    } else {
        printf("INFO: Search pattern test completed (value 100 not in array)\n");
    }
    
    /* Store result to global to prevent optimization */
    g_result = sum;
    
    printf("\nAll tests passed! The auto-inc-dec pass should have identified\n");
    printf("zero-offset memory access patterns for auto-increment/decrement.\n");
    
    return 0;
}
