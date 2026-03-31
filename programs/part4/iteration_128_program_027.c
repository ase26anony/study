/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_use(int val);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int* g_volatile_ptr = NULL;

/* Test 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_zero_offset_plus_plus(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Explicit zero offset - should not be folded by frontend */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    /* Use result to prevent elimination */
    dummy_use(sum);
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_plus_equals(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Array access with explicit zero index */
        int val = p[0];
        sum += val;
        /* Separate increment with assignment */
        p += 1;
    }
    
    dummy_use(sum);
    return sum;
}

/* Test 3: Using structure with pointer member */
struct PointerHolder {
    int* ptr;
    int count;
};

__attribute__((noinline))
int test3_struct_member_access(struct PointerHolder* holder, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure member with zero offset */
        int val = *(holder->ptr + 0);
        sum += val;
        /* Increment structure member */
        holder->ptr = holder->ptr + 1;
    }
    
    dummy_use(sum);
    return sum;
}

/* Test 4: Store instead of load with decrement */
__attribute__((noinline))
void test4_store_zero_offset_decrement(int* arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate decrement */
        p--;
    }
    
    /* Force memory side effect */
    asm volatile("" : : "r"(arr[0]) : "memory");
}

/* Test 5: Mixed pattern with volatile to prevent optimization */
__attribute__((noinline))
int test5_mixed_volatile_access(int* arr, int n) {
    int sum = 0;
    volatile int* volatile_p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Force zero offset computation */
        int* tmp = volatile_p;
        int val = *(tmp + 0);
        sum += val;
        
        /* Increment with barrier */
        asm volatile("" : "+r"(tmp));
        tmp = tmp + 1;
        volatile_p = tmp;
    }
    
    return sum;
}

/* Test 6: Pointer arithmetic in separate statements */
__attribute__((noinline))
int test6_separate_arithmetic(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Break the access into parts */
        int* addr = p + 0;  /* Explicit zero offset */
        int val = *addr;    /* Dereference */
        sum += val;
        
        /* Separate pointer update */
        p = p + 1;          /* Increment by 1 */
    }
    
    dummy_use(sum);
    return sum;
}

/* Test 7: While loop variant */
__attribute__((noinline))
int test7_while_loop(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    int count = n;
    
    while (count-- > 0) {
        /* Zero offset access */
        sum += *(p + 0);
        /* Post-increment */
        p++;
    }
    
    return sum;
}

/* Dummy function implementation */
void dummy_use(int val) {
    /* Use inline asm to prevent elimination */
    asm volatile("" : : "r"(val));
}

int main(int argc, char** argv) {
    /* Use argc to make array size non-constant */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate and initialize array */
    int* array = (int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Call all test functions */
    total_sum += test1_zero_offset_plus_plus(array, size);
    total_sum += test2_array_zero_plus_equals(array, size);
    
    struct PointerHolder holder = {array, size};
    total_sum += test3_struct_member_access(&holder, size);
    
    test4_store_zero_offset_decrement(array, size, 42);
    
    total_sum += test5_mixed_volatile_access(array, size);
    total_sum += test6_separate_arithmetic(array, size);
    total_sum += test7_while_loop(array, size);
    
    /* Use result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    return 0;
}
