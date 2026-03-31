/* Test program for GCC auto-increment/decrement optimization */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -march=armv7-a -c test.c */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Variant 1: Using *(p + 0) and p++ */
__attribute__((noinline))
static int test_variant1(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        
        /* Separate pointer increment */
        p++;
        
        /* Use the value to prevent elimination */
        sum += val;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : "r"(val) : "memory");
    }
    
    /* Prevent dead code elimination */
    g_volatile_sum += sum;
    return sum;
}

/* Variant 2: Using p[0] and p += 1 */
__attribute__((noinline))
static int test_variant2(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with zero index */
        int val = p[0];
        
        /* Separate pointer increment */
        p += 1;
        
        sum += val;
        asm volatile("" : : "r"(val) : "memory");
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Variant 3: Structure with pointer member */
struct pointer_wrapper {
    int *ptr;
    int dummy;
};

__attribute__((noinline))
static int test_variant3(int *arr, int n) {
    struct pointer_wrapper wrapper;
    wrapper.ptr = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure member with zero offset */
        int val = *(wrapper.ptr + 0);
        
        /* Increment the pointer in the structure */
        wrapper.ptr = wrapper.ptr + 1;
        
        sum += val;
        asm volatile("" : : "r"(val) : "memory");
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Variant 4: Store instead of load with decrement */
__attribute__((noinline))
static void test_variant4(int *arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value;
        
        /* Separate pointer decrement */
        p--;
        
        /* Memory barrier */
        asm volatile("" : : "r"(value) : "memory");
    }
}

/* Variant 5: More complex zero offset expression */
__attribute__((noinline))
static int test_variant5(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex but still zero offset */
        int val = *(p + (0 * 1));
        
        /* Increment with post-increment operator */
        p = p + 1;
        
        sum += val;
        asm volatile("" : : "r"(val) : "memory");
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Variant 6: Loop with pointer comparison */
__attribute__((noinline))
static int test_variant6(int *arr, int n) {
    int *p = arr;
    int *end = arr + n;
    int sum = 0;
    
    while (p < end) {
        /* Access with zero offset */
        int val = *(p + 0);
        
        /* Increment in loop update */
        p++;
        
        sum += val;
        asm volatile("" : : "r"(val) : "memory");
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Main function that calls all variants */
int main(int argc, char **argv) {
    /* Use volatile bound to prevent constant folding */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
    }
    
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Call all test variants */
    total_sum += test_variant1(array, size);
    total_sum += test_variant2(array, size);
    total_sum += test_variant3(array, size);
    
    /* Test store variant */
    test_variant4(array, size, 42);
    
    total_sum += test_variant5(array, size);
    total_sum += test_variant6(array, size);
    
    /* Use results to prevent elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    /* Verify array was modified by store variant */
    int verify_sum = 0;
    for (int i = 0; i < size; i++) {
        verify_sum += array[i];
    }
    printf("Array sum: %d\n", verify_sum);
    
    free(array);
    return 0;
}
