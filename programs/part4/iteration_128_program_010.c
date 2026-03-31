/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Test 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_zero_offset_plus_plus(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access with explicit zero offset - should not fold to *p */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    /* Use volatile to prevent dead code elimination */
    g_volatile_sum += sum;
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_offset(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Separate increment with assignment */
        p += 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 3: Using structure with pointer member */
typedef struct {
    int *current;
    int *end;
} PointerStruct;

__attribute__((noinline))
int test3_struct_pointer(PointerStruct *ps, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure pointer with zero offset */
        int val = *(ps->current + 0);
        sum += val;
        /* Increment the pointer in the structure */
        ps->current = ps->current + 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 4: Store instead of load with decrement */
__attribute__((noinline))
void test4_store_zero_offset_decrement(int *arr, int n, int value) {
    /* Start from the end and decrement */
    int *p = arr + n - 1;
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate decrement */
        p--;
    }
    
    /* Force side effect */
    dummy_external();
}

/* Test 5: More complex zero offset expression */
__attribute__((noinline))
int test5_complex_zero_offset(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex-looking but still zero offset */
        int val = *(p + (i - i));
        sum += val;
        /* Increment with post-increment in separate statement */
        p = p + 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 6: Pointer arithmetic in loop with volatile to prevent folding */
__attribute__((noinline))
int test6_volatile_prevention(int *arr, volatile int n) {
    int *p = arr;
    int sum = 0;
    volatile int *volatile_ptr = &sum;
    
    for (int i = 0; i < n; i++) {
        /* Use inline asm to ensure access isn't eliminated */
        int val;
        __asm__ volatile("" : "=r"(val) : "0"(*(p + 0)));
        sum += val;
        p++;
        
        /* Volatile store to prevent reordering */
        *volatile_ptr = sum;
    }
    
    return sum;
}

/* Test 7: Nested loops to create different patterns */
__attribute__((noinline))
int test7_nested_loops(int *arr, int rows, int cols) {
    int sum = 0;
    
    for (int r = 0; r < rows; r++) {
        int *p = arr + r * cols;
        
        for (int c = 0; c < cols; c++) {
            /* Access with zero offset */
            sum += *(p + 0);
            /* Increment pointer */
            p += 1;
        }
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Dummy external function implementation */
void dummy_external(void) {
    /* Empty but external to prevent optimization */
}

int main(int argc, char **argv) {
    /* Use argc to determine array size at runtime */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
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
    total_sum += test2_array_zero_offset(array, size);
    
    PointerStruct ps = {array, array + size};
    total_sum += test3_struct_pointer(&ps, size);
    
    test4_store_zero_offset_decrement(array, size, 42);
    
    /* Re-initialize array after store test */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    total_sum += test5_complex_zero_offset(array, size);
    total_sum += test6_volatile_prevention(array, size);
    total_sum += test7_nested_loops(array, 10, size/10);
    
    /* Print result to prevent elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    free(array);
    return 0;
}
