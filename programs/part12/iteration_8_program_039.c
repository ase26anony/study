/* test_auto_inc_dec.c - Program to trigger auto-inc-dec pass coverage */

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
    
    /* Access with ptr + 0 pattern */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += *(ptr + 0);  /* This should generate plus with const_int 0 */
        ptr++;              /* Post-increment for find_inc to match */
    }
    
    return sum;
}

/* Test 2: Array indexing with i + 0 in loop */
int test2_array_index_zero(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    /* Multiple zero-offset patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Different ways to express zero offset */
        sum += arr[i + 0];          /* Direct i + 0 */
        sum += arr[(int)(0) + i];   /* Cast zero */
        sum += arr[0 + i];          /* Commutative form */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
void test3_mixed_types(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = i % 256;
        short_arr[i] = i % 32768;
        int_arr[i] = i;
        long_arr[i] = i * 1000L;
    }
    
    /* Process each type with zero-offset access */
    char *cptr = char_arr;
    short *sptr = short_arr;
    int *iptr = int_arr;
    long *lptr = long_arr;
    
    int char_sum = 0, short_sum = 0, int_sum = 0;
    long long_sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Different modes: QImode, HImode, SImode, DImode */
        char_sum += *(cptr + 0);    /* QImode */
        short_sum += *(sptr + 0);   /* HImode */
        int_sum += *(iptr + 0);     /* SImode */
        long_sum += *(lptr + 0);    /* DImode */
        
        cptr++;
        sptr++;
        iptr++;
        lptr++;
    }
    
    /* Use results to prevent optimization */
    printf("Mixed types: %d %d %d %ld\n", char_sum, short_sum, int_sum, long_sum);
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    volatile int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (volatile int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 3;
    }
    
    volatile int *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Volatile access with zero offset */
        sum += vptr[0];     /* Array notation with zero */
        sum += *(vptr + 0); /* Pointer notation with zero */
        vptr++;
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 4;
    }
    
    /* Restrict gives compiler stronger guarantees */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[i + 0];  /* Should be optimizable */
        rptr++;              /* Post-increment */
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
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < ARRAY_SIZE / 10; inner++) {
            /* Conditional access with zero offset */
            if (inner % 2 == 0) {
                sum += ptr[0];          /* Zero offset in true branch */
            } else {
                sum += ptr[1];          /* Non-zero offset in false branch */
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
        arr[i].third = i % 256;
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Access first member (offset 0) */
        sum += sptr->first;     /* This compiles to offset 0 */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Post-decrement with zero offset */
int test8_post_decrement(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 5;
    }
    
    int *ptr = &arr[ARRAY_SIZE - 1];
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
        sum += *(ptr + 0);  /* Zero offset */
        ptr--;              /* Post-decrement */
    }
    
    return sum;
}

/* Test 9: Multiple increments in same loop */
int test9_multiple_increments(void) {
    int arr1[ARRAY_SIZE], arr2[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple zero-offset accesses with different pointers */
        sum += p1[0] + p2[0];
        p1++;
        p2 += 1;  /* Different increment syntax */
    }
    
    return sum;
}

/* Test 10: Complex expression with zero offset */
int test10_complex_zero_expr(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 7;
    }
    
    int *ptr = arr;
    int index = 0;
    
    while (index < ARRAY_SIZE) {
        /* Complex zero expression */
        int zero = 0;
        sum += ptr[zero];           /* Variable holding zero */
        sum += ptr[0 * index];      /* Zero times something */
        sum += ptr[index * 0];      /* Something times zero */
        
        ptr++;
        index++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Run tests based on command line or all by default */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "test1") || (argc > 1 && atoi(argv[1]) == 1)) {
        total_sum += test1_simple_zero_offset();
    }
    
    if (run_all || strstr(argv[0], "test2") || (argc > 1 && atoi(argv[1]) == 2)) {
        total_sum += test2_array_index_zero();
    }
    
    if (run_all || strstr(argv[0], "test3") || (argc > 1 && atoi(argv[1]) == 3)) {
        test3_mixed_types();
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && atoi(argv[1]) == 4)) {
        total_sum += test4_volatile_zero_offset();
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && atoi(argv[1]) == 5)) {
        total_sum += test5_restrict_zero_offset();
    }
    
    if (run_all || strstr(argv[0], "test6") || (argc > 1 && atoi(argv[1]) == 6)) {
        total_sum += test6_nested_conditional();
    }
    
    if (run_all || strstr(argv[0], "test7") || (argc > 1 && atoi(argv[1]) == 7)) {
        total_sum += test7_struct_first_member();
    }
    
    if (run_all || strstr(argv[0], "test8") || (argc > 1 && atoi(argv[1]) == 8)) {
        total_sum += test8_post_decrement();
    }
    
    if (run_all || strstr(argv[0], "test9") || (argc > 1 && atoi(argv[1]) == 9)) {
        total_sum += test9_multiple_increments();
    }
    
    if (run_all || strstr(argv[0], "test10") || (argc > 1 && atoi(argv[1]) == 10)) {
        total_sum += test10_complex_zero_expr();
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
