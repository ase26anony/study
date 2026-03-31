/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination of memory accesses */
extern void use_value(int val);

/* Volatile variable to prevent constant propagation */
volatile int g_volatile_bound = 100;

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
    
    /* Use the result to prevent elimination */
    use_value(sum);
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_plus_equals(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Separate increment with += */
        p += 1;
    }
    
    use_value(sum);
    return sum;
}

/* Test 3: Store variant with *(p + 0) and p-- */
__attribute__((noinline))
void test3_store_zero_offset_minus_minus(int *arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate decrement */
        p--;
    }
    
    /* Force memory writes to be observable */
    asm volatile("" : : "r"(arr[0]) : "memory");
}

/* Test 4: Structure with pointer member */
struct PointerHolder {
    int *ptr;
    int count;
};

__attribute__((noinline))
int test4_struct_member(struct PointerHolder *holder) {
    int *p = holder->ptr;
    int sum = 0;
    
    for (int i = 0; i < holder->count; i++) {
        /* Access through pointer with zero offset */
        int val = *(p + 0);
        sum += val;
        /* Increment pointer */
        p = p + 1;
    }
    
    use_value(sum);
    return sum;
}

/* Test 5: Different type and increment by 2 */
__attribute__((noinline))
long test5_different_increment(short *arr, int n) {
    short *p = arr;
    long sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Zero offset access with different type */
        short val = *(p + 0);
        sum += val;
        /* Increment by 1 (in pointer units, so 2 bytes for short) */
        p++;
    }
    
    use_value((int)sum);
    return sum;
}

/* Test 6: Complex expression with zero offset */
__attribute__((noinline))
int test6_complex_zero_offset(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* More complex zero offset expression */
        int val = *(0 + p + 0);
        sum += val;
        /* Post-increment in separate statement */
        p = p + 1;
    }
    
    use_value(sum);
    return sum;
}

/* Dummy implementation of use_value to prevent dead code elimination */
void use_value(int val) {
    /* Use inline asm to make the value observable */
    asm volatile("" : : "r"(val));
}

int main(int argc, char **argv) {
    /* Use argc to make array size non-constant */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(size * sizeof(int));
    short *arr2 = (short*)malloc(size * sizeof(short));
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    for (int i = 0; i < size; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = (short)(i * 2);
    }
    
    int total_sum = 0;
    
    /* Call all test functions */
    total_sum += test1_zero_offset_plus_plus(arr1, size);
    total_sum += test2_array_zero_plus_equals(arr1, size);
    
    test3_store_zero_offset_minus_minus(arr1, size, 42);
    
    struct PointerHolder holder = {arr1, size};
    total_sum += test4_struct_member(&holder);
    
    total_sum += (int)test5_different_increment(arr2, size);
    total_sum += test6_complex_zero_offset(arr1, size);
    
    /* Print result to prevent elimination of entire computation */
    printf("Total sum: %d\n", total_sum);
    
    free(arr1);
    free(arr2);
    
    return 0;
}
