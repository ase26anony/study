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
int pattern_b_explicit_pointer(int *arr, int n) {
    int total = 0;
    int *p = arr;
    int *end = p + n;
    
    while (p < end) {
        int val = *p;  /* Base register with zero offset */
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
        result += sp->value;  /* Base register with zero offset */
        sp++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_pointer(void) {
    int sum = 0;
    int *local_ptr = global_array;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Use volatile to prevent optimization of pointer */
        int *volatile vptr = local_ptr;
        sum += *vptr;  /* Base register with zero offset */
        local_ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            /* Even indices: use current pointer */
            sum += *ptr;  /* Base register with zero offset */
            ptr++;
        } else {
            /* Odd indices: skip ahead */
            ptr += 2;
        }
    }
    return sum;
}

/* Pattern F: Mixed offset patterns */
__attribute__((noinline))
int pattern_f_mixed_offsets(int *arr, int n) {
    int sum = 0;
    int *base = arr;
    
    /* Create pattern of accesses with different offsets */
    for (int i = 0; i < n; i += 4) {
        sum += base[0];  /* Zero offset */
        sum += base[1];  /* Non-zero offset */
        sum += base[2];  /* Non-zero offset */
        sum += base[3];  /* Non-zero offset */
        base += 4;
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
            total += *row_ptr;  /* Base register with zero offset */
            row_ptr++;
        }
    }
    return total;
}

/* Pattern H: Pointer chain */
__attribute__((noinline))
int pattern_h_pointer_chain(int *arr, int n) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = ptr1;  /* Create alias */
    
    for (int i = 0; i < n; ++i) {
        /* Use both pointers to create complex data flow */
        sum += *ptr1;
        ptr2 = ptr1 + 1;
        if (i % 3 == 0) {
            ptr1 = ptr2;
        } else {
            ptr1++;
        }
    }
    return sum;
}

/* Initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 100;
        static_array[i] = (i * 3) % 100;
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int iterations = ITERATIONS;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = ITERATIONS;
    }
    
    init_arrays();
    
    int total_result = 0;
    
    /* Run each pattern multiple times */
    for (int iter = 0; iter < iterations; ++iter) {
        /* Vary sizes to create different offset patterns */
        int size = (iter % 8) * 32 + 64;
        
        total_result += pattern_a_simple_array(global_array, size);
        total_result += pattern_b_explicit_pointer(static_array, size);
        
        struct Data struct_arr[SIZE];
        for (int i = 0; i < SIZE; ++i) {
            struct_arr[i].value = (i * 7) % 100;
        }
        total_result += pattern_c_struct_pointer(struct_arr, size);
        
        total_result += pattern_d_global_pointer();
        total_result += pattern_e_conditional(global_array, size, 50);
        total_result += pattern_f_mixed_offsets(static_array, size);
        total_result += pattern_g_nested_loops(global_array, 8, size/8);
        total_result += pattern_h_pointer_chain(static_array, size);
    }
    
    /* Use result to prevent dead code elimination */
    if (total_result == 0) {
        asm volatile("" : : "r"(total_result));
    }
    
    return total_result % 256;  /* Return non-zero to indicate execution */
}
