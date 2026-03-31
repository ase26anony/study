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
        sum += arr[i];  /* Should generate base + offset */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with pre-increment */
__attribute__((noinline))
int pattern_b_pointer_arithmetic(int *arr, int n) {
    int *p = arr;
    int *end = p + n;
    int total = 0;
    
    while (p < end) {
        int val = *p;    /* Base register + 0 offset */
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
    struct Data *ptr = arr;
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        result += ptr->value;  /* Base + 0 offset through struct pointer */
        ptr++;                 /* Increment by struct size */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_access(void) {
    int *local_ptr = global_array;
    int sum = 0;
    
    /* Force pointer into register with asm */
    asm volatile("" : : "r"(local_ptr) : "memory");
    
    for (int i = 0; i < SIZE; ++i) {
        sum += *local_ptr;  /* Direct dereference of pointer in register */
        local_ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_pointer(int *arr, int n, int threshold) {
    int *ptr = arr;
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Conditional path that uses pointer */
        if (i % 2 == 0) {
            sum += *ptr;  /* Use pointer before potential modification */
            ptr++;
        } else {
            /* Different path that also uses pointer */
            sum -= *ptr;
            ptr += 2;  /* Different increment */
        }
        
        /* Force compiler to keep both paths */
        if (sum > threshold) {
            sum = threshold;
        }
    }
    return sum;
}

/* Pattern F: Multiple pointers in same basic block */
__attribute__((noinline))
int pattern_f_multiple_pointers(int *arr1, int *arr2, int n) {
    int *p1 = arr1;
    int *p2 = arr2;
    int diff = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Two independent memory accesses with base+0 */
        int val1 = *p1;
        int val2 = *p2;
        diff += val1 - val2;
        
        /* Increment both pointers */
        p1++;
        p2++;
    }
    return diff;
}

/* Pattern G: Pointer with loop-invariant base */
__attribute__((noinline))
int pattern_g_loop_invariant(int *base, int *offsets, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        /* Create addressing pattern: base + 0, then use offset */
        int *current = base + offsets[i];
        sum += *current;  /* current is in register with 0 offset */
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
int main(int argc, char **argv) {
    int iterations = ITERATIONS;
    if (argc > 1) {
        iterations = (argv[1][0] - '0') * ITERATIONS;
    }
    
    initialize_arrays();
    
    int total = 0;
    
    /* Execute each pattern multiple times */
    for (int i = 0; i < iterations; ++i) {
        total += pattern_a_simple_array(global_array, SIZE);
        total += pattern_b_pointer_arithmetic(static_array, SIZE);
        
        struct Data struct_array[SIZE];
        for (int j = 0; j < SIZE; ++j) {
            struct_array[j].value = j;
        }
        total += pattern_c_struct_traversal(struct_array, SIZE);
        
        total += pattern_d_global_access();
        total += pattern_e_conditional_pointer(global_array, SIZE, 1000);
        total += pattern_f_multiple_pointers(global_array, static_array, SIZE);
        
        int offsets[SIZE];
        for (int j = 0; j < SIZE; ++j) {
            offsets[j] = j % 10;
        }
        total += pattern_g_loop_invariant(global_array, offsets, SIZE);
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(total) : "memory");
    
    return total > 0 ? 0 : 1;
}
