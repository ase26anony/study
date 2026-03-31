/* test_auto_inc_dec.c - Program to trigger auto-inc-dec pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define ITERATIONS 100

/* Test 1: Basic pointer arithmetic with zero offset in loops */
int test1_basic_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Access with explicit zero offset - should generate (plus (reg) (const_int 0)) */
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += *(ptr + 0);  /* Explicit zero offset */
        ptr++;
    }
    
    return sum;
}

/* Test 2: Array indexing with zero addition */
int test2_array_zero_index(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    /* Multiple ways to express zero offset */
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i + 0];           /* Addition of zero */
        sum += arr[(int)(0) + i];    /* Cast zero to int */
        sum += arr[0 + i];           /* Zero first */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
int test3_mixed_types(void) {
    char carr[SIZE];
    short sarr[SIZE];
    int iarr[SIZE];
    long larr[SIZE];
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        carr[i] = (char)(i & 0xFF);
        sarr[i] = (short)(i & 0xFFFF);
        iarr[i] = i;
        larr[i] = i * 10L;
    }
    
    /* Char access with zero offset */
    char *cptr = carr;
    for (int i = 0; i < SIZE; i++) {
        sum += cptr[0];  /* QImode access */
        cptr++;
    }
    
    /* Short access with zero offset */
    short *sptr = sarr;
    for (int i = 0; i < SIZE; i++) {
        sum += sptr[0];  /* HImode access */
        sptr++;
    }
    
    /* Int access with zero offset */
    int *iptr = iarr;
    for (int i = 0; i < SIZE; i++) {
        sum += iptr[0];  /* SImode access */
        iptr++;
    }
    
    /* Long access with zero offset */
    long *lptr = larr;
    for (int i = 0; i < SIZE; i++) {
        sum += (int)lptr[0];  /* DImode access */
        lptr++;
    }
    
    return sum;
}

/* Test 4: Volatile pointers with zero offset */
int test4_volatile_access(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 3;
    }
    
    /* Volatile pointer - may prevent some optimizations but creates interesting patterns */
    volatile int *vptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += vptr[i + 0];  /* Zero offset with volatile */
        /* Force pointer arithmetic to not be optimized away */
        asm volatile("" : "+r"(sum) : "r"(vptr));
    }
    
    /* Restrict pointer for aliasing guarantees */
    int *restrict rptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += rptr[0 + i];  /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 5: Nested loops and conditional access */
int test5_complex_patterns(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int outer = 0; outer < ITERATIONS; outer++) {
        for (int inner = 0; inner < SIZE / ITERATIONS; inner++) {
            /* Conditional zero-offset access */
            if (inner % 2 == 0) {
                sum += ptr[0];  /* Even indices use zero offset */
            } else {
                sum += ptr[1];  /* Odd indices use offset 1 */
            }
            
            /* Post-increment in loop update */
            ptr++;
        }
        
        /* Reset pointer with some offset */
        ptr = arr + (outer % 10);
    }
    
    return sum;
}

/* Test 6: Structure access with zero offset (first field) */
struct test_struct {
    int first;   /* Offset 0 */
    int second;
    char third;
};

int test6_struct_zero_offset(void) {
    struct test_struct sarr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        sarr[i].first = i;
        sarr[i].second = i * 2;
        sarr[i].third = (char)i;
    }
    
    struct test_struct *sptr = sarr;
    for (int i = 0; i < SIZE; i++) {
        /* Access first field - offset 0 */
        sum += sptr->first;  /* Equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 7: Post-decrement patterns */
int test7_post_decrement(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 4;
    }
    
    /* Start from end, decrement */
    int *ptr = arr + SIZE - 1;
    for (int i = SIZE - 1; i >= 0; i--) {
        sum += ptr[0];  /* Zero offset with decrement */
        ptr--;
    }
    
    return sum;
}

/* Test 8: Multiple increments in same loop */
int test8_multiple_increments(void) {
    int arr1[SIZE], arr2[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 5;
    }
    
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    for (int i = 0; i < SIZE; i++) {
        /* Both pointers with zero offset */
        sum += ptr1[0] + ptr2[0];
        ptr1++;
        ptr2 += 1;  /* Alternative increment syntax */
    }
    
    return sum;
}

/* Test 9: Pointer arithmetic in loop condition */
int test9_loop_condition(void) {
    char str[SIZE];
    int sum = 0;
    
    /* Create a string with null terminator */
    for (int i = 0; i < SIZE - 1; i++) {
        str[i] = 'A' + (i % 26);
    }
    str[SIZE - 1] = '\0';
    
    /* Classic while(*ptr++) pattern */
    char *ptr = str;
    while (*ptr != '\0') {
        sum += ptr[0];  /* Zero offset access */
        ptr++;
    }
    
    return sum;
}

/* Test 10: Complex expression with zero */
int test10_complex_zero_expr(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 6;
    }
    
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Complex expression that evaluates to zero offset */
        int offset = (i * 0) + (0 * i) + (i - i);
        sum += ptr[offset];  /* Should be ptr[0] */
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
    if (test_to_run == 1 || test_to_run == -1) {
        total_sum += test1_basic_zero_offset();
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == 2 || test_to_run == -1) {
        total_sum += test2_array_zero_index();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == 3 || test_to_run == -1) {
        total_sum += test3_mixed_types();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == 4 || test_to_run == -1) {
        total_sum += test4_volatile_access();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == 5 || test_to_run == -1) {
        total_sum += test5_complex_patterns();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == 6 || test_to_run == -1) {
        total_sum += test6_struct_zero_offset();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == 7 || test_to_run == -1) {
        total_sum += test7_post_decrement();
        printf("Test 7 completed\n");
    }
    
    if (test_to_run == 8 || test_to_run == -1) {
        total_sum += test8_multiple_increments();
        printf("Test 8 completed\n");
    }
    
    if (test_to_run == 9 || test_to_run == -1) {
        total_sum += test9_loop_condition();
        printf("Test 9 completed\n");
    }
    
    if (test_to_run == 10 || test_to_run == -1) {
        total_sum += test10_complex_zero_expr();
        printf("Test 10 completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    /* Use result to prevent dead code elimination */
    if (total_sum == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
