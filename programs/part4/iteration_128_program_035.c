/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Prevent inlining to keep patterns intact */
__attribute__((noinline))
void test1_ptr_plus_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    /* Loop with *(p + 0) access followed by p++ */
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        
        /* Separate pointer increment */
        p++;
        
        /* Use the value to prevent elimination */
        sum += val;
    }
    
    /* Force side effect */
    g_volatile_sum += sum;
}

__attribute__((noinline))
void test2_array_zero_index(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    /* Loop with p[0] access followed by p += 1 */
    for (int i = 0; i < n; i++) {
        /* Memory access with zero index */
        int val = p[0];
        
        /* Separate pointer increment */
        p += 1;
        
        /* Use the value */
        sum += val;
        
        /* Prevent optimization with asm */
        asm volatile("" : : "r"(val));
    }
    
    g_volatile_sum += sum;
}

__attribute__((noinline))
void test3_store_pattern(int *arr, int n, int value) {
    int *p = arr;
    
    /* Store pattern with *(p + 0) followed by decrement */
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        
        /* Separate pointer decrement */
        p--;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
}

__attribute__((noinline))
void test4_mixed_operations(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    /* Mix of load and store with zero offsets */
    for (int i = 0; i < n; i++) {
        /* Load with zero offset */
        int val = *(p + 0);
        
        /* Store with zero offset */
        *(p + 0) = val * 2;
        
        /* Separate increment */
        p = p + 1;
        
        sum += val;
    }
    
    g_volatile_sum += sum;
}

/* Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
void test5_struct_member(struct ptr_wrapper *w) {
    int sum = 0;
    
    /* Loop using structure member pointer */
    while (w->current < w->end) {
        /* Access with zero offset through member */
        int val = *(w->current + 0);
        
        /* Increment member pointer */
        w->current++;
        
        sum += val;
    }
    
    g_volatile_sum += sum;
}

__attribute__((noinline))
void test6_pointer_arithmetic_in_loop(int *arr, int n) {
    int *p = arr;
    int *end = arr + n;
    int sum = 0;
    
    /* While loop with explicit pointer comparison */
    while (p < end) {
        /* Force zero offset through temporary */
        int *tmp = p;
        int val = *(tmp + 0);
        
        /* Separate increment */
        p = p + 1;
        
        sum += val;
        
        /* Call external to prevent optimization */
        dummy_external();
    }
    
    g_volatile_sum += sum;
}

__attribute__((noinline))
void test7_double_pointer(int **arr_ptr, int n) {
    int *p = *arr_ptr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through pointer with zero offset */
        int val = *(p + 0);
        
        /* Increment */
        ++p;
        
        sum += val;
        
        /* Volatile asm to prevent elimination */
        asm volatile("" : : "r"(val) : "memory");
    }
    
    *arr_ptr = p;  /* Update original pointer */
    g_volatile_sum += sum;
}

/* Dummy external function implementation */
void dummy_external(void) {
    /* Empty but external to prevent optimization */
}

int main(int argc, char **argv) {
    /* Use argc to determine array size at runtime */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate and initialize array */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Call all test functions */
    test1_ptr_plus_zero(array, size);
    
    int *array2 = (int *)malloc(size * sizeof(int));
    if (array2) {
        for (int i = 0; i < size; i++) array2[i] = i * 2;
        test2_array_zero_index(array2, size);
        free(array2);
    }
    
    int *array3 = (int *)malloc(size * sizeof(int));
    if (array3) {
        test3_store_pattern(array3, size, 42);
        free(array3);
    }
    
    test4_mixed_operations(array, size);
    
    /* Test with structure */
    struct ptr_wrapper wrapper;
    int *array4 = (int *)malloc(size * sizeof(int));
    if (array4) {
        for (int i = 0; i < size; i++) array4[i] = i * 3;
        wrapper.current = array4;
        wrapper.end = array4 + size;
        test5_struct_member(&wrapper);
        free(array4);
    }
    
    test6_pointer_arithmetic_in_loop(array, size);
    
    int *array5 = (int *)malloc(size * sizeof(int));
    if (array5) {
        for (int i = 0; i < size; i++) array5[i] = i * 4;
        int *ptr = array5;
        test7_double_pointer(&ptr, size);
        free(array5);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total sum (volatile): %d\n", g_volatile_sum);
    printf("Array first element: %d\n", array[0]);
    
    free(array);
    return 0;
}
