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

int pattern_c_struct_traversal(void) {
    static struct Data data_array[SIZE];
    struct Data *ptr = data_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        result += ptr->value;  /* Base + 0 offset through struct pointer */
        ptr++;                 /* Increment after use */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
int pattern_d_global_with_local(void) {
    int *local_ptr = global_array;
    int sum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Force pointer to stay in register with inline asm */
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
            int val = *ptr;  /* Base + 0 offset in conditional path */
            result += val;
            ptr++;
        } else {
            /* Different path with same pointer */
            result -= *ptr;  /* Another base + 0 offset access */
            ptr += 2;        /* Different increment */
        }
        condition >>= 1;
    }
    return result;
}

/* Pattern F: Nested loops with pointer reset */
int pattern_f_nested_loops(void) {
    int *ptr;
    int total = 0;
    
    for (int outer = 0; outer < ITERATIONS; ++outer) {
        ptr = global_array;  /* Reset pointer each iteration */
        for (int inner = 0; inner < SIZE; ++inner) {
            total += *ptr;   /* Base + 0 offset */
            ptr++;           /* Post-increment candidate */
        }
    }
    return total;
}

/* Pattern G: Multiple pointers in same function */
int pattern_g_multiple_pointers(void) {
    int *src = global_array;
    int *dst = static_array;
    int checksum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        int val = *src;      /* First base + 0 offset */
        *dst = val;          /* Second base + 0 offset */
        checksum += val;
        src++;
        dst++;
    }
    return checksum;
}

/* Pattern H: Pointer arithmetic with zero offset */
int pattern_h_explicit_zero_offset(void) {
    int *base = global_array;
    int sum = 0;
    
    /* Create pattern: *(base + 0) */
    for (int i = 0; i < SIZE; ++i) {
        sum += *(base + 0);  /* Explicit zero offset */
        base++;              /* Increment base */
    }
    return sum;
}

/* Main function to drive all patterns */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
    
    /* Run all patterns multiple times */
    for (int iter = 0; iter < (argc > 1 ? atoi(argv[1]) : 10); ++iter) {
        total += pattern_a_simple_array();
        total += pattern_b_pointer_arithmetic();
        total += pattern_c_struct_traversal();
        total += pattern_d_global_with_local();
        total += pattern_e_conditional_blocks(iter);
        total += pattern_f_nested_loops();
        total += pattern_g_multiple_pointers();
        total += pattern_h_explicit_zero_offset();
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(total) : "memory");
    return total > 0 ? 0 : 1;
}
