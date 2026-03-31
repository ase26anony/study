/* auto_inc_test.c - Test auto-increment/decrement pattern recognition */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Test 1: Using *(p + 0) with p++ */
__attribute__((noinline))
int test1_ptr_plus_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Explicit zero offset - should generate *(p + 0) */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    /* Use result to prevent elimination */
    g_volatile_sum += sum;
    return sum;
}

/* Test 2: Using p[0] with p += 1 */
__attribute__((noinline))
int test2_array_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Separate increment with assignment */
        p += 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 3: Using structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
int test3_struct_ptr(struct ptr_wrapper *wrapper) {
    int sum = 0;
    int *p = wrapper->current;
    int n = wrapper->end - p;
    
    for (int i = 0; i < n; i++) {
        /* Force zero offset through temporary */
        int *tmp = p;
        int val = *(tmp + 0);
        sum += val;
        p = p + 1;  /* Separate increment */
    }
    
    wrapper->current = p;
    g_volatile_sum += sum;
    return sum;
}

/* Test 4: Store instead of load */
__attribute__((noinline))
void test4_store_zero_offset(int *dest, int *src, int n) {
    int *d = dest;
    int *s = src;
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(d + 0) = *(s + 0);
        /* Separate increments */
        d = d + 1;
        s = s + 1;
    }
}

/* Test 5: Decrement instead of increment */
__attribute__((noinline))
int test5_decrement(int *arr, int n) {
    int *p = arr + n - 1;  /* Start from end */
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Load with zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate decrement */
        p--;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 6: Complex expression with zero offset */
__attribute__((noinline))
int test6_complex_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* More complex zero offset expression */
        int val = *(0 + p + 0);
        sum += val;
        
        /* Increment with asm to prevent reordering */
        asm volatile("" : : "r"(val));  /* Use val to prevent elimination */
        p = p + 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 7: Multiple memory accesses with same pointer */
__attribute__((noinline))
int test7_multiple_accesses(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Two accesses with zero offset */
        int val1 = *(p + 0);
        int val2 = *(p + 0);  /* Same offset, should still match pattern */
        sum += val1 + val2;
        p++;  /* Single increment after both accesses */
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 8: Pointer arithmetic in loop condition */
__attribute__((noinline))
int test8_ptr_in_condition(int *arr, int n) {
    int *p = arr;
    int *end = arr + n;
    int sum = 0;
    
    while (p < end) {
        int val = *(p + 0);
        sum += val;
        p = p + 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Main function to run all tests */
int main(int argc, char **argv) {
    /* Use argc to determine array size at runtime */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;  /* Non-zero values */
    }
    
    int total_sum = 0;
    
    /* Run all tests */
    total_sum += test1_ptr_plus_zero(array, size);
    total_sum += test2_array_zero(array, size);
    
    struct ptr_wrapper wrapper = {array, array + size};
    total_sum += test3_struct_ptr(&wrapper);
    
    int *dest = (int *)malloc(size * sizeof(int));
    if (dest) {
        test4_store_zero_offset(dest, array, size);
        free(dest);
    }
    
    total_sum += test5_decrement(array, size);
    total_sum += test6_complex_zero(array, size);
    total_sum += test7_multiple_accesses(array, size);
    total_sum += test8_ptr_in_condition(array, size);
    
    /* Use volatile to ensure all computations are kept */
    volatile int final_result = total_sum + g_volatile_sum;
    
    printf("Total sum: %d\n", final_result);
    
    free(array);
    return 0;
}
