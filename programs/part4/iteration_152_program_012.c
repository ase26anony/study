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
        sum += arr[i];  /* Base + offset, may become base + 0 after optimization */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with pre-increment */
__attribute__((noinline))
int pattern_b_pointer_arithmetic(int *arr, int n) {
    int total = 0;
    int *p = arr;
    int *end = p + n;
    
    while (p < end) {
        int val = *p;  /* Direct dereference with base + 0 */
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
        result += sp->value;  /* Base + 0 offset for struct field */
        sp++;                 /* Pointer increment by struct size */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_access(void) {
    int sum = 0;
    int *local_ptr = global_array;  /* Take address of global */
    
    for (int i = 0; i < SIZE; ++i) {
        sum += *local_ptr;  /* Base + 0 dereference */
        local_ptr++;        /* Increment after use */
        
        /* Force pointer to stay in register */
        asm volatile("" : : "r"(local_ptr) : "memory");
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_blocks(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    int count = 0;
    
    while (count < n) {
        if (*ptr > threshold) {  /* Base + 0 dereference in condition */
            sum += *ptr;         /* Another base + 0 dereference */
            ptr += 2;            /* Skip ahead on condition */
            count += 2;
        } else {
            sum -= *ptr;         /* Base + 0 dereference */
            ptr++;               /* Normal increment */
            count++;
        }
        
        /* Prevent optimization of pointer value */
        asm volatile("" : "+r"(ptr) : : "memory");
    }
    return sum;
}

/* Pattern F: Multiple pointers with different bases */
__attribute__((noinline))
int pattern_f_multiple_pointers(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Two independent base + 0 dereferences */
        int val1 = *p1;
        int val2 = *p2;
        
        sum += val1 * val2;
        
        /* Increment both pointers */
        p1++;
        p2++;
        
        /* Force both pointers to be materialized */
        asm volatile("" : : "r"(p1), "r"(p2) : "memory");
    }
    return sum;
}

/* Pattern G: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_g_nested_loops(int *arr, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; ++i) {
        int *row_ptr = arr + i * cols;
        
        for (int j = 0; j < cols; ++j) {
            total += *row_ptr;  /* Base + 0 within inner loop */
            row_ptr++;          /* Increment within row */
        }
    }
    return total;
}

/* Pattern H: Pointer arithmetic with zero offset calculation */
__attribute__((noinline))
int pattern_h_explicit_zero_offset(int *arr, int n) {
    int sum = 0;
    int *base = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force explicit (reg + 0) calculation */
        int *current = base + 0;  /* Should optimize to just base */
        sum += *current;          /* Base + 0 dereference */
        base++;                   /* Increment base */
    }
    return sum;
}

/* Initialize arrays with pattern */
void init_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 100;
        static_array[i] = (i * 3) % 100;
    }
}

/* Main driver that exercises all patterns */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (ITERATIONS / 2) : ITERATIONS;
    int size = (argc > 2) ? (SIZE / 2) : SIZE;
    
    init_arrays();
    
    /* Local array for stack-based tests */
    int local_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        local_array[i] = (i * 7) % 100;
    }
    
    /* Struct array */
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        struct_array[i].value = i % 50;
    }
    
    /* Run each pattern multiple times to ensure coverage */
    for (int i = 0; i < iterations; ++i) {
        result ^= pattern_a_simple_array(local_array, size);
        result ^= pattern_b_pointer_arithmetic(local_array, size);
        result ^= pattern_c_struct_traversal(struct_array, size);
        result ^= pattern_d_global_access();
        result ^= pattern_e_conditional_blocks(local_array, size, 50);
        result ^= pattern_f_multiple_pointers(local_array, static_array, size);
        result ^= pattern_g_nested_loops(local_array, 16, 16);
        result ^= pattern_h_explicit_zero_offset(local_array, size);
        
        /* Modify arrays slightly each iteration */
        local_array[i % size] ^= result;
    }
    
    /* Return result to prevent dead code elimination */
    return result % 256;
}
