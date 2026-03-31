/* auto-inc-dec-test.c
 * Designed to trigger GCC's auto-increment/decrement optimization pass
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
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
    
    /* This loop should generate (mem (plus (reg) (const_int 0))) */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    /* Using ptr[0] and ptr + 0 to force zero offset */
    while (ptr < end) {
        sum += ptr[0];          /* Zero offset array access */
        sum += *(ptr + 0);      /* Another zero offset access */
        ptr++;                  /* Post-increment */
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
    
    /* Post-decrement loop */
    while (ptr >= start) {
        sum += ptr[0];          /* Zero offset */
        ptr--;                  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types to test various modes */
long test3_mixed_data_types(void) {
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
    
    /* Char pointer loop with zero offset */
    char *cptr = char_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += cptr[0];       /* QImode access */
        cptr++;
    }
    
    /* Short pointer loop with zero offset */
    short *sptr = short_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += sptr[0];       /* HImode access */
        sptr++;
    }
    
    /* Int pointer loop with zero offset */
    int *iptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += iptr[0];       /* SImode access */
        iptr++;
    }
    
    /* Long pointer loop with zero offset */
    long *lptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += lptr[0];       /* DImode access */
        lptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    volatile int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    volatile int *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[0];         /* Volatile access with zero offset */
        vptr++;                 /* Post-increment */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for aliasing guarantees */
int test5_restrict_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *restrict rptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];         /* Restrict access with zero offset */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested conditional with zero offset */
int test6_nested_conditional(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Conditional access pattern */
        if (i % 2 == 0) {
            sum += ptr[0];      /* Zero offset in if branch */
        } else {
            sum += ptr[1];      /* Non-zero offset in else branch */
        }
        
        /* Complex update with zero offset computation */
        ptr = ptr + 1;          /* Pointer arithmetic */
        
        /* Another zero offset access in loop */
        if (i % 3 == 0) {
            sum += *(ptr + 0);  /* Another zero offset pattern */
        }
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;      /* At offset 0 */
    int second;
    char third;
};

int test7_struct_first_member(void) {
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)(i % 256);
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Accessing first member (offset 0) */
        sum += sptr->first;     /* Equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit zero offset via cast */
int test8_explicit_zero_cast(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Explicit zero offset using cast */
        sum += ptr[(int)(0)];   /* Force const_int 0 in RTL */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple loops with different step sizes */
int test9_various_step_sizes(void) {
    int arr[ARRAY_SIZE * 4];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 4; i++) {
        arr[i] = i;
    }
    
    /* Step size 1 */
    int *ptr1 = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr1[0];
        ptr1 += 1;  /* Step 1 */
    }
    
    /* Step size 2 */
    int *ptr2 = arr;
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += ptr2[0];
        ptr2 += 2;  /* Step 2 */
    }
    
    /* Step size 4 */
    int *ptr4 = arr;
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        sum += ptr4[0];
        ptr4 += 4;  /* Step 4 */
    }
    
    return sum;
}

/* Test 10: Complex pointer arithmetic with zero */
int test10_complex_arithmetic(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Complex expression that should simplify to base + 0 */
    int *base = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int offset = 0;
        sum += base[offset];    /* Variable that's always 0 */
        base++;
    }
    
    /* Another pattern: (ptr + i) + (0 - i) */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + i + (0 - i));  /* Should simplify to ptr[0] */
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int test_to_run = -1;
    int result = 0;
    
    /* Parse command line to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    if (test_to_run == 1 || test_to_run == -1) {
        result = test1_post_increment_zero_offset();
        printf("Test 1 result: %d\n", result);
    }
    
    if (test_to_run == 2 || test_to_run == -1) {
        result = test2_post_decrement_zero_offset();
        printf("Test 2 result: %d\n", result);
    }
    
    if (test_to_run == 3 || test_to_run == -1) {
        result = (int)test3_mixed_data_types();
        printf("Test 3 result: %d\n", result);
    }
    
    if (test_to_run == 4 || test_to_run == -1) {
        result = test4_volatile_zero_offset();
        printf("Test 4 result: %d\n", result);
    }
    
    if (test_to_run == 5 || test_to_run == -1) {
        result = test5_restrict_zero_offset();
        printf("Test 5 result: %d\n", result);
    }
    
    if (test_to_run == 6 || test_to_run == -1) {
        result = test6_nested_conditional();
        printf("Test 6 result: %d\n", result);
    }
    
    if (test_to_run == 7 || test_to_run == -1) {
        result = test7_struct_first_member();
        printf("Test 7 result: %d\n", result);
    }
    
    if (test_to_run == 8 || test_to_run == -1) {
        result = test8_explicit_zero_cast();
        printf("Test 8 result: %d\n", result);
    }
    
    if (test_to_run == 9 || test_to_run == -1) {
        result = test9_various_step_sizes();
        printf("Test 9 result: %d\n", result);
    }
    
    if (test_to_run == 10 || test_to_run == -1) {
        result = test10_complex_arithmetic();
        printf("Test 10 result: %d\n", result);
    }
    
    /* Final checksum to ensure all code was executed */
    int final_checksum = 
        test1_post_increment_zero_offset() +
        test2_post_decrement_zero_offset() +
        (int)test3_mixed_data_types() +
        test4_volatile_zero_offset() +
        test5_restrict_zero_offset() +
        test6_nested_conditional() +
        test7_struct_first_member() +
        test8_explicit_zero_cast() +
        test9_various_step_sizes() +
        test10_complex_arithmetic();
    
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
