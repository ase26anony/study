/* test_auto_inc_dec.c
 * Program designed to trigger zero-offset memory access patterns
 * for GCC's auto-inc-dec pass (lines 1352-1358 in auto-inc-dec.cc)
 */

#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE 256

/* Use noinline to prevent inlining and preserve loop structure */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset accesses */
    while (n-- > 0) {
        *dst = *src;  /* Base + 0 offset pattern */
        dst++;        /* Post-increment */
        src++;        /* Post-increment */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src creates zero-offset accesses */
    while (n-- > 0) {
        dst--;        /* Pre-decrement */
        src--;        /* Pre-decrement */
        *dst = *src;  /* Base + 0 offset pattern */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ creates zero-offset read */
    while (n-- > 0) {
        sum += *p;  /* Base + 0 offset pattern */
        p++;        /* Post-increment */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_pointers(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Pattern: *start++ = *end-- with zero-offset accesses */
    while (start < end) {
        int temp = *end;   /* Base + 0 offset pattern */
        *end = *start;     /* Base + 0 offset pattern */
        *start = temp;     /* Base + 0 offset pattern */
        start++;
        end--;
    }
}

/* Volatile global to prevent dead code elimination */
volatile int checksum = 0;

int main(void) {
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
    
    /* Test 2: Pre-decrement copy */
    copy_with_pre_decrement(dst2, src, ARRAY_SIZE);
    
    /* Verify both copies match */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != src[i] || dst2[i] != src[i]) {
            printf("Error: Copy mismatch at index %d\n", i);
            return 1;
        }
    }
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    
    /* Test 4: Reverse with pointer arithmetic */
    int test_arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int rev_arr[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    reverse_with_pointers(test_arr, 10);
    
    /* Verify reversal */
    for (i = 0; i < 10; i++) {
        if (test_arr[i] != rev_arr[i]) {
            printf("Error: Reverse mismatch at index %d\n", i);
            return 1;
        }
    }
    
    /* Store checksum to volatile to prevent optimization */
    checksum = sum;
    
    printf("All tests passed! Checksum: %d\n", sum);
    printf("Expected checksum: %d\n", (ARRAY_SIZE * (ARRAY_SIZE - 1) * 3) / 2 + ARRAY_SIZE);
    
    return 0;
}
