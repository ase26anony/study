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
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        
        /* Separate pointer increment */
        p++;
        
        /* Use the value to prevent elimination */
        sum += val;
        asm volatile("" : : "r"(val));  /* Prevent optimization */
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_index(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        
        /* Separate pointer increment */
        p += 1;
        
        sum += val;
        dummy_external(val);  /* External call prevents elimination */
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 3: Store instead of load with p-- */
__attribute__((noinline))
void test3_store_and_decrement(int *arr, int n, int value) {
    int *p = &arr[n - 1];  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        
        /* Separate pointer decrement */
        p--;
        
        asm volatile("" : : "m"(*p));  /* Memory barrier */
    }
}

/* Test 4: Structure with pointer member */
typedef struct {
    int *current;
    int *end;
} PointerStruct;

__attribute__((noinline))
int test4_struct_member(PointerStruct *ps, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure pointer with zero offset */
        int val = *(ps->current + 0);
        
        /* Increment the pointer member */
        ps->current++;
        
        sum += val;
        asm volatile("" : : "r"(val), "m"(ps->current));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 5: Different increment amount (shouldn't match but tests pattern) */
__attribute__((noinline))
int test5_different_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i += 2) {
        int val = *(p + 0);
        p += 2;  /* Increment by 2 */
        sum += val;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 6: Complex expression with zero offset */
__attribute__((noinline))
int test6_complex_zero_offset(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* More complex zero offset expression */
        int val = *(0 + p + 0);
        
        /* Increment in separate statement */
        p = p + 1;
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Dummy external function implementation */
void dummy_external(int x) {
    /* Empty but external linkage prevents optimization */
    asm volatile("" : : "r"(x));
}

int main(int argc, char **argv) {
    /* Use volatile and argc to prevent compile-time optimization */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate array with dynamic size */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Run all tests */
    total_sum += test1_ptr_plus_zero(array, size);
    total_sum += test2_array_zero_index(array, size);
    
    test3_store_and_decrement(array, size, 42);
    
    PointerStruct ps = {array, array + size};
    total_sum += test4_struct_member(&ps, size);
    
    total_sum += test5_different_increment(array, size);
    total_sum += test6_complex_zero_offset(array, size);
    
    /* Use results to prevent dead code elimination */
    printf("Total sum: %d (volatile sum: %d)\n", total_sum, g_volatile_sum);
    
    /* Verify array was modified */
    int verify_sum = 0;
    for (int i = 0; i < size; i++) {
        verify_sum += array[i];
    }
    printf("Array sum: %d\n", verify_sum);
    
    free(array);
    return 0;
}
