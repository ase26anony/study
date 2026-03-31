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
int pattern_b_explicit_pointer(int *arr, int n) {
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
    int padding[3];  /* Force non-trivial size */
};

__attribute__((noinline))
int pattern_c_struct_pointer(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer to be in register before dereference */
        asm volatile("" : : "r"(sp) : "memory");
        result += sp->value;  /* Base + 0 offset for struct access */
        sp++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_pointer(void) {
    int sum = 0;
    int *local_ptr = global_array;  /* Take address of global */
    
    for (int i = 0; i < SIZE; ++i) {
        sum += *local_ptr;  /* Dereference pointer in register */
        local_ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_pointer(int *arr, int n, int threshold) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            /* Even indices: use current pointer */
            result += *ptr;  /* Base + 0 offset */
            ptr++;
        } else {
            /* Odd indices: skip ahead */
            ptr += 2;
            if (ptr < arr + n) {
                result -= *ptr;  /* Different base + 0 offset */
            }
        }
    }
    return result;
}

/* Pattern F: Mixed access patterns in same function */
__attribute__((noinline))
int pattern_f_mixed_access(int *arr1, int *arr2, int n) {
    int sum1 = 0, sum2 = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Alternate between arrays */
        if (i % 3 == 0) {
            sum1 += *p1;  /* First array access */
            p1++;
        } else {
            sum2 += *p2;  /* Second array access */
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
        int *row_ptr = arr + r * cols;
        for (int c = 0; c < cols; ++c) {
            total += *row_ptr;  /* Base + 0 offset within inner loop */
            row_ptr++;
        }
    }
    return total;
}

/* Helper to initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 100;
        static_array[i] = (i * 3) % 100;
    }
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (ITERATIONS / 2) : ITERATIONS;
    
    init_arrays();
    
    /* Local array for testing */
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
        result += pattern_a_simple_array(local_array, SIZE);
        result += pattern_b_explicit_pointer(local_array, SIZE);
        result += pattern_c_struct_pointer(struct_array, SIZE);
        result += pattern_d_global_pointer();
        result += pattern_e_conditional_pointer(local_array, SIZE, 50);
        result += pattern_f_mixed_access(local_array, static_array, SIZE);
        result += pattern_g_nested_loops(local_array, 16, 16);
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(result) : "memory");
    
    return result % 256;  /* Return small value to avoid overflow */
}
