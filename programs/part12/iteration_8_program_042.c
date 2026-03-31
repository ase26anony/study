/* auto-inc-dec-test.c
 * Program designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Lines 1352-1358 of auto-inc-dec.cc: mem_insn setup with reg1_val = 0 and find_inc(true)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Simple post-increment with zero offset in array access */
int test1_post_increment_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Loop with pointer arithmetic using zero offset */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    /* This should generate (mem (plus (reg) (const_int 0))) patterns */
    while (ptr < end) {
        /* Multiple zero-offset access patterns */
        sum += ptr[0];          /* Array index zero */
        sum += *(ptr + 0);      /* Pointer arithmetic with zero */
        ptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_post_decrement_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    int *start = arr;
    
    while (ptr >= start) {
        sum += ptr[0];      /* Zero offset access */
        sum += *(ptr + 0);  /* Another zero offset */
        ptr--;              /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
long test3_mixed_types_zero_offset(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    long total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i * 3);
        int_arr[i] = i * 5;
        long_arr[i] = i * 7L;
    }
    
    /* Process each array with zero-offset access */
    char *cptr = char_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += cptr[0];   /* QImode access */
        cptr++;
    }
    
    short *sptr = short_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += sptr[0];   /* HImode access */
        sptr++;
    }
    
    int *iptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += iptr[0];   /* SImode access */
        iptr++;
    }
    
    long *lptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += lptr[0];   /* DImode access */
        lptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    int arr[ARRAY_SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 11;
    }
    
    /* Volatile pointer with zero offset access */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[0];     /* Should generate (mem (plus (reg) (const_int 0))) */
        /* Cast zero to ensure constant zero offset */
        sum += *(vptr + (int)(0));
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
        arr[i] = i * 13;
    }
    
    /* Restrict pointer allows aggressive optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];     /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset access */
int test6_nested_conditional_zero_offset(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    
    /* Outer loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Inner loop with conditional access */
        for (int j = 0; j < 2; j++) {
            if (j == 0) {
                sum += ptr[0];      /* Zero offset in true branch */
            } else {
                sum += ptr[1];      /* Non-zero offset in false branch */
            }
        }
        ptr += 2;   /* Step by 2 to test different increments */
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
        arr[i].third = (char)(i % 128);
    }
    
    struct test_struct *sptr = arr;
    
    /* Access first member (offset 0) */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr->first;     /* Accesses member at offset 0 */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Complex pointer arithmetic with zero constant */
int test8_complex_zero_arithmetic(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 17;
    }
    
    int *ptr = arr;
    
    /* Multiple ways to express zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Different expressions that should compile to (const_int 0) */
        sum += ptr[0];                      /* Direct zero index */
        sum += *(ptr + 0);                  /* Pointer + 0 */
        sum += *(ptr + (0));                /* Parenthesized zero */
        sum += *(ptr + (1 - 1));            /* Arithmetic resulting in zero */
        sum += *(ptr + (i * 0));            /* Multiplication by zero */
        sum += *(ptr + (i & 0));            /* Bitwise AND with zero */
        
        ptr++;
    }
    
    return sum;
}

/* Test 9: Loop with multiple increments */
int test9_multiple_increments(void) {
    int arr1[ARRAY_SIZE];
    int arr2[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 19;
    }
    
    int *p1 = arr1;
    int *p2 = arr2;
    
    /* Process two arrays with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += p1[0];   /* Zero offset access to first array */
        sum += p2[0];   /* Zero offset access to second array */
        p1++;
        p2++;
    }
    
    return sum;
}

/* Test 10: Function pointer array with zero offset */
typedef int (*func_ptr_t)(void);

int func_a(void) { return 1; }
int func_b(void) { return 2; }
int func_c(void) { return 3; }

int test10_function_pointers(void) {
    func_ptr_t funcs[3] = {func_a, func_b, func_c};
    int sum = 0;
    
    func_ptr_t *fptr = funcs;
    
    /* Call functions through pointer array with zero offset */
    for (int i = 0; i < 3; i++) {
        sum += fptr[0]();   /* Zero offset function call */
        fptr++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int test_to_run = -1;
    int result = 0;
    
    /* Parse command line to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    if (test_to_run == 1 || test_to_run == -1) {
        result = test1_post_increment_zero_offset();
        printf("Test 1 result: %d\n", result);
    }
    
    if (test_to_run == 2 || test_to_run == -1) {
        result = test2_post_decrement_zero_offset();
        printf("Test 2 result: %d\n", result);
    }
    
    if (test_to_run == 3 || test_to_run == -1) {
        result = (int)test3_mixed_types_zero_offset();
        printf("Test 3 result: %d\n", result);
    }
    
    if (test_to_run == 4 || test_to_run == -1) {
        result = test4_volatile_zero_offset();
        printf("Test 4 result: %d\n", result);
    }
    
    if (test_to_run == 5 || test_to_run == -1) {
        result = test5_restrict_zero_offset();
        printf("Test 5 result: %d\n", result);
    }
    
    if (test_to_run == 6 || test_to_run == -1) {
        result = test6_nested_conditional_zero_offset();
        printf("Test 6 result: %d\n", result);
    }
    
    if (test_to_run == 7 || test_to_run == -1) {
        result = test7_struct_first_member();
        printf("Test 7 result: %d\n", result);
    }
    
    if (test_to_run == 8 || test_to_run == -1) {
        result = test8_complex_zero_arithmetic();
        printf("Test 8 result: %d\n", result);
    }
    
    if (test_to_run == 9 || test_to_run == -1) {
        result = test9_multiple_increments();
        printf("Test 9 result: %d\n", result);
    }
    
    if (test_to_run == 10 || test_to_run == -1) {
        result = test10_function_pointers();
        printf("Test 10 result: %d\n", result);
    }
    
    return 0;
}
