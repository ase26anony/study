/* test_autoinc.c - Test program for auto-increment/decrement optimization */
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
        sum += arr[i];  /* Should generate base + offset */
    }
    return sum;
}

/* Pattern B: Explicit pointer arithmetic */
__attribute__((noinline))
int pattern_explicit_pointer(int *arr, int n) {
    int total = 0;
    int *p = arr;
    int *end = p + n;
    
    while (p < end) {
        int val = *p;  /* Base register with zero offset */
        p++;           /* Pointer increment after use */
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
int pattern_struct_pointer(struct Data *arr, int n) {
    int result = 0;
    struct Data *sp = arr;
    
    for (int i = 0; i < n; ++i) {
        /* Force pointer to be in register before dereference */
        asm volatile("" : : "r"(sp) : "memory");
        result += sp->value;  /* Base register access */
        sp++;
    }
    return result;
}

/* Pattern D: Global pointer with local copy */
__attribute__((noinline))
int pattern_global_pointer(void) {
    int sum = 0;
    int *local_ptr = global_array;  /* Take address of global */
    
    for (int i = 0; i < SIZE; ++i) {
        /* Multiple dereferences to create different RTL patterns */
        int val1 = *local_ptr;
        local_ptr++;
        sum += val1;
        
        if (i % 2 == 0) {
            /* Another access pattern */
            int *temp = local_ptr;
            int val2 = *temp;  /* Base register access */
            sum += val2;
        }
    }
    return sum;
}

/* Pattern E: Pointer in conditional blocks */
__attribute__((noinline))
int pattern_conditional(int *arr, int n, int flag) {
    int result = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (flag) {
            /* Path 1: Simple dereference */
            result += *ptr;
            ptr++;
        } else {
            /* Path 2: Dereference with computation */
            int val = *ptr;
            result += val >> 1;
            ptr += 2;
        }
        
        /* Mix with static array access */
        if (i % 3 == 0) {
            int *sptr = static_array;
            result += *sptr;  /* Base + 0 offset */
            sptr++;
        }
    }
    return result;
}

/* Pattern F: Nested loops with pointer reset */
__attribute__((noinline))
int pattern_nested_reset(int *arr, int outer, int inner) {
    int total = 0;
    
    for (int i = 0; i < outer; i++) {
        int *ptr = arr;  /* Reset pointer each outer iteration */
        
        for (int j = 0; j < inner; j++) {
            /* Multiple consecutive loads */
            int a = *ptr;
            ptr++;
            int b = *ptr;
            ptr++;
            
            total += a * b;
        }
    }
    return total;
}

/* Pattern G: Pointer arithmetic with zero offset */
__attribute__((noinline))
int pattern_zero_offset(int *base, int offset) {
    /* Create pointer with explicit zero offset */
    int *ptr = base + offset;
    
    /* Force compiler to keep ptr in register */
    asm volatile("" : "+r"(ptr) : : "memory");
    
    /* This should generate (mem (reg)) pattern */
    int val = *ptr;
    
    /* Use the value to prevent optimization */
    return val * 2;
}

/* Helper to initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < SIZE; i++) {
        global_array[i] = i;
        static_array[i] = i * 2;
    }
}

/* Main function with runtime variability */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (argc * 10) : ITERATIONS;
    
    init_arrays();
    
    /* Local array for testing */
    int local_array[SIZE];
    for (int i = 0; i < SIZE; i++) {
        local_array[i] = i * 3;
    }
    
    /* Struct array */
    struct Data struct_array[SIZE / 4];
    for (int i = 0; i < SIZE / 4; i++) {
        struct_array[i].value = i * 4;
    }
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < iterations; i++) {
        result += pattern_simple_array(local_array, SIZE);
        result += pattern_explicit_pointer(global_array, SIZE);
        result += pattern_struct_pointer(struct_array, SIZE / 4);
        result += pattern_global_pointer();
        result += pattern_conditional(local_array, SIZE, i % 2);
        result += pattern_nested_reset(static_array, 10, SIZE / 10);
        result += pattern_zero_offset(local_array, 0);
    }
    
    /* Return result to prevent dead code elimination */
    return result % 255;
}
