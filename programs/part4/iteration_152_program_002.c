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
        result += sp->value;  /* Base + 0 offset for struct field access */
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
        /* Force pointer to stay in register with inline asm */
        asm volatile("" : : "r"(local_ptr) : "memory");
        sum += *local_ptr;  /* Base register + 0 offset */
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
        if (*ptr > threshold) {  /* Dereference in conditional */
            result += *ptr;
        } else {
            result -= *ptr;
        }
        
        /* Complex but analyzable pointer update */
        if (i % 2 == 0) {
            ptr++;
        } else {
            ptr += 2;
            i++;  /* Compensate for extra increment */
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
        /* Two independent dereferences with base+0 pattern */
        sum1 += *p1;
        sum2 += *p2;
        p1++;
        p2++;
    }
    return sum1 + sum2;
}

/* Pattern G: Pointer chain with intermediate variable */
__attribute__((noinline))
int pattern_g_pointer_chain(int *arr, int n) {
    int sum = 0;
    int *base = arr;
    
    for (int i = 0; i < n; ++i) {
        int *current = base;  /* Copy of base pointer */
        sum += *current;      /* Dereference through copied pointer */
        base++;
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

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (ITERATIONS / 2) : ITERATIONS;
    int size = (argc > 2) ? (SIZE / 2) : SIZE;
    
    initialize_arrays();
    
    /* Local array for stack-based testing */
    int local_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        local_array[i] = (i * 7) % 100;
    }
    
    /* Struct array */
    struct Data struct_array[SIZE/4];
    for (int i = 0; i < SIZE/4; ++i) {
        struct_array[i].value = i * 2;
    }
    
    /* Run each pattern multiple times to ensure coverage */
    for (int i = 0; i < iterations; ++i) {
        total += pattern_a_simple_array(local_array, size);
        total += pattern_b_pointer_arithmetic(local_array, size);
        total += pattern_c_struct_traversal(struct_array, size/4);
        total += pattern_d_global_pointer();
        total += pattern_e_conditional_blocks(local_array, size, 50);
        total += pattern_f_multiple_derefs(local_array, size);
        total += pattern_g_pointer_chain(local_array, size);
        
        /* Also test with static array */
        total += pattern_a_simple_array(static_array, size);
    }
    
    /* Use result to prevent optimization */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
