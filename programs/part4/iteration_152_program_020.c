/* test_autoinc.c - Test program for GCC auto-inc-dec optimization pass */
#include <stddef.h>

/* Pattern A: Simple array loop with index */
int pattern_a_simple_array(int *array, int n) __attribute__((noinline));
int pattern_a_simple_array(int *array, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += array[i];  /* Should generate base + offset */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic */
int pattern_b_pointer_arithmetic(int *array, int n) __attribute__((noinline));
int pattern_b_pointer_arithmetic(int *array, int n) {
    int total = 0;
    int *p = array;
    int *end = p + n;
    
    while (p < end) {
        int val = *p;      /* Load using current pointer value */
        p++;               /* Increment after use */
        total += val * val;
    }
    return total;
}

/* Global array for Pattern D */
static int global_array[256];

/* Pattern D: Global pointer with local copy */
int pattern_d_global_pointer(int n) __attribute__((noinline));
int pattern_d_global_pointer(int n) {
    int result = 0;
    int *local_ptr = global_array;
    
    for (int i = 0; i < n; i++) {
        /* Force pointer to stay in register with inline asm */
        asm volatile("" : : "r"(local_ptr) : "memory");
        result += *local_ptr;  /* Base register + 0 offset */
        local_ptr++;
    }
    return result;
}

/* Pattern C: Struct pointer traversal */
struct DataStruct {
    int value;
    int padding[3];  /* Force non-trivial structure size */
};

int pattern_c_struct_traversal(struct DataStruct *arr, int n) __attribute__((noinline));
int pattern_c_struct_traversal(struct DataStruct *arr, int n) {
    int sum = 0;
    struct DataStruct *sp = arr;
    
    for (int i = 0; i < n; i++) {
        sum += sp->value;  /* Base + 0 offset through struct pointer */
        sp++;
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
int pattern_e_conditional_blocks(int *array, int n, int threshold) __attribute__((noinline));
int pattern_e_conditional_blocks(int *array, int n, int threshold) {
    int result = 0;
    int *ptr = array;
    
    for (int i = 0; i < n; i++) {
        if (*ptr > threshold) {  /* Dereference in condition */
            result += *ptr;       /* Another dereference */
            ptr += 2;             /* Skip ahead on some paths */
        } else {
            result -= *ptr;
            ptr++;                /* Normal increment */
        }
    }
    return result;
}

/* Helper to initialize arrays */
void init_array(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = i % 100;
    }
}

void init_struct_array(struct DataStruct *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i].value = i % 50;
    }
}

/* Main function with runtime variability */
int main(int argc, char **argv) {
    int test_size = argc > 1 ? 1000 : 500;  /* Variable size based on args */
    
    /* Local arrays */
    int local_array[1000];
    struct DataStruct struct_array[200];
    
    /* Initialize data */
    init_array(local_array, 1000);
    init_array(global_array, 256);
    init_struct_array(struct_array, 200);
    
    /* Execute all patterns */
    int total = 0;
    
    total += pattern_a_simple_array(local_array, test_size);
    total += pattern_b_pointer_arithmetic(local_array, test_size);
    total += pattern_d_global_pointer(test_size > 256 ? 256 : test_size);
    total += pattern_c_struct_traversal(struct_array, test_size > 200 ? 200 : test_size);
    total += pattern_e_conditional_blocks(local_array, test_size, 50);
    
    /* Use the result to prevent optimization */
    asm volatile("" : : "r"(total) : "memory");
    
    return total > 0 ? 0 : 1;
}
