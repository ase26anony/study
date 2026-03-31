/* auto_inc_dec_test.c
 * Test program to trigger auto-inc-dec pass coverage for lines 1352-1358
 * Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test_post_increment_zero_offset(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    while (p < end) {
        sum += p[0];  /* Zero offset access */
        p++;
    }
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test_post_decrement_zero_offset(int *arr, int n) {
    int sum = 0;
    int *p = arr + n - 1;
    int *start = arr;
    
    while (p >= start) {
        sum += *(p + 0);  /* Explicit zero offset */
        p--;
    }
    return sum;
}

/* Test 3: Different data types with zero offset */
void test_mixed_types() {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = i % 256;
        short_arr[i] = i % 32768;
        int_arr[i] = i;
        long_arr[i] = i * 2L;
    }
    
    /* Process each with zero-offset access */
    char *cp = char_arr;
    int char_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_sum += cp[0];  /* QImode access */
        cp++;
    }
    
    short *sp = short_arr;
    int short_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        short_sum += *(sp + 0);  /* HImode access */
        sp++;
    }
    
    int *ip = int_arr;
    int int_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_sum += ip[0];  /* SImode access */
        ip++;
    }
    
    long *lp = long_arr;
    long long_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        long_sum += *(lp + 0);  /* DImode access */
        lp++;
    }
    
    /* Use results to prevent optimization */
    printf("Mixed types sums: %d %d %d %ld\n", 
           char_sum, short_sum, int_sum, long_sum);
}

/* Test 4: Volatile pointer with zero offset */
int test_volatile_zero_offset(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    
    for (int i = 0; i < n; i++) {
        sum += p[0];  /* Volatile zero offset access */
        p++;
    }
    return sum;
}

/* Test 5: Restrict pointer for aliasing guarantees */
int test_restrict_zero_offset(int *restrict arr, int n) {
    int sum = 0;
    int *restrict p = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *(p + 0);  /* Zero offset with restrict */
        p++;
    }
    return sum;
}

/* Test 6: Nested loops with conditional zero offset access */
int test_nested_conditional(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            if (j % 2 == 0) {
                sum += p[0];  /* Zero offset in conditional */
            } else {
                sum += p[1];
            }
        }
        p++;
    }
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* At offset 0 */
    int second;
    char third;
};

int test_struct_first_member(struct test_struct *arr, int n) {
    int sum = 0;
    struct test_struct *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Accessing first member = offset 0 */
        sum += p->first;  /* Equivalent to p[0].first */
        p++;
    }
    return sum;
}

/* Test 8: Explicit cast of zero to pointer offset */
int test_explicit_zero_cast(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Force zero offset through cast */
        sum += p[(int)(0)];  /* Explicit zero cast */
        p++;
    }
    return sum;
}

/* Test 9: Multiple increments with different step sizes */
int test_multiple_steps(int *arr, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int *p1 = arr;
    int *p2 = arr;
    int *p3 = arr + n - 1;
    
    /* Step +1 */
    for (int i = 0; i < n; i++) {
        sum1 += p1[0];
        p1++;
    }
    
    /* Step +2 */
    for (int i = 0; i < n; i += 2) {
        sum2 += p2[0];
        p2 += 2;
    }
    
    /* Step -1 */
    for (int i = 0; i < n; i++) {
        sum3 += p3[0];
        p3--;
    }
    
    return sum1 + sum2 + sum3;
}

/* Test 10: Complex pointer arithmetic with zero offset */
int test_complex_arithmetic(int *arr, int n) {
    int sum = 0;
    int *base = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex expression that simplifies to base + 0 */
        int *ptr = base + (i * 0);  /* Should become base + 0 */
        sum += ptr[0];
        base++;
    }
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    /* Initialize test data */
    int int_array[ARRAY_SIZE];
    struct test_struct struct_array[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        struct_array[i].first = i;
        struct_array[i].second = i * 2;
        struct_array[i].third = i % 256;
    }
    
    int total_sum = 0;
    
    /* Run tests based on command line arguments or all by default */
    if (argc > 1) {
        /* Run specific test based on argument */
        int test_num = atoi(argv[1]);
        switch (test_num) {
            case 1:
                total_sum += test_post_increment_zero_offset(int_array, ARRAY_SIZE);
                break;
            case 2:
                total_sum += test_post_decrement_zero_offset(int_array, ARRAY_SIZE);
                break;
            case 3:
                test_mixed_types();
                break;
            case 4:
                total_sum += test_volatile_zero_offset(int_array, ARRAY_SIZE);
                break;
            case 5:
                total_sum += test_restrict_zero_offset(int_array, ARRAY_SIZE);
                break;
            case 6:
                total_sum += test_nested_conditional(int_array, ARRAY_SIZE);
                break;
            case 7:
                total_sum += test_struct_first_member(struct_array, ARRAY_SIZE);
                break;
            case 8:
                total_sum += test_explicit_zero_cast(int_array, ARRAY_SIZE);
                break;
            case 9:
                total_sum += test_multiple_steps(int_array, ARRAY_SIZE);
                break;
            case 10:
                total_sum += test_complex_arithmetic(int_array, ARRAY_SIZE);
                break;
            default:
                /* Run all tests */
                total_sum += test_post_increment_zero_offset(int_array, ARRAY_SIZE);
                total_sum += test_post_decrement_zero_offset(int_array, ARRAY_SIZE);
                test_mixed_types();
                total_sum += test_volatile_zero_offset(int_array, ARRAY_SIZE);
                total_sum += test_restrict_zero_offset(int_array, ARRAY_SIZE);
                total_sum += test_nested_conditional(int_array, ARRAY_SIZE);
                total_sum += test_struct_first_member(struct_array, ARRAY_SIZE);
                total_sum += test_explicit_zero_cast(int_array, ARRAY_SIZE);
                total_sum += test_multiple_steps(int_array, ARRAY_SIZE);
                total_sum += test_complex_arithmetic(int_array, ARRAY_SIZE);
                break;
        }
    } else {
        /* Run all tests */
        total_sum += test_post_increment_zero_offset(int_array, ARRAY_SIZE);
        total_sum += test_post_decrement_zero_offset(int_array, ARRAY_SIZE);
        test_mixed_types();
        total_sum += test_volatile_zero_offset(int_array, ARRAY_SIZE);
        total_sum += test_restrict_zero_offset(int_array, ARRAY_SIZE);
        total_sum += test_nested_conditional(int_array, ARRAY_SIZE);
        total_sum += test_struct_first_member(struct_array, ARRAY_SIZE);
        total_sum += test_explicit_zero_cast(int_array, ARRAY_SIZE);
        total_sum += test_multiple_steps(int_array, ARRAY_SIZE);
        total_sum += test_complex_arithmetic(int_array, ARRAY_SIZE);
    }
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
