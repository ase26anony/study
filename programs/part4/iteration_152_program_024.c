/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stddef.h>

#define SIZE 256
#define ITERATIONS 100

/* Global array for Pattern D */
int global_array[SIZE];

/* Pattern A: Simple array loop */
int pattern_a_simple_array(void) {
    int array[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i;
    }
    
    /* Simple array access with base + offset */
    for (int i = 0; i < SIZE; ++i) {
        sum += array[i];  /* Should generate base + 0 offset when i=0 */
    }
    
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with pre-increment */
int pattern_b_pointer_arithmetic(void) {
    int array[SIZE];
    int total = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i * 2;
    }
    
    int *p = array;
    int *end = p + SIZE;
    
    /* Pointer traversal with dereference before increment */
    while (p < end) {
        int val = *p;      /* Base register + 0 offset */
        p++;               /* Pointer increment after use */
        total += val * val;
        
        /* Force p to stay in register */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    return total;
}

/* Pattern C: Struct pointer traversal */
struct S {
    int data;
    int padding[3];  /* Ensure non-trivial size */
};

int pattern_c_struct_pointer(void) __attribute__((noinline));
int pattern_c_struct_pointer(void) {
    struct S arr_s[SIZE];
    int result = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        arr_s[i].data = i * 3;
    }
    
    struct S *sp = arr_s;
    for (int i = 0; i < SIZE; ++i) {
        result += sp->data;  /* Base + 0 offset through struct pointer */
        sp++;                /* Post-increment opportunity */
    }
    
    return result;
}

/* Pattern D: Global pointer with local copy */
int pattern_d_global_pointer(void) __attribute__((noinline));
int pattern_d_global_pointer(void) {
    int local_sum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < SIZE; i++) {
        global_array[i] = i * 4;
    }
    
    int *global_ptr = &global_array[0];  /* Explicit base + 0 offset */
    int *end_ptr = global_ptr + SIZE;
    
    while (global_ptr < end_ptr) {
        int val = *global_ptr;  /* Base register (global_ptr) + 0 offset */
        local_sum += val;
        global_ptr++;
        
        /* Prevent optimization of pointer */
        asm volatile("" : : "r"(global_ptr) : "memory");
    }
    
    return local_sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional_blocks(int flag) __attribute__((noinline));
int pattern_e_conditional_blocks(int flag) {
    int array[SIZE];
    int result = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i * 5;
    }
    
    int *ptr = array;
    int *end = ptr + SIZE;
    
    while (ptr < end) {
        int val = *ptr;  /* Base + 0 offset */
        
        if (flag) {
            result += val * 2;
            ptr += 2;    /* Modified on true path */
        } else {
            result += val;
            ptr++;       /* Modified on false path */
        }
        
        /* Complex enough to prevent optimization */
        if (val % 2 == 0) {
            result -= 1;
        }
    }
    
    return result;
}

/* Pattern F: Nested loops with pointer reset */
int pattern_f_nested_loops(void) {
    int matrix[16][16];
    int sum = 0;
    
    /* Initialize matrix */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    /* Access each row with pointer */
    for (int i = 0; i < 16; i++) {
        int *row_ptr = &matrix[i][0];  /* Base + 0 offset */
        
        for (int j = 0; j < 16; j++) {
            sum += *row_ptr;  /* Base register (row_ptr) + 0 offset */
            row_ptr++;        /* Post-increment */
        }
    }
    
    return sum;
}

/* Pattern G: Multiple pointers with same base */
int pattern_g_multiple_pointers(void) {
    int array[SIZE];
    int sum1 = 0, sum2 = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i * 7;
    }
    
    int *p1 = &array[0];      /* Base + 0 offset */
    int *p2 = &array[SIZE/2]; /* Different base */
    
    for (int i = 0; i < SIZE/2; i++) {
        sum1 += *p1;  /* Base register p1 + 0 offset */
        sum2 += *p2;  /* Base register p2 + 0 offset */
        p1++;
        p2++;
    }
    
    return sum1 + sum2;
}

/* Main driver that uses all patterns */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Run multiple iterations to ensure loops are executed */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total += pattern_a_simple_array();
        total += pattern_b_pointer_arithmetic();
        total += pattern_c_struct_pointer();
        total += pattern_d_global_pointer();
        total += pattern_e_conditional_blocks(iter % 2);
        total += pattern_f_nested_loops();
        total += pattern_g_multiple_pointers();
    }
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
