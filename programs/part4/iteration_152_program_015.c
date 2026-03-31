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
        sum += arr[i];  /* Should generate base + offset */
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
        int val = *p;  /* Base register with zero offset */
        p++;           /* Pointer increment after use */
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int padding[3];  /* Force non-trivial size */
};

__attribute__((noinline))
int pattern_c_struct_traversal(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer to be in register before dereference */
        asm volatile("" : : "r"(sp) : "memory");
        result += sp->value;  /* Base + 0 offset */
        sp++;                 /* Post-increment */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_access(void) {
    int sum = 0;
    int *local_ptr = global_array;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Multiple accesses to same base */
        int val1 = *local_ptr;
        local_ptr++;
        int val2 = *(local_ptr - 1);  /* Different offset pattern */
        sum += val1 + val2;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional(int *arr, int n, int threshold) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        if (i < threshold) {
            /* Use pointer with zero offset */
            int val = *ptr;
            ptr++;
            result += val;
        } else {
            /* Different offset calculation */
            result += ptr[i - threshold];
        }
    }
    return result;
}

/* Pattern F: Mixed pointer/index access */
__attribute__((noinline))
int pattern_f_mixed_access(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i += 2) {
        /* Alternate between pointer and indexed access */
        sum += *p;      /* Base + 0 */
        p++;
        sum += arr[i];  /* Different base calculation */
    }
    return sum;
}

/* Pattern G: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_g_nested_loops(int *arr, int rows, int cols) {
    int total = 0;
    
    for (int r = 0; r < rows; ++r) {
        int *row_ptr = arr + r * cols;
        
        for (int c = 0; c < cols; ++c) {
            /* Simple dereference with base + 0 */
            total += *row_ptr;
            row_ptr++;
        }
    }
    return total;
}

/* Main driver that uses all patterns */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
    
    /* Create struct array */
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        struct_array[i].value = i * 3;
    }
    
    /* Run patterns multiple times to ensure execution */
    int iterations = (argc > 1) ? ITERATIONS : 10;
    
    for (int iter = 0; iter < iterations; ++iter) {
        /* Pattern A */
        result ^= pattern_a_simple_array(global_array, SIZE);
        
        /* Pattern B */
        result ^= pattern_b_pointer_arithmetic(static_array, SIZE);
        
        /* Pattern C */
        result ^= pattern_c_struct_traversal(struct_array, SIZE);
        
        /* Pattern D */
        result ^= pattern_d_global_access();
        
        /* Pattern E - vary threshold */
        int threshold = iter % SIZE;
        result ^= pattern_e_conditional(global_array, SIZE, threshold);
        
        /* Pattern F */
        result ^= pattern_f_mixed_access(static_array, SIZE);
        
        /* Pattern G - 2D access pattern */
        result ^= pattern_g_nested_loops(global_array, 16, 16);
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(result) : "memory");
    
    return result % 255;
}
