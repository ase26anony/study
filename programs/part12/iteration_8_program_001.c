/* Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024

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
        ptr++;  /* Post-increment for find_inc to match */
    }
    
    return sum;
}

/* Test 2: Post-decrement with array[0] access */
int test2_post_decrement_zero_index(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + SIZE - 1;
    for (int i = 0; i < SIZE; i++) {
        /* Access with array[0] pattern */
        sum += ptr[0];  /* Zero index access */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
int test3_mixed_types_zero_offset(void) {
    char c_arr[SIZE];
    short s_arr[SIZE];
    int i_arr[SIZE];
    long l_arr[SIZE];
    
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        c_arr[i] = (char)(i % 256);
        s_arr[i] = (short)(i * 2);
        i_arr[i] = i * 3;
        l_arr[i] = i * 4L;
    }
    
    /* Char pointer with zero offset */
    char *c_ptr = c_arr;
    for (int i = 0; i < SIZE; i++) {
        sum += (int)(*(c_ptr + 0));  /* QImode access */
        c_ptr++;
    }
    
    /* Short pointer with zero offset */
    short *s_ptr = s_arr;
    for (int i = 0; i < SIZE; i++) {
        sum += (int)(s_ptr[0]);  /* HImode access */
        s_ptr++;
    }
    
    /* Int pointer with zero offset */
    int *i_ptr = i_arr;
    for (int i = 0; i < SIZE; i++) {
        sum += i_ptr[0];  /* SImode access */
        i_ptr++;
    }
    
    /* Long pointer with zero offset */
    long *l_ptr = l_arr;
    for (int i = 0; i < SIZE; i++) {
        sum += (int)(l_ptr[0]);  /* DImode access */
        l_ptr++;
    }
    
    return sum;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    int arr[SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < SIZE; i++) {
        /* Force (plus (reg) (const_int 0)) with volatile */
        sum += vptr[i + 0];  /* Zero offset with volatile */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_zero_offset(void) {
    int arr[SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 5;
    }
    
    /* Restrict pointer with zero offset access */
    for (int i = 0; i < SIZE; i++) {
        /* Multiple zero-offset patterns */
        sum += rptr[0];      /* Direct zero index */
        sum += *(rptr + 0);  /* Pointer + 0 */
        rptr++;              /* Post-increment */
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
    int (*ptr)[4] = arr;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < 4; j++) {
            if (j % 2 == 0) {
                /* Zero offset access in conditional path */
                sum += ptr[i][0];  /* First element access */
            } else {
                sum += ptr[i][j];
            }
        }
        /* Pointer arithmetic with different step sizes */
        ptr = (int (*)[4])((char *)ptr + sizeof(int[4]));
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
    
    /* Initialize structures */
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i * 10;
        arr[i].second = i * 20;
        arr[i].third = (char)(i % 128);
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Access first member (offset 0) */
        sum += sptr->first;  /* Equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Complex pointer arithmetic with zero constant */
int test8_complex_zero_arithmetic(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 7;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE;
    
    /* Multiple zero-offset patterns in same loop */
    while (ptr < end) {
        /* Different ways to express zero offset */
        int zero = 0;
        sum += ptr[0];                    /* Direct zero index */
        sum += *(ptr + 0);                /* Pointer + literal 0 */
        sum += *(ptr + (int)(0));         /* Cast zero to int */
        sum += *(ptr + zero);             /* Variable containing zero */
        sum += *(ptr + (zero * 2));       /* Zero expression */
        
        ptr += 2;  /* Step by 2 to test different increments */
    }
    
    return sum;
}

/* Test 9: Function pointer parameter with zero offset */
static int process_array(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Zero offset access in function */
        sum += ptr[0];  /* Force (plus (reg) (const_int 0)) */
        ptr++;
    }
    return sum;
}

int test9_function_param(void) {
    int arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 3;
    }
    
    return process_array(arr, SIZE);
}

/* Test 10: Switch statement with zero offset cases */
int test10_switch_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        switch (i % 4) {
            case 0:
                sum += ptr[0];  /* Zero offset in case 0 */
                break;
            case 1:
                sum += ptr[1];
                break;
            case 2:
                sum += *(ptr + 0);  /* Another zero offset */
                break;
            case 3:
                sum += ptr[3];
                break;
        }
        ptr++;
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
    
    /* Run selected test or all tests */
    if (test_to_run == -1 || test_to_run == 1) {
        total_sum += test1_post_increment_zero_offset();
    }
    if (test_to_run == -1 || test_to_run == 2) {
        total_sum += test2_post_decrement_zero_index();
    }
    if (test_to_run == -1 || test_to_run == 3) {
        total_sum += test3_mixed_types_zero_offset();
    }
    if (test_to_run == -1 || test_to_run == 4) {
        total_sum += test4_volatile_zero_offset();
    }
    if (test_to_run == -1 || test_to_run == 5) {
        total_sum += test5_restrict_zero_offset();
    }
    if (test_to_run == -1 || test_to_run == 6) {
        total_sum += test6_nested_conditional();
    }
    if (test_to_run == -1 || test_to_run == 7) {
        total_sum += test7_struct_first_member();
    }
    if (test_to_run == -1 || test_to_run == 8) {
        total_sum += test8_complex_zero_arithmetic();
    }
    if (test_to_run == -1 || test_to_run == 9) {
        total_sum += test9_function_param();
    }
    if (test_to_run == -1 || test_to_run == 10) {
        total_sum += test10_switch_zero_offset();
    }
    
    /* Print result to prevent optimization */
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
