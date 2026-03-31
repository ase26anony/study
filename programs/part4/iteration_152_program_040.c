/* test_autoinc.c - Test program for GCC auto-increment/decrement optimization */
#include <stddef.h>

#define SIZE 256
#define ITER 100

/* Global arrays for different access patterns */
int global_array[SIZE];
static int static_array[SIZE];

/* Pattern A: Simple array loop */
int pattern_a_simple_array(void) {
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += global_array[i];  /* Base + offset, may become base + 0 */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with pre-increment */
int pattern_b_pointer_arithmetic(void) {
    int *p = global_array;
    int *end = p + SIZE;
    int total = 0;
    
    while (p < end) {
        int val = *p;      /* Load with base + 0 */
        p++;               /* Post-increment */
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int pad[3];  /* Ensure non-trivial size */
};

int pattern_c_struct_pointer(void) {
    static struct Data data_array[SIZE];
    struct Data *ptr = data_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        result += ptr->value;  /* Base + 0 offset to struct field */
        ptr++;                 /* Post-increment by struct size */
    }
    return result;
}

/* Pattern D: Global pointer with local copy and complex flow */
int pattern_d_global_local_copy(void) {
    int *local_ptr = global_array;
    int sum = 0;
    
    /* Force pointer into register with asm */
    asm volatile("" : : "r"(local_ptr));
    
    for (int i = 0; i < SIZE; i += 2) {
        /* Two consecutive loads with same base */
        int val1 = *local_ptr;       /* First load: base + 0 */
        local_ptr++;
        int val2 = *local_ptr;       /* Second load: new base + 0 */
        sum += val1 + val2;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional_blocks(int flag) {
    int *ptr = static_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        if (flag & 1) {
            result += *ptr;     /* Load in conditional path */
            ptr++;              /* Modification in same block */
        } else {
            result -= *ptr;     /* Another load */
            ptr += 2;           /* Different increment */
        }
        flag >>= 1;
    }
    return result;
}

/* Pattern F: Nested loops with pointer reset */
int pattern_f_nested_loops(void) {
    int *outer_ptr = global_array;
    int outer_sum = 0;
    
    for (int i = 0; i < SIZE/16; ++i) {
        int *inner_ptr = outer_ptr;
        int inner_sum = 0;
        
        for (int j = 0; j < 16; ++j) {
            inner_sum += *inner_ptr;  /* Base + 0 in inner loop */
            inner_ptr++;
        }
        
        outer_sum += inner_sum;
        outer_ptr += 16;
    }
    return outer_sum;
}

/* Pattern G: Pointer arithmetic with zero offset calculation */
int pattern_g_explicit_zero_offset(void) {
    int *base = global_array;
    int sum = 0;
    
    /* Create (plus (reg) (const_int 0)) pattern explicitly */
    for (int i = 0; i < SIZE; ++i) {
        int *ptr = base + i;    /* Calculate offset */
        ptr = ptr + 0;          /* Add zero offset explicitly */
        sum += *ptr;            /* Load with base + 0 */
    }
    return sum;
}

/* Pattern H: Switch statement with different pointer updates */
int pattern_h_switch_pattern(int mode) {
    int *ptr = global_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        switch (mode) {
            case 0:
                result += *ptr;     /* Load then increment */
                ptr++;
                break;
            case 1:
                result -= *ptr;     /* Load then larger increment */
                ptr += 2;
                break;
            case 2:
                result ^= *ptr;     /* Load with no increment */
                break;
            default:
                result |= *ptr;     /* Load with decrement */
                ptr--;
                break;
        }
        mode = (mode + 1) & 3;
    }
    return result;
}

/* Main function to drive all patterns */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = SIZE - i;
    }
    
    /* Run each pattern multiple times */
    for (int iter = 0; iter < ITER; ++iter) {
        total += pattern_a_simple_array();
        total += pattern_b_pointer_arithmetic();
        total += pattern_c_struct_pointer();
        total += pattern_d_global_local_copy();
        total += pattern_e_conditional_blocks(iter);
        total += pattern_f_nested_loops();
        total += pattern_g_explicit_zero_offset();
        total += pattern_h_switch_pattern(iter & 3);
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(total));
    
    return total > 0 ? 0 : 1;
}
