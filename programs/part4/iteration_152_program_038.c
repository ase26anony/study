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
        sum += arr[i];  /* Should generate base + offset */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with pre-increment */
__attribute__((noinline))
int pattern_b_pointer_arithmetic(int *arr, int n) {
    int *p = arr;
    int *end = p + n;
    int total = 0;
    
    while (p < end) {
        int val = *p;    /* Base register + 0 offset */
        p++;             /* Pointer increment */
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
    struct Data *ptr = arr;
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        result += ptr->value;  /* Base + 0 offset through struct */
        ptr++;                 /* Pointer increment by struct size */
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_access(void) {
    int *local_ptr = global_array;
    int sum = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        sum += *local_ptr;    /* Dereference with base register */
        local_ptr++;          /* Increment after use */
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional(int *arr, int n, int threshold) {
    int *ptr = arr;
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer to stay in register with asm */
        asm volatile("" : : "r"(ptr) : "memory");
        
        int val = *ptr;  /* Base + 0 offset */
        
        if (val > threshold) {
            sum += val;
            ptr += 2;    /* Skip ahead on some paths */
        } else {
            sum -= val;
            ptr++;       /* Normal increment */
        }
    }
    return sum;
}

/* Pattern F: Mixed pointer usage with multiple dereferences */
__attribute__((noinline))
int pattern_f_mixed_deref(int *arr1, int *arr2, int n) {
    int *p1 = arr1;
    int *p2 = arr2;
    int diff = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Two independent dereferences with base+0 */
        int a = *p1;
        int b = *p2;
        
        diff += a - b;
        p1++;
        p2++;
    }
    return diff;
}

/* Pattern G: Pointer chain with intermediate computation */
__attribute__((noinline))
int pattern_g_pointer_chain(int *arr, int n) {
    int *current = arr;
    int *next = arr + 1;
    int sum = 0;
    
    for (int i = 0; i < n - 1; ++i) {
        /* Use current pointer value before updating */
        int curr_val = *current;    /* Base + 0 */
        int next_val = *next;       /* Different base + 0 */
        
        sum += curr_val * next_val;
        
        /* Update pointers - creates sequence for auto-inc */
        current = next;
        next++;
    }
    return sum;
}

/* Initialize arrays with predictable values */
void initialize_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 100;
        static_array[i] = (i * 3) % 100;
    }
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    int iterations = ITERATIONS;
    if (argc > 1) {
        iterations = (argv[1][0] - '0') * 10;
    }
    
    initialize_arrays();
    
    int total = 0;
    
    /* Call patterns multiple times to ensure compiler sees them */
    for (int i = 0; i < iterations; ++i) {
        total += pattern_a_simple_array(global_array, SIZE);
        total += pattern_b_pointer_arithmetic(static_array, SIZE);
        
        struct Data struct_array[SIZE];
        for (int j = 0; j < SIZE; ++j) {
            struct_array[j].value = j % 50;
        }
        total += pattern_c_struct_traversal(struct_array, SIZE);
        
        total += pattern_d_global_access();
        total += pattern_e_conditional(global_array, SIZE, 50);
        total += pattern_f_mixed_deref(global_array, static_array, SIZE);
        total += pattern_g_pointer_chain(global_array, SIZE);
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(total) : "memory");
    
    return total > 0 ? 0 : 1;
}
