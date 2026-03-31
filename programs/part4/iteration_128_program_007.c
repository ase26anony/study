/* Compile with: gcc -O2 -fno-omit-frame-pointer -march=armv7-a -fno-tree-loop-distribute-patterns -fdump-rtl-auto_inc_dec -S */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void use_value(int val);

/* Volatile variables to prevent constant propagation */
static volatile int dummy_volatile = 0;

/* Test 1: Load with *(p + 0) and p++ */
__attribute__((noinline, optimize("O2")))
int test1_zero_offset_load(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Explicit zero offset - should not be folded by frontend */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    dummy_volatile = sum; /* Prevent dead code elimination */
    return sum;
}

/* Test 2: Store with *(p + 0) and p += 1 */
__attribute__((noinline, optimize("O2")))
void test2_zero_offset_store(int *arr, int n, int value) {
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate increment with different syntax */
        p += 1;
    }
    
    /* Force side effect */
    asm volatile("" : : "r"(arr[0]) : "memory");
}

/* Test 3: Using p[0] syntax with p = p + 1 */
__attribute__((noinline, optimize("O2")))
int test3_array_zero_index(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        sum += p[0];
        /* Increment as separate assignment */
        p = p + 1;
    }
    
    return sum;
}

/* Test 4: Decrement version with *(p + 0) and p-- */
__attribute__((noinline, optimize("O2")))
int test4_zero_offset_decrement(int *arr, int n) {
    int *p = &arr[n - 1];
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Load with zero offset from end */
        sum += *(p + 0);
        /* Decrement pointer */
        p--;
    }
    
    return sum;
}

/* Test 5: Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline, optimize("O2")))
int test5_struct_member(struct ptr_wrapper *wrapper) {
    int sum = 0;
    int *p = wrapper->current;
    int n = wrapper->end - p;
    
    for (int i = 0; i < n; i++) {
        /* Access through pointer with zero offset */
        int val = *(p + 0);
        sum += val;
        /* Increment */
        ++p;
    }
    
    wrapper->current = p;
    return sum;
}

/* Test 6: Mixed operations to test pattern recognition */
__attribute__((noinline, optimize("O2")))
void test6_mixed_operations(int *arr, int n) {
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Load with zero offset */
        int loaded = *(p + 0);
        /* Store back with offset */
        *(p + 0) = loaded * 2;
        /* Increment */
        p = p + 1;
    }
}

/* Test 7: Prevent optimization with asm volatile */
__attribute__((noinline, optimize("O2")))
int test7_volatile_access(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Force memory access not to be eliminated */
        asm volatile("" : "+r"(sum) : "r"(*(p + 0)) : "memory");
        p++;
    }
    
    return sum;
}

/* Test 8: Different data type to test generalization */
__attribute__((noinline, optimize("O2")))
long test8_different_type(short *arr, int n) {
    short *p = arr;
    long sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Zero offset with different type */
        sum += *(p + 0);
        p += 1;
    }
    
    return sum;
}

int main(int argc, char **argv) {
    /* Use argc to make size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate array dynamically to prevent static analysis */
    int *array1 = (int*)malloc(size * sizeof(int));
    short *array2 = (short*)malloc(size * sizeof(short));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        array1[i] = i;
        array2[i] = i % 100;
    }
    
    int total = 0;
    
    /* Call all test functions */
    total += test1_zero_offset_load(array1, size);
    
    test2_zero_offset_store(array1, size, 42);
    total += test1_zero_offset_load(array1, size); /* Reload to verify */
    
    total += test3_array_zero_index(array1, size);
    total += test4_zero_offset_decrement(array1, size);
    
    struct ptr_wrapper wrapper = {array1, array1 + size};
    total += test5_struct_member(&wrapper);
    
    test6_mixed_operations(array1, size);
    total += test1_zero_offset_load(array1, size);
    
    total += test7_volatile_access(array1, size);
    total += test8_different_type(array2, size);
    
    /* Print result to prevent elimination */
    printf("Total: %d\n", total);
    
    free(array1);
    free(array2);
    
    return 0;
}
