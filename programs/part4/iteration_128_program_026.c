/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_external(int);

/* Volatile variable to prevent constant propagation */
volatile int g_volatile_bound = 100;

/* Prevent inlining to keep pattern intact */
__attribute__((noinline))
static int test_ptr_plus_zero_increment(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Use volatile bound to prevent loop unrolling */
    volatile int bound = n;
    
    for (int i = 0; i < bound; i++) {
        /* Pattern 1: *(p + 0) with explicit zero offset */
        int val = *(p + 0);
        
        /* Separate increment statement */
        p++;
        
        /* Use value to prevent elimination */
        sum += val;
        dummy_external(val);
    }
    
    return sum;
}

__attribute__((noinline))
static int test_array_zero_increment(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    volatile int bound = n;
    
    for (int i = 0; i < bound; i++) {
        /* Pattern 2: p[0] access (should become *(p + 0)) */
        int val = p[0];
        
        /* Separate increment with += 1 */
        p += 1;
        
        sum += val;
        asm volatile("" : : "r"(val)); /* Inline asm to prevent elimination */
    }
    
    return sum;
}

__attribute__((noinline))
static int test_ptr_zero_decrement(int *arr, int n) {
    int sum = 0;
    /* Start from end for decrement pattern */
    int *p = arr + n - 1;
    
    volatile int bound = n;
    
    for (int i = 0; i < bound; i++) {
        /* Pattern 3: Store instead of load */
        int val = i;
        *(p + 0) = val;
        
        /* Separate decrement */
        p--;
        
        sum += val;
        dummy_external(val);
    }
    
    return sum;
}

/* Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
static int test_struct_ptr_zero(int *arr, int n) {
    struct ptr_wrapper wrapper;
    wrapper.current = arr;
    wrapper.end = arr + n;
    
    int sum = 0;
    volatile int bound = n;
    
    for (int i = 0; i < bound; i++) {
        /* Pattern 4: Access through structure member */
        int val = *(wrapper.current + 0);
        
        /* Increment structure member */
        wrapper.current = wrapper.current + 1;
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    
    return sum;
}

__attribute__((noinline))
static int test_mixed_zero_offset(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    volatile int bound = n;
    
    /* Mix of different zero-offset patterns */
    for (int i = 0; i < bound; i++) {
        if (i % 2 == 0) {
            /* Even indices: *(p + 0) */
            int val = *(p + 0);
            sum += val;
            dummy_external(val);
        } else {
            /* Odd indices: p[0] */
            int val = p[0];
            sum += val;
            asm volatile("" : : "r"(val));
        }
        
        /* Always increment after access */
        p = p + 1;
    }
    
    return sum;
}

/* Dummy external function implementation */
void dummy_external(int x) {
    /* Empty but prevents elimination */
    asm volatile("" : : "r"(x));
}

int main(int argc, char **argv) {
    /* Use argc to determine array size at runtime */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Call all test functions */
    total_sum += test_ptr_plus_zero_increment(array, size);
    total_sum += test_array_zero_increment(array, size);
    total_sum += test_ptr_zero_decrement(array, size);
    total_sum += test_struct_ptr_zero(array, size);
    total_sum += test_mixed_zero_offset(array, size);
    
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    return 0;
}
