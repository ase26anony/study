/* auto-inc-dec-test.c - Test program to trigger auto-inc-dec pass coverage */

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
    
    /* Key pattern: ptr[i + 0] with post-increment */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    while (ptr < end) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += ptr[0];          /* Equivalent to *(ptr + 0) */
        ptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_simple_post_dec(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    int *start = arr - 1;
    while (ptr > start) {
        /* Multiple zero-offset patterns */
        sum += *(ptr + 0);      /* Direct pointer arithmetic with 0 */
        sum += ptr[0];          /* Array notation with implicit 0 */
        ptr--;                  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
long test3_mixed_types(void) {
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
    
    /* Char pointer loop */
    char *cptr = char_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += cptr[0];       /* QImode access */
        cptr++;
    }
    
    /* Short pointer loop */
    short *sptr = short_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += sptr[0];       /* HImode access */
        sptr++;
    }
    
    /* Int pointer loop */
    int *iptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += iptr[0];       /* SImode access */
        iptr++;
    }
    
    /* Long pointer loop */
    long *lptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += lptr[0];       /* DImode access */
        lptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    static int arr[ARRAY_SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 11;
    }
    
    /* Volatile pointer with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[i + 0];     /* Force constant 0 addition */
    }
    
    /* Another volatile pattern */
    volatile int *vptr2 = arr;
    int *end = arr + ARRAY_SIZE;
    while (vptr2 < (volatile int *)end) {
        sum += *(vptr2 + 0);    /* Pointer arithmetic with 0 */
        vptr2++;
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_pointer(void) {
    int arr[ARRAY_SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 13;
    }
    
    /* Restrict gives compiler stronger guarantees */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];         /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset */
int test6_nested_conditional(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i % 100;
    }
    
    /* Outer loop */
    for (int block = 0; block < ARRAY_SIZE; block += 64) {
        int *ptr = arr + block;
        
        /* Inner loop with conditional */
        for (int i = 0; i < 64; i++) {
            if (i % 2 == 0) {
                sum += ptr[0];          /* Zero offset in true branch */
            } else {
                sum += ptr[1];          /* Non-zero offset in false branch */
            }
            
            /* Complex update to create interesting patterns */
            if (i % 3 == 0) {
                ptr++;                  /* Sometimes increment */
            } else if (i % 3 == 1) {
                /* Do nothing */
            } else {
                ptr += 0;               /* Explicit add 0 */
            }
        }
    }
    
    return sum;
}

/* Test 7: Structure with zero offset to first member */
struct test_struct {
    int first;      /* Offset 0 */
    int second;
    int third;
};

int test7_struct_first_member(void) {
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = i * 3;
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Accessing first member has offset 0 */
        sum += sptr->first;     /* Equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit cast of zero to pointer offset */
int test8_explicit_zero_cast(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 17;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Various ways to express zero offset with casts */
        sum += ptr[(int)(0)];           /* Cast zero to int */
        sum += ptr[(unsigned int)0];    /* Cast to unsigned */
        sum += ptr[(size_t)0];          /* Cast to size_t */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple increments in same loop */
int test9_multiple_increments(void) {
    int arr1[ARRAY_SIZE];
    int arr2[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 19;
    }
    
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr1[0];         /* First pointer with zero offset */
        sum += ptr2[0];         /* Second pointer with zero offset */
        ptr1++;
        ptr2 += 1;              /* Different increment syntax */
    }
    
    return sum;
}

/* Test 10: Complex pointer arithmetic with zero */
int test10_complex_arithmetic(void) {
    int arr[ARRAY_SIZE * 3];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 3; i++) {
        arr[i] = i % 50;
    }
    
    /* Multiple pointer variables with arithmetic */
    int *base = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int *p1 = base + i;
        int *p2 = base + i + ARRAY_SIZE;
        int *p3 = base + i + ARRAY_SIZE * 2;
        
        /* All use zero offset */
        sum += p1[0] + p2[0] + p3[0];
        
        /* Complex update with zero */
        base = base + 0;        /* Explicit add 0 */
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int test_to_run = -1;
    long total_result = 0;
    
    /* Parse command line to select test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    if (test_to_run == 1 || test_to_run == -1) {
        total_result += test1_simple_post_inc();
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == 2 || test_to_run == -1) {
        total_result += test2_simple_post_dec();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == 3 || test_to_run == -1) {
        total_result += test3_mixed_types();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == 4 || test_to_run == -1) {
        total_result += test4_volatile_access();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == 5 || test_to_run == -1) {
        total_result += test5_restrict_pointer();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == 6 || test_to_run == -1) {
        total_result += test6_nested_conditional();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == 7 || test_to_run == -1) {
        total_result += test7_struct_first_member();
        printf("Test 7 completed\n");
    }
    
    if (test_to_run == 8 || test_to_run == -1) {
        total_result += test8_explicit_zero_cast();
        printf("Test 8 completed\n");
    }
    
    if (test_to_run == 9 || test_to_run == -1) {
        total_result += test9_multiple_increments();
        printf("Test 9 completed\n");
    }
    
    if (test_to_run == 10 || test_to_run == -1) {
        total_result += test10_complex_arithmetic();
        printf("Test 10 completed\n");
    }
    
    /* Print checksum to prevent optimization */
    printf("Total checksum: %ld\n", total_result);
    
    return 0;
}
