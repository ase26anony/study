/* auto-inc-dec-test.c
 * Designed to trigger GCC's auto-inc-dec pass for (plus (reg) (const_int 0)) patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024

/* Test 1: Basic post-increment with zero offset */
int test1_basic_postinc(void) {
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
int test2_postdec(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + SIZE - 1;
    int *start = arr;
    while (ptr >= start) {
        sum += ptr[0];  /* Array access with implicit +0 */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
void test3_mixed_types(void) {
    char carr[SIZE];
    short sarr[SIZE];
    int iarr[SIZE];
    long larr[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        carr[i] = (char)(i % 256);
        sarr[i] = (short)(i * 3);
        iarr[i] = i * 5;
        larr[i] = i * 7L;
    }
    
    /* Char pointer loop */
    {
        char *cptr = carr;
        char *cend = carr + SIZE;
        int csum = 0;
        while (cptr < cend) {
            csum += *(cptr + 0);  /* QImode access */
            cptr++;
        }
        printf("Char sum: %d\n", csum);
    }
    
    /* Short pointer loop */
    {
        short *sptr = sarr;
        short *send = sarr + SIZE;
        int ssum = 0;
        while (sptr < send) {
            ssum += sptr[0];  /* HImode access */
            sptr++;
        }
        printf("Short sum: %d\n", ssum);
    }
    
    /* Int pointer loop */
    {
        int *iptr = iarr;
        int *iend = iarr + SIZE;
        int isum = 0;
        while (iptr < iend) {
            isum += *(iptr + 0);  /* SImode access */
            iptr++;
        }
        printf("Int sum: %d\n", isum);
    }
    
    /* Long pointer loop */
    {
        long *lptr = larr;
        long *lend = larr + SIZE;
        long lsum = 0;
        while (lptr < lend) {
            lsum += lptr[0];  /* DImode access */
            lptr++;
        }
        printf("Long sum: %ld\n", lsum);
    }
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    int arr[SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 11;
    }
    
    /* Volatile access with +0 offset */
    for (int i = 0; i < SIZE; i++) {
        sum += vptr[i + 0];  /* Force (plus (reg) (const_int 0)) */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_ptr(void) {
    int arr[SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 13;
    }
    
    /* Loop with restrict pointer */
    for (int i = 0; i < SIZE; i++) {
        sum += *(rptr + 0);  /* Zero offset access */
        rptr++;  /* Post-increment */
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
                sum += ptr[0];  /* First element */
            } else {
                sum += ptr[1];  /* Second element */
            }
            ptr++;  /* Post-increment in outer loop */
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
        arr[i].first = i * 17;
        arr[i].second = i * 19;
        arr[i].third = (char)(i % 128);
    }
    
    struct test_struct *sptr = arr;
    struct test_struct *send = arr + SIZE;
    
    while (sptr < send) {
        /* Access first field at offset 0 */
        sum += sptr->first;  /* Equivalent to *(sptr + 0) */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit cast of zero to pointer offset */
int test8_explicit_zero_cast(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 23;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE;
    
    while (ptr < end) {
        /* Explicit cast of zero to ensure const_int 0 in RTL */
        sum += ptr[(int)(0)];
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple increments with different step sizes */
int test9_variable_steps(void) {
    int arr[SIZE * 4];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 4; i++) {
        arr[i] = i;
    }
    
    /* Step size 1 */
    {
        int *p1 = arr;
        for (int i = 0; i < SIZE; i++) {
            sum += p1[0];
            p1 += 1;  /* Step 1 */
        }
    }
    
    /* Step size 2 */
    {
        int *p2 = arr;
        for (int i = 0; i < SIZE; i++) {
            sum += *(p2 + 0);
            p2 += 2;  /* Step 2 */
        }
    }
    
    /* Step size 4 */
    {
        int *p4 = arr;
        for (int i = 0; i < SIZE; i++) {
            sum += p4[0];
            p4 += 4;  /* Step 4 */
        }
    }
    
    return sum;
}

/* Test 10: Complex expression with zero offset */
int test10_complex_expr(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 29;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE;
    
    while (ptr < end) {
        /* Complex expression that should simplify to ptr[0] */
        int idx = 0;
        sum += ptr[idx * 1 + 0];  /* Should become (plus (reg) (const_int 0)) */
        ptr++;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Run tests based on command line arguments */
    if (argc == 1) {
        /* Run all tests */
        total += test1_basic_postinc();
        total += test2_postdec();
        test3_mixed_types();
        total += test4_volatile_access();
        total += test5_restrict_ptr();
        total += test6_nested_conditional();
        total += test7_struct_first_field();
        total += test8_explicit_zero_cast();
        total += test9_variable_steps();
        total += test10_complex_expr();
    } else {
        /* Run specific tests based on argument */
        for (int i = 1; i < argc; i++) {
            int test_num = atoi(argv[i]);
            switch (test_num) {
                case 1: total += test1_basic_postinc(); break;
                case 2: total += test2_postdec(); break;
                case 3: test3_mixed_types(); break;
                case 4: total += test4_volatile_access(); break;
                case 5: total += test5_restrict_ptr(); break;
                case 6: total += test6_nested_conditional(); break;
                case 7: total += test7_struct_first_field(); break;
                case 8: total += test8_explicit_zero_cast(); break;
                case 9: total += test9_variable_steps(); break;
                case 10: total += test10_complex_expr(); break;
                default: printf("Unknown test: %d\n", test_num);
            }
        }
    }
    
    printf("Total checksum: %d\n", total);
    return 0;
}
