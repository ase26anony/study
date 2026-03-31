/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_external(int);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Test 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_ptr_plus_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access with explicit zero offset - should generate *(p + 0) */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    /* Use result to prevent elimination */
    g_volatile_sum += sum;
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Separate increment with assignment */
        p += 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 3: Structure with pointer member */
typedef struct {
    int *current;
    int *end;
} PointerStruct;

__attribute__((noinline))
int test3_struct_ptr(PointerStruct *ps, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure pointer with zero offset */
        int val = *(ps->current + 0);
        sum += val;
        /* Increment the pointer in the structure */
        ps->current = ps->current + 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 4: Store instead of load */
__attribute__((noinline))
void test4_store_zero(int *arr, int n, int value) {
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Decrement instead of increment */
        p--;
    }
    
    /* Force memory side effect */
    asm volatile("" : : "r"(arr[0]) : "memory");
}

/* Test 5: Different increment pattern */
__attribute__((noinline))
int test5_mixed_pattern(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple zero-offset accesses */
        int val1 = *(p + 0);
        int val2 = *(0 + p);  /* Commuted form */
        sum += val1 + val2;
        
        /* Complex but constant increment */
        p = p + 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 6: Using asm to ensure pattern isn't optimized away */
__attribute__((noinline))
int test6_volatile_access(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Force the access to happen */
        asm volatile("" : "+r"(sum) : "r"(*(p + 0)) : "memory");
        p++;
    }
    
    return sum;
}

/* Test 7: Nested loops with pointer reset */
__attribute__((noinline))
int test7_nested_loops(int *arr, int rows, int cols) {
    int total = 0;
    
    for (int r = 0; r < rows; r++) {
        int *p = arr + r * cols;
        
        for (int c = 0; c < cols; c++) {
            /* Zero offset access in inner loop */
            total += *(p + 0);
            p += 1;
        }
    }
    
    g_volatile_sum += total;
    return total;
}

/* Main function with runtime-determined array */
int main(int argc, char **argv) {
    /* Use argc to determine size at runtime */
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
    total_sum += test1_ptr_plus_zero(array, size);
    total_sum += test2_array_zero(array, size);
    
    PointerStruct ps = {array, array + size};
    total_sum += test3_struct_ptr(&ps, size);
    
    test4_store_zero(array, size, 42);
    
    total_sum += test5_mixed_pattern(array, size);
    total_sum += test6_volatile_access(array, size);
    total_sum += test7_nested_loops(array, 10, size/10);
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    free(array);
    return 0;
}
