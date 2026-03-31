/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_external(int);

/* Volatile variable to prevent constant propagation */
volatile int g_volatile_bound = 100;

/* Prevent inlining to keep patterns intact */
__attribute__((noinline))
static int test_ptr_plus_zero_increment(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Loop with explicit zero offset and separate increment */
    for (int i = 0; i < n; i++) {
        /* Pattern 1: *(p + 0) with explicit zero offset */
        int val = *(p + 0);  /* Should create mem_insn with reg1_val = 0 */
        sum += val;
        
        /* Separate pointer increment - should be found by find_inc */
        p++;  /* Post-increment by 1 */
        
        /* Use volatile to prevent elimination */
        asm volatile("" : : "r"(val) : "memory");
    }
    return sum;
}

__attribute__((noinline))
static int test_array_zero_increment(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Alternative pattern: p[0] syntax */
    for (int i = 0; i < n; i++) {
        /* Pattern 2: p[0] which should expand to *(p + 0) */
        int val = p[0];  /* Zero offset array access */
        sum ^= val;  /* Different operation to avoid pattern merging */
        
        /* Separate increment with += 1 */
        p += 1;  /* Increment by 1 */
        
        /* External call to prevent optimization */
        dummy_external(val);
    }
    return sum;
}

__attribute__((noinline))
static int test_ptr_zero_decrement(int *arr, int n) {
    int sum = 0;
    int *p = arr + n - 1;  /* Start from end */
    
    /* Reverse traversal with decrement */
    for (int i = 0; i < n; i++) {
        /* Pattern 3: Store operation with zero offset */
        int old_val = *(p + 0);  /* Load with zero offset */
        *(p + 0) = old_val + 1;  /* Store with zero offset */
        sum += old_val;
        
        /* Separate decrement */
        p--;  /* Decrement by 1 */
    }
    return sum;
}

/* Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
static int test_struct_ptr_zero_inc(struct ptr_wrapper *w) {
    int sum = 0;
    
    while (w->current < w->end) {
        /* Pattern 4: Access through structure member with zero offset */
        int val = *((w->current) + 0);  /* Explicit zero offset */
        sum += val;
        
        /* Increment the structure member */
        w->current = w->current + 1;  /* Separate increment */
    }
    return sum;
}

__attribute__((noinline))
static int test_mixed_zero_offset(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Mix of different zero-offset patterns */
    for (int i = 0; i < n; i += 2) {
        /* Multiple memory accesses with zero offset */
        int val1 = *(p + 0);      /* First access */
        p++;                      /* First increment */
        
        int val2 = *(p + 0);      /* Second access with new pointer */
        p += 1;                   /* Second increment */
        
        sum += val1 * val2;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    return sum;
}

/* Main function that calls all test patterns */
int main(int argc, char *argv[]) {
    /* Use argc to make array size non-constant */
    int size = g_volatile_bound + argc;
    if (size < 10) size = 100;
    
    /* Allocate and initialize array */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i * 3 + 1;  /* Non-trivial pattern */
    }
    
    int total = 0;
    
    /* Call each test function */
    total += test_ptr_plus_zero_increment(array, size);
    total += test_array_zero_increment(array, size);
    total += test_ptr_zero_decrement(array, size);
    
    /* Test with structure */
    struct ptr_wrapper wrapper;
    wrapper.current = array;
    wrapper.end = array + size;
    total += test_struct_ptr_zero_inc(&wrapper);
    
    total += test_mixed_zero_offset(array, size);
    
    /* Use result to prevent dead code elimination */
    printf("Total: %d\n", total);
    
    free(array);
    return 0;
}

/* Dummy external function definition */
void dummy_external(int x) {
    /* Empty but external to prevent optimization */
    (void)x;
}
