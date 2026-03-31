/* test_autoinc.c - Test program for auto-increment/decrement optimization */

#include <stddef.h>

#define SIZE 256
#define ITERATIONS 100

/* Global arrays for different access patterns */
int global_array[SIZE];
static int static_array[SIZE];

/* Pattern A: Simple array loop with index */
int pattern_a_simple_array(void) {
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += global_array[i];  /* Should generate base + offset */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with pre-increment */
int pattern_b_pointer_arithmetic(void) __attribute__((noinline));
int pattern_b_pointer_arithmetic(void) {
    int *p = static_array;
    int *end = p + SIZE;
    int total = 0;
    
    while (p < end) {
        int val = *p;      /* Base register + 0 offset */
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

int pattern_c_struct_traversal(void) __attribute__((noinline));
int pattern_c_struct_traversal(void) {
    static struct Data data_array[100];
    struct Data *ptr = data_array;
    int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        result += ptr->value;  /* Base register + 0 offset */
        ptr++;                 /* Post-increment opportunity */
    }
    return result;
}

/* Pattern D: Global pointer with local copy and complex flow */
int pattern_d_complex_flow(int condition) __attribute__((noinline));
int pattern_d_complex_flow(int condition) {
    int *local_ptr = global_array;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i += 2) {
        /* Force pointer into register with asm */
        asm volatile("" : "+r"(local_ptr) : : "memory");
        
        int val1 = *local_ptr;  /* Base + 0 */
        
        if (condition) {
            local_ptr++;  /* Increment on one path */
            int val2 = *local_ptr;  /* Different base */
            sum += val1 + val2;
        } else {
            sum += val1;
            local_ptr += 2;  /* Different increment */
        }
    }
    return sum;
}

/* Pattern E: Nested loops with pointer reset */
int pattern_e_nested_loops(void) __attribute__((noinline));
int pattern_e_nested_loops(void) {
    int matrix[16][16];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    /* Traverse with pointer */
    for (int i = 0; i < 16; ++i) {
        int *row_ptr = matrix[i];  /* Get row base */
        for (int j = 0; j < 16; ++j) {
            sum += *row_ptr;  /* Base + 0 */
            row_ptr++;        /* Next element */
        }
    }
    return sum;
}

/* Pattern F: Multiple pointers in same expression */
int pattern_f_multiple_pointers(void) __attribute__((noinline));
int pattern_f_multiple_pointers(void) {
    int arr1[SIZE], arr2[SIZE];
    int *p1 = arr1;
    int *p2 = arr2;
    int diff = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Both loads should have base+0 pattern */
        int a = *p1++;
        int b = *p2++;
        diff += a - b;
    }
    return diff;
}

/* Pattern G: Pointer with offset 0 in loop with computation */
int pattern_g_pointer_with_computation(void) __attribute__((noinline));
int pattern_g_pointer_with_computation(void) {
    volatile int* ptr = &global_array[0];  /* volatile to prevent optimization */
    int result = 0;
    
    for (int i = 0; i < ITERATIONS; ++i) {
        /* The access should be (mem (reg)) where reg holds &global_array[0] */
        int val = *ptr;
        result += (val * 3) / 2;
        
        /* Force compiler to keep the load by using asm */
        asm volatile("" : : "r"(val) : "memory");
    }
    return result;
}

/* Main function to drive all patterns */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
    
    /* Run patterns multiple times to ensure execution */
    for (int i = 0; i < (argc > 1 ? 10 : 3); ++i) {
        total += pattern_a_simple_array();
        total += pattern_b_pointer_arithmetic();
        total += pattern_c_struct_traversal();
        total += pattern_d_complex_flow(i % 2);
        total += pattern_e_nested_loops();
        total += pattern_f_multiple_pointers();
        total += pattern_g_pointer_with_computation();
    }
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        asm volatile("nop" : : : "memory");
    }
    
    return total != 0 ? 0 : 1;
}
