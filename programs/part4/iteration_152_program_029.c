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
    int padding[3];  /* Ensure non-trivial size */
};

int pattern_c_struct_pointer(void) {
    struct Data arr[SIZE];
    struct Data *ptr = arr;
    int result = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; ++i) {
        arr[i].value = i;
    }
    
    /* Traverse with pointer */
    for (int i = 0; i < SIZE; ++i) {
        result += ptr->value;  /* Base + 0 offset through struct pointer */
        ptr++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
int pattern_d_global_with_local(void) {
    int *local_ptr = global_array;
    int sum = 0;
    
    /* Force pointer into register */
    asm volatile("" : : "r"(local_ptr) : "memory");
    
    for (int i = 0; i < SIZE; ++i) {
        sum += *local_ptr;  /* Should be (mem (reg)) form */
        local_ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional_blocks(int flag) {
    int buffer[SIZE];
    int *ptr = buffer;
    int total = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; ++i) {
        buffer[i] = i;
    }
    
    for (int i = 0; i < SIZE; ++i) {
        if (flag) {
            total += *ptr;  /* Base + 0 offset in one path */
            ptr++;
        } else {
            total -= *ptr;  /* Same pattern in other path */
            ptr += 2;
        }
    }
    return total;
}

/* Pattern F: Nested loops with pointer reset */
int pattern_f_nested_loops(void) {
    int matrix[16][16];
    int sum = 0;
    
    /* Initialize matrix */
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    /* Access with pointer in inner loop */
    for (int i = 0; i < 16; ++i) {
        int *row_ptr = matrix[i];
        for (int j = 0; j < 16; ++j) {
            sum += *row_ptr;  /* Base + 0 offset */
            row_ptr++;
        }
    }
    return sum;
}

/* Pattern G: Pointer arithmetic with zero offset in expression */
int pattern_g_explicit_zero_offset(void) {
    int array[SIZE];
    int *ptr = array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Force (plus (reg) (const_int 0)) pattern */
        result += *(ptr + 0);  /* Explicit zero offset */
        ptr++;
    }
    return result;
}

/* Pattern H: Mixed increment/decrement patterns */
int pattern_h_mixed_inc_dec(void) {
    int data[SIZE];
    int *fwd = data;
    int *rev = data + SIZE - 1;
    int sum = 0;
    
    for (int i = 0; i < SIZE/2; ++i) {
        sum += *fwd;  /* Forward pointer with base + 0 */
        fwd++;
        sum += *rev;  /* Reverse pointer with base + 0 */
        rev--;
    }
    return sum;
}

/* Main driver that uses all patterns */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
    
    /* Run each pattern multiple times to ensure execution */
    for (int i = 0; i < ITERATIONS; ++i) {
        total += pattern_a_simple_array();
        total += pattern_b_pointer_arithmetic();
        total += pattern_c_struct_pointer();
        total += pattern_d_global_with_local();
        total += pattern_e_conditional_blocks(i & 1);
        total += pattern_f_nested_loops();
        total += pattern_g_explicit_zero_offset();
        total += pattern_h_mixed_inc_dec();
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(total) : "memory");
    
    return total > 0 ? 0 : 1;
}
