/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Test 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_ptr_plus_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access with explicit zero offset - should generate *(p + 0) */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    /* Use volatile to ensure loop isn't eliminated */
    g_volatile_sum += sum;
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_index(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access with array[0] notation */
        int val = p[0];
        sum += val;
        /* Separate increment with += */
        p += 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 3: Store instead of load with *(p + 0) = val; p-- */
__attribute__((noinline))
void test3_store_decrement(int *arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate decrement */
        p--;
    }
    
    /* Force side effect */
    asm volatile("" : : "r"(arr[0]) : "memory");
}

/* Test 4: Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
int test4_struct_member(struct ptr_wrapper *wrapper) {
    int sum = 0;
    int *p = wrapper->current;
    int *end = wrapper->end;
    
    while (p < end) {
        /* Access with zero offset */
        int val = *(p + 0);
        sum += val;
        /* Increment pointer */
        p++;
    }
    
    wrapper->current = p;
    g_volatile_sum += sum;
    return sum;
}

/* Test 5: Different increment pattern with post-increment in expression */
__attribute__((noinline))
int test5_mixed_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Force zero offset access */
        int *temp = p + 0;
        int val = *temp;
        sum += val;
        /* Increment in separate statement */
        p = p + 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 6: Using volatile to force memory access */
__attribute__((noinline))
int test6_volatile_access(volatile int *arr, int n) {
    volatile int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Volatile access with zero offset */
        int val = *(p + 0);
        sum += val;
        /* Increment */
        p++;
    }
    
    return sum;
}

/* Main function with runtime-determined array size */
int main(int argc, char **argv) {
    /* Use argc to determine array size at runtime */
    int array_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (array_size <= 0) array_size = 100;
    
    /* Allocate and initialize array */
    int *array = (int *)malloc(array_size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < array_size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Run test 1 */
    total_sum += test1_ptr_plus_zero(array, array_size);
    
    /* Run test 2 */
    total_sum += test2_array_zero_index(array, array_size);
    
    /* Run test 3 (stores) */
    test3_store_decrement(array, array_size, 42);
    
    /* Run test 4 with struct */
    struct ptr_wrapper wrapper;
    wrapper.current = array;
    wrapper.end = array + array_size;
    total_sum += test4_struct_member(&wrapper);
    
    /* Run test 5 */
    total_sum += test5_mixed_increment(array, array_size);
    
    /* Run test 6 with volatile */
    total_sum += test6_volatile_access(array, array_size);
    
    /* Print result to prevent elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    free(array);
    return 0;
}
