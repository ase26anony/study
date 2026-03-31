/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stddef.h>

#define SIZE 256
#define ITERATIONS 100

/* Global arrays for different access patterns */
int global_array[SIZE];
static int static_array[SIZE];

/* Pattern A: Simple array loop */
int pattern_a_simple_array(void) {
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += global_array[i];  /* Base + offset access */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic */
int pattern_b_pointer_arithmetic(void) {
    int *p = static_array;
    int *end = p + SIZE;
    int total = 0;
    
    while (p < end) {
        int val = *p;  /* Base register + 0 offset */
        p++;           /* Pointer increment after use */
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int padding[3];  /* Force non-trivial size */
};

struct Data struct_array[SIZE];

int pattern_c_struct_pointer(void) {
    struct Data *sp = struct_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        result += sp->value;  /* Base + 0 offset through struct pointer */
        sp++;                 /* Increment after use */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
int pattern_d_global_local_copy(void) {
    int *local_ptr = global_array;
    int sum = 0;
    
    /* Use volatile to prevent optimization of pointer */
    volatile int *volatile_ptr = local_ptr;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Force base+0 addressing */
        int val = *(volatile_ptr);
        asm volatile("" : : "r"(volatile_ptr));  /* Keep pointer in register */
        sum += val;
        volatile_ptr++;  /* Increment after use */
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional_blocks(int condition) {
    int *ptr = static_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        if (condition & 1) {
            int val = *ptr;  /* Base + 0 in conditional path */
            result += val;
            ptr++;           /* Increment in same block */
        } else {
            int val = *ptr;  /* Base + 0 in else path */
            result -= val;
            ptr++;           /* Increment in else block */
        }
        condition >>= 1;
    }
    return result;
}

/* Pattern F: Nested loops with pointer reset */
int pattern_f_nested_loops(void) {
    int sum = 0;
    int *ptr;
    
    for (int outer = 0; outer < 10; ++outer) {
        ptr = global_array;  /* Reset to base each iteration */
        for (int inner = 0; inner < SIZE/10; ++inner) {
            sum += *ptr;     /* Base + 0 offset */
            ptr++;           /* Post-increment */
        }
    }
    return sum;
}

/* Pattern G: Pointer arithmetic with zero offset explicitly */
int pattern_g_explicit_zero_offset(void) {
    int *base = global_array;
    int sum = 0;
    
    /* Create pattern: *(base + 0) */
    for (int i = 0; i < SIZE; ++i) {
        sum += *(base + 0);  /* Should generate base + 0 */
        base++;              /* Increment base register */
    }
    return sum;
}

/* Pattern H: Multiple pointers with independent increments */
int pattern_h_multiple_pointers(void) {
    int *p1 = global_array;
    int *p2 = static_array;
    int sum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        int val1 = *p1;  /* Base1 + 0 */
        int val2 = *p2;  /* Base2 + 0 */
        sum += val1 + val2;
        p1++;            /* Post-increment p1 */
        p2++;            /* Post-increment p2 */
    }
    return sum;
}

/* Pattern I: Pointer dereference in loop with computation */
int pattern_i_complex_dereference(void) {
    int *ptr = global_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Multiple uses of *ptr to encourage register holding */
        int val = *ptr;
        result += (val * val) / (val + 1);
        ptr++;
    }
    return result;
}

/* Pattern J: Switch statement with pointer increments */
int pattern_j_switch_increment(int mode) {
    int *ptr = static_array;
    int total = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        switch (mode) {
            case 0:
                total += *ptr;  /* Base + 0 in case 0 */
                ptr++;
                break;
            case 1:
                total -= *ptr;  /* Base + 0 in case 1 */
                ptr++;
                break;
            default:
                total ^= *ptr;  /* Base + 0 in default */
                ptr++;
                break;
        }
        mode = (mode + 1) % 3;
    }
    return total;
}

/* Initialize arrays with non-zero values */
void initialize_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 100;
        static_array[i] = (i * 3) % 100;
        struct_array[i].value = i % 50;
    }
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    int total = 0;
    
    initialize_arrays();
    
    /* Run each pattern multiple times to ensure execution */
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        total += pattern_a_simple_array();
        total += pattern_b_pointer_arithmetic();
        total += pattern_c_struct_pointer();
        total += pattern_d_global_local_copy();
        total += pattern_e_conditional_blocks(iter);
        total += pattern_f_nested_loops();
        total += pattern_g_explicit_zero_offset();
        total += pattern_h_multiple_pointers();
        total += pattern_i_complex_dereference();
        total += pattern_j_switch_increment(iter % 3);
    }
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
