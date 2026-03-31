/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination of memory accesses */
extern void dummy_external(int);

/* Volatile variable to prevent constant propagation */
volatile int g_volatile_bound = 100;

/* Prevent inlining to isolate optimization patterns */
__attribute__((noinline))
int test_ptr_plus_zero_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 1: *(p + 0) with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    return sum;
}

__attribute__((noinline))
int test_array_zero_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 2: p[0] with zero index */
        int val = p[0];
        sum += val;
        /* Increment with += 1 */
        p += 1;
    }
    return sum;
}

__attribute__((noinline))
int test_ptr_plus_zero_decrement(int *arr, int n) {
    int *p = arr + n - 1;  /* Start from end */
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 3: *(p + 0) with decrement */
        int val = *(p + 0);
        sum += val;
        /* Separate decrement statement */
        p--;
    }
    return sum;
}

__attribute__((noinline))
void test_store_ptr_plus_zero(int *arr, int n, int value) {
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 4: Store with *(p + 0) */
        *(p + 0) = value + i;
        /* Increment after store */
        p++;
    }
}

__attribute__((noinline))
int test_mixed_access_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Use asm to ensure access isn't eliminated */
        asm volatile("" : : "r"(*p) : "memory");
        
        /* Multiple zero-offset accesses */
        sum += *(p + 0);
        sum += p[0];
        
        /* Increment with separate statement */
        p = p + 1;
    }
    return sum;
}

__attribute__((noinline))
int test_struct_member_increment(int *arr, int n) {
    struct wrapper {
        int *ptr;
        int count;
    } w;
    
    w.ptr = arr;
    w.count = 0;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through struct member with zero offset */
        int val = *(w.ptr + 0);
        sum += val;
        w.count++;
        
        /* Increment struct member */
        w.ptr++;
    }
    return sum + w.count;
}

__attribute__((noinline))
int test_volatile_bound_increment(int *arr) {
    int *p = arr;
    int sum = 0;
    volatile int bound = g_volatile_bound;
    
    for (int i = 0; i < bound; i++) {
        /* Force zero-offset access */
        int *temp = p;
        sum += *(temp + 0);
        
        /* Increment in separate step */
        p = p + 1;
    }
    return sum;
}

__attribute__((noinline))
int test_complex_expr_zero_offset(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex expression that should still yield *(p + 0) */
        int val = *(p + (i - i));  /* Evaluates to *(p + 0) */
        sum += val;
        
        /* Call external to prevent elimination */
        dummy_external(val);
        
        p++;
    }
    return sum;
}

/* Dummy implementation to satisfy external reference */
void dummy_external(int x) {
    /* Empty but prevents dead code elimination */
    (void)x;
}

int main(int argc, char **argv) {
    /* Use argc to make array size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Call all test functions to trigger different patterns */
    total_sum += test_ptr_plus_zero_increment(array, size);
    total_sum += test_array_zero_increment(array, size);
    total_sum += test_ptr_plus_zero_decrement(array, size);
    
    test_store_ptr_plus_zero(array, size, 42);
    total_sum += test_mixed_access_increment(array, size);
    total_sum += test_struct_member_increment(array, size);
    total_sum += test_volatile_bound_increment(array);
    total_sum += test_complex_expr_zero_offset(array, size);
    
    /* Use result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    return 0;
}
