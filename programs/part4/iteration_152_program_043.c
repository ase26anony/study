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
    int padding[3];
};

__attribute__((noinline))
int pattern_c_struct_traversal(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        result += sp->value;  /* Struct field access */
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
        sum += *local_ptr;  /* Dereference local pointer copy */
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
        /* Force pointer into register with inline asm */
        asm volatile("" : "+r"(ptr) : : "memory");
        
        int val = *ptr;  /* Base + 0 dereference */
        
        if (val > threshold) {
            result += val;
            ptr++;  /* Increment on one path */
        } else {
            result -= val;
            ptr += 2;  /* Different increment on other path */
        }
    }
    return result;
}

/* Pattern F: Mixed access patterns to confuse optimizer */
__attribute__((noinline))
int pattern_f_mixed_access(int *arr1, int *arr2, int n) {
    int sum1 = 0, sum2 = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; i += 2) {
        /* Multiple dereferences with same base */
        sum1 += *p1;
        sum2 += *p2;
        
        /* Increment pointers */
        p1++;
        p2++;
        
        /* Another dereference after increment */
        if (i + 1 < n) {
            sum1 += *p1;
            sum2 += *p2;
        }
    }
    
    return sum1 + sum2;
}

/* Pattern G: Loop with pointer update in middle */
__attribute__((noinline))
int pattern_g_pointer_update_mid_loop(int *arr, int n) {
    int total = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Use current pointer value */
        int current = *ptr;
        
        /* Update pointer based on condition */
        if (current % 2 == 0) {
            ptr++;  /* Simple increment */
        } else {
            ptr += 2;  /* Larger increment */
        }
        
        total += current;
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
    int iterations = ITERATIONS;
    if (argc > 1) {
        iterations = (argv[1][0] - '0') * ITERATIONS;
    }
    
    init_arrays();
    
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; i++) {
        struct_array[i].value = i % 50;
    }
    
    int total_result = 0;
    
    /* Execute patterns multiple times */
    for (int iter = 0; iter < iterations; iter++) {
        total_result += pattern_a_simple_array(global_array, SIZE);
        total_result += pattern_b_pointer_arithmetic(static_array, SIZE);
        total_result += pattern_c_struct_traversal(struct_array, SIZE);
        total_result += pattern_d_global_pointer();
        total_result += pattern_e_conditional_blocks(global_array, SIZE, 50);
        total_result += pattern_f_mixed_access(global_array, static_array, SIZE);
        total_result += pattern_g_pointer_update_mid_loop(global_array, SIZE);
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(total_result));
    
    return total_result % 100;
}
