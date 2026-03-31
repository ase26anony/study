/* test_auto_inc_dec.c - Program to trigger auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Basic post-increment with zero offset */
int test1_basic_post_inc(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Key pattern: pointer + 0 in loop */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    /* Multiple variations of zero-offset access */
    while (ptr < end) {
        sum += ptr[0];           /* Array notation with explicit 0 */
        sum += *(ptr + 0);       /* Pointer arithmetic with 0 */
        ptr++;                   /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_basic_post_dec(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    int *start = arr;
    
    while (ptr >= start) {
        sum += ptr[0];           /* Zero offset */
        ptr--;                   /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
int test3_mixed_types(void) {
    char c_arr[ARRAY_SIZE];
    short s_arr[ARRAY_SIZE];
    int i_arr[ARRAY_SIZE];
    long l_arr[ARRAY_SIZE];
    
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        c_arr[i] = (char)(i % 256);
        s_arr[i] = (short)(i % 32768);
        i_arr[i] = i;
        l_arr[i] = i * 1000L;
    }
    
    /* Char pointer loop */
    char *cptr = c_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += cptr[0];          /* QImode access */
        cptr++;
    }
    
    /* Short pointer loop */
    short *sptr = s_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr[0];          /* HImode access */
        sptr++;
    }
    
    /* Int pointer loop */
    int *iptr = i_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += iptr[0];          /* SImode access */
        iptr++;
    }
    
    /* Long pointer loop */
    long *lptr = l_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += (int)lptr[0];     /* DImode access */
        lptr++;
    }
    
    return sum;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 3;
    }
    
    /* Volatile pointer - may prevent some optimizations but creates patterns */
    volatile int *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[0];          /* Zero offset with volatile */
        vptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_pointer(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 4;
    }
    
    /* Restrict gives stronger aliasing guarantees */
    int *restrict rptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];          /* Zero offset with restrict */
        rptr++;
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
    int *end = arr + ARRAY_SIZE * 2;
    
    /* Complex control flow with zero-offset accesses */
    while (ptr < end) {
        if ((ptr - arr) % 3 == 0) {
            sum += ptr[0];       /* Zero offset in conditional */
        } else if ((ptr - arr) % 3 == 1) {
            sum += *(ptr + 0);   /* Another zero offset variant */
        } else {
            sum += ptr[1];       /* Non-zero offset for contrast */
        }
        
        /* Multiple pointer updates in different paths */
        if (sum % 2 == 0) {
            ptr++;               /* Post-increment */
        } else {
            ptr += 1;            /* Same effect, different syntax */
        }
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
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)(i % 128);
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Access first member - at offset 0 */
        sum += sptr->first;      /* Equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit cast of zero to pointer offset */
int test8_explicit_zero_cast(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 5;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Explicit cast of 0 to different types */
        sum += ptr[(int)(0)];            /* Cast to int */
        sum += ptr[(unsigned)(0)];       /* Cast to unsigned */
        sum += ptr[(size_t)(0)];         /* Cast to size_t */
        sum += ptr[(ptrdiff_t)(0)];      /* Cast to ptrdiff_t */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple induction variables */
int test9_multiple_induction(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 6;
    }
    
    /* Multiple pointers with different update patterns */
    int *ptr1 = arr;
    int *ptr2 = arr;
    int *end = arr + ARRAY_SIZE;
    
    while (ptr1 < end) {
        sum += ptr1[0];          /* Zero offset */
        sum += ptr2[0];          /* Another zero offset */
        
        ptr1++;                  /* Step 1 */
        ptr2 += 1;               /* Same step, different syntax */
    }
    
    return sum;
}

/* Test 10: Combined patterns in complex loop */
int test10_combined_patterns(void) {
    int arr1[ARRAY_SIZE];
    int arr2[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i;
        arr2[i] = ARRAY_SIZE - i;
    }
    
    /* Complex loop with multiple zero-offset patterns */
    int *p1 = arr1;
    int *p2 = arr2;
    volatile int *vp = arr1;
    int *restrict rp = arr2;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Mix of different zero-offset access patterns */
        sum += p1[0] + p2[0];
        sum += *(vp + 0);
        sum += rp[0];
        
        /* Different update steps */
        p1++;
        p2 += 1;
        vp++;
        rp++;
        
        /* Conditional with zero offset */
        if (i % 10 == 0) {
            sum += p1[0];
        }
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Run tests based on command line arguments */
    if (argc == 1) {
        /* Run all tests if no arguments */
        total_sum += test1_basic_post_inc();
        total_sum += test2_basic_post_dec();
        total_sum += test3_mixed_types();
        total_sum += test4_volatile_access();
        total_sum += test5_restrict_pointer();
        total_sum += test6_nested_conditional();
        total_sum += test7_struct_first_member();
        total_sum += test8_explicit_zero_cast();
        total_sum += test9_multiple_induction();
        total_sum += test10_combined_patterns();
    } else {
        /* Run specific tests based on arguments */
        for (int i = 1; i < argc; i++) {
            int test_num = atoi(argv[i]);
            switch (test_num) {
                case 1: total_sum += test1_basic_post_inc(); break;
                case 2: total_sum += test2_basic_post_dec(); break;
                case 3: total_sum += test3_mixed_types(); break;
                case 4: total_sum += test4_volatile_access(); break;
                case 5: total_sum += test5_restrict_pointer(); break;
                case 6: total_sum += test6_nested_conditional(); break;
                case 7: total_sum += test7_struct_first_member(); break;
                case 8: total_sum += test8_explicit_zero_cast(); break;
                case 9: total_sum += test9_multiple_induction(); break;
                case 10: total_sum += test10_combined_patterns(); break;
                default: printf("Unknown test: %d\n", test_num);
            }
        }
    }
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
