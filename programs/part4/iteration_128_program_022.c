/* auto_inc_dec_test.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination of memory accesses */
extern void use_value(int val);
extern void touch_memory(void *ptr);

/* Prevent inlining to keep patterns intact */
__attribute__((noinline))
static void test1_ptr_plus_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int *volatile_ptr = &n;
    int limit = *volatile_ptr;
    
    for (int i = 0; i < limit; i++) {
        /* Pattern: *(p + 0) with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    /* Use the result to prevent dead code elimination */
    use_value(sum);
}

__attribute__((noinline))
static void test2_array_zero_index(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    volatile int vol_n = n;
    
    for (int i = 0; i < vol_n; i++) {
        /* Pattern: p[0] which should become *(p + 0) */
        int val = p[0];
        sum += val;
        /* Separate increment with += 1 */
        p += 1;
    }
    
    use_value(sum);
}

__attribute__((noinline))
static void test3_ptr_zero_offset_store(int *arr, int n, int value) {
    int *p = arr;
    
    volatile int vol_n = n;
    
    for (int i = 0; i < vol_n; i++) {
        /* Pattern: store with *(p + 0) */
        *(p + 0) = value + i;
        /* Separate decrement */
        p--;
    }
    
    /* Touch memory to ensure stores aren't eliminated */
    touch_memory(arr);
}

__attribute__((noinline))
static void test4_mixed_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    /* Use asm to prevent optimization */
    asm volatile("" : "+r"(n));
    
    for (int i = 0; i < n; i++) {
        /* Multiple zero-offset accesses */
        sum += *(p + 0);
        sum += *(p + 0);  /* Second access to same location */
        /* Increment after accesses */
        p = p + 1;
    }
    
    use_value(sum);
}

__attribute__((noinline))
static void test5_struct_with_ptr(int *arr, int n) {
    struct wrapper {
        int *ptr;
        int count;
    } w;
    
    w.ptr = arr;
    w.count = n;
    int sum = 0;
    
    while (w.count-- > 0) {
        /* Access through struct member with zero offset */
        int val = *(w.ptr + 0);
        sum += val;
        /* Increment struct member */
        w.ptr++;
    }
    
    use_value(sum);
}

__attribute__((noinline))
static void test6_double_pointer(int **arr_ptrs, int n) {
    int **pp = arr_ptrs;
    int sum = 0;
    
    volatile int vol_n = n;
    
    for (int i = 0; i < vol_n; i++) {
        /* Dereference with zero offset */
        int *p = *(pp + 0);
        /* Then access through that pointer with zero offset */
        if (p) sum += *(p + 0);
        /* Increment pointer to pointer */
        pp++;
    }
    
    use_value(sum);
}

/* Dummy functions to prevent optimization */
void use_value(int val) {
    /* Use inline asm to ensure the value is used */
    asm volatile("" : : "r"(val));
}

void touch_memory(void *ptr) {
    asm volatile("" : : "r"(ptr));
}

int main(int argc, char **argv) {
    /* Use argc to make array size non-constant */
    int size = argc > 1 ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate arrays dynamically to avoid static addressing */
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    int **arr_ptrs = (int**)malloc(size * sizeof(int*));
    
    if (!arr1 || !arr2 || !arr_ptrs) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
        arr_ptrs[i] = &arr1[i];
    }
    
    /* Call all test functions with different patterns */
    test1_ptr_plus_zero(arr1, size);
    test2_array_zero_index(arr1, size);
    test3_ptr_zero_offset_store(arr2, size, 42);
    test4_mixed_increment(arr1, size);
    test5_struct_with_ptr(arr1, size);
    test6_double_pointer(arr_ptrs, size);
    
    /* Compute a final result to prevent elimination of everything */
    int total = 0;
    for (int i = 0; i < size; i++) {
        total += arr1[i] + arr2[i];
    }
    
    printf("Total: %d\n", total);
    
    free(arr1);
    free(arr2);
    free(arr_ptrs);
    
    return 0;
}
