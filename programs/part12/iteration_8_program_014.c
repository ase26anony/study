/* test_auto_inc_dec.c
 * Program designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Lines 1352-1358 of auto-inc-dec.cc: mem_insn setup with reg1_is_const = true, reg1_val = 0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Test 1: Basic pointer arithmetic with zero offset in post-increment loop */
int test_basic_post_inc(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i % 100;
    }
    
    /* Loop with pointer arithmetic using +0 offset */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    /* This should generate (mem (plus (reg) (const_int 0))) patterns */
    while (ptr < end) {
        /* Multiple zero-offset accesses to increase chances */
        sum += *(ptr + 0);      /* Explicit +0 */
        sum += ptr[0];          /* Array notation with 0 */
        ptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement loop with zero offset */
int test_post_dec(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    
    /* Post-decrement loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 0);      /* +0 offset */
        ptr--;                  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types to test various memory modes */
long test_mixed_types(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    long total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = (char)(i % 128);
        short_arr[i] = (short)(i * 2);
        int_arr[i] = i * 3;
        long_arr[i] = i * 4L;
    }
    
    /* Char pointer loop - QImode */
    char *cptr = char_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += cptr[0];       /* Zero offset */
        cptr++;
    }
    
    /* Short pointer loop - HImode */
    short *sptr = short_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += sptr[0];       /* Zero offset */
        sptr++;
    }
    
    /* Int pointer loop - SImode */
    int *iptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += iptr[0];       /* Zero offset */
        iptr++;
    }
    
    /* Long pointer loop - DImode */
    long *lptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += lptr[0];       /* Zero offset */
        lptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test_volatile_access(void) {
    static int arr[ARRAY_SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i + 1;
    }
    
    /* Loop with volatile pointer */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple forms of zero offset access */
        sum += vptr[0];
        sum += *(vptr + 0);
        vptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test_restrict_pointer(void) {
    int arr[ARRAY_SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * i;
    }
    
    /* Restrict gives compiler stronger aliasing guarantees */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];  /* Zero offset */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero-offset access */
int test_nested_conditional(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i % 10;
    }
    
    int *ptr = arr;
    
    /* Outer loop */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with conditional access */
        for (int inner = 0; inner < ARRAY_SIZE/10; inner++) {
            if (inner % 2 == 0) {
                sum += ptr[0];          /* Zero offset in true branch */
            } else {
                sum += *(ptr + 0);      /* Zero offset in false branch */
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

int test_struct_first_member(void) {
    struct test_struct arr[100];
    int sum = 0;
    
    for (int i = 0; i < 100; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)i;
    }
    
    struct test_struct *sptr = arr;
    
    for (int i = 0; i < 100; i++) {
        /* Accessing first member is at offset 0 */
        sum += sptr->first;     /* This compiles to offset 0 */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit cast of zero to pointer offset */
int test_explicit_zero_cast(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i % 50;
    }
    
    int *ptr = arr;
    
    /* Explicitly cast zero to different types to force (const_int 0) */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[(int)0];             /* Cast to int */
        sum += ptr[(unsigned)0];        /* Cast to unsigned */
        sum += ptr[(size_t)0];          /* Cast to size_t */
        sum += ptr[(ptrdiff_t)0];       /* Cast to ptrdiff_t */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple induction variables with different step sizes */
int test_multiple_induction(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i;
    }
    
    /* Multiple pointers with different update patterns */
    int *ptr1 = arr;
    int *ptr2 = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr1[0];     /* Step by 1 */
        sum += ptr2[0];
        
        ptr1++;             /* Step +1 */
        ptr2 += 2;          /* Step +2 */
    }
    
    /* Now with negative step */
    int *ptr3 = arr + ARRAY_SIZE - 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr3[0];     /* Zero offset */
        ptr3--;             /* Step -1 */
    }
    
    return sum;
}

/* Test 10: Complex expression with zero offset */
int test_complex_zero_expr(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 3;
    }
    
    int *ptr = arr;
    
    /* Complex expression that should still simplify to +0 */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* These should all generate (const_int 0) offsets */
        sum += ptr[0 * 1];                  /* 0 * 1 = 0 */
        sum += ptr[1 - 1];                  /* 1 - 1 = 0 */
        sum += ptr[(i & 0)];                /* i & 0 = 0 */
        sum += ptr[(i ^ i)];                /* i ^ i = 0 */
        sum += ptr[(i << 2) >> 2 - 2];      /* Complex but should be 0 */
        
        ptr++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int test_to_run = -1;
    long total_result = 0;
    
    /* Parse command line to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    printf("Running auto-inc-dec coverage tests...\n");
    
    /* Run selected test or all tests */
    if (test_to_run == 1 || test_to_run == -1) {
        total_result += test_basic_post_inc();
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == 2 || test_to_run == -1) {
        total_result += test_post_dec();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == 3 || test_to_run == -1) {
        total_result += test_mixed_types();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == 4 || test_to_run == -1) {
        total_result += test_volatile_access();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == 5 || test_to_run == -1) {
        total_result += test_restrict_pointer();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == 6 || test_to_run == -1) {
        total_result += test_nested_conditional();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == 7 || test_to_run == -1) {
        total_result += test_struct_first_member();
        printf("Test 7 completed\n");
    }
    
    if (test_to_run == 8 || test_to_run == -1) {
        total_result += test_explicit_zero_cast();
        printf("Test 8 completed\n");
    }
    
    if (test_to_run == 9 || test_to_run == -1) {
        total_result += test_multiple_induction();
        printf("Test 9 completed\n");
    }
    
    if (test_to_run == 10 || test_to_run == -1) {
        total_result += test_complex_zero_expr();
        printf("Test 10 completed\n");
    }
    
    printf("Total checksum: %ld\n", total_result);
    
    /* Use result to prevent dead code elimination */
    if (total_result == 0) {
        printf("Warning: All results are zero!\n");
    }
    
    return 0;
}
