/* auto-inc-dec-test.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Lines 1352-1358: Setting up mem_insn with reg1_is_const=true, reg1_val=0
 * and calling find_inc(true)
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
    
    /* Access with explicit zero offset - should generate (plus (reg) (const_int 0)) */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[i + 0];  /* Explicit zero offset */
        ptr++;  /* Post-increment to create auto-inc pattern */
    }
    
    return sum;
}

/* Test 2: Multiple data types with zero-offset access */
int test2_mixed_types(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)i;
        int_arr[i] = i;
        long_arr[i] = i;
    }
    
    /* Char access with zero offset */
    char *cptr = char_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += cptr[0];  /* Zero offset access */
        cptr += 1;  /* Different increment pattern */
    }
    
    /* Short access with zero offset */
    short *sptr = short_arr;
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += sptr[0];  /* Zero offset access */
        sptr += 2;  /* Step by 2 */
    }
    
    /* Int access with zero offset */
    int *iptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(iptr + 0);  /* Alternative zero offset syntax */
        iptr++;  /* Post-increment */
    }
    
    /* Long access with zero offset */
    long *lptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += (int)lptr[(int)(0)];  /* Cast zero to int */
        lptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 3: Volatile pointer with zero offset */
int test3_volatile_access(void) {
    int arr[ARRAY_SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[i + 0];  /* Zero offset with volatile */
        /* No pointer increment here - volatile might prevent optimization */
    }
    
    /* Another volatile pattern */
    volatile int *vptr2 = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(vptr2 + 0);  /* Alternative syntax */
        vptr2++;  /* Post-increment with volatile */
    }
    
    return sum;
}

/* Test 4: Restrict pointer for aliasing guarantees */
int test4_restrict_pointer(void) {
    int arr[ARRAY_SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 3;
    }
    
    /* Restrict pointer with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];  /* Zero offset with restrict */
        rptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 5: Nested loops and conditional access */
int test5_complex_patterns(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    
    /* Nested loop with conditional zero-offset access */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < 2; j++) {
            if (j == 0) {
                sum += ptr[0];  /* Zero offset in conditional */
            } else {
                sum += ptr[1];  /* Non-zero offset for contrast */
            }
        }
        ptr++;  /* Post-increment in outer loop */
    }
    
    /* Another pattern: pointer arithmetic in loop update */
    int *ptr2 = arr;
    for (int i = 0; i < ARRAY_SIZE; ptr2++, i++) {
        sum += ptr2[0];  /* Zero offset */
    }
    
    return sum;
}

/* Test 6: Post-decrement patterns */
int test6_post_decrement(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 4;
    }
    
    /* Post-decrement with zero offset */
    int *ptr = &arr[ARRAY_SIZE - 1];
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
        sum += ptr[0];  /* Zero offset */
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

int test7_struct_access(void) {
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)(i % 256);
    }
    
    /* Access first member (offset 0) with pointer arithmetic */
    struct test_struct *sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr->first;  /* Accesses member at offset 0 */
        sptr++;  /* Post-increment */
    }
    
    /* Alternative: pointer to first member */
    int *iptr = &arr[0].first;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += iptr[0];  /* Zero offset access */
        iptr += 2;  /* Skip to next struct's first member (assuming no padding) */
    }
    
    return sum;
}

/* Test 8: While loop with zero offset */
int test8_while_loop(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 5;
    }
    
    /* While loop with post-increment */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    while (ptr < end) {
        sum += *(ptr + 0);  /* Zero offset */
        ptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 9: Multiple zero offsets in same expression */
int test9_multiple_zero_offsets(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Multiple memory accesses with zero offset */
    int *ptr1 = arr;
    int *ptr2 = arr + ARRAY_SIZE/2;
    
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        sum += ptr1[0] + ptr2[0];  /* Two zero-offset accesses */
        ptr1++;
        ptr2++;
    }
    
    return sum;
}

/* Test 10: Function pointer parameter with zero offset */
static int process_array(int *arr, int size) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        sum += ptr[0];  /* Zero offset */
        ptr++;  /* Post-increment */
    }
    
    return sum;
}

int test10_function_param(void) {
    int arr[ARRAY_SIZE];
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 6;
    }
    
    return process_array(arr, ARRAY_SIZE);
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    int test_to_run = -1;
    
    /* Parse command line to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    if (test_to_run == 1 || test_to_run == -1) {
        total_sum += test1_basic_zero_offset();
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == 2 || test_to_run == -1) {
        total_sum += test2_mixed_types();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == 3 || test_to_run == -1) {
        total_sum += test3_volatile_access();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == 4 || test_to_run == -1) {
        total_sum += test4_restrict_pointer();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == 5 || test_to_run == -1) {
        total_sum += test5_complex_patterns();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == 6 || test_to_run == -1) {
        total_sum += test6_post_decrement();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == 7 || test_to_run == -1) {
        total_sum += test7_struct_access();
        printf("Test 7 completed\n");
    }
    
    if (test_to_run == 8 || test_to_run == -1) {
        total_sum += test8_while_loop();
        printf("Test 8 completed\n");
    }
    
    if (test_to_run == 9 || test_to_run == -1) {
        total_sum += test9_multiple_zero_offsets();
        printf("Test 9 completed\n");
    }
    
    if (test_to_run == 10 || test_to_run == -1) {
        total_sum += test10_function_param();
        printf("Test 10 completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    /* Use the result to prevent dead code elimination */
    if (total_sum == 0) {
        printf("Warning: All tests returned zero\n");
    }
    
    return 0;
}
