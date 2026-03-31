/* auto-inc-dec-test.c
 * Program designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Lines 1352-1358: Setting up memory instruction with constant zero offset
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
    
    /* Access with pointer + 0 in loop */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    while (ptr < end) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += *(ptr + 0);  /* This should generate plus with const_int 0 */
        ptr++;  /* Post-increment creates pattern for find_inc to match */
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
        sum += ptr[0];  /* Array index 0 creates zero offset */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types to test various modes */
int test3_mixed_types(void) {
    char carr[ARRAY_SIZE];
    short sarr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    long larr[ARRAY_SIZE];
    
    int sum = 0;
    
    /* char access - QImode */
    char *cptr = carr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        cptr[i + 0] = (char)(i & 0xFF);  /* Force + 0 offset */
        sum += cptr[0];  /* Another zero offset access */
        cptr++;
    }
    
    /* short access - HImode */
    short *sptr = sarr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sptr[0] = (short)i;  /* Zero offset */
        sum += sptr[0];
        sptr++;
    }
    
    /* int access - SImode */
    int *iptr = iarr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        iptr[(int)(0)] = i;  /* Cast zero to force pattern */
        sum += iptr[0];
        iptr++;
    }
    
    /* long access - DImode */
    long *lptr = larr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        lptr[0] = i * 100L;
        sum += (int)lptr[0];
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
        arr[i] = i * 3;
    }
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Volatile access with zero offset */
        sum += vptr[i + 0];  /* Should generate plus with const_int 0 */
        /* Simulate pointer increment in next iteration */
        vptr = vptr + 1;
    }
    
    return sum;
}

/* Test 5: Restrict pointer for aliasing guarantees */
int test5_restrict_pointer(void) {
    int arr[ARRAY_SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 4;
    }
    
    /* Restrict allows compiler to assume no aliasing */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];  /* Zero offset */
        rptr++;  /* Post-increment */
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
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < ARRAY_SIZE / 10; inner++) {
            /* Conditional access with zero offset */
            if (inner % 2 == 0) {
                sum += ptr[0];  /* Zero offset access */
            } else {
                sum += ptr[1];  /* Non-zero offset for contrast */
            }
            
            /* Complex update that find_inc needs to analyze */
            if (inner % 3 == 0) {
                ptr++;
            } else {
                ptr += 2;
            }
        }
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* At offset 0 */
    int second;
    char third;
};

int test7_struct_first_member(void) {
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)i;
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Access first member at offset 0 */
        sum += sptr->first;  /* Should be equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Multiple increments in same loop */
int test8_multiple_increments(void) {
    int arr1[ARRAY_SIZE];
    int arr2[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple zero-offset accesses with different pointers */
        sum += ptr1[0] + ptr2[0];
        
        /* Both pointers increment */
        ptr1++;
        ptr2++;
    }
    
    return sum;
}

/* Test 9: Loop with step size 2 */
int test9_step_size_2(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        sum += ptr[0];  /* Zero offset */
        ptr += 2;  /* Step size 2 */
    }
    
    return sum;
}

/* Test 10: Negative increment (decrement) */
int test10_negative_increment(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 0);  /* Zero offset with pointer arithmetic */
        ptr--;  /* Negative increment */
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Run tests based on command line arguments or all by default */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "test1") || argc > 1) {
        total_sum += test1_simple_post_inc();
    }
    
    if (run_all || strstr(argv[0], "test2") || argc > 1) {
        total_sum += test2_post_dec();
    }
    
    if (run_all || strstr(argv[0], "test3") || argc > 1) {
        total_sum += test3_mixed_types();
    }
    
    if (run_all || strstr(argv[0], "test4") || argc > 1) {
        total_sum += test4_volatile_access();
    }
    
    if (run_all || strstr(argv[0], "test5") || argc > 1) {
        total_sum += test5_restrict_pointer();
    }
    
    if (run_all || strstr(argv[0], "test6") || argc > 1) {
        total_sum += test6_nested_conditional();
    }
    
    if (run_all || strstr(argv[0], "test7") || argc > 1) {
        total_sum += test7_struct_first_member();
    }
    
    if (run_all || strstr(argv[0], "test8") || argc > 1) {
        total_sum += test8_multiple_increments();
    }
    
    if (run_all || strstr(argv[0], "test9") || argc > 1) {
        total_sum += test9_step_size_2();
    }
    
    if (run_all || strstr(argv[0], "test10") || argc > 1) {
        total_sum += test10_negative_increment();
    }
    
    /* Print checksum to prevent optimization from removing computations */
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
