/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_use(int val);

/* Volatile variable to prevent constant propagation */
volatile int g_volatile_bound = 100;

/* Test 1: Using *(p + 0) with p++ */
__attribute__((noinline))
int test1_zero_offset_plus_plus(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Explicit zero offset - should not be folded by frontend */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    /* Use result to prevent elimination */
    dummy_use(sum);
    return sum;
}

/* Test 2: Using p[0] with p += 1 */
__attribute__((noinline))
int test2_array_zero_with_plus_eq(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Array access with explicit zero index */
        int val = p[0];
        sum += val;
        /* Separate increment with += */
        p += 1;
    }
    
    dummy_use(sum);
    return sum;
}

/* Test 3: Structure with pointer member */
typedef struct {
    int *current;
    int *end;
} PtrStruct;

__attribute__((noinline))
int test3_struct_ptr_member(int *arr, int n) {
    int sum = 0;
    PtrStruct ps;
    ps.current = arr;
    ps.end = arr + n;
    
    while (ps.current < ps.end) {
        /* Access through structure pointer member */
        int val = *(ps.current + 0);
        sum += val;
        /* Increment pointer in structure */
        ps.current++;
    }
    
    dummy_use(sum);
    return sum;
}

/* Test 4: Store instead of load */
__attribute__((noinline))
void test4_store_with_decrement(int *arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate decrement */
        p--;
    }
    
    /* Force memory writes */
    asm volatile("" : : "r"(arr[0]) : "memory");
}

/* Test 5: Different increment amount (should still match) */
__attribute__((noinline))
int test5_post_increment_load(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Load with post-increment in expression */
        int val = *p;
        sum += val;
        /* Separate increment by 1 */
        p = p + 1;
    }
    
    dummy_use(sum);
    return sum;
}

/* Test 6: Complex zero offset expression */
__attribute__((noinline))
int test6_complex_zero_offset(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* More complex zero offset that should simplify to *(p + 0) */
        int val = *(p + (i - i));
        sum += val;
        p++;
    }
    
    dummy_use(sum);
    return sum;
}

/* Dummy function implementation */
void dummy_use(int val) {
    /* Use inline asm to prevent elimination */
    asm volatile("" : : "r"(val));
}

int main(int argc, char **argv) {
    /* Use argc to make array size non-constant */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate and initialize array */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Run all test functions */
    total_sum += test1_zero_offset_plus_plus(array, size);
    total_sum += test2_array_zero_with_plus_eq(array, size);
    total_sum += test3_struct_ptr_member(array, size);
    
    test4_store_with_decrement(array, size, 42);
    
    total_sum += test5_post_increment_load(array, size);
    total_sum += test6_complex_zero_offset(array, size);
    
    /* Use result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    return 0;
}
