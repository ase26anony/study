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
        sum += global_array[i];
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with pre-increment */
int pattern_b_explicit_pointer(void) {
    int *p = global_array;
    int *end = p + SIZE;
    int total = 0;
    
    while (p < end) {
        int val = *p;  /* Base + 0 offset access */
        p++;           /* Pointer increment after use */
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
        result += ptr->value;  /* Base + 0 offset through struct pointer */
        ptr++;                 /* Post-increment of struct pointer */
    }
    return result;
}

/* Pattern D: Local pointer with global array */
int pattern_d_local_pointer(void) __attribute__((noinline));
int pattern_d_local_pointer(void) {
    int *local_ptr = &global_array[0];  /* Start at base */
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        /* Multiple accesses with same base+0 pattern */
        int val1 = *local_ptr;
        asm volatile("" : : "r"(local_ptr));  /* Force register allocation */
        sum += val1;
        
        /* Another access pattern */
        if (i % 2 == 0) {
            int val2 = *local_ptr;  /* Same base+0 access */
            sum -= val2;
        }
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional_pointer(int mode) __attribute__((noinline));
int pattern_e_conditional_pointer(int mode) {
    int *ptr = static_array;
    int result = 0;
    
    for (int i = 0; i < SIZE; i++) {
        /* Always dereference with base+0 */
        int base_val = *ptr;
        
        /* Conditional pointer modification */
        if (mode == 0) {
            ptr++;  /* Increment on this path */
            result += base_val;
        } else if (mode == 1) {
            /* No increment - just use value */
            result -= base_val;
        } else {
            ptr += 2;  /* Different increment */
            result += base_val * 2;
        }
        
        /* Force ptr to stay in register */
        asm volatile("" : "+r"(ptr));
    }
    return result;
}

/* Pattern F: Nested loops with pointer reset */
int pattern_f_nested_loops(void) {
    int *ptr;
    int total = 0;
    
    for (int outer = 0; outer < ITERATIONS; outer++) {
        ptr = &global_array[0];  /* Reset to base each iteration */
        
        for (int inner = 0; inner < SIZE; inner++) {
            /* Base + 0 access in inner loop */
            total += *ptr;
            ptr++;
        }
    }
    return total;
}

/* Pattern G: Multiple pointers with same base */
int pattern_g_multiple_pointers(void) {
    int *p1 = &global_array[0];
    int *p2 = &global_array[0];
    int sum = 0;
    
    for (int i = 0; i < SIZE/2; i++) {
        /* Two independent base+0 accesses */
        sum += *p1;
        p1++;
        
        sum -= *p2;
        p2++;
    }
    return sum;
}

/* Pattern H: Pointer arithmetic with zero offset */
int pattern_h_explicit_zero_offset(void) {
    int *base = global_array;
    int sum = 0;
    
    /* Force explicit (reg + 0) addressing */
    for (int i = 0; i < SIZE; i++) {
        /* This should generate base + 0 addressing */
        sum += *(base + 0);
        
        /* Then increment base */
        base = base + 1;
    }
    return sum;
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
    
    /* Run all patterns multiple times */
    for (int run = 0; run < (argc > 1 ? 10 : 5); run++) {
        total += pattern_a_simple_array();
        total += pattern_b_explicit_pointer();
        total += pattern_c_struct_pointer();
        total += pattern_d_local_pointer();
        total += pattern_e_conditional_pointer(run % 3);
        total += pattern_f_nested_loops();
        total += pattern_g_multiple_pointers();
        total += pattern_h_explicit_zero_offset();
    }
    
    /* Use result to prevent optimization */
    if (total == 0) {
        asm volatile("nop");
    }
    
    return total != 0 ? 0 : 1;
}
