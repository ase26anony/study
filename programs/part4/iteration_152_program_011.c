/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stddef.h>

#define SIZE 256
#define ITERATIONS 100

/* Global arrays for different access patterns */
int global_array[SIZE];
static int static_array[SIZE];

/* Pattern A: Simple array loop with index */
__attribute__((noinline))
int pattern_a_simple_array(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];  /* Base + offset, offset may become 0 after optimization */
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
    int padding[3];  /* Ensure non-trivial size */
};

__attribute__((noinline))
int pattern_c_struct_traversal(struct Data *arr, int n) {
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
int pattern_d_global_access(void) {
    int sum = 0;
    int *local_ptr = global_array;  /* Take address of global */
    
    for (int i = 0; i < SIZE; ++i) {
        /* Multiple accesses to same base pointer */
        int val1 = *local_ptr;
        local_ptr++;
        int val2 = *local_ptr;  /* Different offset now */
        local_ptr--;
        sum += val1 + val2;
        local_ptr += 2;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional(int *arr, int n, int threshold) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (i < threshold) {
            /* Use pointer with base + 0 offset */
            int val = *ptr;
            ptr++;
            result += val;
        } else {
            /* Different pointer usage pattern */
            result += ptr[i - threshold];
        }
        
        /* Force pointer to stay in register across conditions */
        asm volatile("" : "+r"(ptr) : : "memory");
    }
    return result;
}

/* Pattern F: Mixed access patterns in loop */
__attribute__((noinline))
int pattern_f_mixed_access(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; i++) {
        /* Alternate between two pointers */
        if (i & 1) {
            sum += *p1;  /* Base + 0 offset */
            p1++;
        } else {
            sum += *p2;  /* Base + 0 offset */
            p2++;
        }
    }
    return sum;
}

/* Pattern G: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_g_nested_reset(int *arr, int rows, int cols) {
    int total = 0;
    
    for (int r = 0; r < rows; r++) {
        int *row_ptr = arr + r * cols;
        
        for (int c = 0; c < cols; c++) {
            /* Each inner loop iteration uses base + 0 offset */
            total += *row_ptr;
            row_ptr++;
        }
    }
    return total;
}

/* Helper to initialize arrays */
void initialize_arrays(void) {
    for (int i = 0; i < SIZE; i++) {
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
    
    initialize_arrays();
    
    /* Local array for stack-based access */
    int local_array[SIZE];
    for (int i = 0; i < SIZE; i++) {
        local_array[i] = i * 2;
    }
    
    /* Struct array */
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; i++) {
        struct_array[i].value = i * 5;
    }
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; i++) {
        result += pattern_a_simple_array(local_array, size);
        result += pattern_b_pointer_arithmetic(static_array, size);
        result += pattern_c_struct_traversal(struct_array, size / 4);
        result += pattern_d_global_access();
        result += pattern_e_conditional(local_array, size, size / 2);
        result += pattern_f_mixed_access(local_array, static_array, size);
        result += pattern_g_nested_reset(local_array, 16, 16);
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(result) : "memory");
    
    return result % 256;  /* Return small value to avoid overflow */
}
