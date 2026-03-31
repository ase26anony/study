/* auto-inc-dec-test.c
 * Program to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Lines 1352-1358: Setting up memory instruction with constant zero offset
 * and calling find_inc()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Simple post-increment with zero offset in array access */
int test1_simple_post_increment(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Access with ptr[0] pattern - should generate (plus (reg) (const_int 0)) */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    while (ptr < end) {
        /* Force zero offset access - multiple variations */
        sum += ptr[0];           /* Array index 0 */
        sum += *(ptr + 0);       /* Pointer arithmetic with 0 */
        ptr++;                   /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_post_decrement(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    int *start = arr;
    
    while (ptr >= start) {
        sum += ptr[0];           /* Zero offset access */
        ptr--;                   /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types to test various memory modes */
int test3_mixed_data_types(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i * 2);
        int_arr[i] = i * 3;
        long_arr[i] = i * 4L;
    }
    
    /* Char access with zero offset */
    char *cptr = char_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += cptr[0];          /* QImode access */
        cptr++;
    }
    
    /* Short access with zero offset */
    short *sptr = short_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr[0];          /* HImode access */
        sptr++;
    }
    
    /* Int access with zero offset */
    int *iptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += iptr[0];          /* SImode access */
        iptr++;
    }
    
    /* Long access with zero offset */
    long *lptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += (int)lptr[0];     /* DImode access */
        lptr++;
    }
    
    return sum;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    int arr[ARRAY_SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 5;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[0];          /* Should still generate (plus (reg) (const_int 0)) */
        vptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for aliasing guarantees */
int test5_restrict_pointer(void) {
    int arr[ARRAY_SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 7;
    }
    
    /* Restrict allows aggressive optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];          /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset access */
int test6_nested_conditional(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < ARRAY_SIZE / 10; inner++) {
            /* Conditional access with zero offset */
            if (inner % 2 == 0) {
                sum += ptr[0];          /* Even indices use zero offset */
            } else {
                sum += ptr[0] * 2;      /* Odd indices also use zero offset */
            }
            ptr++;
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
        /* Access first member - at offset 0 */
        sum += sptr->first;      /* Should generate (plus (reg) (const_int 0)) */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit zero offset casting */
int test8_explicit_zero_cast(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 11;
    }
    
    int *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Explicit cast of zero to pointer offset */
        sum += ptr[(int)(0)];    /* Force const_int 0 in RTL */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple increments in loop */
int test9_multiple_increments(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *ptr1 = arr;
    int *ptr2 = arr + ARRAY_SIZE;
    
    /* Two pointers with different step sizes */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr1[0];          /* Step size 1 */
        sum += ptr2[0];          /* Step size 1 from different start */
        ptr1++;
        ptr2++;
    }
    
    return sum;
}

/* Test 10: Complex expression with zero offset */
int test10_complex_zero_expr(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 13;
    }
    
    int *ptr = arr;
    int index = 0;
    
    while (index < ARRAY_SIZE) {
        /* Complex expression that simplifies to + 0 */
        sum += ptr[index * 0];           /* Always zero offset */
        sum += ptr[0 + (index - index)]; /* Also zero offset */
        ptr++;
        index++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    int test_to_run = -1;
    
    /* Parse command line to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run all tests or specific test based on command line */
    if (test_to_run == -1 || test_to_run == 1) {
        total_sum += test1_simple_post_increment();
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 2) {
        total_sum += test2_post_decrement();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 3) {
        total_sum += test3_mixed_data_types();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 4) {
        total_sum += test4_volatile_access();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 5) {
        total_sum += test5_restrict_pointer();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 6) {
        total_sum += test6_nested_conditional();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 7) {
        total_sum += test7_struct_first_member();
        printf("Test 7 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 8) {
        total_sum += test8_explicit_zero_cast();
        printf("Test 8 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 9) {
        total_sum += test9_multiple_increments();
        printf("Test 9 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 10) {
        total_sum += test10_complex_zero_expr();
        printf("Test 10 completed\n");
    }
    
    /* Print checksum to prevent optimization from removing computations */
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
