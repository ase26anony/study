/* auto_inc_test.c - Program to trigger auto-inc-dec pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024

/* Test 1: Basic pointer arithmetic with zero offset in loops */
int test1_basic_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Pattern: ptr[i + 0] with post-increment */
    int *ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += ptr[0];          /* Zero offset array access */
        ptr++;
    }
    
    /* Another variant: *(ptr + 0) */
    ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += *(ptr + 0);      /* Explicit zero offset */
        ptr++;
    }
    
    return sum;
}

/* Test 2: Different data types with zero offset */
int test2_mixed_types(void) {
    char carr[SIZE];
    short sarr[SIZE];
    int iarr[SIZE];
    long larr[SIZE];
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        carr[i] = (char)(i % 256);
        sarr[i] = (short)(i % 65536);
        iarr[i] = i;
        larr[i] = i * 2L;
    }
    
    /* Char access with zero offset */
    char *cptr = carr;
    for (int i = 0; i < SIZE; i++) {
        sum += cptr[0];         /* QImode access */
        cptr++;
    }
    
    /* Short access with zero offset */
    short *sptr = sarr;
    for (int i = 0; i < SIZE; i++) {
        sum += sptr[0];         /* HImode access */
        sptr++;
    }
    
    /* Int access with zero offset */
    int *iptr = iarr;
    for (int i = 0; i < SIZE; i++) {
        sum += iptr[0];         /* SImode access */
        iptr++;
    }
    
    /* Long access with zero offset */
    long *lptr = larr;
    for (int i = 0; i < SIZE; i++) {
        sum += (int)lptr[0];    /* DImode access */
        lptr++;
    }
    
    return sum;
}

/* Test 3: Volatile and restrict qualifiers */
int test3_volatile_restrict(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Volatile pointer with zero offset */
    volatile int *vptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += vptr[i + 0];     /* Zero offset with volatile */
    }
    
    /* Restrict pointer for aliasing guarantees */
    int *restrict rptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += rptr[0];         /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 4: Nested and conditional access patterns */
int test4_complex_patterns(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE * 2;
    
    /* Complex loop with conditional zero-offset access */
    for (int i = 0; ptr < end; i++) {
        if (i % 3 == 0) {
            /* Zero offset in one branch */
            sum += ptr[0];
        } else if (i % 3 == 1) {
            /* Non-zero offset in another branch */
            sum += ptr[1];
        } else {
            /* Another zero offset variant */
            sum += *(ptr + 0);
        }
        
        /* Post-increment in loop update */
        ptr++;
        
        /* Nested loop with zero offset */
        if (i % 10 == 0) {
            int *inner = ptr;
            for (int j = 0; j < 5 && inner < end; j++) {
                sum += inner[0];  /* Zero offset in nested loop */
                inner++;
            }
        }
    }
    
    return sum;
}

/* Test 5: Structure access with zero offset */
struct test_struct {
    int first;   /* Offset 0 */
    int second;
    char third;
};

int test5_struct_zero_offset(void) {
    struct test_struct sarr[SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        sarr[i].first = i;
        sarr[i].second = i * 2;
        sarr[i].third = (char)(i % 256);
    }
    
    /* Access first member (offset 0) */
    struct test_struct *sptr = sarr;
    for (int i = 0; i < SIZE; i++) {
        /* sptr->first generates address with zero offset */
        sum += sptr->first;
        sptr++;
    }
    
    /* Cast to char* for byte access at offset 0 */
    char *cptr = (char *)sarr;
    for (int i = 0; i < SIZE * sizeof(struct test_struct); i++) {
        sum += cptr[0];  /* Byte access at zero offset */
        cptr++;
    }
    
    return sum;
}

/* Test 6: Post-decrement patterns */
int test6_post_decrement(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Post-decrement loop */
    int *ptr = arr + SIZE - 1;
    for (int i = SIZE - 1; i >= 0; i--) {
        sum += ptr[0];  /* Zero offset with post-decrement */
        ptr--;
    }
    
    /* Another variant with while loop */
    ptr = arr + SIZE - 1;
    int *start = arr;
    while (ptr >= start) {
        sum += *(ptr + 0);  /* Explicit zero offset */
        ptr--;
    }
    
    return sum;
}

/* Test 7: Different step sizes */
int test7_variable_steps(void) {
    int arr[SIZE * 4];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 4; i++) {
        arr[i] = i;
    }
    
    /* Step size 1 */
    int *ptr1 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += ptr1[0];
        ptr1 += 1;
    }
    
    /* Step size 2 */
    int *ptr2 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += ptr2[0];
        ptr2 += 2;
    }
    
    /* Step size 4 */
    int *ptr4 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += ptr4[0];
        ptr4 += 4;
    }
    
    return sum;
}

/* Test 8: Cast zero to pointer offset type */
int test8_cast_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    
    /* Cast zero to different types for offset */
    for (int i = 0; i < SIZE; i++) {
        /* Various ways to express zero offset through casts */
        sum += ptr[(int)(0)];
        sum += ptr[(unsigned int)(0)];
        sum += ptr[(size_t)(0)];
        sum += ptr[(ptrdiff_t)(0)];
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
        total_sum += test1_basic_zero_offset();
        total_sum += test2_mixed_types();
        total_sum += test3_volatile_restrict();
        total_sum += test4_complex_patterns();
        total_sum += test5_struct_zero_offset();
        total_sum += test6_post_decrement();
        total_sum += test7_variable_steps();
        total_sum += test8_cast_zero_offset();
    } else {
        /* Run specific tests based on arguments */
        for (int i = 1; i < argc; i++) {
            int test_num = atoi(argv[i]);
            switch (test_num) {
                case 1: total_sum += test1_basic_zero_offset(); break;
                case 2: total_sum += test2_mixed_types(); break;
                case 3: total_sum += test3_volatile_restrict(); break;
                case 4: total_sum += test4_complex_patterns(); break;
                case 5: total_sum += test5_struct_zero_offset(); break;
                case 6: total_sum += test6_post_decrement(); break;
                case 7: total_sum += test7_variable_steps(); break;
                case 8: total_sum += test8_cast_zero_offset(); break;
                default: break;
            }
        }
    }
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
