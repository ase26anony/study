/* auto_inc_dec_test.c - Test program for auto-increment/decrement optimization */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define ITERATIONS 100

/* Test 1: Simple post-increment with zero offset */
int test1_post_increment_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Access with ptr + 0 pattern */
    int *ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += *(ptr + 0);  /* Zero offset addition */
        ptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_post_decrement_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + SIZE - 1;
    int *start = arr;
    while (ptr >= start) {
        sum += ptr[0];  /* Array index 0 creates zero offset */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types with zero offset */
void test3_mixed_data_types(void) {
    char c_arr[SIZE];
    short s_arr[SIZE];
    int i_arr[SIZE];
    long l_arr[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        c_arr[i] = (char)(i % 256);
        s_arr[i] = (short)(i * 2);
        i_arr[i] = i * 3;
        l_arr[i] = i * 4L;
    }
    
    /* Char access with zero offset */
    char *c_ptr = c_arr;
    char *c_end = c_arr + SIZE;
    int char_sum = 0;
    while (c_ptr < c_end) {
        char_sum += c_ptr[0];  /* QImode access with zero offset */
        c_ptr++;
    }
    
    /* Short access with zero offset */
    short *s_ptr = s_arr;
    short *s_end = s_arr + SIZE;
    int short_sum = 0;
    while (s_ptr < s_end) {
        short_sum += *(s_ptr + 0);  /* HImode access */
        s_ptr++;
    }
    
    /* Int access with zero offset */
    int *i_ptr = i_arr;
    int *i_end = i_arr + SIZE;
    int int_sum = 0;
    while (i_ptr < i_end) {
        int_sum += i_ptr[0];  /* SImode access */
        i_ptr++;
    }
    
    /* Long access with zero offset */
    long *l_ptr = l_arr;
    long *l_end = l_arr + SIZE;
    long long_sum = 0;
    while (l_ptr < l_end) {
        long_sum += *(l_ptr + 0);  /* DImode access */
        l_ptr++;
    }
    
    printf("Mixed types sums: char=%d, short=%d, int=%d, long=%ld\n",
           char_sum, short_sum, int_sum, long_sum);
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    volatile int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    volatile int *vptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Force zero offset with volatile */
        sum += vptr[i + 0];  /* (mem (plus (reg) (const_int 0))) with volatile */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 5;
    }
    
    int *restrict rptr = arr;
    int *restrict rend = arr + SIZE;
    while (rptr < rend) {
        /* Zero offset with restrict */
        sum += *(rptr + 0);
        rptr += 1;  /* Different step size */
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset */
int test6_nested_conditional(void) {
    int arr[SIZE][4];
    int sum = 0;
    
    /* Initialize 2D array */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i][j] = i * 4 + j;
        }
    }
    
    /* Nested loop with conditional access */
    for (int i = 0; i < SIZE; i++) {
        int *row_ptr = arr[i];
        for (int j = 0; j < 4; j++) {
            if (j % 2 == 0) {
                /* Zero offset in conditional path */
                sum += row_ptr[0];  /* Always access first element */
            } else {
                sum += row_ptr[j];
            }
        }
    }
    
    return sum;
}

/* Test 7: Multiple step sizes with zero offset */
void test7_multiple_steps(void) {
    int arr[SIZE * 2];
    int sum1 = 0, sum2 = 0, sum4 = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i;
    }
    
    /* Step size 1 */
    int *ptr1 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum1 += ptr1[0];  /* Zero offset */
        ptr1 += 1;
    }
    
    /* Step size 2 */
    int *ptr2 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum2 += *(ptr2 + 0);  /* Zero offset */
        ptr2 += 2;
    }
    
    /* Step size 4 */
    int *ptr4 = arr;
    for (int i = 0; i < SIZE / 2; i++) {
        sum4 += ptr4[0];  /* Zero offset */
        ptr4 += 4;
    }
    
    printf("Step sums: 1=%d, 2=%d, 4=%d\n", sum1, sum2, sum4);
}

/* Test 8: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* At offset 0 */
    int second;
    char third;
};

int test8_struct_first_member(void) {
    struct test_struct arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i * 10;
        arr[i].second = i * 20;
        arr[i].third = (char)(i % 128);
    }
    
    struct test_struct *sptr = arr;
    struct test_struct *send = arr + SIZE;
    while (sptr < send) {
        /* Access first member (offset 0) */
        sum += sptr->first;  /* Equivalent to *(ptr + 0) */
        sptr++;
    }
    
    return sum;
}

/* Test 9: Explicit zero cast as index */
int test9_explicit_zero_cast(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 7;
    }
    
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Explicit zero cast to force constant zero */
        sum += ptr[(int)(0)];  /* Force const_int 0 in RTL */
        ptr++;
    }
    
    return sum;
}

/* Test 10: Complex pointer arithmetic with zero */
int test10_complex_zero_arithmetic(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 11;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE;
    
    /* Complex expression that simplifies to ptr + 0 */
    while (ptr < end) {
        int offset = 0;
        sum += *(ptr + offset);  /* offset is zero */
        
        /* Another zero offset variant */
        sum += ptr[0 * 1];  /* Constant expression evaluates to 0 */
        
        ptr++;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    int test_to_run = -1;
    
    /* Parse command line to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    printf("Auto-inc-dec test program\n");
    
    /* Run all tests or specific one */
    if (test_to_run == -1 || test_to_run == 1) {
        int result = test1_post_increment_zero_offset();
        total_sum += result;
        printf("Test 1 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 2) {
        int result = test2_post_decrement_zero_offset();
        total_sum += result;
        printf("Test 2 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 3) {
        test3_mixed_data_types();
    }
    
    if (test_to_run == -1 || test_to_run == 4) {
        int result = test4_volatile_zero_offset();
        total_sum += result;
        printf("Test 4 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 5) {
        int result = test5_restrict_zero_offset();
        total_sum += result;
        printf("Test 5 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 6) {
        int result = test6_nested_conditional();
        total_sum += result;
        printf("Test 6 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 7) {
        test7_multiple_steps();
    }
    
    if (test_to_run == -1 || test_to_run == 8) {
        int result = test8_struct_first_member();
        total_sum += result;
        printf("Test 8 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 9) {
        int result = test9_explicit_zero_cast();
        total_sum += result;
        printf("Test 9 result: %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 10) {
        int result = test10_complex_zero_arithmetic();
        total_sum += result;
        printf("Test 10 result: %d\n", result);
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
