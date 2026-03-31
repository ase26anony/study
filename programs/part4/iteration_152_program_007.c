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
        /* Force pointer to be in register before dereference */
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
        /* Multiple accesses to same base pointer */
        int val1 = *local_ptr;
        local_ptr++;
        sum += val1;
        
        /* Another access pattern */
        if (i % 2 == 0) {
            int *temp = local_ptr;
            int val2 = *temp;  /* Base + 0 offset */
            sum -= val2;
        }
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_blocks(int *arr, int n, int threshold) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex conditional to prevent optimization */
        if (i < threshold) {
            int val = *ptr;  /* Base + 0 in one branch */
            ptr++;
            result += val;
        } else {
            /* Different pointer usage in else branch */
            int *alt_ptr = ptr;
            int val = *alt_ptr;  /* Another base + 0 pattern */
            ptr += 2;
            result -= val;
        }
    }
    return result;
}

/* Pattern F: Mixed access patterns in nested loops */
__attribute__((noinline))
int pattern_f_nested_loops(int *arr, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = arr + i * cols;
        
        for (int j = 0; j < cols; j++) {
            /* Access with potential base + 0 offset */
            int val = *row_ptr;
            row_ptr++;
            total += val;
            
            /* Additional operation to prevent dead code elimination */
            asm volatile("" : "+r"(total) : : "memory");
        }
    }
    return total;
}

/* Pattern G: Pointer chasing with increment */
__attribute__((noinline))
int pattern_g_pointer_chasing(int *arr, int n) {
    int sum = 0;
    int *current = arr;
    
    for (int i = 0; i < n; i++) {
        /* Load, use, then increment pattern */
        int loaded = *current;
        sum += loaded;
        
        /* Force pointer to stay in register */
        asm volatile("" : : "r"(current) : "memory");
        
        current++;
        
        /* Another access to create multiple opportunities */
        if (i % 4 == 0) {
            int *temp = current;
            int extra = *temp;
            sum ^= extra;
        }
    }
    return sum;
}

/* Helper to initialize arrays */
void initialize_arrays(void) {
    for (int i = 0; i < SIZE; i++) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
}

/* Main driver that uses all patterns */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (ITERATIONS / 2) : ITERATIONS;
    int size = (argc > 2) ? (SIZE / 2) : SIZE;
    
    initialize_arrays();
    
    /* Local array for testing */
    int local_array[SIZE];
    for (int i = 0; i < SIZE; i++) {
        local_array[i] = i * 3;
    }
    
    /* Struct array */
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; i++) {
        struct_array[i].value = i * 4;
    }
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; i++) {
        result ^= pattern_a_simple_array(local_array, size);
        result ^= pattern_b_pointer_arithmetic(static_array, size);
        result ^= pattern_c_struct_traversal(struct_array, size);
        result ^= pattern_d_global_access();
        result ^= pattern_e_conditional_blocks(local_array, size, size/2);
        result ^= pattern_f_nested_loops(local_array, 16, 16);
        result ^= pattern_g_pointer_chasing(static_array, size);
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
    
    return result != 0 ? 0 : 1;
}
