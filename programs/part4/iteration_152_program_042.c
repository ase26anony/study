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
    int padding[3];
};

__attribute__((noinline))
int pattern_c_struct_pointer(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer to be in register before dereference */
        asm volatile("" : : "r"(sp) : "memory");
        result += sp->value;  /* Base register access */
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
            /* Even indices: simple dereference */
            result += *ptr;
            ptr++;
        } else {
            /* Odd indices: dereference with offset 0 */
            int *current = ptr;
            result -= *current;  /* Base register access */
            ptr += 2;
        }
    }
    return result;
}

/* Pattern F: Mixed pointer usage with volatile to prevent optimization */
__attribute__((noinline))
int pattern_f_mixed_access(int *arr, int n) {
    volatile int *vptr = arr;
    int *reg_ptr = (int *)vptr;
    int sum = 0;
    
    for (int i = 0; i < n; i += 2) {
        /* Force base register usage */
        int val = reg_ptr[0];  /* Should be (mem (reg)) */
        sum += val;
        reg_ptr += 2;
    }
    return sum;
}

/* Pattern G: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_g_nested_reset(int *arr, int outer, int inner) {
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        int *ptr = arr;
        for (int i = 0; i < inner; i++) {
            /* Simple dereference with base register */
            total += *ptr;
            ptr++;
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
    int iterations = ITERATIONS;
    if (argc > 1) {
        iterations = (argv[1][0] - '0') * 10;
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    initialize_arrays();
    
    int total_result = 0;
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; i++) {
        total_result += pattern_a_simple_array(static_array, SIZE);
        total_result += pattern_b_explicit_pointer(global_array, SIZE);
        
        struct Data data_array[SIZE];
        for (int j = 0; j < SIZE; j++) {
            data_array[j].value = j;
        }
        total_result += pattern_c_struct_pointer(data_array, SIZE / 4);
        
        total_result += pattern_d_global_pointer();
        total_result += pattern_e_conditional(static_array, SIZE, 50);
        total_result += pattern_f_mixed_access(global_array, SIZE);
        total_result += pattern_g_nested_reset(static_array, 10, SIZE / 10);
    }
    
    /* Use result to prevent dead code elimination */
    if (total_result > 0) {
        return 0;
    } else {
        return 1;
    }
}
