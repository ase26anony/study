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
int test1_zero_offset_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate pointer increment */
        p++;
    }
    
    /* Use volatile to ensure side effect */
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
        /* Pointer increment by 1 */
        p += 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 3: Using structure with pointer member */
struct PointerHolder {
    int *ptr;
    int count;
};

__attribute__((noinline))
int test3_struct_pointer(struct PointerHolder *holder, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure pointer with zero offset */
        int val = *(holder->ptr + 0);
        sum += val;
        /* Increment the pointer in the structure */
        holder->ptr++;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 4: Store instead of load with decrement */
__attribute__((noinline))
void test4_store_decrement(int *arr, int n, int value) {
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

/* Test 5: More complex zero offset expression */
__attribute__((noinline))
int test5_complex_zero_offset(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex but still zero offset */
        int val = *(0 + p + 0);
        sum += val;
        
        /* Increment with different syntax */
        p = p + 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 6: Pointer arithmetic in loop condition */
__attribute__((noinline))
int test6_pointer_in_condition(int *arr, int n) {
    int *p = arr;
    int *end = arr + n;
    int sum = 0;
    
    while (p < end) {
        /* Access with zero offset */
        int val = *(p + 0);
        sum += val;
        
        /* Increment at end of loop */
        p++;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 7: Using asm to prevent elimination */
__attribute__((noinline))
int test7_asm_volatile(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Use asm to ensure memory access isn't eliminated */
        int val;
        __asm__ volatile("" : "=r"(val) : "0"(*(p + 0)));
        sum += val;
        
        p++;
    }
    
    return sum;
}

/* Test 8: Nested loops with pointer update */
__attribute__((noinline))
int test8_nested_loops(int *arr, int rows, int cols) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Zero offset access */
            sum += *(p + 0);
            p++;
        }
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Dummy external function implementation */
void dummy_external(void) {
    /* Empty but external to prevent optimization */
}

int main(int argc, char *argv[]) {
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
    total_sum += test1_zero_offset_increment(array, size);
    total_sum += test2_array_zero_offset(array, size);
    
    struct PointerHolder holder = {array, size};
    total_sum += test3_struct_pointer(&holder, size);
    
    test4_store_decrement(array, size, 42);
    
    /* Re-initialize array after test4 modifies it */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    total_sum += test5_complex_zero_offset(array, size);
    total_sum += test6_pointer_in_condition(array, size);
    total_sum += test7_asm_volatile(array, size);
    
    /* For nested test, use smaller dimensions */
    int rows = 10;
    int cols = size / 10;
    if (cols < 1) cols = 1;
    total_sum += test8_nested_loops(array, rows, cols);
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    free(array);
    return 0;
}
