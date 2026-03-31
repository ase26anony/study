/* auto-inc-dec-test.c
 * Test program to trigger auto-inc-dec pass coverage for lines 1352-1358
 * Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c auto-inc-dec-test.c
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
    
    /* Access with ptr + 0 pattern */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    while (ptr < end) {
        sum += *(ptr + 0);  /* Zero offset addition */
        ptr++;              /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with array[0] access */
int test2_post_decrement_zero_index(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    
    /* Access with ptr[0] which is *(ptr + 0) */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[0];      /* Equivalent to *(ptr + 0) */
        ptr--;              /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types to test various modes */
void test3_mixed_data_types(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = i & 0xFF;
        short_arr[i] = i * 2;
        int_arr[i] = i * 3;
        long_arr[i] = i * 4L;
    }
    
    /* Char access with zero offset */
    char *cptr = char_arr;
    char *cend = char_arr + ARRAY_SIZE;
    int char_sum = 0;
    while (cptr < cend) {
        char_sum += *(cptr + 0);  /* QImode access */
        cptr++;
    }
    
    /* Short access with zero offset */
    short *sptr = short_arr;
    short *send = short_arr + ARRAY_SIZE;
    int short_sum = 0;
    while (sptr < send) {
        short_sum += sptr[0];     /* HImode access */
        sptr++;
    }
    
    /* Int access with zero offset */
    int *iptr = int_arr;
    int *iend = int_arr + ARRAY_SIZE;
    int int_sum = 0;
    while (iptr < iend) {
        int_sum += iptr[0];       /* SImode access */
        iptr++;
    }
    
    /* Long access with zero offset */
    long *lptr = long_arr;
    long *lend = long_arr + ARRAY_SIZE;
    long long_sum = 0;
    while (lptr < lend) {
        long_sum += *(lptr + 0);  /* DImode access */
        lptr++;
    }
    
    printf("Mixed types sums: char=%d, short=%d, int=%d, long=%ld\n",
           char_sum, short_sum, int_sum, long_sum);
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    volatile int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i + 1;
    }
    
    volatile int *vptr = arr;
    volatile int *vend = arr + ARRAY_SIZE;
    
    /* Volatile access with explicit zero offset */
    while (vptr < vend) {
        sum += vptr[0];     /* Should still generate (plus (reg) (const_int 0)) */
        vptr++;
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 3;
    }
    
    int *restrict rptr = arr;
    int *restrict rend = arr + ARRAY_SIZE;
    
    /* Restrict allows better optimization */
    while (rptr < rend) {
        sum += *(rptr + 0);  /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset access */
int test6_nested_conditional(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    
    /* Nested loop structure */
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < ARRAY_SIZE/10; inner++) {
            /* Conditional access with zero offset */
            if (inner % 2 == 0) {
                sum += ptr[0];          /* Zero offset access */
            } else {
                sum += *(ptr + 0) * 2;  /* Another zero offset pattern */
            }
            ptr++;
        }
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;    /* At offset 0 */
    int second;
    char third;
};

int test7_struct_first_member(void) {
    struct test_struct arr[100];
    int sum = 0;
    
    for (int i = 0; i < 100; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = i & 0xFF;
    }
    
    struct test_struct *sptr = arr;
    struct test_struct *send = arr + 100;
    
    /* Access first member (offset 0) */
    while (sptr < send) {
        sum += sptr->first;  /* Accesses at offset 0 */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit cast of zero to pointer offset */
int test8_explicit_zero_cast(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 5;
    }
    
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    /* Explicit cast of zero to ptrdiff_t */
    while (ptr < end) {
        sum += ptr[(ptrdiff_t)0];  /* Explicit zero cast */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple increments in loop */
int test9_multiple_increments(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *ptr1 = arr;
    int *ptr2 = arr + ARRAY_SIZE;
    
    /* Two pointers with zero offset accesses */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr1[0] + ptr2[0];  /* Two zero offset accesses */
        ptr1++;
        ptr2++;
    }
    
    return sum;
}

/* Test 10: Loop with step size 2 */
int test10_step_size_2(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    /* Step by 2, still accessing with zero offset */
    while (ptr < end) {
        sum += *(ptr + 0);  /* Zero offset */
        ptr += 2;           /* Step size 2 */
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Run all tests to ensure all code paths are compiled */
    total_sum += test1_post_increment_zero_offset();
    total_sum += test2_post_decrement_zero_index();
    test3_mixed_data_types();
    total_sum += test4_volatile_zero_offset();
    total_sum += test5_restrict_zero_offset();
    total_sum += test6_nested_conditional();
    total_sum += test7_struct_first_member();
    total_sum += test8_explicit_zero_cast();
    total_sum += test9_multiple_increments();
    total_sum += test10_step_size_2();
    
    /* Use argc to prevent dead code elimination */
    if (argc > 1) {
        printf("Total checksum: %d\n", total_sum);
    }
    
    return 0;
}
