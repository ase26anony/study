/* auto_inc_dec_test.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Test function 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_zero_offset_plus_plus(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        
        /* Separate pointer increment */
        p++;
        
        /* Use the value to prevent elimination */
        sum += val;
        asm volatile("" : : "r"(val) : "memory");
    }
    
    return sum;
}

/* Test function 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_plus_equals(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        
        /* Separate pointer increment with += */
        p += 1;
        
        /* Use the value */
        sum += val;
        dummy_external();
    }
    
    return sum;
}

/* Test function 3: Structure with pointer member */
struct PointerHolder {
    int *ptr;
    int count;
};

__attribute__((noinline))
int test3_struct_member(struct PointerHolder *holder, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure member with zero offset */
        int val = *(holder->ptr + 0);
        
        /* Increment the pointer member */
        holder->ptr = holder->ptr + 1;
        
        /* Use the value */
        sum += val;
        g_volatile_sum += val;  /* Global volatile side effect */
    }
    
    return sum;
}

/* Test function 4: Store instead of load with decrement */
__attribute__((noinline))
void test4_store_zero_offset_decrement(int *arr, int n, int value) {
    int *p = &arr[n - 1];  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with explicit zero offset */
        *(p + 0) = value + i;
        
        /* Separate pointer decrement */
        p--;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
}

/* Test function 5: Different increment pattern with post-increment in expression */
__attribute__((noinline))
int test5_mixed_pattern(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    int i = 0;
    
    while (i < n) {
        /* Access with zero offset, increment in separate statement */
        int val = *(p + 0);
        p = p + 1;  /* Separate assignment */
        
        sum += val;
        i++;
        
        /* Prevent loop unrolling */
        if (i % 16 == 0) dummy_external();
    }
    
    return sum;
}

/* Test function 6: Pointer arithmetic in loop with volatile bound */
__attribute__((noinline))
int test6_volatile_bound(int *arr) {
    int *p = arr;
    int sum = 0;
    volatile int bound = g_volatile_bound;
    
    for (int i = 0; i < bound; i++) {
        /* Force zero offset through temporary */
        int *temp = p;
        int val = *(temp + 0);
        
        /* Increment original pointer */
        p++;
        
        sum += val;
    }
    
    return sum;
}

/* Dummy external function implementation */
void dummy_external(void) {
    /* Empty but prevents optimization */
    asm volatile("" : : : "memory");
}

int main(int argc, char *argv[]) {
    /* Use argc to determine array size at runtime */
    int size = (argc > 1) ? atoi(argv[1]) : 256;
    if (size <= 0) size = 256;
    
    /* Allocate and initialize array */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Call all test functions */
    total_sum += test1_zero_offset_plus_plus(array, size);
    total_sum += test2_array_zero_plus_equals(array, size);
    
    struct PointerHolder holder = {array, size};
    total_sum += test3_struct_member(&holder, size);
    
    test4_store_zero_offset_decrement(array, size, 42);
    
    /* Re-initialize for next test */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    total_sum += test5_mixed_pattern(array, size);
    total_sum += test6_volatile_bound(array);
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    return 0;
}
