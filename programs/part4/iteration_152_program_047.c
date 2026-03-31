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
        p++;           /* Pointer increment after use */
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int padding[3];  /* Ensure structure has size */
};

__attribute__((noinline))
int pattern_c_struct_traversal(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        result += sp->value;  /* Access through struct pointer */
        sp++;                 /* Increment after use */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_with_local(void) {
    int sum = 0;
    int *local_ptr = global_array;  /* Take address of global */
    
    for (int i = 0; i < SIZE; ++i) {
        sum += *local_ptr;  /* Dereference local pointer copy */
        local_ptr++;        /* Increment after use */
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_blocks(int *arr, int n, int threshold) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer to be kept in register with inline asm */
        asm volatile("" : : "r"(ptr) : "memory");
        
        int val = *ptr;  /* Base + 0 dereference */
        
        if (val > threshold) {
            result += val;
            ptr++;  /* Increment on one path */
        } else {
            result -= val;
            ptr++;  /* Increment on another path */
        }
    }
    return result;
}

/* Pattern F: Mixed access patterns to confuse optimizer */
__attribute__((noinline))
int pattern_f_mixed_patterns(int *arr1, int *arr2, int n) {
    int sum1 = 0, sum2 = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Alternate between two pointers */
        if (i % 2 == 0) {
            sum1 += *p1;  /* Dereference p1 */
            p1++;
        } else {
            sum2 += *p2;  /* Dereference p2 */
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
            total += *row_ptr;  /* Dereference row pointer */
            row_ptr++;          /* Move to next column */
        }
    }
    return total;
}

/* Helper to initialize arrays */
void initialize_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 100;
        static_array[i] = (i * 3) % 100;
    }
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (atoi(argv[1]) % ITERATIONS) + 1 : ITERATIONS;
    int size = (argc > 2) ? (atoi(argv[2]) % SIZE) + 1 : SIZE / 2;
    
    initialize_arrays();
    
    /* Local array for stack-based access */
    int local_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        local_array[i] = (i * 7) % 100;
    }
    
    /* Struct array */
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        struct_array[i].value = i % 50;
    }
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; ++i) {
        result ^= pattern_a_simple_array(local_array, size);
        result ^= pattern_b_pointer_arithmetic(static_array, size);
        result ^= pattern_c_struct_traversal(struct_array, size);
        result ^= pattern_d_global_with_local();
        result ^= pattern_e_conditional_blocks(local_array, size, 30);
        result ^= pattern_f_mixed_patterns(local_array, static_array, size);
        result ^= pattern_g_nested_loops(local_array, 8, size / 8);
    }
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
    
    return result != 0 ? 0 : 1;
}
