/* Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define ITERS 100

/* Test 1: Simple post-increment with zero offset */
int test1_post_increment_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Key pattern: ptr + 0 in loop with post-increment */
    int *ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += *(ptr + 0);  /* This should generate reg + 0 */
        ptr++;  /* Post-increment for find_inc to match */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_post_decrement_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + SIZE - 1;
    int *start = arr;
    while (ptr >= start) {
        /* Multiple zero-offset accesses to increase chances */
        sum += ptr[0];      /* Array notation with zero */
        sum += *(ptr + 0);  /* Pointer arithmetic with zero */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types for different memory modes */
long test3_mixed_data_types(void) {
    char carr[SIZE];
    short sarr[SIZE];
    int iarr[SIZE];
    long larr[SIZE];
    long total = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        carr[i] = i & 0xFF;
        sarr[i] = i * 2;
        iarr[i] = i * 3;
        larr[i] = i * 4L;
    }
    
    /* Char access - QImode */
    char *cptr = carr;
    for (int i = 0; i < SIZE; i++) {
        total += cptr[0];  /* Zero offset */
        cptr++;
    }
    
    /* Short access - HImode */
    short *sptr = sarr;
    for (int i = 0; i < SIZE; i++) {
        total += sptr[0];  /* Zero offset */
        sptr++;
    }
    
    /* Int access - SImode */
    int *iptr = iarr;
    for (int i = 0; i < SIZE; i++) {
        total += iptr[0];  /* Zero offset */
        iptr++;
    }
    
    /* Long access - DImode */
    long *lptr = larr;
    for (int i = 0; i < SIZE; i++) {
        total += lptr[0];  /* Zero offset */
        lptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    int arr[SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 5;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < SIZE; i++) {
        /* Force generation of (mem (plus (reg) (const_int 0))) */
        sum += vptr[0];  /* Zero offset access */
        /* Cast to non-volatile to allow pointer arithmetic */
        int *nptr = (int *)vptr;
        nptr++;
        vptr = nptr;
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_zero_offset(void) {
    int arr[SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 7;
    }
    
    /* Restrict gives compiler stronger guarantees */
    for (int i = 0; i < SIZE; i++) {
        /* Multiple zero-offset patterns */
        sum += rptr[0];
        sum += *(rptr + 0);
        sum += *(0 + rptr);  /* Commutative form */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset */
int test6_nested_conditional(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < SIZE/10; inner++) {
            /* Conditional zero-offset access */
            if (inner % 2 == 0) {
                sum += ptr[0];  /* Zero offset */
            } else {
                sum += ptr[1];  /* Non-zero offset for contrast */
            }
            ptr++;
        }
    }
    
    return sum;
}

/* Test 7: Structure with first field at offset 0 */
struct test_struct {
    int first;  /* At offset 0 */
    int second;
    char third;
};

int test7_struct_first_field(void) {
    struct test_struct arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = i & 0xFF;
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Access first field - at offset 0 */
        sum += sptr->first;  /* This compiles to base + 0 */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit zero cast as index */
int test8_explicit_zero_cast(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 11;
    }
    
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Force constant zero through cast */
        sum += ptr[(int)(0)];  /* Explicit zero cast */
        sum += ptr[0];         /* Implicit zero */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple induction variables */
int test9_multiple_induction(void) {
    int arr[SIZE * 3];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 3; i++) {
        arr[i] = i;
    }
    
    /* Multiple pointers with different step sizes */
    int *ptr1 = arr;
    int *ptr2 = arr + SIZE;
    int *ptr3 = arr + SIZE * 2;
    
    for (int i = 0; i < SIZE; i++) {
        /* All using zero offset */
        sum += ptr1[0];
        sum += ptr2[0];
        sum += ptr3[0];
        
        ptr1 += 1;  /* Step 1 */
        ptr2 += 2;  /* Step 2 */
        ptr3 -= 1;  /* Step -1 (decrement) */
    }
    
    return sum;
}

/* Test 10: Complex expression with zero addition */
int test10_complex_zero_expr(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 13;
    }
    
    int *ptr = arr;
    int index = 0;
    for (int i = 0; i < SIZE; i++) {
        /* Complex expression that simplifies to ptr + 0 */
        sum += ptr[index * 0];      /* Always zero offset */
        sum += ptr[0 + (i - i)];    /* Always zero offset */
        sum += ptr[(i & ~i)];       /* Always zero offset */
        
        ptr++;
        index = (index + 1) % 4;
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
        total_result += test1_post_increment_zero_offset();
    }
    if (test_to_run == 2 || test_to_run == -1) {
        total_result += test2_post_decrement_zero_offset();
    }
    if (test_to_run == 3 || test_to_run == -1) {
        total_result += test3_mixed_data_types();
    }
    if (test_to_run == 4 || test_to_run == -1) {
        total_result += test4_volatile_zero_offset();
    }
    if (test_to_run == 5 || test_to_run == -1) {
        total_result += test5_restrict_zero_offset();
    }
    if (test_to_run == 6 || test_to_run == -1) {
        total_result += test6_nested_conditional();
    }
    if (test_to_run == 7 || test_to_run == -1) {
        total_result += test7_struct_first_field();
    }
    if (test_to_run == 8 || test_to_run == -1) {
        total_result += test8_explicit_zero_cast();
    }
    if (test_to_run == 9 || test_to_run == -1) {
        total_result += test9_multiple_induction();
    }
    if (test_to_run == 10 || test_to_run == -1) {
        total_result += test10_complex_zero_expr();
    }
    
    /* Print result to prevent optimization */
    printf("Total checksum: %ld\n", total_result);
    
    return 0;
}
