/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * for memory accesses with zero offset followed by pointer increment.
 */

#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_external(void);

/* Volatile sink to prevent dead code elimination */
static volatile int volatile_sink;

/* Prevent inlining to keep patterns intact */
__attribute__((noinline))
static void test1_ptr_plus_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    /* Use volatile bound to prevent loop unrolling */
    volatile int vol_n = n;
    
    for (int i = 0; i < vol_n; i++) {
        /* Access with explicit zero offset - key pattern */
        int val = *(p + 0);
        sum += val;
        
        /* Separate increment statement */
        p++;
    }
    
    volatile_sink = sum;
}

__attribute__((noinline))
static void test2_array_zero_index(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    volatile int vol_n = n;
    
    for (int i = 0; i < vol_n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        
        /* Increment with += 1 */
        p += 1;
    }
    
    volatile_sink = sum;
}

__attribute__((noinline))
static void test3_store_pattern(int *arr, int n, int value) {
    int *p = arr;
    
    volatile int vol_n = n;
    
    for (int i = 0; i < vol_n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        
        /* Decrement pattern */
        p--;
    }
    
    /* Force memory side effect */
    asm volatile("" : : "r"(arr[0]) : "memory");
}

__attribute__((noinline))
static void test4_mixed_offsets(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    volatile int vol_n = n;
    
    for (int i = 0; i < vol_n; i++) {
        /* Multiple zero-offset accesses */
        sum += *(p + 0);
        sum += *(p + 0);  /* Second access to same location */
        
        /* Increment */
        p = p + 1;
    }
    
    volatile_sink = sum;
}

__attribute__((noinline))
static void test5_struct_member(void) {
    struct Buffer {
        int *ptr;
        int count;
    } buf;
    
    /* Allocate array */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    buf.ptr = array;
    buf.count = 100;
    
    int sum = 0;
    for (int i = 0; i < buf.count; i++) {
        /* Access through struct member with zero offset */
        sum += *(buf.ptr + 0);
        
        /* Increment struct member */
        buf.ptr++;
    }
    
    volatile_sink = sum;
}

__attribute__((noinline))
static void test6_pointer_to_pointer(int **arr, int n) {
    int **p = arr;
    int sum = 0;
    
    volatile int vol_n = n;
    
    for (int i = 0; i < vol_n; i++) {
        /* Double dereference with zero offset */
        int val = **(p + 0);
        sum += val;
        
        p++;
    }
    
    volatile_sink = sum;
}

__attribute__((noinline))
static void test7_volatile_memory_access(volatile int *arr, int n) {
    volatile int *p = arr;
    int sum = 0;
    
    volatile int vol_n = n;
    
    for (int i = 0; i < vol_n; i++) {
        /* Volatile access with zero offset */
        sum += *(p + 0);
        
        p++;
    }
    
    volatile_sink = sum;
}

__attribute__((noinline))
static void test8_post_and_pre_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    volatile int vol_n = n;
    
    for (int i = 0; i < vol_n; i++) {
        /* Test both post and pre increments */
        sum += *(p + 0);
        int *old_p = p++;
        
        /* Use old_p to prevent elimination */
        asm volatile("" : : "r"(old_p) : "memory");
    }
    
    volatile_sink = sum;
}

int main(int argc, char **argv) {
    /* Dynamic array size based on argc to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    if (size > 10000) size = 10000;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    int **ptr_arr = (int**)malloc(size * sizeof(int*));
    
    if (!arr1 || !arr2 || !ptr_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < size; i++) {
        arr1[i] = i * 2;
        arr2[i] = i * 3;
        ptr_arr[i] = &arr2[i];
    }
    
    /* Call all test functions */
    test1_ptr_plus_zero(arr1, size);
    test2_array_zero_index(arr1, size);
    test3_store_pattern(arr2, size, 42);
    test4_mixed_offsets(arr1, size);
    test5_struct_member();
    test6_pointer_to_pointer(ptr_arr, size);
    test7_volatile_memory_access(arr1, size);
    test8_post_and_pre_increment(arr1, size);
    
    /* Aggregate results to prevent elimination */
    int total = volatile_sink;
    
    /* Use results */
    printf("Total: %d\n", total);
    printf("Array elements: %d, %d\n", arr1[0], arr2[0]);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(ptr_arr);
    
    return 0;
}
