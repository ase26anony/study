/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Test 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_zero_offset_inc(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        
        /* Separate increment statement */
        p++;
        
        /* Use the value to prevent elimination */
        sum += val;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : "r"(val));
    }
    
    /* Use sum to prevent dead code elimination */
    g_volatile_sum += sum;
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_inc(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        
        /* Separate increment with assignment */
        p += 1;
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 3: Structure with pointer member */
struct PointerHolder {
    int *ptr;
    int count;
};

__attribute__((noinline))
int test3_struct_ptr(struct PointerHolder *holder, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure pointer with zero offset */
        int val = *(holder->ptr + 0);
        
        /* Increment the pointer in the structure */
        holder->ptr = holder->ptr + 1;
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 4: Store operation with *(p + 0) and decrement */
__attribute__((noinline))
void test4_store_dec(int *arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with explicit zero offset */
        *(p + 0) = value + i;
        
        /* Separate decrement */
        p--;
        
        /* Memory barrier */
        asm volatile("" : : "r"(p));
    }
}

/* Test 5: More complex zero offset expression */
__attribute__((noinline))
int test5_complex_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    int zero = 0;  /* Non-const zero to prevent folding */
    
    for (int i = 0; i < n; i++) {
        /* Zero offset through variable (may still fold, but worth trying) */
        int val = *(p + zero);
        
        /* Increment with post-increment in separate statement */
        p = p + 1;
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 6: Loop with pointer as induction variable */
__attribute__((noinline))
int test6_ptr_induction(int *arr, int n) {
    int *end = arr + n;
    int sum = 0;
    
    for (int *p = arr; p < end; ) {
        /* Load with zero offset */
        int val = *(p + 0);
        
        /* Increment in loop update - separate from memory access */
        p++;
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 7: Multiple memory accesses with same pointer */
__attribute__((noinline))
int test7_multi_access(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* First access with zero offset */
        int val1 = *(p + 0);
        
        /* Some computation */
        sum += val1;
        
        /* Second access with zero offset */
        int val2 = *(p + 0);
        
        /* Increment after both accesses */
        p += 1;
        
        sum += val2;
        asm volatile("" : : "r"(val1), "r"(val2));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Main function */
int main(int argc, char **argv) {
    /* Use argc to determine array size at runtime */
    int size = (argc > 1) ? atoi(argv[1]) : g_volatile_bound;
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Run all test functions */
    total_sum += test1_zero_offset_inc(array, size);
    total_sum += test2_array_zero_inc(array, size);
    
    struct PointerHolder holder = {array, size};
    total_sum += test3_struct_ptr(&holder, size);
    
    test4_store_dec(array, size, 42);
    
    total_sum += test5_complex_zero(array, size);
    total_sum += test6_ptr_induction(array, size);
    total_sum += test7_multi_access(array, size);
    
    /* Print result to prevent elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    free(array);
    return 0;
}
