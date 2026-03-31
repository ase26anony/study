/* auto-inc-dec-test.c
 * Test program to trigger auto-increment/decrement optimization patterns
 * Specifically targets (mem (plus (reg) (const_int 0))) RTL patterns
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
    
    /* Loop with pointer arithmetic using +0 */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    while (ptr < end) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += ptr[0];           /* Zero offset access */
        sum += *(ptr + 0);       /* Alternative zero offset */
        ptr++;                   /* Post-increment */
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
        /* Multiple zero-offset patterns */
        sum += ptr[0];
        sum += *(ptr + 0);
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types to test various memory modes */
int test3_mixed_data_types(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i * 3);
        int_arr[i] = i * 5;
        long_arr[i] = i * 7L;
    }
    
    /* Char pointer loop with zero offset */
    char *cptr = char_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += cptr[0];      /* QImode access */
        sum += *(cptr + 0);
        cptr++;
    }
    
    /* Short pointer loop */
    short *sptr = short_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr[0];      /* HImode access */
        sum += *(sptr + 0);
        sptr++;
    }
    
    /* Int pointer loop */
    int *iptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += iptr[0];      /* SImode access */
        sum += *(iptr + 0);
        iptr++;
    }
    
    /* Long pointer loop */
    long *lptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += (int)lptr[0]; /* DImode access */
        sum += (int)*(lptr + 0);
        lptr++;
    }
    
    return sum;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    int arr[ARRAY_SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 11;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[0];      /* Should generate (mem (plus (reg) (const_int 0))) */
        sum += *(vptr + 0);
        vptr++;              /* Post-increment */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for aliasing guarantees */
int test5_restrict_pointer(void) {
    int arr[ARRAY_SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 13;
    }
    
    /* Restrict pointer with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];
        sum += *(rptr + 0);
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero-offset access */
int test6_nested_conditional(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE * 2;
    
    /* Outer loop */
    for (int outer = 0; outer < 10; outer++) {
        ptr = arr;  /* Reset pointer */
        
        /* Inner loop with conditional access */
        while (ptr < end) {
            if ((ptr - arr) % 3 == 0) {
                /* Conditional zero-offset access */
                sum += ptr[0];
                sum += *(ptr + 0);
            } else if ((ptr - arr) % 3 == 1) {
                /* Different offset to create variety */
                sum += ptr[1];
            } else {
                /* Another zero-offset variant */
                sum += *(ptr + 0);
            }
            
            ptr++;  /* Post-increment in loop update */
        }
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;   /* At offset 0 */
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
    struct test_struct *send = arr + ARRAY_SIZE;
    
    while (sptr < send) {
        /* Access first member (offset 0) */
        sum += sptr->first;      /* Equivalent to sptr[0].first */
        sum += (*(sptr + 0)).first;
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit zero cast as index */
int test8_explicit_zero_cast(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 17;
    }
    
    int *ptr = arr;
    
    /* Use explicit cast of 0 to force constant zero */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int zero = 0;
        /* Try to force const_int 0 pattern */
        sum += ptr[(int)(0)];        /* Cast zero to int */
        sum += ptr[zero];            /* Variable zero */
        sum += *(ptr + (size_t)0);   /* Cast to size_t */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple induction variables */
int test9_multiple_induction(void) {
    int arr[ARRAY_SIZE * 3];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 3; i++) {
        arr[i] = i;
    }
    
    /* Multiple pointers with different step sizes */
    int *ptr1 = arr;
    int *ptr2 = arr;
    int *ptr3 = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* All using zero offset */
        sum += ptr1[0];
        sum += ptr2[0];
        sum += ptr3[0];
        
        /* Different step sizes */
        ptr1 += 1;  /* Step 1 */
        ptr2 += 2;  /* Step 2 */
        ptr3 += 3;  /* Step 3 */
    }
    
    return sum;
}

/* Test 10: Complex pointer arithmetic expression */
int test10_complex_arithmetic(void) {
    int arr[ARRAY_SIZE * 4];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 4; i++) {
        arr[i] = i;
    }
    
    int *base = arr;
    
    /* Complex expression that should simplify to base + 0 */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int *ptr = base + (i * 0);  /* Should become base + 0 */
        sum += ptr[0];
        sum += *(ptr + 0);
    }
    
    /* Another variant */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Arithmetic that should yield +0 */
        sum += *(ptr + i - i);  /* i - i = 0 */
        sum += ptr[i * 0];      /* i * 0 = 0 */
        ptr++;
    }
    
    return sum;
}

/* Main function to run selected tests */
int main(int argc, char *argv[]) {
    int test_to_run = -1;
    int result = 0;
    
    /* Parse command line argument to select test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run all tests if no specific test requested */
    if (test_to_run == -1 || test_to_run == 1) {
        result += test1_post_increment_zero_offset();
    }
    if (test_to_run == -1 || test_to_run == 2) {
        result += test2_post_decrement_zero_offset();
    }
    if (test_to_run == -1 || test_to_run == 3) {
        result += test3_mixed_data_types();
    }
    if (test_to_run == -1 || test_to_run == 4) {
        result += test4_volatile_access();
    }
    if (test_to_run == -1 || test_to_run == 5) {
        result += test5_restrict_pointer();
    }
    if (test_to_run == -1 || test_to_run == 6) {
        result += test6_nested_conditional();
    }
    if (test_to_run == -1 || test_to_run == 7) {
        result += test7_struct_first_member();
    }
    if (test_to_run == -1 || test_to_run == 8) {
        result += test8_explicit_zero_cast();
    }
    if (test_to_run == -1 || test_to_run == 9) {
        result += test9_multiple_induction();
    }
    if (test_to_run == -1 || test_to_run == 10) {
        result += test10_complex_arithmetic();
    }
    
    /* Print result to prevent optimization removal */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
