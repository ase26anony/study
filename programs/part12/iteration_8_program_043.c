/* auto-inc-dec-test.c
 * Program designed to trigger GCC's auto-increment/decrement optimization
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
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
    
    /* Key pattern: ptr + 0 in loop with post-increment */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    while (ptr < end) {
        /* This generates (mem (plus (reg) (const_int 0))) */
        sum += *(ptr + 0);  /* Zero offset addition */
        ptr++;  /* Post-increment */
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
        sum += ptr[0];      /* Array notation with zero index */
        sum += *(ptr + 0);  /* Pointer arithmetic with zero */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types for different memory modes */
int test3_mixed_data_types(void) {
    char char_arr[ARRAY_SIZE];      /* QImode */
    short short_arr[ARRAY_SIZE];    /* HImode */
    int int_arr[ARRAY_SIZE];        /* SImode */
    long long_arr[ARRAY_SIZE];      /* DImode */
    
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i * 3);
        int_arr[i] = i * 5;
        long_arr[i] = i * 7L;
    }
    
    /* Char pointer loop */
    char *cptr = char_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += cptr[0];  /* Zero index access */
        cptr++;
    }
    
    /* Short pointer loop */
    short *sptr = short_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr[0];
        sptr++;
    }
    
    /* Int pointer loop */
    int *iptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += iptr[0];
        iptr++;
    }
    
    /* Long pointer loop */
    long *lptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += (int)lptr[0];
        lptr++;
    }
    
    return sum;
}

/* Test 4: Volatile pointers with zero offset */
int test4_volatile_pointers(void) {
    int arr[ARRAY_SIZE];
    volatile int *vptr = arr;  /* Volatile pointer */
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 11;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Cast zero to ensure (const_int 0) in RTL */
        sum += vptr[i + (int)(0)];  /* Force zero offset */
    }
    
    /* Another volatile pattern */
    volatile int *vptr2 = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(vptr2 + 0);  /* Pointer + 0 */
        vptr2++;
    }
    
    return sum;
}

/* Test 5: Restrict pointers for alias analysis */
int test5_restrict_pointers(void) {
    int arr[ARRAY_SIZE];
    int *restrict rptr = arr;  /* Restrict gives alias guarantee */
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 13;
    }
    
    /* Restrict allows more aggressive optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];  /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops and conditional access */
int test6_nested_conditional(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 17;
    }
    
    int *ptr = arr;
    
    /* Nested loop structure */
    for (int outer = 0; outer < 4; outer++) {
        for (int inner = 0; inner < ARRAY_SIZE/4; inner++) {
            /* Conditional access with zero offset */
            if (inner % 2 == 0) {
                sum += ptr[0];  /* Even indices */
            } else {
                sum += ptr[0] * 2;  /* Odd indices, same zero offset */
            }
            ptr++;
        }
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first_field;  /* At offset 0 */
    int second_field;
    char third_field;
};

int test7_structure_first_member(void) {
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first_field = i * 19;
        arr[i].second_field = i * 23;
        arr[i].third_field = (char)(i % 128);
    }
    
    struct test_struct *sptr = arr;
    
    /* Access first member (offset 0) */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr->first_field;  /* This is at offset 0 */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Complex pointer arithmetic with zero */
int test8_complex_zero_arithmetic(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i * 29;
    }
    
    /* Multiple pointer variables with zero offsets */
    int *ptr1 = arr;
    int *ptr2 = arr + ARRAY_SIZE;
    
    while (ptr1 < ptr2) {
        /* Complex expression that simplifies to ptr1 + 0 */
        int offset = 0;
        sum += ptr1[offset];  /* Variable that's always 0 */
        
        /* Another zero offset pattern */
        sum += *(ptr1 + (0));  /* Parenthesized zero */
        
        ptr1++;
    }
    
    return sum;
}

/* Test 9: Loop with step size 2 */
int test9_step_size_two(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 31;
    }
    
    int *ptr = arr;
    
    /* Step by 2 elements */
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += ptr[0];  /* Zero offset */
        sum += (ptr + 1)[0];  /* Offset 1, but still [0] on new pointer */
        ptr += 2;
    }
    
    return sum;
}

/* Test 10: All patterns combined */
int test10_combined_patterns(void) {
    int arr[ARRAY_SIZE];
    volatile int *vptr = arr;
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 37;
    }
    
    /* Mix of different patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Volatile with zero offset */
        sum += vptr[i + 0];
        
        /* Restrict with zero offset */
        sum += rptr[0];
        
        /* Direct array access with computed zero */
        int zero = 0;
        sum += arr[i + zero];
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Run tests based on command line arguments */
    if (argc == 1) {
        /* Run all tests if no arguments */
        total_sum += test1_post_increment_zero_offset();
        total_sum += test2_post_decrement_zero_offset();
        total_sum += test3_mixed_data_types();
        total_sum += test4_volatile_pointers();
        total_sum += test5_restrict_pointers();
        total_sum += test6_nested_conditional();
        total_sum += test7_structure_first_member();
        total_sum += test8_complex_zero_arithmetic();
        total_sum += test9_step_size_two();
        total_sum += test10_combined_patterns();
    } else {
        /* Run specific tests based on arguments */
        for (int i = 1; i < argc; i++) {
            int test_num = atoi(argv[i]);
            switch (test_num) {
                case 1: total_sum += test1_post_increment_zero_offset(); break;
                case 2: total_sum += test2_post_decrement_zero_offset(); break;
                case 3: total_sum += test3_mixed_data_types(); break;
                case 4: total_sum += test4_volatile_pointers(); break;
                case 5: total_sum += test5_restrict_pointers(); break;
                case 6: total_sum += test6_nested_conditional(); break;
                case 7: total_sum += test7_structure_first_member(); break;
                case 8: total_sum += test8_complex_zero_arithmetic(); break;
                case 9: total_sum += test9_step_size_two(); break;
                case 10: total_sum += test10_combined_patterns(); break;
                default: printf("Unknown test: %d\n", test_num);
            }
        }
    }
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
