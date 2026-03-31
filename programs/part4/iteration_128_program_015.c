/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_external(int);

/* Volatile variable to prevent constant propagation */
volatile int g_volatile_bound = 100;

/* Prevent inlining to keep patterns intact */
__attribute__((noinline))
int test_variant1(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Loop with *(p + 0) access and p++ increment */
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        
        /* Separate pointer increment */
        p++;
        
        /* Use the value to prevent elimination */
        sum += val;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : "r"(val));
    }
    
    /* Use sum to prevent dead code elimination */
    dummy_external(sum);
    return sum;
}

__attribute__((noinline))
int test_variant2(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Loop with p[0] access and p += 1 increment */
    for (int i = 0; i < n; i++) {
        /* Array access with zero index */
        int val = p[0];
        
        /* Separate pointer increment */
        p += 1;
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    
    dummy_external(sum);
    return sum;
}

__attribute__((noinline))
int test_variant3(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Loop with store instead of load */
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = i;
        
        /* Pointer decrement */
        p--;
        
        /* Read back to ensure side effect */
        sum += *(p + 1);
        asm volatile("" : : "r"(i));
    }
    
    dummy_external(sum);
    return sum;
}

/* Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
int test_variant4(int *arr, int n) {
    struct ptr_wrapper wrapper;
    wrapper.current = arr;
    wrapper.end = arr + n;
    
    int sum = 0;
    
    /* Loop using structure member */
    while (wrapper.current < wrapper.end) {
        /* Access through structure member with zero offset */
        int val = *(wrapper.current + 0);
        
        /* Increment structure member */
        wrapper.current++;
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    
    dummy_external(sum);
    return sum;
}

/* Dummy external function implementation */
void dummy_external(int x) {
    /* Empty but prevents elimination */
    asm volatile("" : : "r"(x));
}

int main(int argc, char **argv) {
    /* Use volatile to prevent compile-time optimization */
    volatile int array_size = (argc > 1) ? atoi(argv[1]) : g_volatile_bound;
    
    if (array_size <= 0) array_size = 100;
    
    /* Dynamically allocate array to prevent static analysis */
    int *array = (int*)malloc(sizeof(int) * array_size);
    if (!array) return 1;
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < array_size; i++) {
        array[i] = i * 3 + (argc % 7); /* Non-constant pattern */
    }
    
    int total_sum = 0;
    
    /* Call all variants to test different patterns */
    total_sum += test_variant1(array, array_size);
    total_sum += test_variant2(array, array_size);
    
    /* For variant3, we need to reinitialize since it modifies the array */
    for (int i = 0; i < array_size; i++) {
        array[i] = i * 2 + (argc % 5);
    }
    total_sum += test_variant3(array, array_size);
    
    /* Reinitialize for variant4 */
    for (int i = 0; i < array_size; i++) {
        array[i] = i * 4 + (argc % 3);
    }
    total_sum += test_variant4(array, array_size);
    
    /* Print result to prevent elimination */
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    return 0;
}
