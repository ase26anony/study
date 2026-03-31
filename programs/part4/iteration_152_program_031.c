/* test_autoinc.c - Test program for auto-increment/decrement optimization coverage */

#include <stddef.h>

#define SIZE 256
#define ITERATIONS 100

/* Global arrays for different access patterns */
int global_array[SIZE];
static int static_array[SIZE];

/* Pattern A: Simple array loop */
__attribute__((noinline))
int pattern_simple_array(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];  /* Base + offset, may become base + 0 after optimization */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic with pre-increment */
__attribute__((noinline))
int pattern_pointer_arithmetic(int *arr, int n) {
    int total = 0;
    int *p = arr;
    int *end = p + n;
    
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
    int padding[3];
};

__attribute__((noinline))
int pattern_struct_traversal(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        result += sp->value;  /* Base + 0 offset through struct pointer */
        sp++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_global_local(void) {
    int sum = 0;
    int *local_ptr = global_array;  /* Take address of global */
    
    for (int i = 0; i < SIZE; ++i) {
        sum += *local_ptr;  /* Base + 0 offset */
        local_ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_conditional(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer to stay in register with asm */
        asm volatile("" : : "r"(ptr) : "memory");
        
        int val = *ptr;  /* Base + 0 offset */
        
        if (val > threshold) {
            sum += val;
            ptr += 2;  /* Skip next element */
        } else {
            sum -= val;
            ptr++;     /* Normal increment */
        }
    }
    return sum;
}

/* Pattern F: Mixed access patterns to prevent optimization */
__attribute__((noinline))
int pattern_mixed_access(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Alternate between two pointers */
        int val1 = *p1;  /* Base + 0 */
        p1++;
        
        int val2 = *p2;  /* Base + 0 */
        p2++;
        
        sum += val1 * val2;
    }
    return sum;
}

/* Pattern G: Pointer dereference with zero offset explicitly */
__attribute__((noinline))
int pattern_explicit_zero_offset(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Create scenario where offset might be zero */
    for (int i = 0; i < n; i += 2) {
        int *current = ptr + 0;  /* Explicit zero offset */
        sum += *current;         /* Should be (mem (reg)) */
        ptr += 2;
    }
    return sum;
}

/* Initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i % 100;
        static_array[i] = (i * 3) % 100;
    }
}

/* Main driver that calls all patterns */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (ITERATIONS / 2) : ITERATIONS;
    int size = (argc > 2) ? (SIZE / 2) : SIZE;
    
    init_arrays();
    
    /* Local array on stack */
    int local_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        local_array[i] = (i * 7) % 100;
    }
    
    /* Struct array */
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        struct_array[i].value = i % 50;
    }
    
    /* Call all patterns multiple times */
    for (int i = 0; i < iterations; ++i) {
        result += pattern_simple_array(local_array, size);
        result += pattern_pointer_arithmetic(global_array, size);
        result += pattern_struct_traversal(struct_array, size);
        result += pattern_global_local();
        result += pattern_conditional(static_array, size, 50);
        result += pattern_mixed_access(local_array, global_array, size);
        result += pattern_explicit_zero_offset(local_array, size);
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result));
    
    return result % 1000;
}
