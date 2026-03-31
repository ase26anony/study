/* Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void dummy_external(int);

/* Volatile variable to prevent constant propagation */
volatile int g_volatile_bound = 100;

/* Prevent inlining to isolate patterns */
__attribute__((noinline))
void test_ptr_plus_zero_increment(int *arr, int n) {
    int *p = arr;
    volatile int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 1: *(p + 0) with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    /* Use the result to prevent dead code elimination */
    dummy_external(sum);
}

__attribute__((noinline))
void test_array_zero_increment(int *arr, int n) {
    int *p = arr;
    volatile int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 2: p[0] access (should become *(p + 0)) */
        int val = p[0];
        sum += val;
        /* Separate increment with += 1 */
        p += 1;
    }
    
    dummy_external(sum);
}

__attribute__((noinline))
void test_ptr_plus_zero_decrement(int *arr, int n) {
    int *p = arr + n - 1;  /* Start from end */
    volatile int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 3: *(p + 0) with decrement */
        int val = *(p + 0);
        sum += val;
        /* Separate decrement statement */
        p--;
    }
    
    dummy_external(sum);
}

__attribute__((noinline))
void test_store_ptr_plus_zero(int *arr, int n, int value) {
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 4: Store with *(p + 0) */
        *(p + 0) = value + i;
        /* Separate increment */
        p++;
    }
    
    /* Verify stores by reading back */
    volatile int check = arr[n/2];
    dummy_external(check);
}

/* Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
void test_struct_ptr_zero_offset(struct ptr_wrapper *w, int n) {
    volatile int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 5: Access through structure member with zero offset */
        int val = *(w->current + 0);
        sum += val;
        /* Increment the structure member */
        w->current++;
    }
    
    dummy_external(sum);
}

__attribute__((noinline))
void test_mixed_offsets(int *arr, int n) {
    int *p = arr;
    volatile int sum = 0;
    
    /* Mix zero and non-zero offsets to test pattern recognition */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            /* Zero offset case */
            sum += *(p + 0);
        } else {
            /* Non-zero offset - should not match the pattern */
            sum += *(p + 1);
            p++;  /* Extra increment to compensate */
        }
        p++;
    }
    
    dummy_external(sum);
}

/* Dummy function definition to satisfy extern declaration */
void dummy_external(int x) {
    /* Use inline asm to prevent elimination */
    asm volatile("" : : "r"(x));
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
        array[i] = i * 3 + 1;
    }
    
    /* Create structure for struct test */
    struct ptr_wrapper wrapper;
    wrapper.current = array;
    wrapper.end = array + size;
    
    volatile int total = 0;
    
    /* Call all test functions */
    test_ptr_plus_zero_increment(array, size);
    total += array[0];  /* Use array value */
    
    test_array_zero_increment(array, size);
    total += array[size/2];
    
    test_ptr_plus_zero_decrement(array, size);
    total += array[size-1];
    
    test_store_ptr_plus_zero(array, size, 42);
    total += array[0];
    
    /* Reset wrapper for struct test */
    wrapper.current = array;
    test_struct_ptr_zero_offset(&wrapper, size);
    total += array[0];
    
    test_mixed_offsets(array, size);
    total += array[size-1];
    
    /* Print result to prevent elimination */
    printf("Total: %d\n", total);
    
    free(array);
    return 0;
}
