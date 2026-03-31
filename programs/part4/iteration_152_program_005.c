/* test_autoinc.c - Test program for auto-increment/decrement optimization coverage */

#include <stddef.h>

#define SIZE 256
static int global_array[SIZE];

/* Pattern A: Simple array loop with index */
__attribute__((noinline))
static int pattern_a_simple_array(void) {
    int local_array[SIZE];
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        local_array[i] = i;
        global_array[i] = i * 2;
    }
    
    /* This should generate base+0 addressing when accessing array[0] */
    sum += local_array[0];
    sum += global_array[0];
    
    /* Loop with array indexing - may generate base+offset */
    for (int i = 0; i < SIZE; i++) {
        sum += local_array[i];
    }
    
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with post-increment */
__attribute__((noinline))
static int pattern_b_pointer_arithmetic(void) {
    int local_array[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        local_array[i] = i + 1;
    }
    
    int *ptr = local_array;
    int *end = ptr + SIZE;
    
    /* Critical pattern: dereference ptr before increment */
    while (ptr < end) {
        int val = *ptr;  /* This should generate (mem (reg)) pattern */
        ptr++;           /* Post-increment */
        sum += val * val;
        
        /* Force ptr to stay in register */
        asm volatile("" : : "r"(ptr) : "memory");
    }
    
    return sum;
}

/* Pattern C: Struct pointer traversal */
__attribute__((noinline))
static int pattern_c_struct_traversal(void) {
    struct Data {
        int value;
        int padding[3];
    };
    
    struct Data data_array[SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        data_array[i].value = i * 3;
    }
    
    struct Data *sp = data_array;
    struct Data *send = sp + SIZE;
    
    while (sp < send) {
        /* Access through struct pointer - base+0 offset */
        int val = sp->value;
        sp++;
        sum += val;
        
        /* Mix with conditional to create complex flow */
        if (val % 2 == 0) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Pattern D: Global pointer with local copy and multiple uses */
__attribute__((noinline))
static int pattern_d_global_pointer(void) {
    static int initialized = 0;
    if (!initialized) {
        for (int i = 0; i < SIZE; i++) {
            global_array[i] = i * 4;
        }
        initialized = 1;
    }
    
    int sum = 0;
    int *global_ptr = &global_array[0];  /* Base address */
    
    /* Multiple dereferences of same base pointer */
    sum += *global_ptr;           /* First deref: base+0 */
    sum += *(global_ptr + 1);     /* Different offset */
    sum += *global_ptr;           /* Second deref: same base+0 */
    
    /* Loop with pointer increment */
    int *loop_ptr = global_ptr;
    for (int i = 0; i < 10; i++) {
        int val = *loop_ptr;      /* Should be base+0 in each iteration */
        loop_ptr++;
        sum += val;
    }
    
    return sum;
}

/* Pattern E: Pointer in conditional blocks with complex flow */
__attribute__((noinline))
static int pattern_e_conditional_flow(int mode) {
    int buffer[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        buffer[i] = i * 5;
    }
    
    int *ptr = buffer;
    int *end = ptr + SIZE;
    
    while (ptr < end) {
        int val;
        
        switch (mode % 3) {
            case 0:
                val = *ptr;        /* Load before increment */
                ptr++;
                sum += val;
                break;
            case 1:
                ptr++;
                val = *(ptr - 1);  /* Load after increment with offset */
                sum += val * 2;
                break;
            case 2:
                val = *ptr;        /* Load, then conditional increment */
                if (val % 2 == 0) {
                    ptr++;
                } else {
                    ptr += 2;
                }
                sum += val * 3;
                break;
        }
        
        mode = (mode * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return sum;
}

/* Pattern F: Nested loops with pointer reset */
__attribute__((noinline))
static int pattern_f_nested_loops(void) {
    int matrix[16][16];
    int sum = 0;
    
    /* Initialize matrix */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    /* Process rows with pointer arithmetic */
    for (int i = 0; i < 16; i++) {
        int *row_ptr = matrix[i];  /* Base of row */
        
        /* Process each element in row */
        for (int j = 0; j < 16; j++) {
            int val = *row_ptr;    /* Base+0 access */
            row_ptr++;             /* Move to next element */
            sum += val;
        }
    }
    
    return sum;
}

/* Main function to drive all patterns */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (argv[1][0] % 10) + 1 : 3;
    
    for (int i = 0; i < iterations; i++) {
        total += pattern_a_simple_array();
        total += pattern_b_pointer_arithmetic();
        total += pattern_c_struct_traversal();
        total += pattern_d_global_pointer();
        total += pattern_e_conditional_flow(i);
        total += pattern_f_nested_loops();
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    /* Return something based on computation */
    return total % 256;
}
