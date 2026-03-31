/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_use(int val);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int* g_volatile_ptr = NULL;

/* Prevent inlining to keep patterns intact */
__attribute__((noinline))
int test_ptr_plus_zero_increment(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 1: *(p + 0) with explicit zero offset */
        int val = *(p + 0);  /* Should create mem_insn with reg1_val = 0 */
        sum += val;
        
        /* Separate increment statement */
        p++;  /* Should be found by find_inc() */
    }
    
    /* Use result to prevent elimination */
    dummy_use(sum);
    return sum;
}

__attribute__((noinline))
int test_array_zero_increment(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 2: p[0] with array notation */
        int val = p[0];  /* Should also create zero offset pattern */
        sum += val;
        
        /* Separate increment with += 1 */
        p += 1;  /* Alternative increment form */
    }
    
    dummy_use(sum);
    return sum;
}

__attribute__((noinline))
int test_store_decrement(int* arr, int n, int value) {
    int* p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Pattern 3: Store with zero offset */
        *(p + 0) = value;  /* Store operation with zero offset */
        
        /* Separate decrement */
        p--;  /* Decrement pattern */
    }
    
    /* Verify by reading back */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    dummy_use(sum);
    return sum;
}

/* Structure with pointer member */
struct ptr_wrapper {
    int* current;
    int* end;
};

__attribute__((noinline))
int test_struct_ptr_increment(struct ptr_wrapper* wrapper, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pattern 4: Access through structure member */
        int val = *(wrapper->current + 0);  /* Zero offset through struct */
        sum += val;
        
        /* Increment the structure member */
        wrapper->current++;  /* Separate increment */
    }
    
    dummy_use(sum);
    return sum;
}

__attribute__((noinline))
int test_mixed_operations(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    
    /* Mix of operations in loop */
    for (int i = 0; i < n; i++) {
        /* Multiple zero-offset accesses */
        int val1 = *(p + 0);
        sum += val1;
        p++;  /* Increment after first access */
        
        if (i + 1 < n) {
            int val2 = *(p + 0);  /* Another zero offset access */
            sum += val2;
            /* No increment here - let next iteration handle it */
        }
    }
    
    dummy_use(sum);
    return sum;
}

/* Dummy function implementation */
void dummy_use(int val) {
    /* Use inline asm to prevent elimination */
    asm volatile("" : : "r"(val) : "memory");
}

int main(int argc, char** argv) {
    /* Use argc to make size non-constant */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate array with dynamic size */
    int* array = (int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Test 1: Basic *(p + 0) with p++ */
    total_sum += test_ptr_plus_zero_increment(array, size);
    
    /* Test 2: p[0] with p += 1 */
    total_sum += test_array_zero_increment(array, size);
    
    /* Test 3: Store with decrement */
    total_sum += test_store_decrement(array, size, 42);
    
    /* Test 4: Structure member access */
    struct ptr_wrapper wrapper;
    wrapper.current = array;
    wrapper.end = array + size;
    total_sum += test_struct_ptr_increment(&wrapper, size);
    
    /* Re-initialize for next test */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Test 5: Mixed operations */
    total_sum += test_mixed_operations(array, size);
    
    /* Use result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    return 0;
}
