/* Test program for GCC auto-increment/decrement optimization */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -march=armv7-a -c test.c */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Test 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_zero_offset_inc(int *arr, int n) {
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

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_inc(int *arr, int n) {
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

/* Test 3: Using structure with pointer member */
struct ptr_wrapper {
    int *ptr;
    int count;
};

__attribute__((noinline))
int test3_struct_ptr(struct ptr_wrapper *w, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure pointer with zero offset */
        int val = *(w->ptr + 0);
        sum += val;
        /* Increment the pointer in structure */
        w->ptr = w->ptr + 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 4: Store instead of load with decrement */
__attribute__((noinline))
void test4_store_zero_dec(int *arr, int n, int value) {
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

/* Test 5: Complex zero offset calculation */
__attribute__((noinline))
int test5_complex_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* More complex zero offset expression */
        int val = *(0 + p + 0);
        sum += val;
        /* Increment with post-increment in separate statement */
        int *old = p;
        p = old + 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 6: Using asm to prevent optimization */
__attribute__((noinline))
int test6_asm_volatile(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Force memory access to survive */
        int val;
        __asm__ volatile("" : "=r"(val) : "0"(*(p + 0)));
        sum += val;
        p++;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 7: Nested loops with zero offset */
__attribute__((noinline))
int test7_nested_loops(int *arr, int rows, int cols) {
    int sum = 0;
    int *p = arr;
    
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            /* Zero offset access in inner loop */
            sum += *(p + 0);
            p += 1;
        }
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Dummy external function implementation */
void dummy_external(void) {
    /* Empty but external to prevent inlining */
}

int main(int argc, char **argv) {
    /* Use argc to make array size non-constant */
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
    
    int total = 0;
    
    /* Run all tests */
    total += test1_zero_offset_inc(array, size);
    total += test2_array_zero_inc(array, size);
    
    struct ptr_wrapper w = {array, size};
    total += test3_struct_ptr(&w, size);
    
    test4_store_zero_dec(array, size, 42);
    
    /* Re-initialize for store test */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    total += test5_complex_zero(array, size);
    total += test6_asm_volatile(array, size);
    total += test7_nested_loops(array, 10, size/10);
    
    printf("Total sum: %d\n", total);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    free(array);
    return 0;
}
