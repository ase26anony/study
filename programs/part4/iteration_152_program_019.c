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
        int val = *p;    /* Base + 0 offset when p points to array element */
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
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Access through struct pointer - base + 0 offset */
        int val = sp->value;
        sp++;  /* Increment after use */
        result += val;
        
        /* Force pointer to stay in register */
        asm volatile("" : : "r"(sp) : "memory");
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
        int val = *local_ptr;
        local_ptr++;  /* Increment after use */
        sum += val;
        
        /* Prevent optimization of pointer */
        asm volatile("" : : "r"(local_ptr) : "memory");
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_blocks(int *arr, int n, int threshold) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Simple dereference with base + 0 offset */
        int val = *ptr;
        
        if (val > threshold) {
            ptr++;  /* Increment on one path */
            result += val * 2;
        } else {
            ptr++;  /* Increment on another path */
            result += val;
        }
        
        /* Force pointer to be used in addressing mode */
        asm volatile("" : : "r"(ptr) : "memory");
    }
    return result;
}

/* Pattern F: Mixed access patterns in nested loops */
__attribute__((noinline))
int pattern_f_mixed_patterns(int *arr, int n) {
    int total = 0;
    int *outer_ptr = arr;
    
    for (int i = 0; i < n / 4; ++i) {
        int *inner_ptr = outer_ptr;
        
        for (int j = 0; j < 4; ++j) {
            /* Base + 0 offset access */
            int val = *inner_ptr;
            inner_ptr++;
            total += val;
            
            /* Prevent dead code elimination */
            asm volatile("" : : "r"(inner_ptr) : "memory");
        }
        
        outer_ptr += 4;
    }
    return total;
}

/* Pattern G: Pointer with zero offset in array initialization */
__attribute__((noinline))
void pattern_g_pointer_init(int *dest, int *src, int n) {
    int *d = dest;
    int *s = src;
    
    for (int i = 0; i < n; ++i) {
        /* Load with base + 0 offset */
        int val = *s;
        s++;
        
        /* Store with base + 0 offset */
        *d = val;
        d++;
        
        /* Keep pointers live */
        asm volatile("" : : "r"(s), "r"(d) : "memory");
    }
}

/* Helper to initialize arrays */
void initialize_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = SIZE - i;
    }
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (ITERATIONS / 2) : ITERATIONS;
    int size = (argc > 2) ? (SIZE / 2) : SIZE;
    
    initialize_arrays();
    
    /* Local array for testing */
    int local_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        local_array[i] = i * 2;
    }
    
    /* Struct array for pattern C */
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        struct_array[i].value = i * 3;
    }
    
    /* Execute patterns multiple times */
    for (int i = 0; i < iterations; ++i) {
        result += pattern_a_simple_array(local_array, size);
        result += pattern_b_pointer_arithmetic(static_array, size);
        result += pattern_c_struct_traversal(struct_array, size);
        result += pattern_d_global_pointer();
        result += pattern_e_conditional_blocks(local_array, size, size/2);
        result += pattern_f_mixed_patterns(static_array, size);
        
        /* Copy between arrays */
        int temp_array[SIZE];
        pattern_g_pointer_init(temp_array, local_array, size);
        result += temp_array[size/2];  /* Use result to prevent elimination */
    }
    
    /* Return result to prevent dead code elimination */
    return result % 256;
}
