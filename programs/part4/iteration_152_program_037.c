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
        /* Force pointer to be kept in register with asm */
        asm volatile("" : : "r"(sp) : "memory");
        result += sp->value;  /* Base + 0 offset for struct access */
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
        /* Use volatile to prevent optimization of the load */
        int val = *local_ptr;
        asm volatile("" : "+r"(val) : : "memory");
        sum += val;
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
        if (i % 2 == 0) {
            /* Even indices: dereference then increment */
            int val = *ptr;  /* Base + 0 */
            ptr++;
            result += val;
        } else {
            /* Odd indices: increment then dereference */
            ptr++;
            int val = *ptr;  /* Different base + 0 */
            result -= val;
        }
    }
    return result;
}

/* Pattern F: Multiple pointer dereferences in same basic block */
__attribute__((noinline))
int pattern_f_multiple_derefs(int *a, int *b, int n) {
    int sum = 0;
    int *pa = a;
    int *pb = b;
    
    for (int i = 0; i < n; ++i) {
        /* Two independent dereferences with base+0 pattern */
        int val1 = *pa;
        int val2 = *pb;
        sum += val1 * val2;
        pa++;
        pb++;
    }
    return sum;
}

/* Pattern G: Pointer with zero offset explicitly */
__attribute__((noinline))
int pattern_g_explicit_zero_offset(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Create pattern: *(ptr + 0) which should become base + 0 */
        int val = *(ptr + 0);
        sum += val;
        ptr += 1;  /* Increment by 1, not ptr++ to vary pattern */
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
        iterations = (argv[1][0] - '0') * 10;
        if (iterations < 10) iterations = ITERATIONS;
    }
    
    init_arrays();
    
    int total = 0;
    
    /* Execute all patterns multiple times */
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
        total += pattern_f_multiple_derefs(global_array, static_array, SIZE);
        total += pattern_g_explicit_zero_offset(global_array, SIZE);
    }
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        asm volatile("" : : "r"(total) : "memory");
    }
    
    return total % 256;  /* Return non-zero to indicate execution */
}
