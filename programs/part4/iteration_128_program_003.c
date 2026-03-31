/* Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Variant 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test_variant1(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate pointer increment */
        p++;
    }
    
    /* Use result to prevent elimination */
    g_volatile_sum += sum;
    return sum;
}

/* Variant 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test_variant2(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Pointer increment with assignment */
        p += 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Variant 3: Using structure with pointer member */
struct ptr_wrapper {
    int *ptr;
};

__attribute__((noinline))
int test_variant3(int *arr, int n) {
    struct ptr_wrapper wrapper;
    wrapper.ptr = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure member with zero offset */
        int val = *(wrapper.ptr + 0);
        sum += val;
        /* Increment the pointer in the structure */
        wrapper.ptr = wrapper.ptr + 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Variant 4: Store instead of load with decrement */
__attribute__((noinline))
void test_variant4(int *arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate pointer decrement */
        p--;
    }
    
    /* Force side effect */
    dummy_external();
}

/* Variant 5: Mixed operations in loop */
__attribute__((noinline))
int test_variant5(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    /* Use volatile bound to prevent loop unrolling */
    volatile int bound = n;
    
    for (int i = 0; i < bound; i++) {
        /* Multiple memory accesses with zero offset */
        int val1 = *(p + 0);
        sum += val1;
        
        /* Inline asm to prevent elimination */
        __asm__ volatile("" : : "r"(val1));
        
        /* Pointer increment */
        p = p + 1;
    }
    
    return sum;
}

/* Variant 6: Pointer arithmetic in separate statements */
__attribute__((noinline))
int test_variant6(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    while (n-- > 0) {
        /* Zero offset access in separate steps */
        int *temp = p + 0;
        int val = *temp;
        sum += val;
        
        /* Separate increment */
        p = p + 1;
    }
    
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
    
    /* Call all test variants */
    total_sum += test_variant1(array, size);
    total_sum += test_variant2(array, size);
    total_sum += test_variant3(array, size);
    
    test_variant4(array, size, 42);
    
    total_sum += test_variant5(array, size);
    total_sum += test_variant6(array, size);
    
    /* Use results to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    /* Verify array was modified */
    int verify_sum = 0;
    for (int i = 0; i < size; i++) {
        verify_sum += array[i];
    }
    printf("Array sum: %d\n", verify_sum);
    
    free(array);
    return 0;
}

/* Dummy external function implementation */
void dummy_external(void) {
    /* Empty but prevents optimization */
}
