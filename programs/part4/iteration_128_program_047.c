/* auto_inc_test.c - Test auto-increment/decrement pattern recognition */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Test 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_zero_offset_inc(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        
        /* Separate pointer increment */
        p++;
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_inc(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        
        /* Separate pointer increment with assignment */
        p += 1;
        
        /* Use volatile to prevent elimination */
        volatile int dummy = val;
        (void)dummy;
    }
    
    return sum;
}

/* Test 3: Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int count;
};

__attribute__((noinline))
int test3_struct_ptr(struct ptr_wrapper *wrapper, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure pointer with zero offset */
        int val = *(wrapper->current + 0);
        sum += val;
        
        /* Increment the pointer in structure */
        wrapper->current++;
        
        /* External call to prevent optimization */
        dummy_external();
    }
    
    return sum;
}

/* Test 4: Store instead of load with decrement */
__attribute__((noinline))
void test4_store_zero_dec(int *arr, int n, int value) {
    int *p = &arr[n - 1];  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        
        /* Separate pointer decrement */
        p--;
        
        /* Memory barrier */
        asm volatile("" : : "m"(*p));
    }
}

/* Test 5: Different increment pattern with char pointer */
__attribute__((noinline))
int test5_char_ptr(char *arr, int n) {
    char *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Char access with zero offset */
        char val = *(p + 0);
        sum += val;
        
        /* Pointer increment */
        p = p + 1;
    }
    
    return sum;
}

/* Test 6: Mixed pattern in same function */
__attribute__((noinline))
int test6_mixed_patterns(int *arr, int n) {
    int *p1 = arr;
    int *p2 = &arr[n/2];
    int sum = 0;
    
    for (int i = 0; i < n/2; i++) {
        /* First pattern: load with zero offset, then increment */
        int val1 = *(p1 + 0);
        p1++;
        
        /* Second pattern: different pointer, same idea */
        int val2 = p2[0];
        p2 += 1;
        
        sum += val1 + val2;
    }
    
    return sum;
}

/* Dummy external function implementation */
void dummy_external(void) {
    /* Empty but prevents elimination */
    asm volatile("");
}

int main(int argc, char **argv) {
    /* Use argc to make size non-constant */
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
    
    int total_sum = 0;
    
    /* Run all test functions */
    total_sum += test1_zero_offset_inc(array, size);
    total_sum += test2_array_zero_inc(array, size);
    
    struct ptr_wrapper wrapper = {array, size};
    total_sum += test3_struct_ptr(&wrapper, size);
    
    test4_store_zero_dec(array, size, 42);
    
    /* Create char array for char test */
    char *char_array = (char*)malloc(size);
    for (int i = 0; i < size; i++) {
        char_array[i] = (char)(i % 256);
    }
    total_sum += test5_char_ptr(char_array, size);
    
    total_sum += test6_mixed_patterns(array, size);
    
    /* Use result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    free(char_array);
    
    return 0;
}
