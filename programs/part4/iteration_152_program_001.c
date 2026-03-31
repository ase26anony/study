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
int pattern_b_pointer_arithmetic(int *array, int n) {
    int *p = array;
    int *end = p + n;
    int total = 0;
    
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
    int padding[3];  /* Ensure structure has size */
};

__attribute__((noinline))
int pattern_c_struct_traversal(struct Data *arr, int n) {
    struct Data *ptr = arr;
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        result += ptr->value;  /* Base + 0 offset for struct field access */
        ptr++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_with_local(void) {
    int *local_ptr = global_array;
    int sum = 0;
    
    /* Force pointer to stay in register */
    asm volatile("" : : "r"(local_ptr) : "memory");
    
    for (int i = 0; i < SIZE; ++i) {
        sum += *local_ptr;  /* Base register + 0 offset */
        local_ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_blocks(int *arr, int n, int threshold) {
    int *ptr = arr;
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Conditional modification of pointer */
        if (*ptr > threshold) {
            sum += *ptr;  /* Use current pointer value */
            ptr++;        /* Increment after use */
        } else {
            /* Different path with same pattern */
            sum -= *ptr;
            ptr++;
        }
    }
    return sum;
}

/* Pattern F: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_f_nested_loops(int *arr, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; ++i) {
        int *row_ptr = arr + i * cols;
        
        for (int j = 0; j < cols; ++j) {
            total += *row_ptr;  /* Base + 0 offset */
            row_ptr++;
        }
    }
    return total;
}

/* Pattern G: Pointer arithmetic with zero offset explicitly */
__attribute__((noinline))
int pattern_g_explicit_zero_offset(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Create pattern: *(ptr + 0) */
    for (int i = 0; i < n; ++i) {
        sum += *(ptr + 0);  /* Explicit zero offset */
        ptr = ptr + 1;      /* Separate increment */
    }
    return sum;
}

/* Pattern H: Mixed access patterns */
__attribute__((noinline))
int pattern_h_mixed_access(int *arr1, int *arr2, int n) {
    int *p1 = arr1;
    int *p2 = arr2;
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Alternate between two pointers */
        if (i % 2 == 0) {
            result += *p1;  /* Base + 0 offset */
            p1++;
        } else {
            result += *p2;  /* Another base + 0 offset */
            p2++;
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

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (ITERATIONS / 2) : ITERATIONS;
    int size = (argc > 2) ? (SIZE / 2) : SIZE;
    
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
        result += pattern_a_simple_array(local_array, size);
        result += pattern_b_pointer_arithmetic(static_array, size);
        result += pattern_c_struct_traversal(struct_array, size);
        result += pattern_d_global_with_local();
        result += pattern_e_conditional_blocks(local_array, size, 50);
        result += pattern_f_nested_loops(local_array, 16, 16);
        result += pattern_g_explicit_zero_offset(static_array, size);
        result += pattern_h_mixed_access(local_array, static_array, size);
        
        /* Prevent optimization of result */
        asm volatile("" : "+r"(result) : : "memory");
    }
    
    /* Return result to prevent dead code elimination */
    return result % 256;
}
