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
        /* Force pointer to be kept in register */
        asm volatile("" : : "r"(sp) : "memory");
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
        if (i % 2 == 0) {
            /* Even indices: use current pointer */
            result += *ptr;  /* Base + 0 offset */
            ptr++;
        } else {
            /* Odd indices: skip ahead */
            ptr += 2;
            result -= 1;
        }
    }
    return result;
}

/* Pattern F: Mixed operations to prevent optimization */
__attribute__((noinline))
int pattern_f_mixed_operations(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i += 2) {
        /* Multiple dereferences with the same base */
        int a = *p;      /* First load - base + 0 */
        p++;
        int b = *p;      /* Second load - potentially base + 0 after increment */
        p++;
        
        /* Complex enough to prevent elimination */
        sum += (a * b) + (a >> 3) - (b << 2);
    }
    return sum;
}

/* Pattern G: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_g_nested_loops(int *arr, int outer, int inner) {
    int total = 0;
    
    for (int i = 0; i < outer; ++i) {
        int *ptr = arr;  /* Reset pointer each outer iteration */
        
        for (int j = 0; j < inner; ++j) {
            total += *ptr;  /* Base + 0 offset */
            ptr++;
            
            /* Prevent dead code elimination */
            if (total < 0) total = 0;
        }
    }
    return total;
}

/* Initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 100;
        static_array[i] = (i * 3) % 100;
    }
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create runtime variability */
    int base_size = (argc > 1) ? (SIZE / 2) : SIZE;
    int iterations = (argc > 2) ? ITERATIONS : (ITERATIONS / 2);
    
    init_arrays();
    
    /* Local array for stack-based patterns */
    int local_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        local_array[i] = (i * 7) % 100;
    }
    
    /* Struct array */
    struct Data struct_array[SIZE / 4];
    for (int i = 0; i < SIZE / 4; ++i) {
        struct_array[i].value = i * 2;
    }
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; ++i) {
        result ^= pattern_a_simple_array(local_array, base_size);
        result ^= pattern_b_pointer_arithmetic(static_array, base_size);
        result ^= pattern_c_struct_traversal(struct_array, base_size / 4);
        result ^= pattern_d_global_pointer();
        result ^= pattern_e_conditional_blocks(local_array, base_size, 50);
        result ^= pattern_f_mixed_operations(static_array, base_size);
        result ^= pattern_g_nested_loops(local_array, 10, base_size / 10);
    }
    
    /* Return result to prevent optimization */
    return result % 256;
}
