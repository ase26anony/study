/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void use_value(int val);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int* g_volatile_ptr = NULL;

/* Test 1: Using *(p + 0) with p++ */
__attribute__((noinline))
int test1_zero_offset_plus_plus(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate pointer increment */
        p++;
    }
    
    /* Use result to prevent elimination */
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Test 2: Using p[0] with p += 1 */
__attribute__((noinline))
int test2_array_zero_with_plus_equals(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Pointer increment with += */
        p += 1;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Test 3: Structure with pointer member */
typedef struct {
    int* current;
    int* end;
} Iterator;

__attribute__((noinline))
int test3_struct_iterator(int* arr, int n) {
    int sum = 0;
    Iterator iter;
    iter.current = arr;
    iter.end = arr + n;
    
    while (iter.current < iter.end) {
        /* Access through struct member with zero offset */
        int val = *(iter.current + 0);
        sum += val;
        /* Increment struct member */
        iter.current++;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Test 4: Store instead of load with decrement */
__attribute__((noinline))
int test4_store_with_decrement(int* arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate pointer decrement */
        p--;
    }
    
    /* Verify by reading back */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Test 5: Mixed pattern with volatile to prevent folding */
__attribute__((noinline))
int test5_mixed_with_volatile(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Use volatile to get bound, preventing constant propagation */
    int bound = g_volatile_bound;
    if (bound > n) bound = n;
    
    for (int i = 0; i < bound; i++) {
        /* Force zero offset computation */
        int offset = 0;
        int val = *(p + offset);
        
        /* Use value to prevent elimination */
        asm volatile("" : : "r"(val));
        
        sum += val;
        
        /* Different increment forms */
        if (i % 2 == 0) {
            p = p + 1;
        } else {
            p++;
        }
    }
    
    return sum;
}

/* Test 6: Double pointer with zero offset */
__attribute__((noinline))
int test6_double_pointer(int** ptr_arr, int n) {
    int sum = 0;
    int **pp = ptr_arr;
    
    for (int i = 0; i < n; i++) {
        /* Dereference with zero offset */
        int* p = *(pp + 0);
        /* Then dereference the inner pointer */
        sum += *p;
        pp++;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

int main(int argc, char** argv) {
    /* Use argc to make array size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    int* array = (int*)malloc(size * sizeof(int));
    int** ptr_array = (int**)malloc(size * sizeof(int*));
    
    if (!array || !ptr_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    for (int i = 0; i < size; i++) {
        array[i] = i * 3 + 1;
        ptr_array[i] = &array[i];
    }
    
    /* Update volatile pointer to prevent optimizations */
    g_volatile_ptr = array;
    
    int total = 0;
    
    /* Run all test functions */
    total += test1_zero_offset_plus_plus(array, size);
    total += test2_array_zero_with_plus_equals(array, size);
    total += test3_struct_iterator(array, size);
    total += test4_store_with_decrement(array, size, 5);
    total += test5_mixed_with_volatile(array, size);
    total += test6_double_pointer(ptr_array, size);
    
    /* Use total to prevent dead code elimination */
    printf("Total: %d\n", total);
    
    free(ptr_array);
    free(array);
    
    return 0;
}
