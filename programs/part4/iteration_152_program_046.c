/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stddef.h>

#define SIZE 256
#define ITERATIONS 100

/* Global arrays for different access patterns */
int global_array[SIZE];
static int static_array[SIZE];

/* Pattern A: Simple array loop with index */
__attribute__((noinline))
int pattern_a_simple_array(void) {
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += global_array[i];  /* Base + offset, offset may become 0 after optimization */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic */
__attribute__((noinline))
int pattern_b_pointer_arithmetic(void) {
    int *p = static_array;
    int *end = p + SIZE;
    int total = 0;
    
    while (p < end) {
        int val = *p;  /* Direct dereference with base register + 0 offset */
        p++;           /* Pointer increment after use */
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int padding[3];  /* Ensure structure has size */
};

__attribute__((noinline))
int pattern_c_struct_pointer(void) {
    static struct Data data_array[SIZE];
    struct Data *ptr = data_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Force pointer to be in register before dereference */
        asm volatile("" : : "r"(ptr) : "memory");
        result += ptr->value;  /* Base + 0 offset for struct access */
        ptr++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy and complex flow */
__attribute__((noinline))
int pattern_d_global_with_local_copy(void) {
    int *local_ptr = global_array;
    int sum = 0;
    
    /* Create multiple dereferences with the same base */
    for (int i = 0; i < SIZE; i += 4) {
        /* Multiple loads from same base address */
        sum += local_ptr[0];  /* Base + 0 offset */
        sum += local_ptr[1];
        sum += local_ptr[2];
        sum += local_ptr[3];
        local_ptr += 4;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_blocks(int condition) {
    int *ptr = static_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        if (condition & 1) {
            /* Load with base + 0 offset */
            int val = *ptr;
            result += val;
            ptr++;  /* Increment on this path */
        } else {
            /* Different offset calculation */
            result += ptr[i];
        }
        
        /* Mix with other operations to prevent optimization */
        condition = (condition * 1103515245 + 12345) & 0x7fffffff;
    }
    return result;
}

/* Pattern F: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_f_nested_loops(void) {
    int sum = 0;
    int *ptr;
    
    for (int outer = 0; outer < ITERATIONS; ++outer) {
        ptr = global_array;  /* Reset pointer each iteration */
        
        for (int inner = 0; inner < SIZE; ++inner) {
            /* Force register usage */
            register int *reg_ptr asm("r12") = ptr;
            asm volatile("" : "+r"(reg_ptr));
            
            sum += *reg_ptr;  /* Direct dereference */
            reg_ptr++;
            ptr = reg_ptr;
        }
    }
    return sum;
}

/* Pattern G: Multiple base registers with zero offset */
__attribute__((noinline))
int pattern_g_multiple_bases(void) {
    int *ptr1 = global_array;
    int *ptr2 = static_array;
    int sum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Two independent loads with base + 0 */
        sum += *ptr1;
        sum += *ptr2;
        
        ptr1++;
        ptr2++;
    }
    return sum;
}

/* Helper to initialize arrays */
void initialize_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = SIZE - i;
    }
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int total = 0;
    
    initialize_arrays();
    
    /* Call patterns multiple times with different conditions */
    for (int i = 0; i < (argc > 1 ? 10 : 5); ++i) {
        total += pattern_a_simple_array();
        total += pattern_b_pointer_arithmetic();
        total += pattern_c_struct_pointer();
        total += pattern_d_global_with_local_copy();
        total += pattern_e_conditional_blocks(i);
        total += pattern_f_nested_loops();
        total += pattern_g_multiple_bases();
    }
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        asm volatile("nop" : : : "memory");
    }
    
    return total % 256;
}
