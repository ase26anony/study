/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for (plus (reg) (const_int 0)) patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define ITERS 100

/* Test 1: Simple post-increment with zero offset */
int test1_post_increment(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Loop with pointer arithmetic using +0 */
    int *ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += *(ptr + 0);
        ptr++;  /* Post-increment for find_inc to match */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_post_decrement(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + SIZE - 1;
    int *start = arr;
    while (ptr >= start) {
        /* Multiple zero-offset accesses to increase chances */
        sum += ptr[0];          /* Array notation with zero */
        sum += *(ptr + 0);      /* Pointer arithmetic with zero */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
int test3_mixed_types(void) {
    char c_arr[SIZE];
    short s_arr[SIZE];
    int i_arr[SIZE];
    long long ll_arr[SIZE];
    
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        c_arr[i] = (char)(i % 256);
        s_arr[i] = (short)(i * 3);
        i_arr[i] = i * 5;
        ll_arr[i] = i * 7LL;
    }
    
    /* Char loop */
    char *c_ptr = c_arr;
    for (int i = 0; i < SIZE; i++) {
        sum += c_ptr[0];    /* QImode access */
        c_ptr++;
    }
    
    /* Short loop */
    short *s_ptr = s_arr;
    for (int i = 0; i < SIZE; i++) {
        sum += s_ptr[0];    /* HImode access */
        s_ptr++;
    }
    
    /* Int loop */
    int *i_ptr = i_arr;
    for (int i = 0; i < SIZE; i++) {
        sum += i_ptr[0];    /* SImode access */
        i_ptr++;
    }
    
    /* Long long loop */
    long long *ll_ptr = ll_arr;
    for (int i = 0; i < SIZE; i++) {
        sum += (int)ll_ptr[0];  /* DImode access */
        ll_ptr++;
    }
    
    return sum;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 11;
    }
    
    /* Volatile pointer - may create different RTL patterns */
    volatile int *vptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Force zero offset with volatile */
        sum += vptr[i + 0];
        /* Explicit pointer increment */
        vptr = vptr + 1;
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_pointer(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 13;
    }
    
    /* Restrict gives stronger aliasing guarantees */
    int *restrict rptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Multiple zero-offset patterns */
        int val = rptr[0];
        val += *(rptr + 0);
        sum += val;
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero-offset access */
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
    for (int i = 0; i < SIZE; i++) {
        int *row_ptr = arr[i];
        for (int j = 0; j < 4; j++) {
            /* Conditional zero-offset access */
            if (j % 2 == 0) {
                sum += row_ptr[0];      /* Access first element with zero offset */
            } else {
                sum += row_ptr[j];
            }
        }
        /* Pointer increment in outer loop */
        /* This creates complex patterns for find_inc to analyze */
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
    struct test_struct arr[SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i * 17;
        arr[i].second = i * 19;
        arr[i].third = (char)(i % 128);
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Access first member - always at offset 0 */
        sum += sptr->first;     /* Equivalent to sptr[0].first */
        sum += sptr[0].first;   /* Explicit zero offset */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Complex pointer arithmetic with zero constant */
int test8_complex_arithmetic(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 23;
    }
    
    /* Multiple pointer variables with zero offsets */
    int *p1 = arr;
    int *p2 = arr + SIZE/2;
    
    for (int i = 0; i < SIZE/2; i++) {
        /* Force (plus (reg) (const_int 0)) in different ways */
        sum += p1[(int)(0)];            /* Cast zero to int */
        sum += *(p2 + (0));             /* Parenthesized zero */
        sum += 0[p1];                   /* Symmetric array access (also offset 0) */
        
        p1++;
        p2++;
    }
    
    return sum;
}

/* Test 9: Loop with step size 2 */
int test9_step_size_2(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 29;
    }
    
    int *ptr = arr;
    for (int i = 0; i < SIZE; i += 2) {
        sum += ptr[0];
        ptr += 2;  /* Step size 2 for find_inc matching */
    }
    
    return sum;
}

/* Test 10: Negative step (decrement by 2) */
int test10_negative_step(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 31;
    }
    
    int *ptr = arr + SIZE - 1;
    for (int i = 0; i < SIZE; i += 2) {
        sum += ptr[0];
        ptr -= 2;  /* Negative step size */
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Run tests based on command line arguments */
    if (argc == 1) {
        /* Run all tests if no arguments */
        total_sum += test1_post_increment();
        total_sum += test2_post_decrement();
        total_sum += test3_mixed_types();
        total_sum += test4_volatile_access();
        total_sum += test5_restrict_pointer();
        total_sum += test6_nested_conditional();
        total_sum += test7_struct_first_member();
        total_sum += test8_complex_arithmetic();
        total_sum += test9_step_size_2();
        total_sum += test10_negative_step();
    } else {
        /* Run specific tests based on arguments */
        for (int i = 1; i < argc; i++) {
            int test_num = atoi(argv[i]);
            switch (test_num) {
                case 1: total_sum += test1_post_increment(); break;
                case 2: total_sum += test2_post_decrement(); break;
                case 3: total_sum += test3_mixed_types(); break;
                case 4: total_sum += test4_volatile_access(); break;
                case 5: total_sum += test5_restrict_pointer(); break;
                case 6: total_sum += test6_nested_conditional(); break;
                case 7: total_sum += test7_struct_first_member(); break;
                case 8: total_sum += test8_complex_arithmetic(); break;
                case 9: total_sum += test9_step_size_2(); break;
                case 10: total_sum += test10_negative_step(); break;
                default: break;
            }
        }
    }
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
