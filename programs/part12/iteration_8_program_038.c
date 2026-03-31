/* auto-inc-dec-test.c
 * Designed to trigger GCC's auto-increment/decrement pass
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test1_simple_post_inc(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Pattern: ptr[i + 0] with post-increment */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    while (ptr < end) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += ptr[0];          /* Zero offset access */
        sum += *(ptr + 0);      /* Alternative zero offset */
        ptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_post_dec(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    int *start = arr;
    while (ptr >= start) {
        sum += ptr[0];          /* Zero offset */
        ptr--;                  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types to test various modes */
long test3_mixed_types(void) {
    char c_arr[ARRAY_SIZE];
    short s_arr[ARRAY_SIZE];
    int i_arr[ARRAY_SIZE];
    long l_arr[ARRAY_SIZE];
    long total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        c_arr[i] = (char)(i % 256);
        s_arr[i] = (short)(i * 2);
        i_arr[i] = i * 3;
        l_arr[i] = i * 4L;
    }
    
    /* Char access - QImode */
    char *c_ptr = c_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += c_ptr[0];      /* Zero offset */
        c_ptr++;
    }
    
    /* Short access - HImode */
    short *s_ptr = s_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += s_ptr[0];      /* Zero offset */
        s_ptr++;
    }
    
    /* Int access - SImode */
    int *i_ptr = i_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += i_ptr[0];      /* Zero offset */
        i_ptr++;
    }
    
    /* Long access - DImode */
    long *l_ptr = l_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += l_ptr[0];      /* Zero offset */
        l_ptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    int arr[ARRAY_SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i + 1;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[i + 0];     /* Force (plus (reg) (const_int 0)) */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for aliasing guarantees */
int test5_restrict_ptr(void) {
    int arr[ARRAY_SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * i;
    }
    
    /* Restrict allows aggressive optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];         /* Zero offset */
        rptr++;                 /* Post-increment */
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional access */
int test6_nested_conditional(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i % 100;
    }
    
    int *ptr = arr;
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < ARRAY_SIZE / 10; inner++) {
            /* Conditional zero-offset access */
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
    struct test_struct s_arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        s_arr[i].first = i;
        s_arr[i].second = i * 2;
        s_arr[i].third = (char)(i % 128);
    }
    
    struct test_struct *s_ptr = s_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Access first member (offset 0) */
        sum += s_ptr->first;    /* Equivalent to s_ptr[0].first */
        s_ptr++;
    }
    
    return sum;
}

/* Test 8: Explicit zero cast as index */
int test8_explicit_zero_cast(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 3;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Explicit cast of zero to pointer offset type */
        sum += ptr[(int)(0)];   /* Force const_int 0 in RTL */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple increments with zero offset */
int test9_multiple_increments(void) {
    int arr1[ARRAY_SIZE];
    int arr2[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i;
        arr2[i] = ARRAY_SIZE - i;
    }
    
    int *p1 = arr1;
    int *p2 = arr2;
    
    /* Two pointers incrementing with zero offset access */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += p1[0] + p2[0];   /* Both with zero offset */
        p1 += 1;                /* Step 1 */
        p2 += 1;                /* Step 1 */
    }
    
    return sum;
}

/* Test 10: Negative step with zero offset */
int test10_negative_step(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 5;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[0];          /* Zero offset */
        ptr -= 2;               /* Negative step of 2 */
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    long total_result = 0;
    
    /* Use command line to select which test to run */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run all tests if no specific test requested */
    if (test_to_run == 0 || test_to_run == 1) {
        total_result += test1_simple_post_inc();
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 2) {
        total_result += test2_post_dec();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 3) {
        total_result += test3_mixed_types();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 4) {
        total_result += test4_volatile_access();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 5) {
        total_result += test5_restrict_ptr();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 6) {
        total_result += test6_nested_conditional();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 7) {
        total_result += test7_struct_first_member();
        printf("Test 7 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 8) {
        total_result += test8_explicit_zero_cast();
        printf("Test 8 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 9) {
        total_result += test9_multiple_increments();
        printf("Test 9 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 10) {
        total_result += test10_negative_step();
        printf("Test 10 completed\n");
    }
    
    printf("Total checksum: %ld\n", total_result);
    
    /* Prevent dead code elimination */
    volatile int dummy = total_result;
    
    return 0;
}
