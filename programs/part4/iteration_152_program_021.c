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
        sum += arr[i];  /* Base + offset access */
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
        int val = *p;    /* Base + 0 offset access */
        p++;             /* Pointer increment after use */
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
        result += sp->value;  /* Base + 0 offset through struct pointer */
        sp++;                 /* Pointer increment */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_pointer(void) {
    int sum = 0;
    int *local_ptr = global_array;  /* Take address of global */
    
    for (int i = 0; i < SIZE; ++i) {
        sum += *local_ptr;  /* Base + 0 offset from global pointer */
        local_ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_pointer(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + n;
    
    while (ptr < end) {
        int val = *ptr;  /* Base + 0 offset access */
        
        if (val > threshold) {
            sum += val;
            ptr += 2;    /* Skip next element on condition */
        } else {
            sum -= val;
            ptr++;       /* Normal increment */
        }
        
        /* Force pointer to stay in register */
        asm volatile("" : : "r"(ptr) : "memory");
    }
    return sum;
}

/* Pattern F: Mixed access patterns in nested loops */
__attribute__((noinline))
int pattern_f_mixed_access(int *arr, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; ++i) {
        int *row_ptr = arr + i * cols;
        
        for (int j = 0; j < cols; ++j) {
            /* Multiple base+0 accesses in inner loop */
            int a = *row_ptr;
            row_ptr++;
            
            int b = *row_ptr;
            row_ptr++;
            
            total += a * b;
        }
    }
    return total;
}

/* Pattern G: Pointer chain with intermediate variable */
__attribute__((noinline))
int pattern_g_pointer_chain(int *arr, int n) {
    int sum = 0;
    int *current = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Create chain of pointer operations */
        int *temp = current;
        int val = *temp;  /* Base + 0 offset through temp pointer */
        sum += val;
        
        current = temp + 1;  /* Update through temp */
    }
    return sum;
}

/* Initialize arrays with pattern */
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
    int iterations = (argc > 1) ? (ITERATIONS / 2) : ITERATIONS;
    int size = (argc > 2) ? (SIZE / 2) : SIZE;
    
    initialize_arrays();
    
    /* Create local array on stack */
    int local_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        local_array[i] = (i * 7) % 100;
    }
    
    /* Create struct array */
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        struct_array[i].value = i % 50;
    }
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; ++i) {
        result ^= pattern_a_simple_array(local_array, size);
        result ^= pattern_b_pointer_arithmetic(static_array, size);
        result ^= pattern_c_struct_traversal(struct_array, size / 4);
        result ^= pattern_d_global_pointer();
        result ^= pattern_e_conditional_pointer(local_array, size, 50);
        result ^= pattern_f_mixed_access(local_array, 16, 16);
        result ^= pattern_g_pointer_chain(static_array, size);
    }
    
    /* Use result to prevent dead code elimination */
    return result % 255;
}
