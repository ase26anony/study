/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_use(int val);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int* g_volatile_ptr = NULL;

/* Test 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_zero_offset_plus_plus(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate pointer increment */
        p++;
    }
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_offset(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Separate pointer increment with assignment */
        p += 1;
    }
    return sum;
}

/* Test 3: Structure with pointer member */
struct ptr_wrapper {
    int* current;
    int* end;
};

__attribute__((noinline))
int test3_struct_member(int* arr, int n) {
    struct ptr_wrapper wrapper;
    wrapper.current = arr;
    wrapper.end = arr + n;
    
    int sum = 0;
    while (wrapper.current < wrapper.end) {
        /* Access through structure member with zero offset */
        int val = *(wrapper.current + 0);
        sum += val;
        /* Increment structure member */
        wrapper.current = wrapper.current + 1;
    }
    return sum;
}

/* Test 4: Store instead of load with decrement */
__attribute__((noinline))
void test4_store_decrement(int* arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate pointer decrement */
        p--;
    }
}

/* Test 5: Mixed pattern with volatile to force memory access */
__attribute__((noinline))
int test5_mixed_volatile(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    volatile int* vp = (volatile int*)p;
    
    for (int i = 0; i < n; i++) {
        /* Force memory access with inline asm */
        int val;
        asm volatile("" : "=r"(val) : "m"(*vp));
        sum += val;
        
        /* Access with zero offset */
        val = *(p + 0);
        sum += val;
        
        /* Increment both pointers */
        p = p + 1;
        vp = (volatile int*)p;
    }
    return sum;
}

/* Test 6: Double increment pattern */
__attribute__((noinline))
int test6_double_increment(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i += 2) {
        /* Two consecutive accesses with zero offset */
        int val1 = *(p + 0);
        p = p + 1;
        
        int val2 = *(p + 0);
        p = p + 1;
        
        sum += val1 + val2;
    }
    return sum;
}

/* Test 7: Function pointer parameter */
__attribute__((noinline))
int test7_pointer_param(int* p, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access parameter directly */
        sum += *(p + 0);
        /* Increment parameter */
        p++;
    }
    return sum;
}

/* Main function that exercises all patterns */
int main(int argc, char** argv) {
    /* Use argc to make array size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : g_volatile_bound;
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    int* array = (int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Run all tests */
    total_sum += test1_zero_offset_plus_plus(array, size);
    total_sum += test2_array_zero_offset(array, size);
    total_sum += test3_struct_member(array, size);
    
    test4_store_decrement(array, size, 42);
    
    total_sum += test5_mixed_volatile(array, size);
    total_sum += test6_double_increment(array, size);
    total_sum += test7_pointer_param(array, size);
    
    /* Use results to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    /* Verify array was modified by test4 */
    int verify_sum = 0;
    for (int i = 0; i < size; i++) {
        verify_sum += array[i];
    }
    printf("Array sum after modifications: %d\n", verify_sum);
    
    free(array);
    return 0;
}
