/* test_auto_inc_dec.c - Program to trigger auto-inc-dec pass coverage */

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
    
    /* Pattern: ptr[i + 0] with post-increment */
    int *ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += ptr[0];          /* Zero offset access */
        sum += *(ptr + 0);      /* Another zero offset pattern */
        ptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_post_dec(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + SIZE - 1;
    int *start = arr;
    while (ptr >= start) {
        sum += ptr[0];          /* Zero offset */
        ptr--;                  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types with zero offset */
void test3_mixed_types(void) {
    char char_arr[SIZE];
    short short_arr[SIZE];
    int int_arr[SIZE];
    long long_arr[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i * 3);
        int_arr[i] = i * 5;
        long_arr[i] = i * 7L;
    }
    
    /* Char loop with zero offset */
    char *cptr = char_arr;
    char *cend = char_arr + SIZE;
    int char_sum = 0;
    while (cptr < cend) {
        char_sum += cptr[0];    /* QImode access with zero offset */
        cptr++;
    }
    
    /* Short loop with zero offset */
    short *sptr = short_arr;
    short *send = short_arr + SIZE;
    int short_sum = 0;
    while (sptr < send) {
        short_sum += sptr[0];   /* HImode access with zero offset */
        sptr++;
    }
    
    /* Int loop with zero offset */
    int *iptr = int_arr;
    int *iend = int_arr + SIZE;
    int int_sum = 0;
    while (iptr < iend) {
        int_sum += iptr[0];     /* SImode access with zero offset */
        iptr++;
    }
    
    /* Long loop with zero offset */
    long *lptr = long_arr;
    long *lend = long_arr + SIZE;
    long long_sum = 0;
    while (lptr < lend) {
        long_sum += lptr[0];    /* DImode access with zero offset */
        lptr++;
    }
    
    printf("Mixed types sums: char=%d, short=%d, int=%d, long=%ld\n",
           char_sum, short_sum, int_sum, long_sum);
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    int arr[SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 11;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < SIZE; i++) {
        sum += vptr[i + 0];     /* Zero offset with volatile */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_ptr(void) {
    int arr[SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 13;
    }
    
    /* Restrict pointer with zero offset */
    for (int i = 0; i < SIZE; i++) {
        sum += rptr[i + 0];     /* Zero offset with restrict */
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset */
int test6_nested_conditional(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i % 100;
    }
    
    int *ptr = arr;
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < SIZE / 10; inner++) {
            /* Conditional zero offset access */
            if (inner % 2 == 0) {
                sum += ptr[0];          /* Zero offset */
            } else {
                sum += ptr[1];          /* Non-zero offset for contrast */
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
    struct test_struct arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)(i % 128);
    }
    
    struct test_struct *sptr = arr;
    struct test_struct *send = arr + SIZE;
    while (sptr < send) {
        /* Access first member (offset 0) */
        sum += sptr->first;     /* Equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit cast of zero to pointer offset */
int test8_explicit_zero_cast(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 17;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end) {
        /* Explicit cast of zero to offset type */
        sum += ptr[(int)(0)];   /* Force const_int 0 in RTL */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple increments in same loop */
int test9_multiple_increments(void) {
    int arr1[SIZE], arr2[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 19;
    }
    
    int *p1 = arr1;
    int *p2 = arr2;
    int *end1 = arr1 + SIZE;
    
    while (p1 < end1) {
        sum += p1[0];   /* Zero offset from p1 */
        sum += p2[0];   /* Zero offset from p2 */
        p1++;
        p2++;
    }
    
    return sum;
}

/* Test 10: Complex expression with zero offset */
int test10_complex_expr(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 23;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE;
    int index = 0;
    
    while (ptr < end) {
        /* Complex expression that should simplify to ptr[0] */
        sum += ptr[index * 0];  /* Should become ptr[0] */
        sum += ptr[1 - 1];      /* Should become ptr[0] */
        sum += ptr[2 + (-2)];   /* Should become ptr[0] */
        ptr++;
        index++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Run tests based on command line arguments or all by default */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "test1") || (argc > 1 && strcmp(argv[1], "1") == 0)) {
        total_sum += test1_simple_post_inc();
    }
    
    if (run_all || strstr(argv[0], "test2") || (argc > 1 && strcmp(argv[1], "2") == 0)) {
        total_sum += test2_post_dec();
    }
    
    if (run_all || strstr(argv[0], "test3") || (argc > 1 && strcmp(argv[1], "3") == 0)) {
        test3_mixed_types();
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && strcmp(argv[1], "4") == 0)) {
        total_sum += test4_volatile_access();
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && strcmp(argv[1], "5") == 0)) {
        total_sum += test5_restrict_ptr();
    }
    
    if (run_all || strstr(argv[0], "test6") || (argc > 1 && strcmp(argv[1], "6") == 0)) {
        total_sum += test6_nested_conditional();
    }
    
    if (run_all || strstr(argv[0], "test7") || (argc > 1 && strcmp(argv[1], "7") == 0)) {
        total_sum += test7_struct_first_member();
    }
    
    if (run_all || strstr(argv[0], "test8") || (argc > 1 && strcmp(argv[1], "8") == 0)) {
        total_sum += test8_explicit_zero_cast();
    }
    
    if (run_all || strstr(argv[0], "test9") || (argc > 1 && strcmp(argv[1], "9") == 0)) {
        total_sum += test9_multiple_increments();
    }
    
    if (run_all || strstr(argv[0], "test10") || (argc > 1 && strcmp(argv[1], "10") == 0)) {
        total_sum += test10_complex_expr();
    }
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
