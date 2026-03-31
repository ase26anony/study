/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stddef.h>

#define SIZE 256
#define ITER 100

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
        int val = *p;    /* Base register with zero offset */
        p++;             /* Pointer increment after use */
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int pad[3];  /* Force non-trivial size */
};

__attribute__((noinline))
int pattern_c_struct_traversal(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer to be in register before increment */
        asm volatile("" : : "r"(sp) : "memory");
        result += sp->value;
        sp++;
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
        int val2 = *local_ptr;
        local_ptr++;
        sum += val1 + val2;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional(int *arr, int n, int threshold) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            /* Even indices: use then increment */
            int val = *ptr;
            ptr++;
            result += val;
        } else {
            /* Odd indices: increment then use */
            ptr++;
            int val = *ptr;
            result -= val;
        }
    }
    return result;
}

/* Pattern F: Mixed offset patterns */
__attribute__((noinline))
int pattern_f_mixed_offsets(int *arr, int n) {
    int sum = 0;
    int *base = arr;
    
    /* Access with explicit zero offset */
    sum += base[0];
    
    /* Force base to stay in register */
    int *alias = base;
    
    /* Multiple zero-offset accesses */
    for (int i = 0; i < n; i += 2) {
        sum += *alias;
        alias += 2;
    }
    
    return sum;
}

/* Pattern G: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_g_nested_loops(int *arr, int rows, int cols) {
    int total = 0;
    
    for (int r = 0; r < rows; r++) {
        int *row_ptr = arr + r * cols;
        
        for (int c = 0; c < cols; c++) {
            /* Base + 0 offset in inner loop */
            total += *row_ptr;
            row_ptr++;
        }
    }
    return total;
}

/* Helper to initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < SIZE; i++) {
        global_array[i] = i % 100;
        static_array[i] = (i * 3) % 100;
    }
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int iterations = ITER;
    if (argc > 1) {
        iterations = iterations * 2;  /* Create variability */
    }
    
    init_arrays();
    
    int total = 0;
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; i++) {
        total += pattern_a_simple_array(static_array, SIZE);
        total += pattern_b_pointer_arithmetic(global_array, SIZE);
        
        struct Data struct_arr[SIZE];
        for (int j = 0; j < SIZE; j++) {
            struct_arr[j].value = j;
        }
        total += pattern_c_struct_traversal(struct_arr, SIZE / 4);
        
        total += pattern_d_global_access();
        total += pattern_e_conditional(static_array, SIZE, 50);
        total += pattern_f_mixed_offsets(global_array, SIZE);
        total += pattern_g_nested_loops(static_array, 16, 16);
    }
    
    /* Use result to prevent optimization */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
