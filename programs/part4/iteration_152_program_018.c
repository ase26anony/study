/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stddef.h>

#define SIZE 256
#define ITERATIONS 100

/* Global arrays for different access patterns */
int global_array[SIZE];
static int static_array[SIZE];

/* Pattern A: Simple array loop */
int __attribute__((noinline)) pattern_simple_array(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        /* This should generate base + offset addressing */
        sum += arr[i];
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic */
int __attribute__((noinline)) pattern_pointer_arithmetic(int *arr, int n) {
    int total = 0;
    int *p = arr;
    int *end = p + n;
    
    while (p < end) {
        /* Load using current pointer value before increment */
        int val = *p;
        p++;  /* Pointer increment after use */
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int padding[3];  /* Force non-trivial structure size */
};

int __attribute__((noinline)) pattern_struct_traversal(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Access struct member with pointer */
        result += sp->value;
        sp++;  /* Pointer increment */
        
        /* Force pointer to stay in register */
        asm volatile("" : : "r"(sp) : "memory");
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
int __attribute__((noinline)) pattern_global_access(void) {
    int local_sum = 0;
    int *local_ptr = global_array;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Dereference local copy of global pointer */
        local_sum += *local_ptr;
        local_ptr++;
    }
    return local_sum;
}

/* Pattern E: Pointer in conditional blocks */
int __attribute__((noinline)) pattern_conditional(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        if (*ptr > threshold) {
            /* Use pointer dereference in true branch */
            sum += *ptr;
            ptr += 2;  /* Skip next element */
        } else {
            /* Use pointer dereference in false branch */
            sum -= *ptr;
            ptr++;  /* Normal increment */
        }
        
        /* Prevent optimization of pointer value */
        asm volatile("" : "+r"(ptr) : : "memory");
    }
    return sum;
}

/* Pattern F: Multiple pointer dereferences in same basic block */
int __attribute__((noinline)) pattern_multiple_derefs(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Two independent dereferences with zero offset */
        int a = *p1;
        int b = *p2;
        
        sum += a * b;
        
        /* Increment both pointers */
        p1++;
        p2++;
    }
    return sum;
}

/* Pattern G: Pointer to pointer (indirect access) */
int __attribute__((noinline)) pattern_indirect(int **ptr_arr, int n) {
    int sum = 0;
    int **pp = ptr_arr;
    
    for (int i = 0; i < n; ++i) {
        /* Double dereference: first gets pointer, second gets value */
        int *inner_ptr = *pp;
        if (inner_ptr) {
            sum += *inner_ptr;  /* Base + 0 offset from inner_ptr */
        }
        pp++;
    }
    return sum;
}

/* Helper to initialize arrays */
void initialize_arrays(void) {
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
    int size = (argc > 2) ? (SIZE / 2) : SIZE;
    
    initialize_arrays();
    
    /* Call all patterns multiple times */
    for (int i = 0; i < iterations; ++i) {
        result ^= pattern_simple_array(global_array, size);
        result ^= pattern_pointer_arithmetic(static_array, size);
        
        struct Data struct_arr[SIZE];
        for (int j = 0; j < SIZE; ++j) {
            struct_arr[j].value = j * 3;
        }
        result ^= pattern_struct_traversal(struct_arr, size);
        
        result ^= pattern_global_access();
        result ^= pattern_conditional(global_array, size, size / 2);
        
        result ^= pattern_multiple_derefs(global_array, static_array, size);
        
        /* Create pointer array for indirect pattern */
        int *ptr_array[SIZE];
        for (int j = 0; j < size; ++j) {
            ptr_array[j] = &global_array[j];
        }
        result ^= pattern_indirect(ptr_array, size);
    }
    
    /* Use result to prevent dead code elimination */
    return result % 255;
}
