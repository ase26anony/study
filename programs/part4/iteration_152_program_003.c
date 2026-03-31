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
        sum += global_array[i];  /* Base + offset, may become base + 0 */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with post-increment */
int pattern_b_pointer_arithmetic(void) {
    int *p = static_array;
    int *end = p + SIZE;
    int total = 0;
    
    while (p < end) {
        int val = *p;  /* Base + 0 offset */
        p++;           /* Pointer increment after use */
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int padding[3];  /* Ensure non-trivial size */
};

int pattern_c_struct_pointer(void) {
    static struct Data data_array[SIZE];
    struct Data *ptr = data_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Force ptr to stay in register with asm */
        asm volatile("" : : "r"(ptr) : "memory");
        result += ptr->value;  /* Base + 0 offset */
        ptr++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy and complex flow */
int pattern_d_global_with_local_copy(void) {
    int *local_ptr = global_array;
    int sum = 0;
    volatile int *volatile_ptr = global_array;  /* Prevent some optimizations */
    
    for (int i = 0; i < SIZE; i += 2) {
        /* Multiple dereferences with same base */
        int val1 = *local_ptr;      /* First dereference - base + 0 */
        local_ptr++;
        int val2 = *volatile_ptr;   /* Force memory read */
        sum += val1 + val2;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional_blocks(int condition) {
    int *ptr = static_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        if (condition & 1) {
            result += *ptr;  /* Base + 0 in one branch */
            ptr++;
        } else {
            result -= *ptr;  /* Same base + 0 in other branch */
            ptr += 2;
        }
        condition >>= 1;
    }
    return result;
}

/* Pattern F: Nested loops with pointer reset */
int pattern_f_nested_loops(void) {
    int matrix[SIZE][4];
    int sum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        int *row_ptr = matrix[i];
        for (int j = 0; j < 4; ++j) {
            sum += *row_ptr;  /* Base + 0 offset */
            row_ptr++;
        }
    }
    return sum;
}

/* Pattern G: Pointer arithmetic with zero offset explicitly */
int pattern_g_explicit_zero_offset(void) {
    int *base_ptr = global_array;
    int sum = 0;
    
    /* Create pattern: *(base_ptr + 0) */
    for (int i = 0; i < SIZE; ++i) {
        int val = *(base_ptr + 0);  /* Explicit zero offset */
        sum += val;
        base_ptr++;  /* Increment after use */
    }
    return sum;
}

/* Pattern H: Switch statement with different pointer updates */
int pattern_h_switch_pattern(int mode) {
    int *ptr = static_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        switch (mode) {
            case 0:
                result += *ptr;  /* Base + 0 */
                ptr++;
                break;
            case 1:
                result -= *ptr;  /* Same base + 0 */
                ptr += 2;
                break;
            default:
                result ^= *ptr;  /* Same base + 0 */
                ptr += 3;
                break;
        }
        mode = (mode + 1) % 3;
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
    
    /* Run each pattern multiple times */
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        total += pattern_a_simple_array();
        total += pattern_b_pointer_arithmetic();
        total += pattern_c_struct_pointer();
        total += pattern_d_global_with_local_copy();
        total += pattern_e_conditional_blocks(iter);
        total += pattern_f_nested_loops();
        total += pattern_g_explicit_zero_offset();
        total += pattern_h_switch_pattern(iter % 3);
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(total) : "memory");
    return total > 0 ? 0 : 1;
}
