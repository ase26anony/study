/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void dummy_external(int);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Test 1: Using *(p + 0) with p++ */
__attribute__((noinline))
int test1_zero_offset_plus_plus(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        
        /* Separate pointer increment */
        p++;
        
        /* Use the value to prevent elimination */
        sum += val;
        asm volatile("" : : "r"(val)); /* Prevent optimization */
    }
    
    return sum;
}

/* Test 2: Using p[0] with p += 1 */
__attribute__((noinline))
int test2_array_zero_with_plus_eq(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        
        /* Separate pointer increment */
        p += 1;
        
        sum += val;
        dummy_external(val); /* External call prevents elimination */
    }
    
    return sum;
}

/* Test 3: Structure with pointer member */
typedef struct {
    int *current;
    int *end;
} PointerStruct;

__attribute__((noinline))
int test3_struct_member(PointerStruct *ps, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure pointer with zero offset */
        int val = *(ps->current + 0);
        
        /* Increment the structure member */
        ps->current++;
        
        sum += val;
        g_volatile_sum += val; /* Global volatile side effect */
    }
    
    return sum;
}

/* Test 4: Store instead of load with decrement */
__attribute__((noinline))
void test4_store_with_decrement(int *arr, int n, int value) {
    int *p = arr + n - 1; /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value;
        
        /* Separate pointer decrement */
        p--;
        
        /* Prevent dead store elimination */
        asm volatile("" : : "m"(*p));
    }
}

/* Test 5: Multiple memory accesses with zero offset */
__attribute__((noinline))
int test5_multiple_accesses(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Two separate accesses with zero offset */
        int val1 = *(p + 0);
        int val2 = *(p + 0);
        
        /* Single increment after both accesses */
        p++;
        
        sum += val1 + val2;
    }
    
    return sum;
}

/* Test 6: Pointer arithmetic in loop condition */
__attribute__((noinline))
int test6_pointer_in_condition(int *arr, int n) {
    int *p = arr;
    int *end = arr + n;
    int sum = 0;
    
    while (p < end) {
        /* Access with zero offset */
        int val = *(p + 0);
        
        /* Increment in loop update */
        p++;
        
        sum += val;
    }
    
    return sum;
}

/* Dummy external function implementation */
void dummy_external(int x) {
    /* Empty but prevents optimization */
    asm volatile("" : : "r"(x));
}

int main(int argc, char **argv) {
    /* Use argc to make array size non-constant */
    int size = g_volatile_bound + argc;
    if (size < 10) size = 100;
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Run all test functions */
    total_sum += test1_zero_offset_plus_plus(array, size);
    total_sum += test2_array_zero_with_plus_eq(array, size);
    
    PointerStruct ps = {array, array + size};
    total_sum += test3_struct_member(&ps, size);
    
    test4_store_with_decrement(array, size, 42);
    
    /* Re-initialize for store test */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    total_sum += test5_multiple_accesses(array, size);
    total_sum += test6_pointer_in_condition(array, size);
    
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    return 0;
}
