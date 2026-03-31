/* test_autoinc.c - Test program for GCC auto-inc-dec optimization pass */
#include <stddef.h>

#define ARRAY_SIZE 256

/* Global arrays for different access patterns */
int global_array[ARRAY_SIZE];
static int static_array[ARRAY_SIZE];

/* Pattern A: Simple array loop with index */
int pattern_a_simple_array(void) {
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        sum += global_array[i];  /* Base + offset access */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with pre-increment */
int pattern_b_pointer_arithmetic(void) {
    int *p = static_array;
    int *end = p + ARRAY_SIZE;
    int total = 0;
    
    while (p < end) {
        int val = *p;      /* Base + 0 offset access */
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
    static struct Data data_array[100];
    struct Data *ptr = data_array;
    int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        result += ptr->value;  /* Base + 0 offset through struct pointer */
        ptr++;                 /* Increment after use */
    }
    return result;
}

/* Pattern D: Global pointer with local copy and complex flow */
int pattern_d_complex_flow(int condition) {
    int *local_ptr = global_array;
    int sum = 0;
    
    /* Create conditional flow to complicate analysis */
    if (condition > 0) {
        for (int i = 0; i < ARRAY_SIZE/2; ++i) {
            sum += *local_ptr;  /* Base + 0 offset */
            local_ptr++;
        }
    } else {
        local_ptr = &global_array[ARRAY_SIZE/2];
        for (int i = 0; i < ARRAY_SIZE/2; ++i) {
            sum -= *local_ptr;  /* Base + 0 offset from different start */
            local_ptr++;
        }
    }
    
    /* Force pointer to stay in register */
    asm volatile("" : : "r"(local_ptr));
    return sum;
}

/* Pattern E: Nested loops with pointer reset */
int pattern_e_nested_loops(void) {
    int matrix[10][10];
    int sum = 0;
    
    /* Initialize matrix */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Access with pointer in inner loop */
    for (int i = 0; i < 10; ++i) {
        int *row_ptr = matrix[i];  /* Get pointer to row start */
        for (int j = 0; j < 10; ++j) {
            sum += *row_ptr;  /* Base + 0 offset */
            row_ptr++;        /* Increment to next element */
        }
    }
    return sum;
}

/* Pattern F: Mixed access patterns in same function */
int pattern_f_mixed_patterns(void) {
    int array1[100], array2[100];
    int *p1 = array1;
    int *p2 = array2;
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        array1[i] = i;
        array2[i] = 100 - i;
    }
    
    /* Alternate between two pointers */
    for (int i = 0; i < 50; ++i) {
        result += *p1;  /* Base + 0 from p1 */
        p1++;
        result += *p2;  /* Base + 0 from p2 */
        p2++;
    }
    
    return result;
}

/* Pattern G: Pointer chain with intermediate variable */
int pattern_g_pointer_chain(void) {
    int buffer[128];
    int *ptr = buffer;
    int *current;
    int sum = 0;
    
    for (int i = 0; i < 128; ++i) {
        buffer[i] = i * 2;
    }
    
    for (int i = 0; i < 128; ++i) {
        current = ptr;      /* Copy pointer to intermediate */
        sum += *current;    /* Base + 0 through intermediate */
        ptr++;              /* Increment original pointer */
    }
    
    return sum;
}

/* Main function to exercise all patterns */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = ARRAY_SIZE - i;
    }
    
    /* Run all patterns with varying conditions based on argc */
    total += pattern_a_simple_array();
    total += pattern_b_pointer_arithmetic();
    total += pattern_c_struct_traversal();
    total += pattern_d_complex_flow(argc);
    total += pattern_e_nested_loops();
    total += pattern_f_mixed_patterns();
    total += pattern_g_pointer_chain();
    
    /* Use result to prevent optimization */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
