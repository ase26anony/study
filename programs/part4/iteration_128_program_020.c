/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * for memory accesses with zero offset followed by pointer increment.
 */

#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_external(int);

/* Volatile sink to prevent dead code elimination */
static volatile int volatile_sink = 0;

/* Prevent inlining to ensure pattern is visible in RTL */
__attribute__((noinline))
static int test_ptr_plus_zero_increment(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Use volatile to prevent constant propagation */
    volatile int *volatile_ptr = &arr[0];
    p = (int*)volatile_ptr;
    
    for (int i = 0; i < n; i++) {
        /* Core pattern: memory access with explicit zero offset */
        int val = *(p + 0);  /* Should generate *(reg + 0) in RTL */
        sum += val;
        
        /* Separate pointer increment */
        p++;  /* Should be a separate increment operation */
    }
    
    volatile_sink = sum;
    return sum;
}

__attribute__((noinline))
static int test_array_zero_increment(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Force pointer through volatile to prevent optimization */
    volatile int vol_n = n;
    int limit = vol_n;
    
    for (int i = 0; i < limit; i++) {
        /* Alternative pattern: array access with index 0 */
        int val = p[0];  /* Should also generate *(reg + 0) */
        sum += val;
        
        /* Separate increment with assignment */
        p += 1;  /* Different increment syntax */
    }
    
    volatile_sink += sum;
    return sum;
}

__attribute__((noinline))
static int test_ptr_zero_decrement(int *arr, int n) {
    int sum = 0;
    int *p = &arr[n - 1];  /* Start from end */
    
    /* Use asm to prevent optimization */
    asm volatile("" : "+r"(p) : : "memory");
    
    for (int i = n - 1; i >= 0; i--) {
        /* Store variant with zero offset */
        int old_val = *(p + 0);  /* Load with zero offset */
        sum += old_val;
        
        /* Decrement after access */
        p--;  /* Decrement pattern */
    }
    
    volatile_sink += sum;
    return sum;
}

__attribute__((noinline))
static int test_mixed_operations(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Complex loop to prevent simplification */
    volatile int vol_counter = 0;
    
    while (vol_counter < n) {
        /* Multiple zero-offset accesses */
        int val1 = *(p + 0);
        sum += val1;
        
        /* Force side effect */
        asm volatile("" : : "r"(val1) : "memory");
        
        /* Increment with different syntax */
        p = p + 1;
        
        vol_counter++;
    }
    
    return sum;
}

/* Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
static int test_struct_ptr(struct ptr_wrapper *wrapper) {
    int sum = 0;
    
    while (wrapper->current != wrapper->end) {
        /* Access through structure pointer with zero offset */
        int val = *(wrapper->current + 0);
        sum += val;
        
        /* Increment the structure member */
        wrapper->current++;
    }
    
    return sum;
}

__attribute__((noinline))
static int test_double_increment(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i += 2) {
        /* Two consecutive accesses with zero offset */
        int val1 = *(p + 0);
        sum += val1;
        p++;  /* First increment */
        
        if (i + 1 < n) {
            int val2 = *(p + 0);
            sum += val2;
            p++;  /* Second increment */
        }
    }
    
    return sum;
}

/* Main function that calls all test variants */
int main(int argc, char **argv) {
    /* Use argc to determine array size (non-constant) */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;  /* Simple sequence */
    }
    
    int total = 0;
    
    /* Call all test functions */
    total += test_ptr_plus_zero_increment(array, size);
    total += test_array_zero_increment(array, size);
    total += test_ptr_zero_decrement(array, size);
    total += test_mixed_operations(array, size);
    
    /* Test with structure */
    struct ptr_wrapper wrapper;
    wrapper.current = array;
    wrapper.end = array + size;
    total += test_struct_ptr(&wrapper);
    
    /* Reset for next test */
    wrapper.current = array;
    total += test_double_increment(array, size);
    
    /* Use result to prevent elimination */
    printf("Total sum: %d\n", total);
    volatile_sink = total;
    
    free(array);
    return 0;
}
