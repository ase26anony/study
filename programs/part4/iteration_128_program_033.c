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
int test1_zero_offset_plus_plus(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        /* Separate pointer increment */
        p++;
        
        sum += val;
        /* Use volatile to prevent elimination */
        g_volatile_sum = sum;
    }
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_plus_equals(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        /* Separate pointer increment */
        p += 1;
        
        sum += val;
        /* Inline asm to prevent elimination */
        asm volatile("" : : "r"(val));
    }
    return sum;
}

/* Test 3: Using structure with pointer member */
struct PointerHolder {
    int *ptr;
    int count;
};

__attribute__((noinline))
int test3_struct_member(struct PointerHolder *holder, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure pointer with zero offset */
        int val = *(holder->ptr + 0);
        /* Increment the pointer member */
        holder->ptr = holder->ptr + 1;
        
        sum += val;
        dummy_external(val);
    }
    return sum;
}

/* Test 4: Store instead of load with decrement */
__attribute__((noinline))
void test4_store_zero_offset_decrement(int *arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value;
        /* Separate pointer decrement */
        p--;
        
        /* Prevent elimination */
        asm volatile("" : : "m"(*p));
    }
}

/* Test 5: Different increment amount (should not match, but included for completeness) */
__attribute__((noinline))
int test5_different_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i += 2) {
        int val = *(p + 0);
        p = p + 2;  /* Increment by 2, not 1 */
        
        sum += val;
        g_volatile_sum = sum;
    }
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
        p = 1 + p;  /* p = p + 1 written differently */
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    return sum;
}

/* Dummy external function implementation */
void dummy_external(int x) {
    /* Empty but prevents elimination */
    (void)x;
}

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
    total_sum += test1_zero_offset_plus_plus(array, size);
    total_sum += test2_array_zero_plus_equals(array, size);
    
    struct PointerHolder holder = {array, size};
    total_sum += test3_struct_member(&holder, size);
    
    test4_store_zero_offset_decrement(array, size, 42);
    
    /* Re-initialize array for test5 */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    total_sum += test5_different_increment(array, size);
    
    total_sum += test6_complex_zero_offset(array, size);
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    return 0;
}
