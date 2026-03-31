/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stddef.h>

#define SIZE 256
#define ITERATIONS 100

/* Global arrays for different access patterns */
int global_array[SIZE];
static int static_array[SIZE];

/* Pattern A: Simple array loop with index */
__attribute__((noinline))
int pattern_a_simple_array(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];  /* Base + offset access */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with pre-increment */
__attribute__((noinline))
int pattern_b_pointer_arithmetic(int *arr, int n) {
    int total = 0;
    int *p = arr;
    int *end = p + n;
    
    while (p < end) {
        int val = *p;  /* Base register + 0 offset */
        p++;           /* Pointer increment after use */
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int padding[3];  /* Force non-trivial size */
};

__attribute__((noinline))
int pattern_c_struct_traversal(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        result += sp->value;  /* Base + 0 offset through struct pointer */
        sp++;                 /* Post-increment of struct pointer */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_access(void) {
    int sum = 0;
    int *local_ptr = global_array;  /* Take address of global */
    
    for (int i = 0; i < SIZE; ++i) {
        sum += *local_ptr;  /* Dereference with base register */
        local_ptr++;        /* Increment after use */
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional_pointer(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer into register with inline asm */
        asm volatile("" : : "r"(ptr) : "memory");
        
        int val = *ptr;  /* Base + 0 offset */
        
        if (val > threshold) {
            sum += val;
            ptr++;  /* Increment on one path */
        } else {
            sum -= val;
            ptr++;  /* Increment on another path */
        }
    }
    return sum;
}

/* Pattern F: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_f_nested_loops(int *arr, int outer, int inner) {
    int total = 0;
    
    for (int i = 0; i < outer; ++i) {
        int *ptr = arr;  /* Reset pointer each outer iteration */
        
        for (int j = 0; j < inner; ++j) {
            total += *ptr;  /* Base + 0 offset */
            ptr++;          /* Post-increment */
        }
    }
    return total;
}

/* Pattern G: Pointer with offset calculation that might fold to 0 */
__attribute__((noinline))
int pattern_g_zero_offset(int *arr, int n) {
    int sum = 0;
    int *base = arr;
    
    for (int i = 0; i < n; ++i) {
        /* This might generate (mem (plus (reg) (const_int 0))) */
        int *current = base + 0;  /* Force zero offset calculation */
        sum += *current;          /* Dereference */
        base++;                   /* Increment base */
    }
    return sum;
}

/* Pattern H: Mixed access patterns in same function */
__attribute__((noinline))
int pattern_h_mixed_access(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Alternate between two pointers */
        if (i & 1) {
            sum += *p1;  /* Base + 0 from p1 */
            p1++;
        } else {
            sum += *p2;  /* Base + 0 from p2 */
            p2++;
        }
    }
    return sum;
}

/* Initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (ITERATIONS / 2) : ITERATIONS;
    
    init_arrays();
    
    /* Local array for stack-based access */
    int local_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        local_array[i] = i * 3;
    }
    
    /* Struct array */
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        struct_array[i].value = i * 4;
    }
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; ++i) {
        result += pattern_a_simple_array(local_array, SIZE);
        result += pattern_b_pointer_arithmetic(static_array, SIZE);
        result += pattern_c_struct_traversal(struct_array, SIZE);
        result += pattern_d_global_access();
        result += pattern_e_conditional_pointer(local_array, SIZE, 100);
        result += pattern_f_nested_loops(static_array, 10, SIZE/10);
        result += pattern_g_zero_offset(local_array, SIZE);
        result += pattern_h_mixed_access(local_array, static_array, SIZE);
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
    
    return result != 0 ? 0 : 1;
}
