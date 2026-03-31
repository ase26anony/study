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
int test1_ptr_plus_zero_increment(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate pointer increment */
        p++;
    }
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_bracket_zero_plus_equals(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Pointer increment with += */
        p += 1;
    }
    return sum;
}

/* Test 3: Structure with pointer member */
typedef struct {
    int* current;
    int* end;
} PtrWalker;

__attribute__((noinline))
int test3_struct_ptr_member(int* arr, int n) {
    int sum = 0;
    PtrWalker walker;
    walker.current = arr;
    walker.end = arr + n;
    
    while (walker.current < walker.end) {
        /* Access through structure member with zero offset */
        int val = *(walker.current + 0);
        sum += val;
        /* Increment the structure member */
        walker.current++;
    }
    return sum;
}

/* Test 4: Store instead of load with decrement */
__attribute__((noinline))
void test4_store_zero_decrement(int* arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    int i;
    
    for (i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate pointer decrement */
        p--;
    }
}

/* Test 5: Mixed pattern with volatile to force memory access */
__attribute__((noinline))
int test5_mixed_volatile(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Use asm to ensure access isn't eliminated */
        asm volatile("" : : "r"(*(p + 0)) : "memory");
        sum += *(p + 0);
        p = p + 1;  /* Another form of increment */
    }
    return sum;
}

/* Test 6: Double pointer with zero offset */
__attribute__((noinline))
int test6_double_ptr(int** ptr_arr, int n) {
    int sum = 0;
    int **pp = ptr_arr;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Access through double pointer with zero offset */
        int* p = *(pp + 0);
        sum += *p;
        pp++;
    }
    return sum;
}

/* Test 7: Char pointer with zero offset (different size increment) */
__attribute__((noinline))
int test7_char_ptr(char* arr, int n) {
    int sum = 0;
    char *p = arr;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Char access with zero offset */
        char val = *(p + 0);
        sum += val;
        p++;  /* Increment by sizeof(char) */
    }
    return sum;
}

/* Main function that calls all tests */
int main(int argc, char** argv) {
    int i, total_sum = 0;
    
    /* Use argc to make array size non-constant */
    int array_size = g_volatile_bound + argc;
    if (array_size < 10) array_size = 100;
    
    /* Allocate and initialize arrays */
    int* array1 = (int*)malloc(array_size * sizeof(int));
    int* array2 = (int*)malloc(array_size * sizeof(int));
    int** ptr_array = (int**)malloc(array_size * sizeof(int*));
    char* char_array = (char*)malloc(array_size * sizeof(char));
    
    if (!array1 || !array2 || !ptr_array || !char_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (i = 0; i < array_size; i++) {
        array1[i] = i % 100;
        array2[i] = (i * 3) % 100;
        ptr_array[i] = &array1[i];
        char_array[i] = (char)(i % 256);
    }
    
    g_volatile_ptr = array1;  /* Prevent optimizations */
    
    /* Run all tests */
    total_sum += test1_ptr_plus_zero_increment(array1, array_size);
    total_sum += test2_bracket_zero_plus_equals(array1, array_size);
    total_sum += test3_struct_ptr_member(array2, array_size);
    
    test4_store_zero_decrement(array2, array_size, 42);
    
    total_sum += test5_mixed_volatile(array1, array_size);
    total_sum += test6_double_ptr(ptr_array, array_size);
    total_sum += test7_char_ptr(char_array, array_size);
    
    /* Use the result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    /* Clean up */
    free(array1);
    free(array2);
    free(ptr_array);
    free(char_array);
    
    return 0;
}
