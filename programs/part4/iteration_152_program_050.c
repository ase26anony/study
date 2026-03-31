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
    int *p = arr;
    int *end = p + n;
    int total = 0;
    
    while (p < end) {
        int val = *p;  /* Direct dereference with base register */
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
    struct Data *ptr = arr;
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Access through struct pointer */
        asm volatile("" : : "r"(ptr) : "memory");  /* Force ptr in register */
        result += ptr->value;
        ptr++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_access(void) {
    int *local_ptr = global_array;
    int sum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Dereference global through local pointer */
        int val = *local_ptr;
        local_ptr++;
        sum += val;
        
        /* Also access static array */
        sum += static_array[i];
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_pointer(int *arr, int n, int threshold) {
    int *ptr = arr;
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            /* Even indices: use current pointer */
            sum += *ptr;
            ptr++;
        } else {
            /* Odd indices: use pointer with offset 0 */
            int *current = ptr;
            sum += *current;  /* Base register + 0 offset */
            /* Don't increment here */
        }
        
        /* Force pointer to stay in register across conditions */
        asm volatile("" : : "r"(ptr) : "memory");
    }
    return sum;
}

/* Pattern F: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_f_nested_reset(int *arr, int outer, int inner) {
    int total = 0;
    
    for (int i = 0; i < outer; ++i) {
        int *ptr = arr;  /* Reset pointer each outer iteration */
        
        for (int j = 0; j < inner; ++j) {
            /* Multiple accesses with same base */
            int val1 = *ptr;
            ptr++;
            int val2 = *ptr;  /* New base + 0 after increment */
            
            total += val1 + val2;
        }
    }
    return total;
}

/* Pattern G: Pointer arithmetic with zero offset */
__attribute__((noinline))
int pattern_g_explicit_zero_offset(int *arr, int n) {
    int sum = 0;
    int *base = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Create pattern: *(base + 0) */
        int *current = base + 0;  /* Should generate base + 0 */
        sum += *current;
        base++;  /* Increment base for next iteration */
    }
    return sum;
}

/* Pattern H: Mixed pointer and index access */
__attribute__((noinline))
int pattern_h_mixed_access(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i += 2) {
        /* Alternate between pointer and indexed access */
        sum += *ptr;        /* Pointer dereference */
        sum += arr[i + 1];  /* Indexed access */
        ptr += 2;
    }
    return sum;
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
    
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        struct_array[i].value = i * 3;
    }
    
    /* Determine sizes based on argc for variability */
    int size1 = (argc > 1) ? (SIZE / 2) : SIZE;
    int size2 = (argc > 2) ? (SIZE / 4) : (SIZE / 2);
    
    /* Execute all patterns multiple times */
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        result += pattern_a_simple_array(global_array, size1);
        result += pattern_b_pointer_arithmetic(global_array, size1);
        result += pattern_c_struct_traversal(struct_array, size2);
        result += pattern_d_global_access();
        result += pattern_e_conditional_pointer(global_array, size1, 10);
        result += pattern_f_nested_reset(global_array, 10, size2 / 10);
        result += pattern_g_explicit_zero_offset(global_array, size1);
        result += pattern_h_mixed_access(global_array, size1);
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(result) : "memory");
    return result > 0 ? 0 : 1;
}
