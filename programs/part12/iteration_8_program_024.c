/* auto-inc-dec-test.c
 * Test program to trigger auto-inc-dec pass coverage for lines 1352-1358
 * Specifically targets (mem (plus (reg) (const_int 0))) patterns
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
    
    /* Loop with pointer arithmetic using + 0 */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    while (ptr < end) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += *(ptr + 0);
        ptr++;  /* Post-increment for find_inc to match */
    }
    
    return sum;
}

/* Test 2: Post-decrement with array index + 0 */
int test2_post_decrement_zero_index(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Access with explicit zero offset */
        sum += ptr[0 + 0];  /* Double zero to ensure const_int 0 */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
long test3_mixed_types_zero_offset(void) {
    char c_arr[ARRAY_SIZE];
    short s_arr[ARRAY_SIZE];
    int i_arr[ARRAY_SIZE];
    long l_arr[ARRAY_SIZE];
    long total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        c_arr[i] = (char)(i % 128);
        s_arr[i] = (short)(i * 3);
        i_arr[i] = i * 5;
        l_arr[i] = i * 7L;
    }
    
    /* Process each array with zero-offset access */
    char *c_ptr = c_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += *(c_ptr + 0);  /* QImode access */
        c_ptr++;
    }
    
    short *s_ptr = s_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += s_ptr[0];  /* HImode access */
        s_ptr++;
    }
    
    int *i_ptr = i_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += *(i_ptr + 0);  /* SImode access */
        i_ptr++;
    }
    
    long *l_ptr = l_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += l_ptr[0];  /* DImode access */
        l_ptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 11;
    }
    
    volatile int *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Volatile access with zero offset */
        sum += vptr[i + 0];  /* Index with + 0 */
        /* No pointer increment - let loop index drive access */
    }
    
    /* Second loop with volatile and pointer increment */
    volatile int *vptr2 = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(vptr2 + 0);
        vptr2++;
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 13;
    }
    
    int *restrict rptr = arr;
    int *end = arr + ARRAY_SIZE;
    while (rptr < end) {
        /* Restrict allows aggressive optimization */
        sum += rptr[0];  /* Zero offset */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset */
int test6_nested_conditional(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 17;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE / 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Conditional access pattern */
            if (j % 2 == 0) {
                sum += *(ptr + 0);  /* Even: zero offset */
            } else {
                sum += ptr[1];      /* Odd: offset 1 */
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
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first = i * 19;
        arr[i].second = i * 23;
        arr[i].third = (char)(i % 127);
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Access first member (offset 0) */
        sum += sptr->first;  /* Equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Pointer arithmetic with cast to zero */
int test8_cast_to_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 29;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Cast zero to ptrdiff_t to force const_int 0 */
        sum += ptr[(ptrdiff_t)0];
        ptr++;
    }
    
    return sum;
}

/* Test 9: Complex loop with multiple zero-offset accesses */
int test9_complex_multiple_accesses(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 31;
    }
    
    int *s = src;
    int *d = dst;
    int *end = src + ARRAY_SIZE;
    
    /* Copy with zero-offset accesses */
    while (s < end) {
        /* Read with zero offset */
        int val = *(s + 0);
        /* Write with zero offset */
        *(d + 0) = val;
        sum += val;
        s++;
        d++;
    }
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            return -1;
        }
    }
    
    return sum;
}

/* Test 10: Loop with step size 2 and zero offset */
int test10_step_size_two(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 37;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        sum += ptr[0];  /* Zero offset */
        ptr += 2;       /* Step size 2 */
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    int test_to_run = -1;
    
    /* Use command line argument to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    if (test_to_run == 1 || test_to_run == -1) {
        total += test1_post_increment_zero_offset();
    }
    if (test_to_run == 2 || test_to_run == -1) {
        total += test2_post_decrement_zero_index();
    }
    if (test_to_run == 3 || test_to_run == -1) {
        total += test3_mixed_types_zero_offset();
    }
    if (test_to_run == 4 || test_to_run == -1) {
        total += test4_volatile_zero_offset();
    }
    if (test_to_run == 5 || test_to_run == -1) {
        total += test5_restrict_zero_offset();
    }
    if (test_to_run == 6 || test_to_run == -1) {
        total += test6_nested_conditional();
    }
    if (test_to_run == 7 || test_to_run == -1) {
        total += test7_struct_first_member();
    }
    if (test_to_run == 8 || test_to_run == -1) {
        total += test8_cast_to_zero_offset();
    }
    if (test_to_run == 9 || test_to_run == -1) {
        total += test9_complex_multiple_accesses();
    }
    if (test_to_run == 10 || test_to_run == -1) {
        total += test10_step_size_two();
    }
    
    printf("Total checksum: %d\n", total);
    return 0;
}
