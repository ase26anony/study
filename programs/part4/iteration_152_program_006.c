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
int pattern_b_pointer_arithmetic(int *arr, int n) {
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
int pattern_c_struct_traversal(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        result += sp->value;  /* Struct member access */
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
        sum += *local_ptr;  /* Dereference local pointer */
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
        /* Force pointer into register with asm */
        asm volatile("" : : "r"(ptr) : "memory");
        
        int val = *ptr;  /* Base register access */
        
        if (val > threshold) {
            result += val;
            ptr += 2;  /* Skip ahead on some paths */
        } else {
            result -= val;
            ptr++;     /* Normal increment */
        }
    }
    return result;
}

/* Pattern F: Multiple dereferences in same basic block */
__attribute__((noinline))
int pattern_f_multiple_derefs(int *arr, int n) {
    int sum1 = 0, sum2 = 0;
    int *p1 = arr;
    int *p2 = arr + n/2;
    
    for (int i = 0; i < n/2; ++i) {
        /* Two independent pointer dereferences */
        sum1 += *p1;  /* First base+0 access */
        sum2 += *p2;  /* Second base+0 access */
        p1++;
        p2++;
    }
    return sum1 + sum2;
}

/* Pattern G: Pointer with zero offset explicitly */
__attribute__((noinline))
int pattern_g_explicit_zero_offset(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Force compiler to consider ptr + 0 */
    for (int i = 0; i < n; ++i) {
        int *current = ptr + 0;  /* Explicit zero offset */
        sum += *current;         /* Should be (mem (reg)) */
        ptr++;
    }
    return sum;
}

/* Helper to initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 100;
        static_array[i] = (i * 3) % 100;
    }
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int iterations = ITERATIONS;
    if (argc > 1) {
        iterations = iterations * 2;  /* Create variability */
    }
    
    init_arrays();
    
    int total = 0;
    
    /* Run all patterns multiple times */
    for (int i = 0; i < iterations; ++i) {
        total += pattern_a_simple_array(global_array, SIZE);
        total += pattern_b_pointer_arithmetic(static_array, SIZE);
        
        struct Data struct_array[SIZE];
        for (int j = 0; j < SIZE; ++j) {
            struct_array[j].value = j;
        }
        total += pattern_c_struct_traversal(struct_array, SIZE);
        
        total += pattern_d_global_pointer();
        total += pattern_e_conditional_blocks(global_array, SIZE, 50);
        total += pattern_f_multiple_derefs(global_array, SIZE);
        total += pattern_g_explicit_zero_offset(static_array, SIZE);
    }
    
    /* Use result to prevent optimization */
    if (total > 0) {
        return 0;
    }
    return 1;
}
