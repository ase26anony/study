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

/* Pattern B: Explicit pointer arithmetic */
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
    int padding[3];  /* Force non-trivial size */
};

int pattern_c_struct_pointer(void) {
    static struct Data data_array[SIZE];
    struct Data *ptr = data_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Multiple loads from same base pointer */
        result += ptr->value;
        ptr++;  /* Increment after use */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
int pattern_d_global_with_local(void) {
    int *local_ptr = global_array;
    int sum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Force pointer to stay in register */
        asm volatile("" : : "r"(local_ptr) : "memory");
        sum += *local_ptr;  /* Base register + 0 offset */
        local_ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional_blocks(int condition) {
    int *ptr = static_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        if (condition & 1) {
            /* Use pointer in one branch */
            result += *ptr;  /* Base + 0 */
            ptr++;
        } else {
            /* Different operation in other branch */
            result -= *ptr;  /* Same pattern */
            ptr += 2;
        }
        condition >>= 1;
    }
    return result;
}

/* Pattern F: Nested loops with pointer reset */
int pattern_f_nested_loops(void) {
    int *outer_ptr = global_array;
    int total = 0;
    
    for (int i = 0; i < SIZE/16; ++i) {
        int *inner_ptr = outer_ptr;
        for (int j = 0; j < 16; ++j) {
            total += *inner_ptr;  /* Base + 0 */
            inner_ptr++;
        }
        outer_ptr += 16;
    }
    return total;
}

/* Pattern G: Pointer with multiple uses */
int pattern_g_multiple_uses(void) {
    int *ptr = static_array;
    int sum1 = 0, sum2 = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Two loads from same pointer before increment */
        int val1 = *ptr;      /* First load - base + 0 */
        sum1 += val1;
        
        int val2 = *ptr;      /* Second load - same pattern */
        sum2 += val2;
        
        ptr++;                /* Increment after both uses */
    }
    return sum1 + sum2;
}

/* Pattern H: Volatile pointer to force register usage */
int pattern_h_volatile_access(void) {
    volatile int *volatile_ptr = global_array;
    int sum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Volatile load should generate explicit memory access */
        sum += *volatile_ptr;
        volatile_ptr++;
    }
    return sum;
}

/* Main driver that exercises all patterns */
int main(int argc, char *argv[]) {
    int iterations = argc > 1 ? ITERATIONS : 10;
    int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
    
    /* Run patterns multiple times to ensure compiler sees hot loops */
    for (int i = 0; i < iterations; ++i) {
        total += pattern_a_simple_array();
        total += pattern_b_pointer_arithmetic();
        total += pattern_c_struct_pointer();
        total += pattern_d_global_with_local();
        total += pattern_e_conditional_blocks(i);
        total += pattern_f_nested_loops();
        total += pattern_g_multiple_uses();
        total += pattern_h_volatile_access();
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(total) : "memory");
    return total % 256;
}
