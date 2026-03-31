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
        result += sp->value;  /* Base + 0 offset for struct pointer */
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
        sum += *local_ptr;  /* Dereference with base register */
        local_ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_blocks(int *arr, int n, int threshold) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer to stay in register across conditional */
        asm volatile("" : : "r"(ptr) : "memory");
        
        if (i % 2 == 0) {
            result += *ptr;  /* Use pointer */
            ptr++;           /* Modify on some paths */
        } else {
            result -= *ptr;  /* Use pointer without modifying */
        }
        
        /* Ensure pointer is used again */
        if (i == threshold) {
            ptr = arr;  /* Reset pointer on some condition */
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
        /* Alternate between arrays */
        if (i % 3 == 0) {
            sum1 += *p1;  /* Base + 0 */
            p1++;
        } else if (i % 3 == 1) {
            sum2 += *p2;  /* Base + 0 */
            p2++;
        } else {
            /* Use both pointers without increment */
            sum1 += *p1;
            sum2 += *p2;
        }
    }
    return sum1 + sum2;
}

/* Pattern G: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_g_nested_loops(int *arr, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; ++i) {
        int *row_ptr = arr + i * cols;
        
        for (int j = 0; j < cols; ++j) {
            total += *row_ptr;  /* Base + 0 in inner loop */
            row_ptr++;
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
    int iterations = (argc > 1) ? (argc * 10) : ITERATIONS;
    int size = (argc > 2) ? (argc * 8) : SIZE;
    
    /* Initialize data */
    initialize_arrays();
    
    /* Local array for testing */
    int local_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        local_array[i] = (i * 7) % 100;
    }
    
    /* Struct array */
    struct Data struct_array[SIZE/4];
    for (int i = 0; i < SIZE/4; ++i) {
        struct_array[i].value = i * 2;
    }
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; ++i) {
        result += pattern_a_simple_array(local_array, size % SIZE);
        result += pattern_b_pointer_arithmetic(static_array, size % SIZE);
        result += pattern_c_struct_traversal(struct_array, (size/4) % (SIZE/4));
        result += pattern_d_global_pointer();
        result += pattern_e_conditional_blocks(local_array, size % SIZE, i % 10);
        result += pattern_f_mixed_patterns(local_array, static_array, size % SIZE);
        result += pattern_g_nested_loops(local_array, 8, size % 32);
    }
    
    /* Use result to prevent dead code elimination */
    return result % 1000;
}
