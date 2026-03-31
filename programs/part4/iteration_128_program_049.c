/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Test 1: Using *(p + 0) with p++ */
__attribute__((noinline))
int test1_ptr_plus_zero_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        
        /* Separate pointer increment */
        p++;
        
        /* Use the value to prevent elimination */
        sum += val;
        asm volatile("" : : "r"(val));  /* Prevent dead code elimination */
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 2: Using p[0] with p += 1 */
__attribute__((noinline))
int test2_array_zero_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        
        /* Separate pointer increment */
        p += 1;
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 3: Using structure with pointer member */
typedef struct {
    int *current;
    int *end;
} PointerStruct;

__attribute__((noinline))
int test3_struct_ptr_increment(int *arr, int n) {
    PointerStruct ps;
    ps.current = arr;
    ps.end = arr + n;
    
    int sum = 0;
    
    while (ps.current < ps.end) {
        /* Access through structure member with zero offset */
        int val = *(ps.current + 0);
        
        /* Increment the pointer in the structure */
        ps.current = ps.current + 1;
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 4: Store instead of load with decrement */
__attribute__((noinline))
int test4_store_decrement(int *arr, int n) {
    int *p = arr + n - 1;  /* Start from end */
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        int val = i * 2;
        *(p + 0) = val;
        
        /* Separate pointer decrement */
        p--;
        
        /* Read back to verify */
        sum += *p;
        asm volatile("" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 5: Mixed pattern with different increment amounts */
__attribute__((noinline))
int test5_mixed_pattern(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i += 2) {
        /* Two accesses with zero offset */
        int val1 = *(p + 0);
        p++;
        
        int val2 = *(p + 0);
        p++;
        
        sum += val1 + val2;
        asm volatile("" : : "r"(val1), "r"(val2));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 6: Pointer arithmetic in separate statements */
__attribute__((noinline))
int test6_separate_arithmetic(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Zero offset in separate calculation */
        int *temp = p + 0;
        int val = *temp;
        
        /* Increment in separate statement */
        int *new_p = p + 1;
        p = new_p;
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Main function */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent compile-time optimization */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate and initialize array */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;  /* Simple sequence 1, 2, 3, ... */
    }
    
    int total_sum = 0;
    
    /* Run all test functions */
    total_sum += test1_ptr_plus_zero_increment(array, size);
    total_sum += test2_array_zero_increment(array, size);
    total_sum += test3_struct_ptr_increment(array, size);
    total_sum += test4_store_decrement(array, size);
    total_sum += test5_mixed_pattern(array, size);
    total_sum += test6_separate_arithmetic(array, size);
    
    /* Use results to prevent elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    /* Dummy external call */
    dummy_external();
    
    free(array);
    return 0;
}

/* Dummy external function definition */
void dummy_external(void) {
    /* Empty but prevents elimination */
}
