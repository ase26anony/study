/* auto_inc_test.c - Test auto-increment/decrement pattern recognition */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Test 1: Using *(p + 0) with p++ */
__attribute__((noinline))
int test1_zero_offset_inc(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Explicit zero offset - should not be folded by frontend */
        int val = *(p + 0);
        
        /* Separate increment statement */
        p++;
        
        /* Use the value to prevent elimination */
        sum += val;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : "r"(val) : "memory");
    }
    
    /* Use volatile store to ensure loop isn't eliminated */
    g_volatile_sum += sum;
    return sum;
}

/* Test 2: Using p[0] with p += 1 */
__attribute__((noinline))
int test2_array_zero_inc(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with explicit zero index */
        int val = p[0];
        
        /* Separate increment with assignment */
        p += 1;
        
        sum += val;
        asm volatile("" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 3: Store instead of load with decrement */
__attribute__((noinline))
int test3_store_zero_dec(int *arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        
        /* Separate decrement */
        p--;
        
        /* Read back to ensure store happens */
        sum += arr[n - 1 - i];
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 4: Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
int test4_struct_ptr(int *arr, int n) {
    struct ptr_wrapper wrapper;
    wrapper.current = arr;
    wrapper.end = arr + n;
    
    int sum = 0;
    
    while (wrapper.current < wrapper.end) {
        /* Access through structure member with zero offset */
        int val = *(wrapper.current + 0);
        
        /* Increment the structure member */
        wrapper.current++;
        
        sum += val;
        
        /* Call external to create barrier */
        dummy_external();
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 5: Different increment patterns in same function */
__attribute__((noinline))
int test5_mixed_patterns(int *arr, int n) {
    int *p1 = arr;
    int *p2 = arr + n/2;
    int sum = 0;
    
    /* First half: increment after access */
    for (int i = 0; i < n/2; i++) {
        sum += *(p1 + 0);
        p1 = p1 + 1;  /* Different syntax for increment */
    }
    
    /* Second half: decrement before access */
    for (int i = 0; i < n/2; i++) {
        p2 = p2 - 1;
        sum += *(p2 + 0);
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 6: Pointer arithmetic in loop condition */
__attribute__((noinline))
int test6_ptr_in_condition(int *arr, int n) {
    int *p = arr;
    int *end = arr + n;
    int sum = 0;
    
    while (p < end) {
        /* Force zero offset through temporary */
        int *tmp = p;
        int val = *(tmp + 0);
        
        /* Increment in loop update */
        p++;
        
        sum += val;
        
        /* Use inline asm to force memory access */
        asm volatile("" : "+r"(val) : : "memory");
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Dummy external function definition */
void dummy_external(void) {
    /* Empty but prevents elimination */
    asm volatile("" : : : "memory");
}

int main(int argc, char **argv) {
    /* Use volatile and argc to prevent constant folding */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate array with dynamic size */
    int *array = (int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total = 0;
    
    /* Call all test functions */
    total += test1_zero_offset_inc(array, size);
    total += test2_array_zero_inc(array, size);
    total += test3_store_zero_dec(array, size, 10);
    total += test4_struct_ptr(array, size);
    total += test5_mixed_patterns(array, size);
    total += test6_ptr_in_condition(array, size);
    
    /* Print result to prevent elimination */
    printf("Total sum: %d\n", total);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    free(array);
    return 0;
}
