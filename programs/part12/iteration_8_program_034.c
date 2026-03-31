/* test_auto_inc_dec.c - Program to trigger auto-inc-dec pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Simple pointer arithmetic with zero offset in loop */
int test_simple_zero_offset(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + n;
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    for (; ptr < end; ptr++) {
        sum += ptr[0];          /* Zero offset access */
    }
    
    return sum;
}

/* Test 2: Multiple zero offset patterns with different types */
long test_mixed_types(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    long total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i % 65536);
        int_arr[i] = i;
        long_arr[i] = i * 2L;
    }
    
    /* Process char array with zero offset */
    char *cptr = char_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += cptr[0];      /* QImode access with zero offset */
        cptr++;
    }
    
    /* Process short array with zero offset */
    short *sptr = short_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += sptr[0];      /* HImode access with zero offset */
        sptr++;
    }
    
    /* Process int array with zero offset */
    int *iptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += iptr[0];      /* SImode access with zero offset */
        iptr++;
    }
    
    /* Process long array with zero offset */
    long *lptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += lptr[0];      /* DImode access with zero offset */
        lptr++;
    }
    
    return total;
}

/* Test 3: Volatile pointer with zero offset */
int test_volatile_zero_offset(volatile int *arr, int n) {
    int sum = 0;
    volatile int *vptr = arr;
    
    /* Volatile prevents optimization of the zero offset */
    for (int i = 0; i < n; i++) {
        sum += vptr[0];        /* Zero offset with volatile */
        vptr++;
    }
    
    return sum;
}

/* Test 4: Restrict pointer for aliasing guarantees */
int test_restrict_zero_offset(int *restrict arr1, int *restrict arr2, int n) {
    int sum = 0;
    int *restrict ptr1 = arr1;
    int *restrict ptr2 = arr2;
    
    /* Restrict gives compiler stronger guarantees */
    for (int i = 0; i < n; i++) {
        ptr1[0] = ptr2[0];     /* Zero offset copy */
        sum += ptr1[0];
        ptr1++;
        ptr2++;
    }
    
    return sum;
}

/* Test 5: Nested loops with conditional zero offset access */
int test_nested_conditional(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Conditional zero offset access */
        if (i % 2 == 0) {
            ptr[0] = i * 2;    /* Zero offset store */
        } else {
            sum += ptr[0];     /* Zero offset load */
        }
        
        /* Sometimes skip the increment */
        if (i % 3 != 0) {
            ptr++;
        }
    }
    
    return sum;
}

/* Test 6: Post-decrement with zero offset */
int test_post_decrement(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    
    /* Post-decrement loop */
    for (int i = n - 1; i >= 0; i--) {
        sum += ptr[0];         /* Zero offset access */
        ptr--;
    }
    
    return sum;
}

/* Test 7: Explicit zero addition in pointer arithmetic */
int test_explicit_zero_add(int *arr, int n) {
    int sum = 0;
    
    /* Force (ptr + 0) pattern */
    for (int i = 0; i < n; i++) {
        sum += *(arr + i + 0);  /* Explicit zero addition */
    }
    
    /* Another variant with pointer variable */
    int *ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += *(ptr + 0);      /* Another zero offset */
        ptr++;
    }
    
    return sum;
}

/* Test 8: Structure with first member at offset 0 */
struct test_struct {
    int first;      /* At offset 0 */
    int second;
    char third;
};

int test_struct_offset(struct test_struct *arr, int n) {
    int sum = 0;
    struct test_struct *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Access first member at offset 0 */
        sum += ptr->first;     /* Equivalent to ptr[0].first */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Cast zero to pointer offset type */
int test_cast_zero_offset(int *arr, int n) {
    int sum = 0;
    
    /* Cast zero to different types to force constant zero */
    for (int i = 0; i < n; i++) {
        sum += arr[(int)(0) + i];      /* Cast zero to int */
    }
    
    int *ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += ptr[(size_t)(0)];       /* Cast zero to size_t */
        ptr++;
    }
    
    return sum;
}

/* Test 10: Complex loop with multiple zero offset patterns */
int test_complex_patterns(int *arr, int n) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = arr + n/2;
    
    /* Multiple pointers with zero offsets */
    for (int i = 0; i < n/2; i++) {
        /* Multiple zero offset accesses */
        int val1 = ptr1[0];     /* First zero offset */
        int val2 = ptr2[0];     /* Second zero offset */
        
        ptr1[0] = val2;         /* Zero offset store */
        ptr2[0] = val1;         /* Another zero offset store */
        
        sum += ptr1[0] + ptr2[0];
        
        /* Different increment patterns */
        ptr1 += 1;
        ptr2 += 1;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int test_to_run = -1;
    
    /* Parse which test to run from command line */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Initialize test data */
    int int_array[ARRAY_SIZE];
    int int_array2[ARRAY_SIZE];
    struct test_struct struct_array[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        int_array2[i] = i * 3;
        struct_array[i].first = i;
        struct_array[i].second = i * 2;
        struct_array[i].third = (char)(i % 256);
    }
    
    int result = 0;
    
    /* Run selected test or all tests */
    if (test_to_run == 1 || test_to_run == -1) {
        result += test_simple_zero_offset(int_array, ARRAY_SIZE);
    }
    
    if (test_to_run == 2 || test_to_run == -1) {
        result += test_mixed_types();
    }
    
    if (test_to_run == 3 || test_to_run == -1) {
        result += test_volatile_zero_offset(int_array, ARRAY_SIZE);
    }
    
    if (test_to_run == 4 || test_to_run == -1) {
        result += test_restrict_zero_offset(int_array, int_array2, ARRAY_SIZE);
    }
    
    if (test_to_run == 5 || test_to_run == -1) {
        result += test_nested_conditional(int_array, ARRAY_SIZE);
    }
    
    if (test_to_run == 6 || test_to_run == -1) {
        result += test_post_decrement(int_array, ARRAY_SIZE);
    }
    
    if (test_to_run == 7 || test_to_run == -1) {
        result += test_explicit_zero_add(int_array, ARRAY_SIZE);
    }
    
    if (test_to_run == 8 || test_to_run == -1) {
        result += test_struct_offset(struct_array, ARRAY_SIZE);
    }
    
    if (test_to_run == 9 || test_to_run == -1) {
        result += test_cast_zero_offset(int_array, ARRAY_SIZE);
    }
    
    if (test_to_run == 10 || test_to_run == -1) {
        result += test_complex_patterns(int_array, ARRAY_SIZE);
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
