/* auto-inc-dec-test.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c auto-inc-dec-test.c
 */

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
    
    /* Access with ptr[0] pattern - should generate (plus (reg) (const_int 0)) */
    int *ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end) {
        /* Force zero offset access */
        sum += ptr[0];  /* Should generate: (mem (plus (reg) (const_int 0))) */
        ptr++;  /* Post-increment for pattern matching */
    }
    
    return sum;
}

/* Test 2: Post-decrement with explicit +0 arithmetic */
int test2_post_decrement_zero_arithmetic(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + SIZE - 1;
    int *start = arr;
    
    while (ptr >= start) {
        /* Explicit +0 in pointer arithmetic */
        sum += *(ptr + 0);  /* Should generate: (mem (plus (reg) (const_int 0))) */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
int test3_mixed_types_zero_offset(void) {
    char char_arr[SIZE];
    short short_arr[SIZE];
    int int_arr[SIZE];
    long long_arr[SIZE];
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        char_arr[i] = i % 256;
        short_arr[i] = i * 2;
        int_arr[i] = i * 3;
        long_arr[i] = i * 4;
    }
    
    /* Char access with zero offset */
    char *cptr = char_arr;
    for (int i = 0; i < SIZE; i++) {
        sum += cptr[0];  /* QImode access */
        cptr++;
    }
    
    /* Short access with zero offset */
    short *sptr = short_arr;
    for (int i = 0; i < SIZE; i++) {
        sum += sptr[0];  /* HImode access */
        sptr++;
    }
    
    /* Int access with zero offset */
    int *iptr = int_arr;
    for (int i = 0; i < SIZE; i++) {
        sum += iptr[0];  /* SImode access */
        iptr++;
    }
    
    /* Long access with zero offset */
    long *lptr = long_arr;
    for (int i = 0; i < SIZE; i++) {
        sum += lptr[0];  /* DImode access */
        lptr++;
    }
    
    return sum;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Volatile pointer - may affect optimization but creates interesting RTL */
    volatile int *vptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Force zero offset with volatile */
        sum += vptr[i + 0];  /* Should generate: (mem (plus (reg) (const_int 0))) */
    }
    
    /* Also test with restrict for aliasing guarantees */
    int *restrict rptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += rptr[0];
        rptr++;
    }
    
    return sum;
}

/* Test 5: Nested loops with conditional zero offset access */
int test5_nested_conditional_zero_offset(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i % 100;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE * 2;
    
    /* Complex control flow to stress pattern matching */
    while (ptr < end) {
        if ((ptr - arr) % 3 == 0) {
            /* Access with zero offset in true branch */
            sum += ptr[0];  /* (mem (plus (reg) (const_int 0))) */
        } else if ((ptr - arr) % 3 == 1) {
            /* Different offset in else-if branch */
            sum += ptr[1];
        } else {
            /* Another zero offset access */
            sum += *(ptr + 0);  /* Alternative form */
        }
        
        /* Post-increment in loop update */
        ptr++;
        
        /* Nested small loop */
        for (int j = 0; j < 2; j++) {
            if (ptr < end - 1) {
                sum += ptr[0];  /* Another zero offset access */
            }
        }
    }
    
    return sum;
}

/* Test 6: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* At offset 0 */
    int second;
    char third;
};

int test6_struct_first_member(void) {
    struct test_struct arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = i % 256;
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Accessing first member - at offset 0 */
        sum += sptr->first;  /* Should generate zero offset pattern */
        sptr++;
    }
    
    return sum;
}

/* Test 7: Array of pointers with zero index */
int test7_pointer_array_zero_index(void) {
    int data[SIZE];
    int *ptr_arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        data[i] = i;
        ptr_arr[i] = &data[i];
    }
    
    /* Access through pointer array with zero offset */
    int **pptr = ptr_arr;
    for (int i = 0; i < SIZE; i++) {
        /* Double dereference with zero offset */
        sum += pptr[0][0];  /* Should create complex addressing patterns */
        pptr++;
    }
    
    return sum;
}

/* Test 8: Different step sizes with zero offset */
int test8_various_step_sizes(void) {
    int arr[SIZE * 4];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 4; i++) {
        arr[i] = i;
    }
    
    /* Step size 1 */
    int *ptr1 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += ptr1[0];
        ptr1 += 1;
    }
    
    /* Step size 2 */
    int *ptr2 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += ptr2[0];
        ptr2 += 2;
    }
    
    /* Step size 4 */
    int *ptr4 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += ptr4[0];
        ptr4 += 4;
    }
    
    /* Negative step */
    int *ptr_neg = arr + SIZE - 1;
    for (int i = 0; i < SIZE; i++) {
        sum += ptr_neg[0];
        ptr_neg -= 1;
    }
    
    return sum;
}

/* Test 9: Cast zero to pointer offset type */
int test9_cast_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Cast zero to different types to force (const_int 0) */
        sum += ptr[(int)(0)];          /* Cast to int */
        sum += ptr[(unsigned)(0)];     /* Cast to unsigned */
        sum += ptr[(size_t)(0)];       /* Cast to size_t */
        sum += ptr[(ptrdiff_t)(0)];    /* Cast to ptrdiff_t */
        ptr++;
    }
    
    return sum;
}

/* Test 10: Combined patterns in single loop */
int test10_combined_patterns(void) {
    int arr[SIZE * 3];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 3; i++) {
        arr[i] = i % 1000;
    }
    
    /* Multiple pointer variables in same loop */
    int *ptr1 = arr;
    int *ptr2 = arr + SIZE;
    int *ptr3 = arr + SIZE * 2;
    
    for (int i = 0; i < SIZE; i++) {
        /* Different zero-offset patterns */
        sum += ptr1[0];        /* Array subscript */
        sum += *(ptr2 + 0);    /* Pointer arithmetic */
        sum += *ptr3;          /* Direct dereference (implicit offset 0) */
        
        /* All pointers increment */
        ptr1++;
        ptr2++;
        ptr3++;
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
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 2) {
        total_sum += test2_post_decrement_zero_arithmetic();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 3) {
        total_sum += test3_mixed_types_zero_offset();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 4) {
        total_sum += test4_volatile_zero_offset();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 5) {
        total_sum += test5_nested_conditional_zero_offset();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 6) {
        total_sum += test6_struct_first_member();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 7) {
        total_sum += test7_pointer_array_zero_index();
        printf("Test 7 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 8) {
        total_sum += test8_various_step_sizes();
        printf("Test 8 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 9) {
        total_sum += test9_cast_zero_offset();
        printf("Test 9 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 10) {
        total_sum += test10_combined_patterns();
        printf("Test 10 completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    /* Use result to prevent dead code elimination */
    if (total_sum > 0) {
        return 0;
    } else {
        return 1;
    }
}
