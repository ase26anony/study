/* auto_inc_dec_test.c
 * Designed to trigger GCC's auto-increment/decrement optimization pass
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Basic pointer arithmetic with zero offset in loops */
int test1_basic_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Pattern: ptr[i + 0] with post-increment */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    while (ptr < end) {
        /* This should generate (mem (plus (reg) (const_int 0))) */
        sum += ptr[0];          /* Zero offset access */
        ptr++;                  /* Post-increment */
    }
    
    /* Another variant with explicit + 0 */
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 0);      /* *(ptr + 0) pattern */
        ptr++;
    }
    
    return sum;
}

/* Test 2: Different data types for mixed-size memory operations */
int test2_mixed_data_types(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i % 65536);
        int_arr[i] = i;
        long_arr[i] = i * 2L;
    }
    
    /* Char access with zero offset */
    char *cptr = char_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += cptr[0];         /* QImode access */
        cptr++;
    }
    
    /* Short access with zero offset */
    short *sptr = short_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr[0];         /* HImode access */
        sptr++;
    }
    
    /* Int access with zero offset */
    int *iptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += iptr[0];         /* SImode access */
        iptr++;
    }
    
    /* Long access with zero offset */
    long *lptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += (int)lptr[0];    /* DImode access */
        lptr++;
    }
    
    return sum;
}

/* Test 3: Volatile pointers with zero offset */
int test3_volatile_access(void) {
    int arr[ARRAY_SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 3;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[i + 0];     /* Volatile + zero offset */
    }
    
    /* Another volatile pattern */
    vptr = arr;
    int *end = arr + ARRAY_SIZE;
    while (vptr < end) {
        sum += *(vptr + 0);     /* *(volatile_ptr + 0) */
        vptr++;
    }
    
    return sum;
}

/* Test 4: Restrict pointers for aliasing guarantees */
int test4_restrict_pointers(void) {
    int arr[ARRAY_SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 5;
    }
    
    /* Restrict pointer with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];         /* restrict + zero offset */
        rptr++;
    }
    
    return sum;
}

/* Test 5: Nested and conditional access patterns */
int test5_complex_control_flow(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i % 100;
    }
    
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE * 2;
    
    /* Complex loop with conditional zero-offset access */
    while (ptr < end) {
        if ((ptr - arr) % 3 == 0) {
            /* Conditional zero-offset access */
            sum += ptr[0];      /* Zero offset in if branch */
            ptr++;
        } else if ((ptr - arr) % 3 == 1) {
            /* Different offset in else-if */
            sum += ptr[1];
            ptr += 2;
        } else {
            /* Another zero-offset access */
            sum += *(ptr + 0);  /* *(ptr + 0) pattern */
            ptr += 3;
        }
    }
    
    /* Nested loops with zero offset */
    for (int i = 0; i < 10; i++) {
        ptr = arr + i * 100;
        for (int j = 0; j < 100; j++) {
            if (j % 2 == 0) {
                sum += ptr[0];  /* Zero offset in nested loop */
            }
            ptr++;
        }
    }
    
    return sum;
}

/* Test 6: Structure access with zero byte offset */
struct test_struct {
    int first;      /* Offset 0 */
    int second;
    char third;
};

int test6_struct_zero_offset(void) {
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)(i % 256);
    }
    
    /* Access first member (offset 0) */
    struct test_struct *sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr->first;     /* Structure field at offset 0 */
        sptr++;
    }
    
    /* Alternative: cast to char* and add 0 */
    char *cptr = (char *)arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *((int *)(cptr + 0));  /* Explicit zero offset */
        cptr += sizeof(struct test_struct);
    }
    
    return sum;
}

/* Test 7: Post-decrement patterns */
int test7_post_decrement(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 7;
    }
    
    /* Post-decrement with zero offset */
    int *ptr = arr + ARRAY_SIZE - 1;
    while (ptr >= arr) {
        sum += ptr[0];          /* Zero offset with decrement */
        ptr--;
    }
    
    /* Another decrement pattern */
    ptr = arr + ARRAY_SIZE;
    do {
        ptr--;
        sum += *(ptr + 0);      /* *(ptr + 0) with decrement */
    } while (ptr > arr);
    
    return sum;
}

/* Test 8: Cast zero to pointer offset type */
int test8_explicit_zero_cast(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 11;
    }
    
    /* Explicit cast of zero to ptrdiff_t */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[(ptrdiff_t)0];  /* Explicit zero cast */
        ptr++;
    }
    
    /* Cast zero to int */
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[(int)(0)];      /* Cast integer zero */
        ptr++;
    }
    
    /* Cast zero to size_t */
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[(size_t)0];     /* Cast size_t zero */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple induction variables */
int test9_multiple_induction(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i % 50;
    }
    
    /* Loop with multiple pointers */
    int *ptr1 = arr;
    int *ptr2 = arr + ARRAY_SIZE;
    int *end1 = arr + ARRAY_SIZE;
    int *end2 = arr + ARRAY_SIZE * 2;
    
    while (ptr1 < end1 && ptr2 < end2) {
        sum += ptr1[0];         /* Zero offset for ptr1 */
        sum += ptr2[0];         /* Zero offset for ptr2 */
        ptr1 += 1;              /* Step 1 */
        ptr2 += 2;              /* Step 2 */
    }
    
    return sum;
}

/* Test 10: Combined patterns */
int test10_combined_patterns(void) {
    volatile int varray[ARRAY_SIZE];
    int *restrict rptr = (int *)varray;
    int sum = 0;
    
    /* Initialize volatile array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        varray[i] = i * 13;
    }
    
    /* Complex pattern: volatile base with restrict pointer access */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (i % 4 == 0) {
            sum += rptr[0];     /* Zero offset with restrict */
        } else if (i % 4 == 1) {
            sum += varray[i + 0]; /* Volatile with zero offset */
        } else if (i % 4 == 2) {
            sum += *(rptr + 0); /* *(restrict_ptr + 0) */
        } else {
            sum += *((int *)((char *)rptr + 0)); /* Cast chain */
        }
        rptr++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    int test_to_run = -1;
    
    /* Parse command line argument to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    if (test_to_run == 1 || test_to_run == -1) {
        total_sum += test1_basic_zero_offset();
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == 2 || test_to_run == -1) {
        total_sum += test2_mixed_data_types();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == 3 || test_to_run == -1) {
        total_sum += test3_volatile_access();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == 4 || test_to_run == -1) {
        total_sum += test4_restrict_pointers();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == 5 || test_to_run == -1) {
        total_sum += test5_complex_control_flow();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == 6 || test_to_run == -1) {
        total_sum += test6_struct_zero_offset();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == 7 || test_to_run == -1) {
        total_sum += test7_post_decrement();
        printf("Test 7 completed\n");
    }
    
    if (test_to_run == 8 || test_to_run == -1) {
        total_sum += test8_explicit_zero_cast();
        printf("Test 8 completed\n");
    }
    
    if (test_to_run == 9 || test_to_run == -1) {
        total_sum += test9_multiple_induction();
        printf("Test 9 completed\n");
    }
    
    if (test_to_run == 10 || test_to_run == -1) {
        total_sum += test10_combined_patterns();
        printf("Test 10 completed\n");
    }
    
    /* Print checksum to prevent optimization removal */
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
