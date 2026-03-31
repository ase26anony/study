/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stdlib.h>

#define SIZE 256
static int global_array[SIZE];

/* Pattern A: Simple array loop with index */
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
        int val = *p;    /* Base register with 0 offset */
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
        result += sp->value;  /* Base register access */
        sp++;                 /* Post-increment opportunity */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_access(void) {
    int sum = 0;
    int *local_ptr = global_array;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Force pointer to stay in register with asm */
        asm volatile("" : : "r"(local_ptr) : "memory");
        sum += *local_ptr;  /* Base register access */
        local_ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        int val = *ptr;  /* Base register access */
        
        if (val > threshold) {
            sum += val;
            ptr += 2;    /* Skip next element */
        } else {
            sum -= val;
            ptr++;       /* Normal increment */
        }
        
        /* Prevent tail merging optimizations */
        asm volatile("" : : "r"(ptr) : "memory");
    }
    return sum;
}

/* Pattern F: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_f_nested_loops(int *matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; ++i) {
        int *row_ptr = matrix + i * cols;
        
        for (int j = 0; j < cols; ++j) {
            total += *row_ptr;  /* Base register access */
            row_ptr++;          /* Post-increment opportunity */
        }
    }
    return total;
}

/* Pattern G: Pointer chain with intermediate variable */
__attribute__((noinline))
int pattern_g_pointer_chain(int *arr, int n) {
    int sum = 0;
    int *current = arr;
    
    for (int i = 0; i < n; ++i) {
        int *alias = current;    /* Create alias pointer */
        int val = *alias;        /* Access through alias */
        sum += val;
        current = alias + 1;     /* Update through alias */
    }
    return sum;
}

/* Main function with runtime variability */
int main(int argc, char **argv) {
    int test_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (test_size > SIZE) test_size = SIZE;
    
    /* Initialize test data */
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 64;
    }
    
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        struct_array[i].value = i % 32;
    }
    
    int matrix[SIZE][4];
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < 4; ++j) {
            matrix[i][j] = i + j;
        }
    }
    
    int result = 0;
    
    /* Execute all patterns */
    result += pattern_a_simple_array(global_array, test_size);
    result += pattern_b_pointer_arithmetic(global_array, test_size);
    result += pattern_c_struct_traversal(struct_array, test_size);
    result += pattern_d_global_access();
    result += pattern_e_conditional(global_array, test_size, 32);
    result += pattern_f_nested_loops(&matrix[0][0], test_size/4, 4);
    result += pattern_g_pointer_chain(global_array, test_size);
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        asm volatile("" : : "r"(result));
    }
    
    return result != 0 ? 0 : 1;
}
