/* auto-inc-dec-test.c
 * Test program to trigger auto-inc-dec pass coverage for lines 1352-1358
 * Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c auto-inc-dec-test.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Basic pointer arithmetic with zero offset in loop */
int test1_basic_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Access with ptr + 0 pattern */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    while (ptr < end) {
        /* This should generate (mem (plus (reg) (const_int 0))) */
        sum += *(ptr + 0);  /* Zero offset access */
        ptr++;  /* Post-increment for find_inc to match */
    }
    
    return sum;
}

/* Test 2: Different data types with zero offset */
int test2_mixed_types(void) {
    char c_arr[ARRAY_SIZE];
    short s_arr[ARRAY_SIZE];
    int i_arr[ARRAY_SIZE];
    long l_arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        c_arr[i] = (char)(i % 256);
        s_arr[i] = (short)(i % 65536);
        i_arr[i] = i;
        l_arr[i] = i * 2L;
    }
    
    /* Char access with zero offset */
    char *c_ptr = c_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += c_ptr[0];  /* QImode access with zero offset */
        c_ptr++;
    }
    
    /* Short access with zero offset */
    short *s_ptr = s_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += s_ptr[0];  /* HImode access with zero offset */
        s_ptr++;
    }
    
    /* Int access with zero offset */
    int *i_ptr = i_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += i_ptr[0];  /* SImode access with zero offset */
        i_ptr++;
    }
    
    /* Long access with zero offset */
    long *l_ptr = l_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += (int)l_ptr[0];  /* DImode access with zero offset */
        l_ptr++;
    }
    
    return sum;
}

/* Test 3: Volatile pointer with zero offset */
int test3_volatile_access(void) {
    int arr[ARRAY_SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[i + 0];  /* Zero offset with volatile */
    }
    
    /* Another pattern: volatile pointer increment */
    vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(vptr + 0);  /* *(ptr + 0) pattern */
        vptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 4: Restrict pointer for alias analysis */
int test4_restrict_pointer(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Use restrict to give compiler alias guarantees */
    int *restrict rptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];  /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 5: Nested loops and conditional access */
int test5_complex_patterns(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i % 100;
    }
    
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE * 2;
    
    /* Nested loop structure */
    for (int outer = 0; outer < 2; outer++) {
        int *inner_ptr = ptr + outer * ARRAY_SIZE;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Conditional zero-offset access */
            if (i % 2 == 0) {
                sum += inner_ptr[0];  /* Zero offset in true branch */
            } else {
                sum += inner_ptr[1];  /* Non-zero offset in false branch */
            }
            inner_ptr++;
        }
    }
    
    /* Another pattern: pointer arithmetic in loop update */
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += ptr[0];  /* Access with zero offset */
        ptr += 2;  /* Step by 2 for different increment pattern */
    }
    
    return sum;
}

/* Test 6: Post-decrement patterns */
int test6_post_decrement(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Post-decrement loop */
    int *ptr = arr + ARRAY_SIZE - 1;
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
        sum += ptr[0];  /* Zero offset access */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* At offset 0 */
    int second;
    char third;
};

int test7_struct_offset_zero(void) {
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)(i % 256);
    }
    
    /* Access first member (offset 0) */
    struct test_struct *sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr->first;  /* Accesses member at offset 0 */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Cast zero to pointer offset */
int test8_cast_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Cast zero to different types to force (const_int 0) */
        sum += ptr[(int)(0)];           /* Cast to int */
        sum += ptr[(unsigned)(0)];      /* Cast to unsigned */
        sum += ptr[(size_t)(0)];        /* Cast to size_t */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple zero offsets in same expression */
int test9_multiple_zero_offsets(void) {
    int arr1[ARRAY_SIZE];
    int arr2[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 3;
    }
    
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple zero-offset accesses in same loop iteration */
        sum += ptr1[0] + ptr2[0];  /* Two zero-offset accesses */
        ptr1++;
        ptr2++;
    }
    
    return sum;
}

/* Test 10: Zero offset with different step sizes */
int test10_variable_steps(void) {
    int arr[ARRAY_SIZE * 4];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 4; i++) {
        arr[i] = i % 50;
    }
    
    /* Step by 1 */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[0];
        ptr += 1;
    }
    
    /* Step by 2 */
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[0];
        ptr += 2;
    }
    
    /* Step by 4 */
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[0];
        ptr += 4;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    int test_to_run = -1;
    
    /* Parse command line argument to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    if (test_to_run == -1 || test_to_run == 1) {
        total_sum += test1_basic_zero_offset();
    }
    if (test_to_run == -1 || test_to_run == 2) {
        total_sum += test2_mixed_types();
    }
    if (test_to_run == -1 || test_to_run == 3) {
        total_sum += test3_volatile_access();
    }
    if (test_to_run == -1 || test_to_run == 4) {
        total_sum += test4_restrict_pointer();
    }
    if (test_to_run == -1 || test_to_run == 5) {
        total_sum += test5_complex_patterns();
    }
    if (test_to_run == -1 || test_to_run == 6) {
        total_sum += test6_post_decrement();
    }
    if (test_to_run == -1 || test_to_run == 7) {
        total_sum += test7_struct_offset_zero();
    }
    if (test_to_run == -1 || test_to_run == 8) {
        total_sum += test8_cast_zero_offset();
    }
    if (test_to_run == -1 || test_to_run == 9) {
        total_sum += test9_multiple_zero_offsets();
    }
    if (test_to_run == -1 || test_to_run == 10) {
        total_sum += test10_variable_steps();
    }
    
    /* Print result to prevent optimization removal */
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
