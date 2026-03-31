/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stddef.h>

#define SIZE 256
#define ITER 1000

/* Global arrays for different access patterns */
int global_array[SIZE];
static int static_array[SIZE];

/* Pattern A: Simple array loop */
int pattern_a_simple_array(void) {
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += global_array[i];  /* Base + offset, offset may become 0 */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with pre-increment */
int pattern_b_pointer_arithmetic(void) {
    int *p = global_array;
    int *end = p + SIZE;
    int total = 0;
    
    while (p < end) {
        int val = *p;      /* Load using current pointer value */
        p++;               /* Increment after use */
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
        result += ptr->value;  /* Access through struct pointer */
        ptr++;                 /* Increment struct pointer */
    }
    return result;
}

/* Pattern D: Local pointer with global array */
int pattern_d_local_global_pointer(void) {
    int *local_ptr = &global_array[0];  /* Start at base */
    int sum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Force pointer to stay in register with asm */
        asm volatile("" : : "r"(local_ptr) : "memory");
        sum += *local_ptr;  /* Base + 0 offset */
        local_ptr = &global_array[i];  /* Reset to different base */
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional_pointer(int flag) {
    int *ptr = static_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        if (flag) {
            result += *ptr;      /* Use pointer */
            ptr++;               /* Increment on true path */
        } else {
            result -= *ptr;      /* Use same pointer */
            /* No increment on false path */
        }
        /* Mix with array access to create complex pattern */
        result += static_array[i % 16];
    }
    return result;
}

/* Pattern F: Nested loops with pointer reset */
int pattern_f_nested_loops(void) {
    int sum = 0;
    int *ptr;
    
    for (int outer = 0; outer < 10; ++outer) {
        ptr = &global_array[0];  /* Reset to base each iteration */
        for (int inner = 0; inner < 16; ++inner) {
            sum += *ptr;         /* Base + 0 offset */
            ptr++;               /* Increment inner pointer */
        }
    }
    return sum;
}

/* Pattern G: Pointer with switch statement */
int pattern_g_switch_pointer(int mode) {
    int *ptr = global_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        switch (mode) {
            case 0:
                result += *ptr;  /* Load with base + 0 */
                ptr++;
                break;
            case 1:
                result -= *ptr;
                /* No increment */
                break;
            case 2:
                result ^= *ptr;
                ptr += 2;  /* Different increment */
                break;
        }
    }
    return result;
}

/* Helper to prevent optimization */
static int __attribute__((noinline)) 
use_pointer_dereference(int *p) {
    return *p;  /* Simple dereference */
}

/* Pattern H: Function call with pointer argument */
int pattern_h_function_call(void) {
    int *ptr = global_array;
    int sum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        sum += use_pointer_dereference(ptr);  /* Call forces register use */
        ptr++;
    }
    return sum;
}

/* Main driver that uses all patterns */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
    
    /* Run patterns multiple times to ensure execution */
    for (int iter = 0; iter < ITER; ++iter) {
        total += pattern_a_simple_array();
        total += pattern_b_pointer_arithmetic();
        total += pattern_c_struct_pointer();
        total += pattern_d_local_global_pointer();
        total += pattern_e_conditional_pointer(iter & 1);
        total += pattern_f_nested_loops();
        total += pattern_g_switch_pointer(iter % 3);
        total += pattern_h_function_call();
    }
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        asm volatile("" : : "r"(total) : "memory");
    }
    
    return total != 0 ? 0 : 1;
}
