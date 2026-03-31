/* auto-inc-dec-test.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Lines 1352-1358: mem_insn setup with reg1_is_const=true, reg1_val=0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Basic post-increment with zero offset in array access */
int test1_basic_postinc(void) {
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
        sum += ptr[0];          /* Zero offset array access */
        sum += *(ptr + 0);      /* Alternative zero offset */
        ptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_postdec(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    int *start = arr;
    while (ptr >= start) {
        sum += ptr[0];          /* Zero offset */
        ptr--;                  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types to test various modes */
int test3_mixed_types(void) {
    char c_arr[ARRAY_SIZE];
    short s_arr[ARRAY_SIZE];
    int i_arr[ARRAY_SIZE];
    long long ll_arr[ARRAY_SIZE];
    
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        c_arr[i] = (char)(i % 256);
        s_arr[i] = (short)(i * 3);
        i_arr[i] = i * 5;
        ll_arr[i] = i * 7LL;
    }
    
    /* Char access with zero offset */
    char *c_ptr = c_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += c_ptr[0];        /* QImode access */
        c_ptr++;
    }
    
    /* Short access with zero offset */
    short *s_ptr = s_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += s_ptr[0];        /* HImode access */
        s_ptr++;
    }
    
    /* Int access with zero offset */
    int *i_ptr = i_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += i_ptr[0];        /* SImode access */
        i_ptr++;
    }
    
    /* Long long access with zero offset */
    long long *ll_ptr = ll_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += (int)ll_ptr[0];  /* DImode access */
        ll_ptr++;
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
        sum += vptr[i + 0];     /* Force (plus (reg) (const_int 0)) */
    }
    
    /* Alternative volatile pattern */
    volatile int *vptr2 = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(vptr2 + 0);    /* Another zero offset pattern */
        vptr2++;
    }
    
    return sum;
}

/* Test 5: Restrict qualifier for alias analysis */
int test5_restrict_ptr(void) {
    int arr[ARRAY_SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 13;
    }
    
    /* Restrict pointer with zero offset in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];         /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops and conditional access */
int test6_complex_control_flow(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 17;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE / 4; i++) {
        /* Outer loop */
        for (int j = 0; j < 4; j++) {
            /* Inner loop with conditional */
            if ((i + j) % 2 == 0) {
                sum += ptr[0];  /* Zero offset in conditional */
            } else {
                sum += ptr[0] * 2; /* Same zero offset, different use */
            }
        }
        ptr++;  /* Post-increment */
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
        arr[i].first = i * 19;
        arr[i].second = i * 23;
        arr[i].third = (char)(i % 128);
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Access first member (offset 0) */
        sum += sptr->first;     /* Equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit zero cast as index */
int test8_explicit_zero_cast(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 29;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Explicit cast of 0 to different types as index */
        sum += ptr[(int)0];             /* Cast to int */
        sum += ptr[(unsigned int)0];    /* Cast to unsigned */
        sum += ptr[(size_t)0];          /* Cast to size_t */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple increments with zero offset */
int test9_multiple_increments(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i * 31;
    }
    
    /* Multiple pointers with different step sizes */
    int *ptr1 = arr;
    int *ptr2 = arr;
    int *ptr3 = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr1[0];     /* Step 1 */
        ptr1++;
        
        sum += ptr2[0];     /* Step 2 */
        ptr2 += 2;
        
        sum += ptr3[0];     /* Step -1 (backwards) */
        ptr3--;
        ptr3 += 2;          /* Net +1 */
    }
    
    return sum;
}

/* Test 10: Combined patterns */
int test10_combined_patterns(void) {
    int arr1[ARRAY_SIZE];
    int arr2[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i * 37;
        arr2[i] = i * 41;
    }
    
    /* Mix different patterns */
    int *p1 = arr1;
    int *p2 = arr2;
    
    /* Forward loop */
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        sum += p1[0] + p2[0];
        p1++;
        p2++;
    }
    
    /* Backward loop */
    p1 = arr1 + ARRAY_SIZE / 2 - 1;
    p2 = arr2 + ARRAY_SIZE / 2 - 1;
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        sum += p1[0] - p2[0];
        p1--;
        p2--;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    int test_to_run = -1;
    
    /* Parse command line to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run all tests or specific test */
    if (test_to_run == -1 || test_to_run == 1) {
        total_sum += test1_basic_postinc();
    }
    if (test_to_run == -1 || test_to_run == 2) {
        total_sum += test2_postdec();
    }
    if (test_to_run == -1 || test_to_run == 3) {
        total_sum += test3_mixed_types();
    }
    if (test_to_run == -1 || test_to_run == 4) {
        total_sum += test4_volatile_access();
    }
    if (test_to_run == -1 || test_to_run == 5) {
        total_sum += test5_restrict_ptr();
    }
    if (test_to_run == -1 || test_to_run == 6) {
        total_sum += test6_complex_control_flow();
    }
    if (test_to_run == -1 || test_to_run == 7) {
        total_sum += test7_struct_first_member();
    }
    if (test_to_run == -1 || test_to_run == 8) {
        total_sum += test8_explicit_zero_cast();
    }
    if (test_to_run == -1 || test_to_run == 9) {
        total_sum += test9_multiple_increments();
    }
    if (test_to_run == -1 || test_to_run == 10) {
        total_sum += test10_combined_patterns();
    }
    
    /* Print result to prevent optimization removal */
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
