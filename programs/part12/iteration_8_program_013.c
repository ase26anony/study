/* auto-inc-dec-test.c
 * Program designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Lines 1352-1358: Setting up memory instruction with constant zero offset
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
    for (; p < end; p++) {
        sum += p[0];  /* Zero offset array access */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test_post_decrement_zero_offset(int *arr, int n) {
    int sum = 0;
    int *p = arr + n - 1;
    int *start = arr;
    
    /* Decrementing loop with zero offset */
    for (; p >= start; p--) {
        sum += *(p + 0);  /* Explicit zero offset */
    }
    
    return sum;
}

/* Test 3: Different data types for different memory modes */
long test_mixed_data_types(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    long total = 0;
    
    /* Char access - QImode */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i + 0] = (char)(i % 256);
        total += char_arr[i];
    }
    
    /* Short access - HImode */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        short_arr[(int)(0) + i] = (short)(i % 65536);  /* Cast zero to int */
        total += short_arr[i];
    }
    
    /* Int access - SImode */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i * 2;
        total += int_arr[0 + i];  /* Zero offset addition */
    }
    
    /* Long access - DImode */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        long_arr[i] = i * 3L;
        total += long_arr[i + 0];  /* Another zero offset */
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test_volatile_zero_offset(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    
    /* Volatile prevents some optimizations but should still generate the pattern */
    for (int i = 0; i < n; i++) {
        sum += p[0];  /* Zero offset with volatile */
        p++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test_restrict_zero_offset(int *restrict arr1, int *restrict arr2, int n) {
    int sum = 0;
    
    /* Restrict gives stronger aliasing guarantees */
    for (int i = 0; i < n; i++) {
        arr1[i + 0] = i;  /* Zero offset */
        arr2[0 + i] = i * 2;  /* Another zero offset */
        sum += arr1[i] + arr2[i];
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset access */
int test_nested_conditional(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Conditional access pattern */
        if (i % 2 == 0) {
            p[0] = i;  /* Zero offset in if branch */
        } else {
            *(p + 0) = i * 2;  /* Zero offset in else branch */
        }
        
        /* Nested loop with pointer arithmetic */
        for (int j = 0; j < 4; j++) {
            sum += p[0];  /* Zero offset in nested loop */
        }
        
        p++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* Offset 0 */
    int second;
    char third;
};

int test_struct_offset_zero(struct test_struct *arr, int n) {
    int sum = 0;
    struct test_struct *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Accessing first member - at offset 0 */
        sum += p->first;  /* Should generate (mem (plus (reg) (const_int 0))) */
        p++;
    }
    
    return sum;
}

/* Test 8: Multiple induction variables with different step sizes */
int test_multiple_induction(int *arr, int n) {
    int sum = 0;
    int *p1 = arr;
    int *p2 = arr;
    
    /* Two pointers with different update patterns */
    for (int i = 0; i < n; i += 2) {
        sum += p1[0];  /* Zero offset */
        sum += p2[0];  /* Another zero offset */
        
        p1 += 1;  /* Step 1 */
        p2 += 2;  /* Step 2 */
    }
    
    return sum;
}

/* Test 9: Pointer arithmetic with explicit zero constant */
int test_explicit_zero_constant(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Force constant 0 in pointer arithmetic */
    const int ZERO = 0;
    
    for (int i = 0; i < n; i++) {
        sum += *(p + ZERO);  /* Using const zero */
        p++;
    }
    
    return sum;
}

/* Test 10: Complex expression with zero offset */
int test_complex_zero_expr(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex expression that simplifies to zero offset */
        int offset = (i * 0) + (0 * n) + 0;
        sum += p[offset];  /* Should be p[0] */
        p++;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Initialize test arrays */
    int int_array[ARRAY_SIZE];
    int int_array2[ARRAY_SIZE];
    struct test_struct struct_array[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        int_array2[i] = i * 2;
        struct_array[i].first = i;
        struct_array[i].second = i * 2;
        struct_array[i].third = (char)(i % 256);
    }
    
    int result = 0;
    
    /* Use command line arguments to control which tests run */
    /* This ensures all code is compiled even if not all executed */
    if (argc > 1) {
        int test_num = atoi(argv[1]);
        
        switch (test_num) {
            case 1:
                result = test_post_increment_zero_offset(int_array, ARRAY_SIZE);
                break;
            case 2:
                result = test_post_decrement_zero_offset(int_array, ARRAY_SIZE);
                break;
            case 3:
                result = (int)test_mixed_data_types();
                break;
            case 4:
                result = test_volatile_zero_offset(int_array, ARRAY_SIZE);
                break;
            case 5:
                result = test_restrict_zero_offset(int_array, int_array2, ARRAY_SIZE);
                break;
            case 6:
                result = test_nested_conditional(int_array, ARRAY_SIZE);
                break;
            case 7:
                result = test_struct_offset_zero(struct_array, ARRAY_SIZE);
                break;
            case 8:
                result = test_multiple_induction(int_array, ARRAY_SIZE);
                break;
            case 9:
                result = test_explicit_zero_constant(int_array, ARRAY_SIZE);
                break;
            case 10:
                result = test_complex_zero_expr(int_array, ARRAY_SIZE);
                break;
            default:
                /* Run all tests */
                result = test_post_increment_zero_offset(int_array, ARRAY_SIZE);
                result += test_post_decrement_zero_offset(int_array, ARRAY_SIZE);
                result += (int)test_mixed_data_types();
                result += test_volatile_zero_offset(int_array, ARRAY_SIZE);
                result += test_restrict_zero_offset(int_array, int_array2, ARRAY_SIZE);
                result += test_nested_conditional(int_array, ARRAY_SIZE);
                result += test_struct_offset_zero(struct_array, ARRAY_SIZE);
                result += test_multiple_induction(int_array, ARRAY_SIZE);
                result += test_explicit_zero_constant(int_array, ARRAY_SIZE);
                result += test_complex_zero_expr(int_array, ARRAY_SIZE);
                break;
        }
    } else {
        /* Default: run a subset of tests */
        result = test_post_increment_zero_offset(int_array, ARRAY_SIZE);
        result += test_post_decrement_zero_offset(int_array, ARRAY_SIZE);
        result += test_restrict_zero_offset(int_array, int_array2, ARRAY_SIZE);
    }
    
    /* Print result to prevent optimization removal */
    printf("Result: %d\n", result);
    
    return 0;
}
