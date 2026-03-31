/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stddef.h>

#define SIZE 256
#define ITERATIONS 100

/* Global arrays for different access patterns */
int global_array[SIZE];
static int static_array[SIZE];

/* Pattern A: Simple array loop */
__attribute__((noinline))
int pattern_a_simple_array(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];  /* Base + offset, may become base + 0 after optimization */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic */
__attribute__((noinline))
int pattern_b_pointer_arithmetic(int *arr, int n) {
    int total = 0;
    int *p = arr;
    int *end = p + n;
    
    while (p < end) {
        int val = *p;  /* Direct dereference with base register + 0 offset */
        p++;
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int padding[3];  /* Force non-trivial structure size */
};

__attribute__((noinline))
int pattern_c_struct_traversal(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        result += sp->value;  /* Base + 0 offset for struct field access */
        sp++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_access(void) {
    int local_sum = 0;
    int *ptr = &global_array[0];  /* Take address of global */
    
    for (int i = 0; i < SIZE; ++i) {
        local_sum += *ptr;  /* Dereference with base register */
        ptr++;  /* Increment after use */
    }
    return local_sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_blocks(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer to stay in register through conditional */
        if (*ptr > threshold) {  /* Dereference in condition */
            sum += *ptr;  /* Another dereference */
        } else {
            sum -= *ptr;
        }
        
        /* Increment after use in multiple paths */
        ptr++;
    }
    return sum;
}

/* Pattern F: Mixed access patterns to confuse optimizer */
__attribute__((noinline))
int pattern_f_mixed_access(int *arr1, int *arr2, int n) {
    int sum1 = 0, sum2 = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Alternate between arrays */
        if (i & 1) {
            sum1 += *p1;  /* Base + 0 */
            p1++;
        } else {
            sum2 += *p2;  /* Base + 0 */
            p2++;
        }
    }
    return sum1 + sum2;
}

/* Pattern G: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_g_nested_loops(int *arr, int rows, int cols) {
    int total = 0;
    
    for (int r = 0; r < rows; ++r) {
        int *row_ptr = arr + r * cols;  /* Calculate row start */
        
        for (int c = 0; c < cols; ++c) {
            total += *row_ptr;  /* Base + 0 within inner loop */
            row_ptr++;
        }
    }
    return total;
}

/* Helper to force pointer into register */
__attribute__((noinline))
void touch_pointer(void *ptr) {
    /* Use inline asm to ensure pointer stays in register */
    asm volatile("" : : "r"(ptr) : "memory");
}

/* Pattern H: Complex expression with pointer */
__attribute__((noinline))
int pattern_h_complex_expr(int *arr, int n) {
    int *ptr = arr;
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Complex expression that uses the pointer multiple times */
        result = (result + *ptr) * (*ptr + 1);  /* Two dereferences */
        ptr++;
        
        /* Touch pointer to keep it in register */
        if (i % 8 == 0) {
            touch_pointer(ptr);
        }
    }
    return result;
}

/* Initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 100;
        static_array[i] = (i * 3) % 100;
    }
}

/* Main driver that exercises all patterns */
int main(int argc, char **argv) {
    int iterations = ITERATIONS;
    if (argc > 1) {
        iterations = 1;  /* Reduce iterations for quick testing */
    }
    
    init_arrays();
    
    int total = 0;
    
    /* Execute patterns multiple times to ensure compiler runs optimizations */
    for (int i = 0; i < iterations; ++i) {
        total += pattern_a_simple_array(static_array, SIZE);
        total += pattern_b_pointer_arithmetic(global_array, SIZE);
        
        struct Data struct_array[SIZE];
        for (int j = 0; j < SIZE; ++j) {
            struct_array[j].value = j;
        }
        total += pattern_c_struct_traversal(struct_array, SIZE);
        
        total += pattern_d_global_access();
        total += pattern_e_conditional_blocks(static_array, SIZE, 50);
        total += pattern_f_mixed_access(global_array, static_array, SIZE);
        total += pattern_g_nested_loops(global_array, 16, 16);
        total += pattern_h_complex_expr(static_array, SIZE);
    }
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        return 1;
    }
    return 0;
}
