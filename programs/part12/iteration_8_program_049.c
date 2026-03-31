/* Test program for auto-inc-dec pass coverage */
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
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    int *ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end) {
        /* Force zero offset access */
        sum += ptr[0];  /* Equivalent to *(ptr + 0) */
        ptr++;  /* Post-increment */
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
        sum += *(ptr + 0);  /* Explicit zero offset */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types to test various modes */
void test3_mixed_types(void) {
    char carr[SIZE];
    short sarr[SIZE];
    int iarr[SIZE];
    long larr[SIZE];
    
    /* Char access - QImode */
    char *cptr = carr;
    for (int i = 0; i < SIZE; i++) {
        cptr[0] = (char)(i & 0xFF);  /* Zero offset */
        cptr++;
    }
    
    /* Short access - HImode */
    short *sptr = sarr;
    for (int i = 0; i < SIZE; i++) {
        *(sptr + 0) = (short)i;  /* Zero offset */
        sptr++;
    }
    
    /* Int access - SImode */
    int *iptr = iarr;
    for (int i = 0; i < SIZE; i++) {
        iptr[0] = i;  /* Zero offset */
        iptr++;
    }
    
    /* Long access - DImode */
    long *lptr = larr;
    for (int i = 0; i < SIZE; i++) {
        *(lptr + 0) = i * 100L;  /* Zero offset */
        lptr++;
    }
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    volatile int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 3;
    }
    
    volatile int *vptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Force zero offset with volatile */
        sum += vptr[i + 0];  /* Index with explicit zero */
        vptr++;  /* This increment should still be considered */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_pointer(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 4;
    }
    
    int *restrict rptr = arr;
    int *restrict rend = arr + SIZE;
    
    while (rptr < rend) {
        /* Multiple zero-offset accesses */
        int val = rptr[0];  /* First access */
        sum += val;
        rptr[0] = val * 2;  /* Second access */
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
            /* Conditional zero offset access */
            if (inner % 2 == 0) {
                ptr[0] = inner;  /* Zero offset */
            } else {
                *(ptr + 0) = inner * 2;  /* Alternative zero offset */
            }
            sum += ptr[0];
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
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Access first field (offset 0) */
        sptr->first = i;  /* This compiles to offset 0 */
        sum += sptr->first;
        sptr++;
    }
    
    return sum;
}

/* Test 8: Pointer arithmetic with explicit zero cast */
int test8_explicit_zero_cast(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 5;
    }
    
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Force zero through cast */
        sum += ptr[(int)(0)];  /* Explicit zero cast */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple increments with different steps */
int test9_variable_steps(void) {
    int arr[SIZE * 4];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 4; i++) {
        arr[i] = i;
    }
    
    /* Step by 1 */
    int *ptr1 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += ptr1[0];
        ptr1 += 1;  /* Step 1 */
    }
    
    /* Step by 2 */
    int *ptr2 = arr;
    for (int i = 0; i < SIZE/2; i++) {
        sum += ptr2[0];
        ptr2 += 2;  /* Step 2 */
    }
    
    /* Step by 4 */
    int *ptr4 = arr;
    for (int i = 0; i < SIZE/4; i++) {
        sum += *(ptr4 + 0);
        ptr4 += 4;  /* Step 4 */
    }
    
    return sum;
}

/* Test 10: Complex control flow with zero offset */
int test10_complex_flow(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 6;
    }
    
    int *ptr = arr;
    int count = 0;
    
    while (count < SIZE) {
        /* Switch based on count */
        switch (count % 4) {
            case 0:
                ptr[0] = count;  /* Zero offset */
                break;
            case 1:
                *(ptr + 0) = count * 2;  /* Alternative zero offset */
                break;
            case 2:
                /* Nested if */
                if (count % 8 == 0) {
                    ptr[0] = count * 3;
                } else {
                    *(ptr + 0) = count * 4;
                }
                break;
            case 3:
                /* Loop within case */
                for (int j = 0; j < 2; j++) {
                    ptr[0] += j;
                }
                break;
        }
        
        sum += ptr[0];
        ptr++;
        count++;
    }
    
    return sum;
}

/* Main function to run tests based on command line */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    int test_to_run = -1;
    
    /* Parse command line argument for specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run all tests or specific one */
    if (test_to_run == -1 || test_to_run == 1) {
        total_sum += test1_post_increment();
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 2) {
        total_sum += test2_post_decrement();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 3) {
        test3_mixed_types();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 4) {
        total_sum += test4_volatile_access();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 5) {
        total_sum += test5_restrict_pointer();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 6) {
        total_sum += test6_nested_conditional();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 7) {
        total_sum += test7_struct_first_field();
        printf("Test 7 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 8) {
        total_sum += test8_explicit_zero_cast();
        printf("Test 8 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 9) {
        total_sum += test9_variable_steps();
        printf("Test 9 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 10) {
        total_sum += test10_complex_flow();
        printf("Test 10 completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
