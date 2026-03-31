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
        sum += arr[i];  /* Base + offset access */
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
        int val = *p;  /* Direct pointer dereference with base+0 */
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
        /* Access through struct pointer - base+0 offset */
        result += sp->value;
        sp++;  /* Increment after use */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_pointer(void) {
    int sum = 0;
    int *local_ptr = global_array;  /* Take address of global */
    
    for (int i = 0; i < SIZE; ++i) {
        /* Dereference local copy of global pointer */
        sum += *local_ptr;
        local_ptr++;  /* Increment after use */
        
        /* Force pointer to stay in register */
        asm volatile("" : : "r"(local_ptr) : "memory");
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_pointer(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Conditional modification of pointer */
        if (*ptr > threshold) {
            sum += *ptr;  /* Use current pointer */
            ptr++;        /* Increment in one path */
        } else {
            sum -= *ptr;  /* Use current pointer */
            /* No increment here - creates interesting flow */
        }
        /* Always increment to continue loop */
        ptr++;
    }
    return sum;
}

/* Pattern F: Multiple pointer dereferences in sequence */
__attribute__((noinline))
int pattern_f_multiple_derefs(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Two independent base+0 dereferences */
        int a = *p1;
        int b = *p2;
        sum += a * b;
        
        p1++;
        p2++;
    }
    return sum;
}

/* Pattern G: Pointer with zero offset explicitly */
__attribute__((noinline))
int pattern_g_explicit_zero_offset(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force base + 0 addressing */
        int *current = ptr + 0;  /* Explicit zero offset */
        sum += *current;
        ptr++;
    }
    return sum;
}

/* Helper to initialize arrays */
void initialize_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 100;
        static_array[i] = (i * 3) % 100;
    }
}

/* Main driver that uses all patterns */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (atoi(argv[1]) % SIZE) + 1 : ITERATIONS;
    
    initialize_arrays();
    
    /* Call all patterns multiple times */
    for (int i = 0; i < iterations; ++i) {
        result += pattern_a_simple_array(static_array, SIZE);
        result += pattern_b_pointer_arithmetic(global_array, SIZE);
        
        struct Data struct_array[SIZE];
        for (int j = 0; j < SIZE; ++j) {
            struct_array[j].value = j;
        }
        result += pattern_c_struct_traversal(struct_array, SIZE);
        
        result += pattern_d_global_pointer();
        result += pattern_e_conditional_pointer(static_array, SIZE, 50);
        result += pattern_f_multiple_derefs(global_array, static_array, SIZE);
        result += pattern_g_explicit_zero_offset(global_array, SIZE);
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(result) : "memory");
    
    return result % 255;  /* Return non-zero to indicate execution */
}
