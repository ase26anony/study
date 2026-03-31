/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stddef.h>

#define SIZE 256
#define ITERATIONS 100

/* Global array for Pattern D */
int global_array[SIZE];

/* Pattern A: Simple array loop */
int pattern_a_simple_array(int *array, int n) __attribute__((noinline));
int pattern_a_simple_array(int *array, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += array[i];  /* Should generate base + offset */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic */
int pattern_b_pointer_arithmetic(int *array, int n) __attribute__((noinline));
int pattern_b_pointer_arithmetic(int *array, int n) {
    int total = 0;
    int *p = array;
    int *end = p + n;
    
    while (p < end) {
        int val = *p;      /* Base register with zero offset */
        p++;               /* Pointer increment after use */
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int padding[3];  /* Force non-trivial structure size */
};

int pattern_c_struct_traversal(struct Data *arr, int n) __attribute__((noinline));
int pattern_c_struct_traversal(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        result += sp->value;  /* Base + 0 offset for struct field */
        sp++;                 /* Increment after use */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
int pattern_d_global_pointer(void) __attribute__((noinline));
int pattern_d_global_pointer(void) {
    int sum = 0;
    int *local_ptr = global_array;  /* Take address of global */
    
    for (int i = 0; i < SIZE; ++i) {
        /* Force pointer to stay in register with inline asm */
        asm volatile("" : : "r"(local_ptr));
        sum += *local_ptr;  /* Base register with zero offset */
        local_ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional(int *array, int n, int threshold) __attribute__((noinline));
int pattern_e_conditional(int *array, int n, int threshold) {
    int result = 0;
    int *ptr = array;
    
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            /* Even indices: use then increment */
            int val = *ptr;  /* Base + 0 offset */
            ptr++;
            result += val;
        } else {
            /* Odd indices: increment then use */
            ptr++;
            int val = *ptr;  /* Different offset pattern */
            result -= val;
        }
    }
    return result;
}

/* Pattern F: Nested loops with pointer reset */
int pattern_f_nested_reset(int *array, int rows, int cols) __attribute__((noinline));
int pattern_f_nested_reset(int *array, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; ++i) {
        int *row_ptr = array + i * cols;
        
        for (int j = 0; j < cols; ++j) {
            total += *row_ptr;  /* Base + 0 offset within inner loop */
            row_ptr++;
        }
    }
    return total;
}

/* Pattern G: Pointer with multiple uses */
int pattern_g_multiple_uses(int *array, int n) __attribute__((noinline));
int pattern_g_multiple_uses(int *array, int n) {
    int sum1 = 0, sum2 = 0;
    int *p = array;
    
    for (int i = 0; i < n; ++i) {
        /* First use: base + 0 offset */
        int val1 = *p;
        sum1 += val1;
        
        /* Second use: same base + 0 offset */
        int val2 = *p;
        sum2 += val2 * 2;
        
        p++;  /* Increment after both uses */
    }
    return sum1 + sum2;
}

/* Helper to initialize arrays */
void initialize_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 100;
    }
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int local_array[SIZE];
    struct Data struct_array[SIZE];
    int rows = 16, cols = 16;
    int matrix[rows * cols];
    
    /* Initialize data */
    initialize_arrays();
    for (int i = 0; i < SIZE; ++i) {
        local_array[i] = (i * 3) % 100;
        struct_array[i].value = i % 50;
    }
    for (int i = 0; i < rows * cols; ++i) {
        matrix[i] = i % 20;
    }
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? ITERATIONS : ITERATIONS / 2;
    int total = 0;
    
    /* Execute patterns multiple times */
    for (int iter = 0; iter < iterations; ++iter) {
        total += pattern_a_simple_array(local_array, SIZE);
        total += pattern_b_pointer_arithmetic(local_array, SIZE);
        total += pattern_c_struct_traversal(struct_array, SIZE);
        total += pattern_d_global_pointer();
        total += pattern_e_conditional(local_array, SIZE, 50);
        total += pattern_f_nested_reset(matrix, rows, cols);
        total += pattern_g_multiple_uses(local_array, SIZE);
        
        /* Modify data slightly each iteration */
        local_array[iter % SIZE] += 1;
    }
    
    /* Return result to prevent optimization */
    return total % 256;
}
