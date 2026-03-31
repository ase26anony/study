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
int pattern_b_explicit_pointer(int *array, int n) {
    int total = 0;
    int *p = array;
    int *end = p + n;
    
    while (p < end) {
        int val = *p;  /* Direct dereference with base register + 0 offset */
        p++;           /* Pointer increment after use */
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
int pattern_c_struct_pointer(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer to be in register before dereference */
        asm volatile("" : : "r"(sp) : "memory");
        result += sp->value;  /* Base + 0 offset for struct field */
        sp++;                 /* Post-increment */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_pointer(void) {
    int sum = 0;
    int *local_ptr = global_array;  /* Take address of global */
    
    for (int i = 0; i < SIZE; ++i) {
        /* Multiple dereferences to create different RTL patterns */
        int val1 = *local_ptr;
        local_ptr++;
        sum += val1;
        
        /* Another dereference to create more opportunities */
        if (i % 2 == 0) {
            int *temp = local_ptr - 1;  /* Create base+0 pattern */
            sum += *temp;
        }
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional(int *arr, int n, int condition) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (condition) {
            /* Path 1: Simple dereference */
            result += *ptr;
            ptr++;
        } else {
            /* Path 2: Dereference with offset 0 */
            int *current = ptr;
            result += *current * 2;
            ptr += 2;
        }
        
        /* Mix in another access pattern */
        if (i % 3 == 0) {
            int temp = *(ptr - 1);  /* Should create base + 0 */
            result -= temp;
        }
    }
    return result;
}

/* Pattern F: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_f_nested_loops(int *arr, int rows, int cols) {
    int total = 0;
    
    for (int r = 0; r < rows; r++) {
        int *row_ptr = arr + r * cols;
        
        for (int c = 0; c < cols; c++) {
            /* Inner loop: base + 0 pattern */
            total += *row_ptr;
            row_ptr++;
        }
    }
    return total;
}

/* Pattern G: Pointer arithmetic with zero offset */
__attribute__((noinline))
int pattern_g_zero_offset(int *base, int index) {
    /* Force computation of address with zero offset */
    int *ptr = base + index;
    ptr = ptr - index;  /* Should result in base + 0 */
    
    asm volatile("" : : "r"(ptr) : "memory");
    return *ptr;  /* Direct dereference of base register */
}

/* Helper to initialize arrays */
void initialize_arrays(void) {
    for (int i = 0; i < SIZE; i++) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (argc * 10) : ITERATIONS;
    int size = (argc > 2) ? (argc * 8) : SIZE / 2;
    
    if (size > SIZE) size = SIZE;
    
    initialize_arrays();
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; i++) {
        result += pattern_a_simple_array(global_array, size);
        result += pattern_b_explicit_pointer(static_array, size);
        
        struct Data struct_array[SIZE];
        for (int j = 0; j < SIZE; j++) {
            struct_array[j].value = j * 3;
        }
        result += pattern_c_struct_pointer(struct_array, size);
        
        result += pattern_d_global_pointer();
        result += pattern_e_conditional(global_array, size, i % 2);
        result += pattern_f_nested_loops(global_array, 8, size / 8);
        result += pattern_g_zero_offset(global_array, i % size);
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(result) : "memory");
    return result % 256;
}
