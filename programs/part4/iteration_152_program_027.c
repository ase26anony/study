/* test_autoinc.c - Test program for auto-increment/decrement optimization coverage */

#include <stddef.h>

/* Pattern A: Simple array loop */
int pattern_a_simple_array(int *array, int n) __attribute__((noinline));
int pattern_a_simple_array(int *array, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += array[i];  /* Should generate base + offset */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic */
int pattern_b_explicit_pointer(int *array, int n) __attribute__((noinline));
int pattern_b_explicit_pointer(int *array, int n) {
    int total = 0;
    int *p = array;
    int *end = p + n;
    
    while (p < end) {
        int val = *p;  /* Load before increment - key pattern */
        p++;
        total += val * val;
    }
    return total;
}

/* Pattern C: Struct pointer traversal */
struct Data {
    int value;
    int padding[3];  /* Force non-trivial structure size */
};

int pattern_c_struct_array(struct Data *arr, int n) __attribute__((noinline));
int pattern_c_struct_array(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        result += sp->value;  /* Struct member access */
        sp++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
static int global_array[256];

int pattern_d_global_array(void) __attribute__((noinline));
int pattern_d_global_array(void) {
    int local_sum = 0;
    int *local_ptr = global_array;
    
    /* Use asm to force pointer into register */
    asm volatile("" : : "r"(local_ptr) : "memory");
    
    for (int i = 0; i < 256; ++i) {
        local_sum += *local_ptr;  /* Dereference pointer in register */
        local_ptr++;
    }
    return local_sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional(int *array, int n, int threshold) __attribute__((noinline));
int pattern_e_conditional(int *array, int n, int threshold) {
    int sum = 0;
    int *ptr = array;
    
    for (int i = 0; i < n; ++i) {
        /* Conditional that uses pointer before potential modification */
        if (*ptr > threshold) {
            sum += *ptr;  /* First dereference */
            ptr++;        /* Increment after use */
        } else {
            /* Different path with same pattern */
            sum -= *ptr;
            ptr++;
        }
    }
    return sum;
}

/* Pattern F: Multiple loads with same base */
int pattern_f_multiple_loads(int *a, int *b, int n) __attribute__((noinline));
int pattern_f_multiple_loads(int *a, int *b, int n) {
    int sum = 0;
    int *pa = a;
    int *pb = b;
    
    for (int i = 0; i < n; ++i) {
        /* Two independent loads with base+0 addressing */
        int val_a = *pa;
        int val_b = *pb;
        sum += val_a + val_b;
        pa++;
        pb++;
    }
    return sum;
}

/* Pattern G: Nested loops with pointer reset */
int pattern_g_nested_loops(int *array, int rows, int cols) __attribute__((noinline));
int pattern_g_nested_loops(int *array, int rows, int cols) {
    int total = 0;
    int *row_ptr = array;
    
    for (int r = 0; r < rows; ++r) {
        int *col_ptr = row_ptr;
        for (int c = 0; c < cols; ++c) {
            total += *col_ptr;  /* Inner loop pointer dereference */
            col_ptr++;
        }
        row_ptr += cols;
    }
    return total;
}

/* Main function to drive all patterns */
int main(int argc, char **argv) {
    /* Use argc to create runtime variability */
    int size = (argc > 1) ? 100 : 200;
    int threshold = (argc > 2) ? 50 : 100;
    
    /* Test arrays */
    int array1[256];
    int array2[256];
    struct Data struct_array[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; ++i) {
        array1[i] = i;
        array2[i] = i * 2;
        global_array[i] = i % 64;
    }
    
    for (int i = 0; i < 100; ++i) {
        struct_array[i].value = i * 3;
    }
    
    /* Execute all patterns */
    int result = 0;
    
    result += pattern_a_simple_array(array1, size);
    result += pattern_b_explicit_pointer(array1, size);
    result += pattern_c_struct_array(struct_array, 100);
    result += pattern_d_global_array();
    result += pattern_e_conditional(array1, size, threshold);
    result += pattern_f_multiple_loads(array1, array2, size);
    result += pattern_g_nested_loops(array1, 16, 16);
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(result) : "memory");
    
    return result > 0 ? 0 : 1;
}
