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
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(val));
    }
    
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
        
        /* Separate pointer increment with assignment */
        p += 1;
        
        /* Prevent optimization */
        dummy_external();
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Variant 3: Using structure with pointer member */
struct pointer_wrapper {
    int *ptr;
    int dummy;
};

__attribute__((noinline))
int test_variant3(int *arr, int n) {
    struct pointer_wrapper wrapper;
    wrapper.ptr = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure member with zero offset */
        int val = *(wrapper.ptr + 0);
        sum += val;
        
        /* Increment the pointer in the structure */
        wrapper.ptr = wrapper.ptr + 1;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(val), "r"(wrapper.ptr));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Variant 4: Store operation instead of load */
__attribute__((noinline))
void test_variant4(int *arr, int n, int value) {
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Store with explicit zero offset */
        *(p + 0) = value + i;
        
        /* Separate pointer decrement */
        p--;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p));
    }
}

/* Variant 5: Different increment pattern */
__attribute__((noinline))
int test_variant5(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    volatile int *volatile_ptr = &sum;
    
    for (int i = 0; i < n; i++) {
        /* Complex zero offset expression */
        int val = *(0 + p);
        *volatile_ptr += val;
        
        /* Post-increment in separate statement */
        p = p + 1;
        
        /* Use volatile to prevent elimination */
        (void)*volatile_ptr;
    }
    
    return sum;
}

/* Variant 6: While loop with pointer increment */
__attribute__((noinline))
int test_variant6(int *arr, int n) {
    int *p = arr;
    int *end = arr + n;
    int sum = 0;
    
    while (p < end) {
        /* Access with zero offset */
        int val = *(p + 0);
        sum ^= val;  /* Different operation */
        
        /* Pointer increment at end of loop */
        p++;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Dummy external function implementation */
void dummy_external(void) {
    /* Empty but prevents optimization */
    asm volatile("");
}

int main(int argc, char **argv) {
    /* Use argc to determine array size at runtime */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(size * sizeof(int));
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
    test_variant4(array + size/2, size/4, 42);
    
    total_sum += test_variant5(array, size);
    total_sum += test_variant6(array, size);
    
    /* Use results to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    free(array);
    return 0;
}
