/* test_autoinc.c - Test program for auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1000

/* Global array for Pattern D */
int global_array[SIZE];

/* Struct for Pattern C */
struct Data {
    int value;
    int padding[3]; /* Force non-contiguous access */
};

/* Pattern A: Simple array loop with index */
int pattern_a_simple_array(int *arr, int n) __attribute__((noinline));
int pattern_a_simple_array(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        /* This should generate base + offset addressing */
        sum += arr[i];
    }
    return sum;
}

/* Pattern A variant: More complex index calculation */
int pattern_a_complex_index(int *arr, int n) __attribute__((noinline));
int pattern_a_complex_index(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i += 2) {
        /* Multiple accesses with same base, different offsets */
        sum += arr[i] + arr[i + 1];
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic */
int pattern_b_pointer_arithmetic(int *arr, int n) __attribute__((noinline));
int pattern_b_pointer_arithmetic(int *arr, int n) {
    int *p = arr;
    int *end = p + n;
    int sum = 0;
    
    while (p < end) {
        /* Load using current pointer value */
        int val = *p;
        /* Force pointer to stay in register */
        asm volatile("" : : "r"(p) : "memory");
        p++;
        sum += val * val;
    }
    return sum;
}

/* Pattern B variant: Pointer increment in loop */
int pattern_b_pointer_increment(int *arr, int n) __attribute__((noinline));
int pattern_b_pointer_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Dereference then increment */
        int val = *p;
        p++;
        sum += val;
    }
    return sum;
}

/* Pattern C: Struct pointer traversal */
int pattern_c_struct_traversal(struct Data *arr, int n) __attribute__((noinline));
int pattern_c_struct_traversal(struct Data *arr, int n) {
    struct Data *sp = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member */
        sum += sp->value;
        sp++;
    }
    return sum;
}

/* Pattern D: Global pointer with local copy */
int pattern_d_global_pointer(void) __attribute__((noinline));
int pattern_d_global_pointer(void) {
    int *ptr = &global_array[0];
    int sum = 0;
    
    /* Create local pointer to global */
    for (int i = 0; i < SIZE; i++) {
        sum += *ptr;
        ptr++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional_pointer(int *arr, int n, int flag) __attribute__((noinline));
int pattern_e_conditional_pointer(int *arr, int n, int flag) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        if (flag) {
            /* Use pointer on true path */
            sum += *p;
            p++;
        } else {
            /* Different pointer usage on false path */
            sum += arr[i];
        }
    }
    
    /* Force p to be used to prevent optimization */
    asm volatile("" : : "r"(p) : "memory");
    return sum;
}

/* Pattern E variant: Switch statement with pointer */
int pattern_e_switch_pointer(int *arr, int n, int mode) __attribute__((noinline));
int pattern_e_switch_pointer(int *arr, int n, int mode) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        switch (mode) {
            case 0:
                sum += *p;
                p++;
                break;
            case 1:
                sum += p[0] + p[1];
                p += 2;
                i++; /* Skip extra iteration */
                break;
            case 2:
                sum += arr[i];
                break;
        }
    }
    return sum;
}

/* Helper to initialize arrays */
void init_arrays(int *arr, struct Data *sarr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = i % 100;
        sarr[i].value = i % 100;
        global_array[i] = i % 100;
    }
}

/* Main function with runtime variability */
int main(int argc, char **argv) {
    int local_array[SIZE];
    struct Data struct_array[SIZE];
    
    /* Initialize data */
    init_arrays(local_array, struct_array, SIZE);
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations < 1) iterations = 10;
    
    int total = 0;
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; i++) {
        total += pattern_a_simple_array(local_array, SIZE);
        total += pattern_a_complex_index(local_array, SIZE);
        total += pattern_b_pointer_arithmetic(local_array, SIZE);
        total += pattern_b_pointer_increment(local_array, SIZE);
        total += pattern_c_struct_traversal(struct_array, SIZE);
        total += pattern_d_global_pointer();
        total += pattern_e_conditional_pointer(local_array, SIZE, i % 2);
        total += pattern_e_switch_pointer(local_array, SIZE, i % 3);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Total: %d\n", total);
    
    return 0;
}
