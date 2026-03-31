/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void dummy_external(int);

/* Volatile sink to prevent dead code elimination */
static volatile int volatile_sink;

/* Prevent inlining to isolate patterns */
__attribute__((noinline))
static void test1_ptr_plus_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pattern: *(p + 0) with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    volatile_sink = sum;
}

__attribute__((noinline))
static void test2_array_zero_index(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pattern: p[0] which should become *(p + 0) */
        int val = p[0];
        sum += val;
        /* Separate increment with += 1 */
        p += 1;
    }
    
    volatile_sink = sum;
}

__attribute__((noinline))
static void test3_store_pattern(int *arr, int n, int value) {
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Store pattern with zero offset */
        *(p + 0) = value + i;
        /* Decrement pattern */
        p--;
    }
}

__attribute__((noinline))
static void test4_mixed_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Use asm to ensure access isn't eliminated */
        asm volatile("" : : "r"(*(p + 0)));
        sum += *(p + 0);
        
        /* Different increment forms */
        if (i % 2 == 0) {
            p = p + 1;
        } else {
            p++;
        }
    }
    
    volatile_sink = sum;
}

__attribute__((noinline))
static void test5_struct_member(int *arr, int n) {
    struct wrapper {
        int *ptr;
        int count;
    } w = {arr, n};
    
    int sum = 0;
    
    for (int i = 0; i < w.count; i++) {
        /* Access through struct member with zero offset */
        int val = *(w.ptr + 0);
        sum += val;
        /* Increment struct member */
        w.ptr++;
    }
    
    volatile_sink = sum;
}

__attribute__((noinline))
static void test6_post_increment_load(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Load with zero offset, then post-increment in same statement */
        int val = *(p + 0);
        p++;  /* Separate statement to match pattern */
        sum += val;
    }
    
    volatile_sink = sum;
}

__attribute__((noinline))
static void test7_negative_offset(int *arr, int n) {
    int *p = arr + n - 1;  /* Start from end */
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access with zero offset */
        int val = *(p + 0);
        sum += val;
        /* Decrement pointer */
        p--;
    }
    
    volatile_sink = sum;
}

__attribute__((noinline))
static void test8_volatile_bound(int *arr, volatile int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Force zero offset pattern */
        int *temp = p;
        int val = *(temp + 0);
        sum += val;
        p++;
    }
    
    volatile_sink = sum;
}

int main(int argc, char **argv) {
    /* Use argc to make array size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Dynamic allocation prevents compile-time analysis */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Call all test functions */
    test1_ptr_plus_zero(array, size);
    test2_array_zero_index(array, size);
    test3_store_pattern(array, size, 42);
    test4_mixed_increment(array, size);
    test5_struct_member(array, size);
    test6_post_increment_load(array, size);
    test7_negative_offset(array, size);
    
    /* Use volatile variable for bound */
    volatile int volatile_size = size;
    test8_volatile_bound(array, volatile_size);
    
    /* Use results to prevent elimination */
    printf("Result: %d\n", volatile_sink);
    
    free(array);
    return 0;
}
