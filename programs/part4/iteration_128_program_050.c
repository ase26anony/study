/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination of memory accesses */
extern void use_value(int val);
extern void touch_pointer(void *ptr);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int *g_volatile_base;

/* Prevent inlining to isolate optimization patterns */
__attribute__((noinline))
int test_ptr_plus_zero_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access with explicit zero offset - should generate *(p + 0) */
        int val = *(p + 0);
        sum += val;
        
        /* Separate increment statement */
        p++;
    }
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : : "r"(sum));
    return sum;
}

__attribute__((noinline))
int test_array_zero_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        
        /* Pointer increment with += operator */
        p += 1;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

__attribute__((noinline))
int test_ptr_plus_zero_decrement(int *arr, int n) {
    /* Start from end and decrement */
    int *p = arr + n - 1;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        
        /* Separate decrement statement */
        p--;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

__attribute__((noinline))
int test_store_ptr_plus_zero(int *arr, int n, int value) {
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        
        /* Increment after store */
        p++;
    }
    
    /* Verify some stores happened */
    int check = arr[n/2];
    asm volatile("" : : "r"(check));
    return check;
}

/* Structure with pointer member */
struct PointerHolder {
    int *ptr;
    int count;
};

__attribute__((noinline))
int test_struct_member_increment(struct PointerHolder *h, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure member with zero offset */
        int val = *(h->ptr + 0);
        sum += val;
        
        /* Increment the structure member */
        h->ptr++;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

__attribute__((noinline))
int test_volatile_based_loop(void) {
    /* Use volatile to prevent optimization of loop bounds */
    int size = g_volatile_bound;
    int *arr = (int*)g_volatile_base;
    
    if (!arr || size <= 0) return 0;
    
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Force zero-offset access pattern */
        int val = *(p + 0);
        sum += val;
        
        /* Separate increment */
        p = p + 1;
    }
    
    return sum;
}

__attribute__((noinline))
int test_mixed_operations(int *arr, int n) {
    int *p1 = arr;
    int *p2 = arr + n/2;
    int sum = 0;
    
    for (int i = 0; i < n/2; i++) {
        /* Load from p1 with zero offset */
        int val1 = *(p1 + 0);
        
        /* Load from p2 with zero offset */
        int val2 = *(p2 + 0);
        
        sum += val1 * val2;
        
        /* Increment both pointers */
        p1++;
        p2 += 1;  /* Different syntax for increment */
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    /* Determine array size at runtime */
    int size = (argc > 1) ? atoi(argv[1]) : 1000;
    if (size <= 0) size = 1000;
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i % 100;
    }
    
    /* Set up volatile globals */
    g_volatile_bound = size;
    g_volatile_base = array;
    
    int total = 0;
    
    /* Test various patterns */
    total += test_ptr_plus_zero_increment(array, size);
    total += test_array_zero_increment(array, size);
    total += test_ptr_plus_zero_decrement(array, size);
    
    /* Test store pattern */
    test_store_ptr_plus_zero(array, size, 42);
    
    /* Test structure pattern */
    struct PointerHolder holder = {array, size};
    total += test_struct_member_increment(&holder, size);
    
    /* Test volatile-based pattern */
    total += test_volatile_based_loop();
    
    /* Test mixed operations */
    total += test_mixed_operations(array, size);
    
    /* Print result to prevent elimination */
    printf("Total: %d\n", total);
    
    free(array);
    return 0;
}
