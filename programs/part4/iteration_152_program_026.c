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
        int val = *p;      /* Base + 0 offset */
        p++;               /* Pointer increment after use */
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int padding[3];  /* Ensure structure has size */
};

int pattern_c_struct_pointer(void) {
    static struct Data data_array[SIZE];
    struct Data *ptr = data_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        result += ptr->value;  /* Base + 0 offset through struct pointer */
        ptr++;                 /* Increment after use */
    }
    return result;
}

/* Pattern D: Local pointer with global array */
int pattern_d_local_pointer(void) {
    int *local_ptr = &global_array[0];  /* Explicit address of first element */
    int sum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Force pointer to stay in register with inline asm */
        asm volatile("" : : "r"(local_ptr) : "memory");
        sum += *local_ptr;  /* Base + 0 offset */
        local_ptr++;        /* Increment after use */
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional_pointer(int condition) {
    static int cond_array[SIZE * 2];
    int *ptr = cond_array;
    int total = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        if (condition & 1) {
            total += *ptr;      /* Base + 0 offset in one branch */
            ptr++;
        } else {
            total -= *ptr;      /* Same pointer, different use */
            ptr += 2;           /* Different increment */
        }
        condition >>= 1;
    }
    return total;
}

/* Pattern F: Nested loops with pointer reset */
int pattern_f_nested_loops(void) {
    int matrix[SIZE][4];
    int sum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        int *row_ptr = matrix[i];  /* Address of row start */
        for (int j = 0; j < 4; ++j) {
            sum += *row_ptr;       /* Base + 0 offset */
            row_ptr++;             /* Increment within inner loop */
        }
    }
    return sum;
}

/* Pattern G: Pointer with multiple uses */
int pattern_g_multiple_uses(void) {
    int *ptr = static_array;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i += 2) {
        int val1 = *ptr;       /* First use: base + 0 */
        ptr++;
        int val2 = *ptr;       /* Second use: new base + 0 */
        ptr++;
        sum += val1 + val2;
    }
    return sum;
}

/* Pattern H: Pointer arithmetic with zero offset */
int pattern_h_explicit_zero_offset(void) {
    int *base = global_array;
    int sum = 0;
    
    /* Create pattern: *(base + 0) */
    for (int i = 0; i < SIZE; ++i) {
        sum += *(base + 0);  /* Explicit zero offset */
        base++;              /* Change base for next iteration */
    }
    return sum;
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
    
    /* Run patterns multiple times to ensure execution */
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        total += pattern_a_simple_array();
        total += pattern_b_pointer_arithmetic();
        total += pattern_c_struct_pointer();
        total += pattern_d_local_pointer();
        total += pattern_e_conditional_pointer(iter);
        total += pattern_f_nested_loops();
        total += pattern_g_multiple_uses();
        total += pattern_h_explicit_zero_offset();
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(total));
    return total > 0 ? 0 : 1;
}
