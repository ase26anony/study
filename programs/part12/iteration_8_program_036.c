/* auto_inc_dec_test.c
 * Designed to trigger GCC's auto-inc-dec pass for lines 1352-1358
 * Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test1_post_increment_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Access with pointer + 0 pattern */
    int *ptr = arr;
    int *end = arr + SIZE;
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    while (ptr < end) {
        sum += ptr[0];          /* Zero offset access */
        ptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with explicit +0 */
int test2_post_decrement_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + SIZE - 1;
    int *start = arr;
    
    /* Decrementing loop with zero offset */
    while (ptr >= start) {
        sum += *(ptr + 0);      /* Explicit +0 offset */
        ptr--;                  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types for different memory modes */
void test3_mixed_data_types(void) {
    char char_arr[SIZE];
    short short_arr[SIZE];
    int int_arr[SIZE];
    long long_arr[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        char_arr[i] = i & 0xFF;
        short_arr[i] = i * 2;
        int_arr[i] = i * 3;
        long_arr[i] = i * 4L;
    }
    
    /* Char access - QImode */
    {
        char *cptr = char_arr;
        char *cend = char_arr + SIZE;
        volatile char checksum = 0;  /* volatile to prevent optimization */
        
        while (cptr < cend) {
            checksum += cptr[0];     /* Zero offset */
            cptr++;
        }
    }
    
    /* Short access - HImode */
    {
        short *sptr = short_arr;
        short *send = short_arr + SIZE;
        volatile short sum = 0;
        
        for (int i = 0; i < SIZE; i++) {
            sum += *(sptr + 0);      /* Explicit +0 */
            sptr++;
        }
    }
    
    /* Int access - SImode */
    {
        int *iptr = int_arr;
        int *iend = int_arr + SIZE;
        volatile int sum = 0;
        
        while (iptr < iend) {
            sum += iptr[0];          /* Zero offset */
            iptr++;
        }
    }
    
    /* Long access - DImode */
    {
        long *lptr = long_arr;
        long *lend = long_arr + SIZE;
        volatile long sum = 0;
        
        for (int i = 0; i < SIZE; i++) {
            sum += *(lptr + 0);      /* Explicit +0 */
            lptr++;
        }
    }
}

/* Test 4: Volatile pointers with restrict */
int test4_volatile_restrict(void) {
    int base_arr[SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        base_arr[i] = i + 1;
    }
    
    /* Volatile pointer - may generate different RTL */
    volatile int *vptr = base_arr;
    volatile int *vend = base_arr + SIZE;
    
    while (vptr < vend) {
        sum += vptr[0];         /* Zero offset with volatile */
        vptr++;
    }
    
    /* Restrict pointer - gives aliasing guarantees */
    {
        int *restrict rptr = base_arr;
        int *restrict rend = base_arr + SIZE;
        int rsum = 0;
        
        while (rptr < rend) {
            rsum += rptr[0];    /* Zero offset with restrict */
            rptr++;
        }
        sum += rsum;
    }
    
    return sum;
}

/* Test 5: Nested loops with conditional access */
int test5_nested_conditional(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i % 100;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE * 2;
    
    /* Complex control flow */
    while (ptr < end) {
        /* Conditional zero-offset access */
        if ((ptr - arr) % 3 == 0) {
            sum += ptr[0];          /* Zero offset in if branch */
        } else if ((ptr - arr) % 3 == 1) {
            sum -= *(ptr + 0);      /* Explicit +0 in else-if */
        } else {
            sum ^= ptr[0];          /* Another zero offset */
        }
        
        /* Nested loop with different step */
        for (int j = 0; j < 2; j++) {
            if (ptr + j < end) {
                sum += (ptr + j)[0]; /* Offset based on j, but [0] access */
            }
        }
        
        ptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 6: Structure with first member at offset 0 */
struct test_struct {
    int first;      /* At offset 0 */
    int second;
    char third;
};

int test6_struct_first_member(void) {
    struct test_struct arr[SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = i & 0xFF;
    }
    
    struct test_struct *sptr = arr;
    struct test_struct *send = arr + SIZE;
    
    /* Access first member (offset 0) */
    while (sptr < send) {
        sum += sptr->first;     /* Accesses member at offset 0 */
        sptr++;
    }
    
    return sum;
}

/* Test 7: Array indexing with explicit zero cast */
int test7_explicit_zero_cast(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 3;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE;
    
    /* Cast zero to different types to force +0 pattern */
    while (ptr < end) {
        sum += ptr[(int)(0)];           /* Cast to int */
        sum += ptr[(unsigned)(0)];      /* Cast to unsigned */
        sum += ptr[(size_t)(0)];        /* Cast to size_t */
        ptr++;
    }
    
    return sum;
}

/* Test 8: Multiple induction variables */
int test8_multiple_induction(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Multiple pointers with different update patterns */
    int *ptr1 = arr;
    int *ptr2 = arr;
    int *end = arr + SIZE;
    
    while (ptr1 < end) {
        /* Both access with zero offset */
        sum += ptr1[0];
        sum += ptr2[0];
        
        /* Different increments */
        ptr1 += 1;
        ptr2 = ptr1;  /* ptr2 gets updated differently */
    }
    
    return sum;
}

/* Test 9: Loop with step size 2 */
int test9_step_size_2(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE;
    
    /* Step by 2, still accessing with zero offset */
    for (int i = 0; i < SIZE; i += 2) {
        sum += ptr[0];      /* Zero offset */
        ptr += 2;           /* Step size 2 */
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc to control which tests run, ensuring all code is compiled */
    if (argc == 1) {
        /* Run all tests */
        total += test1_post_increment_zero_offset();
        total += test2_post_decrement_zero_offset();
        test3_mixed_data_types();
        total += test4_volatile_restrict();
        total += test5_nested_conditional();
        total += test6_struct_first_member();
        total += test7_explicit_zero_cast();
        total += test8_multiple_induction();
        total += test9_step_size_2();
    } else {
        /* Run specific tests based on argument */
        for (int i = 1; i < argc; i++) {
            int test_num = atoi(argv[i]);
            switch (test_num) {
                case 1: total += test1_post_increment_zero_offset(); break;
                case 2: total += test2_post_decrement_zero_offset(); break;
                case 3: test3_mixed_data_types(); break;
                case 4: total += test4_volatile_restrict(); break;
                case 5: total += test5_nested_conditional(); break;
                case 6: total += test6_struct_first_member(); break;
                case 7: total += test7_explicit_zero_cast(); break;
                case 8: total += test8_multiple_induction(); break;
                case 9: total += test9_step_size_2(); break;
                default: break;
            }
        }
    }
    
    /* Print checksum to prevent optimization and verify correctness */
    printf("Checksum: %d\n", total);
    
    return 0;
}
