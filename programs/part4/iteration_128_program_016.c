/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_use(int val);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int* g_volatile_ptr = NULL;

/* Prevent inlining to keep patterns intact */
__attribute__((noinline))
int test_ptr_plus_zero_increment(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 1: *(p + 0) with explicit zero offset */
        int val = *(p + 0);  /* This should create mem_loc with reg1_val = 0 */
        sum += val;
        
        /* Separate increment statement */
        p++;  /* This should be found by find_inc() */
    }
    
    /* Use result to prevent elimination */
    dummy_use(sum);
    return sum;
}

__attribute__((noinline))
int test_array_zero_increment(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 2: p[0] syntax (should become *(p + 0)) */
        int val = p[0];  /* Array access with zero index */
        sum += val;
        
        /* Separate increment with assignment */
        p += 1;  /* p = p + 1 */
    }
    
    dummy_use(sum);
    return sum;
}

__attribute__((noinline))
int test_ptr_zero_decrement(int* arr, int n) {
    int sum = 0;
    /* Start from end for decrement pattern */
    int* p = arr + n - 1;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 3: Store operation with zero offset */
        int old_val = *(p + 0);  /* Load with zero offset */
        sum += old_val;
        
        /* Modify and store back */
        *(p + 0) = old_val + 1;  /* Store with zero offset */
        
        /* Separate decrement */
        p--;  /* Decrement by 1 */
    }
    
    dummy_use(sum);
    return sum;
}

/* Structure with pointer member */
struct ptr_wrapper {
    int* current;
    int* end;
};

__attribute__((noinline))
int test_struct_ptr_zero_inc(int* arr, int n) {
    struct ptr_wrapper wrapper;
    wrapper.current = arr;
    wrapper.end = arr + n;
    
    int sum = 0;
    while (wrapper.current < wrapper.end) {
        /* Pattern 4: Access through structure member */
        int val = *(wrapper.current + 0);  /* Explicit zero offset */
        sum += val;
        
        /* Increment through structure member */
        wrapper.current = wrapper.current + 1;  /* Separate increment */
    }
    
    dummy_use(sum);
    return sum;
}

__attribute__((noinline))
int test_mixed_offsets(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    
    /* Mix of zero and non-zero offsets to test pattern matching */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            /* Use zero offset */
            sum += *(p + 0);
        } else {
            /* Use non-zero offset - should not match the pattern */
            sum += *(p + 1);
            p++;  /* Extra increment to maintain alignment */
        }
        p++;  /* Common increment */
    }
    
    dummy_use(sum);
    return sum;
}

/* Dummy function implementation */
void dummy_use(int val) {
    /* Use inline asm to ensure the value is used */
    asm volatile("" : : "r"(val) : "memory");
}

int main(int argc, char** argv) {
    /* Use argc to determine array size at runtime */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    int* array = (int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;  /* Simple sequence 1, 2, 3, ... */
    }
    
    /* Use volatile to prevent optimization */
    g_volatile_bound = size;
    g_volatile_ptr = array;
    
    int total_sum = 0;
    
    /* Call all test functions */
    total_sum += test_ptr_plus_zero_increment(g_volatile_ptr, g_volatile_bound);
    total_sum += test_array_zero_increment(array, size);
    total_sum += test_ptr_zero_decrement(array, size);
    total_sum += test_struct_ptr_zero_inc(array, size);
    total_sum += test_mixed_offsets(array, size);
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    return 0;
}
