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
    int total = 0;
    int *p = arr;
    int *end = p + n;
    
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
    int padding[3];  /* Force non-trivial size */
};

__attribute__((noinline))
int pattern_c_struct_traversal(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        result += sp->value;  /* Base register + 0 offset */
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
        sum += *local_ptr;  /* Base register + 0 offset */
        local_ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_pointer(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer into register with asm */
        asm volatile("" : "+r"(ptr) : : "memory");
        
        int val = *ptr;  /* Base register + 0 offset */
        
        if (val > threshold) {
            sum += val;
            ptr++;  /* Increment on one path */
        } else {
            sum -= val;
            ptr++;  /* Increment on another path */
        }
    }
    return sum;
}

/* Pattern F: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_f_nested_loops(int *arr, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; ++i) {
        int *row_ptr = arr + i * cols;
        
        for (int j = 0; j < cols; ++j) {
            total += *row_ptr;  /* Base register + 0 offset */
            row_ptr++;
        }
    }
    return total;
}

/* Pattern G: Pointer with switch statement */
__attribute__((noinline))
int pattern_g_switch_pointer(int *arr, int n) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        switch (i % 3) {
            case 0:
                result += *ptr;  /* Base register + 0 offset */
                ptr++;
                break;
            case 1:
                result -= *ptr;  /* Base register + 0 offset */
                ptr++;
                break;
            case 2:
                result ^= *ptr;  /* Base register + 0 offset */
                ptr++;
                break;
        }
    }
    return result;
}

/* Pattern H: Multiple pointers in same function */
__attribute__((noinline))
int pattern_h_multiple_pointers(int *arr1, int *arr2, int n) {
    int sum1 = 0, sum2 = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        sum1 += *p1;  /* Base register + 0 offset */
        sum2 += *p2;  /* Another base register + 0 offset */
        p1++;
        p2++;
    }
    return sum1 + sum2;
}

/* Main function to drive all patterns */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
    
    /* Create struct array */
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        struct_array[i].value = i * 3;
    }
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (argc * 10) : ITERATIONS;
    int size = (argc > 2) ? (argc * 8) : SIZE;
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; ++i) {
        result += pattern_a_simple_array(global_array, size);
        result += pattern_b_pointer_arithmetic(global_array, size);
        result += pattern_c_struct_traversal(struct_array, size);
        result += pattern_d_global_pointer();
        result += pattern_e_conditional_pointer(global_array, size, size/2);
        result += pattern_f_nested_loops(global_array, 16, 16);
        result += pattern_g_switch_pointer(global_array, size);
        result += pattern_h_multiple_pointers(global_array, static_array, size);
    }
    
    /* Return result to prevent optimization */
    return result > 0 ? 0 : 1;
}
