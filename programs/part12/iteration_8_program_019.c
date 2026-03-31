/* auto-inc-dec-test.c
 * Program designed to trigger auto-inc-dec pass coverage for lines 1352-1358
 * Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c auto-inc-dec-test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test1_simple_post_inc(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    while (ptr < end) {
        /* Access with explicit zero offset */
        sum += ptr[0];  /* Should generate plus (reg) (const_int 0) */
        ptr++;  /* Post-increment for find_inc to match */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_post_dec(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    int *start = arr;
    
    while (ptr >= start) {
        /* Multiple zero-offset access patterns */
        sum += *(ptr + 0);  /* Explicit pointer arithmetic with 0 */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types for different memory modes */
void test3_mixed_types(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    volatile char *cptr = char_arr;
    volatile short *sptr = short_arr;
    volatile int *iptr = int_arr;
    volatile long *lptr = long_arr;
    
    /* Char access - QImode */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        cptr[i + 0] = (char)(i & 0xFF);  /* Force plus with const_int 0 */
    }
    
    /* Short access - HImode */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sptr[(int)(0) + i] = (short)i;  /* Cast zero to force pattern */
    }
    
    /* Int access - SImode */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        iptr[0 + i] = i;  /* Simple zero addition */
    }
    
    /* Long access - DImode */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        lptr[i] = i;  /* Implicit zero offset */
    }
}

/* Test 4: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* Offset 0 */
    int second;
    char third;
};

int test4_struct_first_member(void) {
    struct test_struct arr[100];
    int sum = 0;
    
    for (int i = 0; i < 100; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)i;
    }
    
    struct test_struct *ptr = arr;
    struct test_struct *end = arr + 100;
    
    while (ptr < end) {
        /* Access first member (offset 0) */
        sum += ptr->first;  /* Should generate address with plus 0 */
        ptr++;
    }
    
    return sum;
}

/* Test 5: Nested loops with conditional access */
int test5_nested_conditional(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    for (int outer = 0; outer < 10; outer++) {
        int *inner_ptr = ptr;
        
        while (inner_ptr < end) {
            /* Conditional access with zero offset */
            if (*inner_ptr % 2 == 0) {
                sum += inner_ptr[0];  /* Zero offset access */
            } else {
                sum -= *(inner_ptr + 0);  /* Another zero offset pattern */
            }
            
            inner_ptr++;
        }
        
        /* Modify pointer for next iteration */
        ptr += 10;
    }
    
    return sum;
}

/* Test 6: Restrict pointers for alias analysis */
int test6_restrict_pointers(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *restrict rptr = arr;
    int *restrict rend = arr + ARRAY_SIZE;
    
    while (rptr < rend) {
        /* Multiple zero-offset accesses */
        int val1 = rptr[0];
        int val2 = *(rptr + 0);
        sum += val1 + val2;
        rptr++;
    }
    
    return sum;
}

/* Test 7: Different step sizes */
int test7_variable_steps(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i;
    }
    
    /* Step size 1 */
    int *ptr1 = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr1[0];
        ptr1 += 1;
    }
    
    /* Step size 2 */
    int *ptr2 = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr2[0];
        ptr2 += 2;
    }
    
    /* Step size 4 */
    int *ptr4 = arr;
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        sum += *(ptr4 + 0);
        ptr4 += 4;
    }
    
    return sum;
}

/* Test 8: Complex pointer arithmetic with zero */
int test8_complex_zero_arithmetic(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Multiple ways to express zero offset */
    int *ptr = arr;
    
    /* Method 1: Array index with zero */
    sum += ptr[0];
    
    /* Method 2: Pointer addition with zero */
    sum += *(ptr + 0);
    
    /* Method 3: Pointer addition with computed zero */
    int zero = 0;
    sum += *(ptr + zero);
    
    /* Method 4: Cast zero to ptrdiff_t */
    sum += ptr[(ptrdiff_t)0];
    
    /* Loop with mixed patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (i % 3 == 0) {
            sum += ptr[0];
        } else if (i % 3 == 1) {
            sum += *(ptr + 0);
        } else {
            sum += ptr[(int)(0)];
        }
        ptr++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Run tests based on command line arguments */
    if (argc == 1) {
        /* Run all tests if no arguments */
        total_sum += test1_simple_post_inc();
        total_sum += test2_post_dec();
        test3_mixed_types();
        total_sum += test4_struct_first_member();
        total_sum += test5_nested_conditional();
        total_sum += test6_restrict_pointers();
        total_sum += test7_variable_steps();
        total_sum += test8_complex_zero_arithmetic();
    } else {
        /* Run specific tests based on arguments */
        for (int i = 1; i < argc; i++) {
            int test_num = atoi(argv[i]);
            switch (test_num) {
                case 1: total_sum += test1_simple_post_inc(); break;
                case 2: total_sum += test2_post_dec(); break;
                case 3: test3_mixed_types(); break;
                case 4: total_sum += test4_struct_first_member(); break;
                case 5: total_sum += test5_nested_conditional(); break;
                case 6: total_sum += test6_restrict_pointers(); break;
                case 7: total_sum += test7_variable_steps(); break;
                case 8: total_sum += test8_complex_zero_arithmetic(); break;
                default: printf("Unknown test: %d\n", test_num);
            }
        }
    }
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
