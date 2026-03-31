/* auto_inc_dec_test.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Simple pointer arithmetic with zero offset in loop */
int test1_simple_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Access with explicit zero offset - should generate (plus (reg) (const_int 0)) */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 0);  /* Explicit zero offset */
        ptr++;
    }
    
    return sum;
}

/* Test 2: Array indexing with zero addition */
int test2_array_index_zero(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    /* Multiple zero-offset patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i + 0];  /* i + 0 should generate const_int 0 in address */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
void test3_mixed_types(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    int sum = 0;
    
    /* Char access with zero offset */
    char *cptr = char_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *((char *)(cptr + 0)) = (char)i;  /* QImode access */
        cptr++;
    }
    
    /* Short access with zero offset */
    short *sptr = short_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(sptr + 0) = (short)i;  /* HImode access */
        sptr++;
    }
    
    /* Int access with zero offset */
    int *iptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        iptr[0] = i;  /* SImode access - array notation with zero */
        iptr++;
    }
    
    /* Long access with zero offset */
    long *lptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(lptr + 0) = i;  /* DImode access */
        lptr++;
    }
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    volatile int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize volatile array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Volatile pointer with zero offset access */
    volatile int *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[0];  /* Should still generate (plus (reg) (const_int 0)) */
        vptr++;
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
    
    /* Restrict gives compiler stronger aliasing guarantees */
    int *restrict rptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(rptr + 0);  /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset */
int test6_nested_conditional(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE / 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Conditional access with zero offset */
            if (j % 2 == 0) {
                sum += ptr[0];  /* Zero offset in conditional */
            } else {
                sum += ptr[1];
            }
        }
        ptr += 4;
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* Offset 0 */
    int second;
    int third;
};

int test7_struct_first_member(void) {
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = i * 3;
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Accessing first member - at offset 0 */
        sum += sptr->first;  /* Should be equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Post-decrement with zero offset */
int test8_post_decrement(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = &arr[ARRAY_SIZE - 1];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 0);  /* Zero offset with post-decrement */
        ptr--;
    }
    
    return sum;
}

/* Test 9: Cast zero to pointer offset type */
int test9_cast_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Explicit cast of zero to pointer offset */
        sum += ptr[(int)(0)];  /* Cast zero to int */
        ptr++;
    }
    
    return sum;
}

/* Test 10: Complex expression with zero offset */
int test10_complex_zero_expr(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Complex expression that simplifies to +0 */
        sum += ptr[1 - 1];  /* 1-1 = 0 */
        ptr++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Run tests based on command line arguments or all by default */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "test1")) {
        total_sum += test1_simple_zero_offset();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "test2"))) {
        total_sum += test2_array_index_zero();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "test3"))) {
        test3_mixed_types();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "test4"))) {
        total_sum += test4_volatile_zero_offset();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "test5"))) {
        total_sum += test5_restrict_zero_offset();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "test6"))) {
        total_sum += test6_nested_conditional();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "test7"))) {
        total_sum += test7_struct_first_member();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "test8"))) {
        total_sum += test8_post_decrement();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "test9"))) {
        total_sum += test9_cast_zero_offset();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "test10"))) {
        total_sum += test10_complex_zero_expr();
    }
    
    /* Print result to prevent optimization away */
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
