/* auto-inc-dec-test.c
 * Program to trigger specific RTL patterns for GCC auto-inc-dec pass coverage
 * Targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test1_post_increment_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Access with pointer + 0 in loop */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    while (ptr < end) {
        sum += *(ptr + 0);  /* Zero offset access */
        ptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_post_decrement_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    int *start = arr;
    
    /* Decrementing loop with zero offset */
    while (ptr >= start) {
        sum += ptr[0];  /* Array notation with implicit zero offset */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
long test3_mixed_types_zero_offset(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    long total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i * 2);
        int_arr[i] = i * 3;
        long_arr[i] = i * 4L;
    }
    
    /* Char pointer loop */
    char *cptr = char_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += *(cptr + 0);  /* Zero offset */
        cptr++;
    }
    
    /* Short pointer loop */
    short *sptr = short_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += sptr[0];  /* Zero offset */
        sptr++;
    }
    
    /* Int pointer loop */
    int *iptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += *(iptr + 0);  /* Zero offset */
        iptr++;
    }
    
    /* Long pointer loop */
    long *lptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += lptr[0];  /* Zero offset */
        lptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i + 1;
    }
    
    volatile int *vptr = arr;
    
    /* Volatile access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[i + 0];  /* Explicit zero offset */
        vptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for aliasing guarantees */
int test5_restrict_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 5;
    }
    
    int *restrict rptr = arr;
    
    /* Restrict pointer with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(rptr + 0);  /* Zero offset */
        rptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 6: Nested conditional with zero offset */
int test6_conditional_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    
    /* Complex control flow with zero offset access */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (i % 2 == 0) {
            sum += ptr[0];  /* Zero offset in if branch */
        } else {
            sum -= ptr[0];  /* Zero offset in else branch */
        }
        
        if (i % 3 == 0) {
            sum += *(ptr + 0) * 2;  /* Another zero offset */
        }
        
        ptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* At offset 0 */
    int second;
    char third;
};

int test7_struct_zero_offset(void) {
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)(i % 128);
    }
    
    struct test_struct *sptr = arr;
    
    /* Access first member (offset 0) */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr->first;  /* Accesses member at offset 0 */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit cast of zero to pointer offset */
int test8_explicit_zero_cast(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 7;
    }
    
    int *ptr = arr;
    
    /* Explicit cast of zero to offset type */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[(int)(0)];  /* Cast zero to int */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple loops with different step sizes */
int test9_variable_steps_zero_offset(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i;
    }
    
    /* Step size 1 */
    int *ptr1 = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr1 + 0);
        ptr1 += 1;  /* Step 1 */
    }
    
    /* Step size 2 */
    int *ptr2 = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr2[0];
        ptr2 += 2;  /* Step 2 */
    }
    
    /* Step size -1 (reverse) */
    int *ptr3 = arr + ARRAY_SIZE - 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr3 + 0);
        ptr3 -= 1;  /* Step -1 */
    }
    
    return sum;
}

/* Test 10: Combined patterns in complex loop */
int test10_complex_pattern(void) {
    int arr1[ARRAY_SIZE];
    int arr2[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i;
        arr2[i] = ARRAY_SIZE - i;
    }
    
    int *p1 = arr1;
    int *p2 = arr2;
    
    /* Complex loop with multiple zero-offset accesses */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple zero-offset accesses to different arrays */
        int val1 = p1[0];
        int val2 = *(p2 + 0);
        
        sum += val1 * val2;
        
        /* Conditional zero-offset access */
        if (val1 > val2) {
            sum += p1[0];
        }
        
        p1++;
        p2++;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int test_to_run = -1;
    int result = 0;
    
    /* Use command line argument to select test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run all tests if no argument or specific test if specified */
    if (test_to_run == -1 || test_to_run == 1) {
        result = test1_post_increment_zero_offset();
        printf("Test 1 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 2) {
        result = test2_post_decrement_zero_offset();
        printf("Test 2 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 3) {
        result = (int)test3_mixed_types_zero_offset();
        printf("Test 3 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 4) {
        result = test4_volatile_zero_offset();
        printf("Test 4 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 5) {
        result = test5_restrict_zero_offset();
        printf("Test 5 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 6) {
        result = test6_conditional_zero_offset();
        printf("Test 6 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 7) {
        result = test7_struct_zero_offset();
        printf("Test 7 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 8) {
        result = test8_explicit_zero_cast();
        printf("Test 8 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 9) {
        result = test9_variable_steps_zero_offset();
        printf("Test 9 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 10) {
        result = test10_complex_pattern();
        printf("Test 10 result: %d\n", result);
    }
    
    /* Final checksum to verify all computations */
    if (test_to_run == -1) {
        printf("All tests completed.\n");
    }
    
    return 0;
}
