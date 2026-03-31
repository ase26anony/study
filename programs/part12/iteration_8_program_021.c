/* test_auto_inc_dec.c - Program to trigger auto-inc-dec pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test_simple_post_inc(int *arr, int n) {
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
int test_simple_post_dec(int *arr, int n) {
    int sum = 0;
    int *p = arr + n - 1;
    int *start = arr;
    
    while (p >= start) {
        sum += *(p + 0);  /* Zero offset via pointer arithmetic */
        p--;
    }
    return sum;
}

/* Test 3: Different data types for different memory modes */
long test_mixed_types(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    long total = 0;
    
    /* Char access - QImode */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i + 0] = (char)(i % 256);  /* Zero offset */
        total += char_arr[i];
    }
    
    /* Short access - HImode */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        short_arr[(int)(0) + i] = (short)(i % 65536);  /* Cast zero to int */
        total += short_arr[i];
    }
    
    /* Int access - SImode */
    int *int_ptr = int_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_ptr[0] = i;  /* Direct zero offset */
        total += int_ptr[0];
        int_ptr++;
    }
    
    /* Long access - DImode */
    long *long_ptr = long_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(long_ptr + 0) = i * 1000L;  /* Pointer + 0 */
        total += *(long_ptr + 0);
        long_ptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test_volatile_access(volatile int *arr, int n) {
    int sum = 0;
    volatile int *vptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += vptr[i + 0];  /* Volatile access with zero offset */
        vptr++;
    }
    return sum;
}

/* Test 5: Restrict pointer for aliasing guarantees */
int test_restrict_pointer(int *restrict arr1, int *restrict arr2, int n) {
    int sum = 0;
    int *restrict p1 = arr1;
    int *restrict p2 = arr2;
    
    for (int i = 0; i < n; i++) {
        p1[0] = i;          /* Zero offset write */
        p2[0] = p1[0] * 2;  /* Zero offset read and write */
        sum += p2[0];
        p1++;
        p2++;
    }
    return sum;
}

/* Test 6: Nested loops with conditional zero-offset access */
int test_nested_conditional(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Conditional zero-offset access */
        if (i % 2 == 0) {
            ptr[0] = i * 2;  /* Even indices */
        } else {
            *(ptr + 0) = i * 3;  /* Odd indices - different zero-offset syntax */
        }
        
        /* Nested loop with pointer arithmetic */
        for (int j = 0; j < 4; j++) {
            if (j % 2 == 0) {
                sum += ptr[0];  /* Zero offset in nested loop */
            }
        }
        
        ptr++;
    }
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;    /* Offset 0 */
    int second;
    char third;
};

int test_struct_offset(struct test_struct *arr, int n) {
    int sum = 0;
    struct test_struct *sptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Accessing first member is at offset 0 */
        sum += sptr->first;  /* This should generate plus with const_int 0 */
        sptr++;
    }
    return sum;
}

/* Test 8: Different step sizes */
int test_variable_steps(int *arr, int n) {
    int sum = 0;
    
    /* Step size 1 */
    int *p1 = arr;
    for (int i = 0; i < n; i += 1) {
        sum += p1[0];
        p1 += 1;
    }
    
    /* Step size 2 */
    int *p2 = arr;
    for (int i = 0; i < n; i += 2) {
        sum += *(p2 + 0);
        p2 += 2;
    }
    
    /* Step size 4 (negative direction) */
    int *p3 = arr + n - 1;
    for (int i = 0; i < n; i += 4) {
        sum += p3[0];
        p3 -= 4;
    }
    
    return sum;
}

/* Test 9: Complex expression with zero offset */
int test_complex_zero_expr(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex expression that should simplify to ptr + 0 */
        int offset = (i * 0) + (0 * i) + 0;
        sum += ptr[offset];
        ptr++;
    }
    return sum;
}

/* Test 10: Multiple zero-offset accesses in same loop */
int test_multiple_zero_accesses(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Multiple zero-offset accesses to same location */
        int temp = ptr[0];      /* First access */
        temp += *(ptr + 0);     /* Second access, different syntax */
        temp += ptr[0 + 0];     /* Third access, constant expression */
        sum += temp;
        ptr++;
    }
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    /* Initialize test arrays */
    int int_array[ARRAY_SIZE];
    int int_array2[ARRAY_SIZE];
    struct test_struct struct_array[ARRAY_SIZE];
    
    /* Initialize arrays with values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        int_array2[i] = i * 2;
        struct_array[i].first = i * 3;
        struct_array[i].second = i * 4;
        struct_array[i].third = (char)(i % 256);
    }
    
    int total_sum = 0;
    
    /* Run tests based on command line arguments or all by default */
    if (argc > 1) {
        /* Run specific test based on argument */
        int test_num = atoi(argv[1]);
        switch (test_num) {
            case 1:
                total_sum += test_simple_post_inc(int_array, ARRAY_SIZE);
                break;
            case 2:
                total_sum += test_simple_post_dec(int_array, ARRAY_SIZE);
                break;
            case 3:
                total_sum += test_mixed_types();
                break;
            case 4:
                total_sum += test_volatile_access(int_array, ARRAY_SIZE);
                break;
            case 5:
                total_sum += test_restrict_pointer(int_array, int_array2, ARRAY_SIZE);
                break;
            case 6:
                total_sum += test_nested_conditional(int_array, ARRAY_SIZE);
                break;
            case 7:
                total_sum += test_struct_offset(struct_array, ARRAY_SIZE);
                break;
            case 8:
                total_sum += test_variable_steps(int_array, ARRAY_SIZE);
                break;
            case 9:
                total_sum += test_complex_zero_expr(int_array, ARRAY_SIZE);
                break;
            case 10:
                total_sum += test_multiple_zero_accesses(int_array, ARRAY_SIZE);
                break;
            default:
                /* Run all tests */
                total_sum += test_simple_post_inc(int_array, ARRAY_SIZE);
                total_sum += test_simple_post_dec(int_array, ARRAY_SIZE);
                total_sum += test_mixed_types();
                total_sum += test_volatile_access(int_array, ARRAY_SIZE);
                total_sum += test_restrict_pointer(int_array, int_array2, ARRAY_SIZE);
                total_sum += test_nested_conditional(int_array, ARRAY_SIZE);
                total_sum += test_struct_offset(struct_array, ARRAY_SIZE);
                total_sum += test_variable_steps(int_array, ARRAY_SIZE);
                total_sum += test_complex_zero_expr(int_array, ARRAY_SIZE);
                total_sum += test_multiple_zero_accesses(int_array, ARRAY_SIZE);
                break;
        }
    } else {
        /* Run all tests by default */
        total_sum += test_simple_post_inc(int_array, ARRAY_SIZE);
        total_sum += test_simple_post_dec(int_array, ARRAY_SIZE);
        total_sum += test_mixed_types();
        total_sum += test_volatile_access(int_array, ARRAY_SIZE);
        total_sum += test_restrict_pointer(int_array, int_array2, ARRAY_SIZE);
        total_sum += test_nested_conditional(int_array, ARRAY_SIZE);
        total_sum += test_struct_offset(struct_array, ARRAY_SIZE);
        total_sum += test_variable_steps(int_array, ARRAY_SIZE);
        total_sum += test_complex_zero_expr(int_array, ARRAY_SIZE);
        total_sum += test_multiple_zero_accesses(int_array, ARRAY_SIZE);
    }
    
    /* Print result to prevent optimization removal */
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
