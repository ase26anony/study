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
        sum += arr[i];  /* Should generate base + offset */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic */
__attribute__((noinline))
int pattern_b_explicit_pointer(int *arr, int n) {
    int total = 0;
    int *p = arr;
    int *end = p + n;
    
    while (p < end) {
        int val = *p;    /* Base register with zero offset */
        p++;             /* Pointer increment after use */
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
int pattern_c_struct_pointer(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force base+0 addressing by using pointer directly */
        asm volatile("" : : "r"(sp) : "memory");
        result += sp->value;
        sp++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_d_global_pointer(void) {
    int sum = 0;
    int *local_ptr = global_array;
    
    for (int i = 0; i < SIZE; ++i) {
        /* Multiple accesses to same base+0 pattern */
        int val1 = *local_ptr;
        local_ptr++;
        sum += val1;
        
        /* Another access pattern */
        if (i % 2 == 0) {
            int *temp = local_ptr - 1;
            sum += *temp;  /* Different base+0 pattern */
        }
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_e_conditional(int *arr, int n, int flag) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ++i) {
        if (flag) {
            /* Path 1: Use then increment */
            result += *ptr;
            ptr++;
        } else {
            /* Path 2: Increment then use */
            ptr++;
            result += *(ptr - 1);
        }
        
        /* Complex addressing to confuse optimizer */
        switch (i % 3) {
            case 0:
                result += *ptr;  /* Base+0 in switch */
                break;
            case 1:
                ptr++;
                result += *(ptr - 1);
                break;
            case 2:
                /* Keep pointer stable */
                asm volatile("" : "+r"(ptr) : : "memory");
                result += *ptr;
                break;
        }
    }
    return result;
}

/* Pattern F: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_f_nested_loops(int *arr, int outer, int inner) {
    int total = 0;
    
    for (int i = 0; i < outer; ++i) {
        int *ptr = arr;
        
        for (int j = 0; j < inner; ++j) {
            /* Simple base+0 access in inner loop */
            total += *ptr;
            ptr++;
            
            /* Force register allocation */
            asm volatile("" : : "r"(ptr) : "memory");
        }
    }
    return total;
}

/* Pattern G: Multiple base registers */
__attribute__((noinline))
int pattern_g_multiple_bases(int *a, int *b, int n) {
    int sum = 0;
    int *pa = a;
    int *pb = b;
    
    for (int i = 0; i < n; ++i) {
        /* Alternate between two base registers */
        sum += *pa + *pb;
        pa++;
        pb++;
    }
    return sum;
}

/* Helper to initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < SIZE; ++i) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
}

/* Main driver with runtime variability */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc for runtime variability */
    int iterations = (argc > 1) ? (argc % 50 + 10) : ITERATIONS;
    
    init_arrays();
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; ++i) {
        result += pattern_a_simple_array(global_array, SIZE);
        result += pattern_b_explicit_pointer(static_array, SIZE);
        
        struct Data struct_array[SIZE];
        for (int j = 0; j < SIZE; ++j) {
            struct_array[j].value = j * 3;
        }
        result += pattern_c_struct_pointer(struct_array, SIZE);
        
        result += pattern_d_global_pointer();
        result += pattern_e_conditional(global_array, SIZE, i % 2);
        result += pattern_f_nested_loops(static_array, 10, SIZE/10);
        result += pattern_g_multiple_bases(global_array, static_array, SIZE);
    }
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(result) : "memory");
    
    return result % 256;  /* Return non-zero to indicate execution */
}
