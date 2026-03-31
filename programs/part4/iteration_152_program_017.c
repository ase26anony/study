/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stddef.h>

#define SIZE 256
#define ITERATIONS 100

/* Global arrays for different access patterns */
int global_array[SIZE];
static int static_array[SIZE];

/* Pattern A: Simple array loop */
__attribute__((noinline))
int pattern_a_simple_array(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];  /* Base + offset, may become base + 0 after optimization */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic */
__attribute__((noinline))
int pattern_b_pointer_arithmetic(int *arr, int n) {
    int total = 0;
    int *p = arr;
    int *end = p + n;
    
    while (p < end) {
        int val = *p;    /* Direct dereference with base register + 0 offset */
        p++;             /* Pointer increment after use */
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int padding[3];  /* Ensure structure has size > sizeof(int) */
};

__attribute__((noinline))
int pattern_c_struct_traversal(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer to be in register before dereference */
        asm volatile("" : : "r"(sp) : "memory");
        result += sp->value;  /* Base + 0 offset for struct field access */
        sp++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_pointer(void) {
    int sum = 0;
    int *local_ptr = global_array;  /* Take address of global */
    
    for (int i = 0; i < SIZE; i++) {
        /* Multiple dereferences to create different RTL patterns */
        int val1 = *local_ptr;
        local_ptr++;
        sum += val1;
        
        /* Another dereference with the updated pointer */
        if (i < SIZE - 1) {
            int val2 = *local_ptr;
            sum += val2;
        }
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_blocks(int *arr, int n, int condition) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (condition & 1) {
            /* Path 1: Dereference then increment */
            int val = *ptr;  /* Base + 0 */
            ptr++;
            result += val;
        } else {
            /* Path 2: Increment then dereference */
            ptr++;
            int val = *ptr;  /* Different offset pattern */
            result -= val;
        }
        condition ^= (i << 3);  /* Vary condition */
    }
    return result;
}

/* Pattern F: Mixed offset patterns to trigger different paths */
__attribute__((noinline))
int pattern_f_mixed_offsets(int *arr, int n) {
    int sum = 0;
    int *base = arr;
    
    /* Access with explicit 0 offset through pointer */
    sum += *base;  /* Should generate (mem (reg)) pattern */
    
    /* Access with calculated offset that might fold to 0 */
    int offset = 0;
    sum += base[offset];  /* Another base + 0 pattern */
    
    /* Loop with pointer that gets used then incremented */
    for (int i = 0; i < n; i++) {
        int *current = base + i;
        asm volatile("" : : "r"(current) : "memory");  /* Force in register */
        sum += *current;  /* Base register + 0 */
    }
    
    return sum;
}

/* Helper to initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < SIZE; i++) {
        global_array[i] = i * 3 + 1;
        static_array[i] = i * 5 - 2;
    }
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (ITERATIONS / 2) : ITERATIONS;
    int size = (argc > 2) ? (SIZE / 2) : SIZE;
    
    init_arrays();
    
    /* Call all patterns multiple times */
    for (int i = 0; i < iterations; i++) {
        total += pattern_a_simple_array(global_array, size);
        total += pattern_b_pointer_arithmetic(static_array, size);
        
        struct Data struct_arr[SIZE];
        for (int j = 0; j < SIZE; j++) {
            struct_arr[j].value = j * 7 + 3;
        }
        total += pattern_c_struct_traversal(struct_arr, size);
        
        total += pattern_d_global_pointer();
        total += pattern_e_conditional_blocks(global_array, size, i);
        total += pattern_f_mixed_offsets(static_array, size);
    }
    
    /* Use result to prevent optimization */
    if (total == 0) {
        asm volatile("" : : "r"(total));
    }
    
    return total != 0 ? 0 : 1;
}
