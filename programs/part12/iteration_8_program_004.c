/* Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test1_simple_post_inc(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Key pattern: ptr + 0 in loop */
    int *ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end) {
        sum += *(ptr + 0);  /* This should generate (plus (reg) (const_int 0)) */
        ptr++;
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_simple_post_dec(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + SIZE - 1;
    int *start = arr;
    while (ptr >= start) {
        sum += ptr[0];  /* Array index 0 generates +0 offset */
        ptr--;
    }
    
    return sum;
}

/* Test 3: Different data types for different memory modes */
void test3_mixed_types(void) {
    char c_arr[SIZE];
    short s_arr[SIZE];
    int i_arr[SIZE];
    long long ll_arr[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        c_arr[i] = (char)(i & 0xFF);
        s_arr[i] = (short)(i * 2);
        i_arr[i] = i * 3;
        ll_arr[i] = i * 4LL;
    }
    
    /* Char access - QImode */
    {
        char *c_ptr = c_arr;
        char *c_end = c_arr + SIZE;
        char c_sum = 0;
        while (c_ptr < c_end) {
            c_sum += c_ptr[0];  /* Zero offset */
            c_ptr++;
        }
        printf("Char sum: %d\n", (int)c_sum);
    }
    
    /* Short access - HImode */
    {
        short *s_ptr = s_arr;
        short *s_end = s_arr + SIZE;
        short s_sum = 0;
        while (s_ptr < s_end) {
            s_sum += *(s_ptr + 0);  /* Plus const_int 0 */
            s_ptr++;
        }
        printf("Short sum: %d\n", (int)s_sum);
    }
    
    /* Int access - SImode */
    {
        int *i_ptr = i_arr;
        int *i_end = i_arr + SIZE;
        int i_sum = 0;
        while (i_ptr < i_end) {
            i_sum += i_ptr[0];  /* Zero offset */
            i_ptr++;
        }
        printf("Int sum: %d\n", i_sum);
    }
    
    /* Long long access - DImode */
    {
        long long *ll_ptr = ll_arr;
        long long *ll_end = ll_arr + SIZE;
        long long ll_sum = 0;
        while (ll_ptr < ll_end) {
            ll_sum += *(ll_ptr + 0);  /* Plus const_int 0 */
            ll_ptr++;
        }
        printf("Long long sum: %lld\n", ll_sum);
    }
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    int arr[SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 5;
    }
    
    for (int i = 0; i < SIZE; i++) {
        /* Force multiple zero-offset patterns with volatile */
        sum += vptr[i + 0];  /* i + 0 should not be optimized away with volatile */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_ptr(void) {
    int arr[SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 7;
    }
    
    /* Using restrict gives compiler stronger guarantees */
    for (int i = 0; i < SIZE; i++) {
        sum += rptr[0];  /* Always access first element */
        rptr++;          /* But pointer moves */
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero-offset access */
int test6_nested_conditional(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < SIZE / 10; inner++) {
            /* Conditional access pattern */
            if (inner % 2 == 0) {
                sum += ptr[0];  /* Zero offset */
            } else {
                sum += *(ptr + 0);  /* Another zero offset pattern */
            }
            ptr++;
        }
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* At offset 0 */
    int second;
    char third;
};

int test7_struct_first_member(void) {
    struct test_struct arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i * 11;
        arr[i].second = i * 13;
        arr[i].third = (char)(i & 0xFF);
    }
    
    struct test_struct *sptr = arr;
    struct test_struct *send = arr + SIZE;
    
    while (sptr < send) {
        /* Accessing first member - at offset 0 */
        sum += sptr->first;  /* Should generate (plus (reg) (const_int 0)) */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit zero cast as index */
int test8_explicit_zero_cast(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 17;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE;
    
    while (ptr < end) {
        /* Explicit cast of 0 to ensure it's not optimized */
        sum += ptr[(int)(0)];  /* Should generate const_int 0 */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple zero-offset patterns in same loop */
int test9_multiple_patterns(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 19;
    }
    
    int *ptr1 = arr;
    int *ptr2 = arr + SIZE/2;
    int *end1 = arr + SIZE/2;
    int *end2 = arr + SIZE;
    
    /* Two pointers, both with zero-offset access */
    while (ptr1 < end1 && ptr2 < end2) {
        sum += ptr1[0];      /* First zero offset */
        sum += *(ptr2 + 0);  /* Second zero offset, different syntax */
        ptr1++;
        ptr2++;
    }
    
    return sum;
}

/* Test 10: Complex expression with zero addition */
int test10_complex_zero_expr(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 23;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE;
    
    while (ptr < end) {
        /* Complex-looking but still zero offset */
        sum += ptr[1 - 1];        /* 1-1 = 0 */
        sum += ptr[2 * 0];        /* 2*0 = 0 */
        sum += ptr[0 << 4];       /* 0 << anything = 0 */
        ptr++;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    int test_to_run = -1;
    
    /* Use command line to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run all tests or selected test */
    if (test_to_run == -1 || test_to_run == 1) {
        total_sum += test1_simple_post_inc();
        printf("Test1 result: %d\n", test1_simple_post_inc());
    }
    
    if (test_to_run == -1 || test_to_run == 2) {
        total_sum += test2_simple_post_dec();
        printf("Test2 result: %d\n", test2_simple_post_dec());
    }
    
    if (test_to_run == -1 || test_to_run == 3) {
        test3_mixed_types();
    }
    
    if (test_to_run == -1 || test_to_run == 4) {
        total_sum += test4_volatile_access();
        printf("Test4 result: %d\n", test4_volatile_access());
    }
    
    if (test_to_run == -1 || test_to_run == 5) {
        total_sum += test5_restrict_ptr();
        printf("Test5 result: %d\n", test5_restrict_ptr());
    }
    
    if (test_to_run == -1 || test_to_run == 6) {
        total_sum += test6_nested_conditional();
        printf("Test6 result: %d\n", test6_nested_conditional());
    }
    
    if (test_to_run == -1 || test_to_run == 7) {
        total_sum += test7_struct_first_member();
        printf("Test7 result: %d\n", test7_struct_first_member());
    }
    
    if (test_to_run == -1 || test_to_run == 8) {
        total_sum += test8_explicit_zero_cast();
        printf("Test8 result: %d\n", test8_explicit_zero_cast());
    }
    
    if (test_to_run == -1 || test_to_run == 9) {
        total_sum += test9_multiple_patterns();
        printf("Test9 result: %d\n", test9_multiple_patterns());
    }
    
    if (test_to_run == -1 || test_to_run == 10) {
        total_sum += test10_complex_zero_expr();
        printf("Test10 result: %d\n", test10_complex_zero_expr());
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
