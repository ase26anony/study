/* test_auto_inc_dec.c - Program to trigger auto-inc-dec pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test_post_increment_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Access with ptr + 0 pattern */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    while (ptr < end) {
        /* This should generate (mem (plus (reg) (const_int 0))) */
        sum += *(ptr + 0);
        ptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test_post_decrement_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    int *start = arr;
    while (ptr >= start) {
        /* Zero offset access */
        sum += ptr[0];
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types with zero offset */
void test_mixed_types_zero_offset(void) {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    volatile char *cptr = char_arr;
    volatile short *sptr = short_arr;
    volatile int *iptr = int_arr;
    volatile long *lptr = long_arr;
    
    /* Char access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        cptr[i + 0] = (char)(i & 0xFF);
        cptr++;
    }
    
    /* Short access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sptr[0] = (short)(i & 0xFFFF);
        sptr++;
    }
    
    /* Int access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        iptr[(int)(0)] = i * 3;  /* Cast zero to int */
        iptr++;
    }
    
    /* Long access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(lptr + 0) = i * 4L;
        lptr++;
    }
}

/* Test 4: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* At offset 0 */
    int second;
    char third;
};

int test_struct_zero_offset(void) {
    struct test_struct arr[100];
    int sum = 0;
    
    struct test_struct *sptr = arr;
    struct test_struct *end = arr + 100;
    
    while (sptr < end) {
        /* Access first member at offset 0 */
        sum += sptr->first;  /* Should generate zero offset pattern */
        sptr->second = sum;
        sptr->third = (char)(sum & 0xFF);
        sptr++;
    }
    
    return sum;
}

/* Test 5: Nested loops with conditional zero offset access */
int test_nested_conditional(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i % 10;
    }
    
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    for (int outer = 0; outer < 10; outer++) {
        int *inner_ptr = ptr;
        int *inner_end = ptr + 100;
        
        while (inner_ptr < inner_end) {
            /* Conditional zero offset access */
            if (outer % 2 == 0) {
                sum += inner_ptr[0];  /* Zero offset */
            } else {
                sum -= inner_ptr[0];  /* Zero offset */
            }
            
            /* Another conditional with different offset */
            if (sum > 1000) {
                inner_ptr[0] = sum % 256;  /* Zero offset */
            }
            
            inner_ptr++;
        }
        
        ptr += 100;
    }
    
    return sum;
}

/* Test 6: Restrict pointers for alias analysis */
int test_restrict_pointers(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i;
    }
    
    int *restrict sptr = src;
    int *restrict dptr = dst;
    int *restrict end = src + ARRAY_SIZE;
    
    /* Copy with zero offset access */
    while (sptr < end) {
        dptr[0] = sptr[0];  /* Both zero offsets */
        sum += dptr[0];
        sptr++;
        dptr++;
    }
    
    return sum;
}

/* Test 7: Multiple step sizes */
int test_various_steps(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i;
    }
    
    /* Step size 1 */
    int *ptr1 = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr1[0];
        ptr1 += 1;
    }
    
    /* Step size 2 */
    int *ptr2 = arr;
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += ptr2[0];
        ptr2 += 2;
    }
    
    /* Step size 4 */
    int *ptr4 = arr;
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        sum += ptr4[0];
        ptr4 += 4;
    }
    
    return sum;
}

/* Test 8: Complex pointer arithmetic with zero */
int test_complex_zero_arithmetic(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * i;
    }
    
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    /* Multiple zero offsets in same expression */
    while (ptr < end) {
        /* Complex expression that should simplify to ptr[0] */
        sum += *(ptr + (0 * 1) + 0);
        
        /* Another zero offset pattern */
        int temp = 0;
        sum += ptr[temp];  /* temp is 0 */
        
        ptr++;
    }
    
    return sum;
}

/* Test 9: Loop with pointer in condition */
int test_pointer_in_condition(void) {
    char str[ARRAY_SIZE];
    int count = 0;
    
    /* Create a string with null terminator */
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        str[i] = 'A' + (i % 26);
    }
    str[ARRAY_SIZE - 1] = '\0';
    
    char *p = str;
    /* Classic while(*p++) pattern */
    while (*p != '\0') {
        /* Access with zero offset */
        if (p[0] == 'A') {
            count++;
        }
        p++;
    }
    
    return count;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    printf("Running auto-inc-dec coverage tests...\n");
    
    /* Run tests based on command line arguments or all by default */
    if (argc > 1) {
        /* Run specific test based on argument */
        int test_num = atoi(argv[1]);
        switch (test_num) {
            case 1:
                total_sum += test_post_increment_zero_offset();
                break;
            case 2:
                total_sum += test_post_decrement_zero_offset();
                break;
            case 3:
                test_mixed_types_zero_offset();
                break;
            case 4:
                total_sum += test_struct_zero_offset();
                break;
            case 5:
                total_sum += test_nested_conditional();
                break;
            case 6:
                total_sum += test_restrict_pointers();
                break;
            case 7:
                total_sum += test_various_steps();
                break;
            case 8:
                total_sum += test_complex_zero_arithmetic();
                break;
            case 9:
                total_sum += test_pointer_in_condition();
                break;
            default:
                /* Run all tests */
                total_sum += test_post_increment_zero_offset();
                total_sum += test_post_decrement_zero_offset();
                test_mixed_types_zero_offset();
                total_sum += test_struct_zero_offset();
                total_sum += test_nested_conditional();
                total_sum += test_restrict_pointers();
                total_sum += test_various_steps();
                total_sum += test_complex_zero_arithmetic();
                total_sum += test_pointer_in_condition();
                break;
        }
    } else {
        /* Run all tests */
        total_sum += test_post_increment_zero_offset();
        total_sum += test_post_decrement_zero_offset();
        test_mixed_types_zero_offset();
        total_sum += test_struct_zero_offset();
        total_sum += test_nested_conditional();
        total_sum += test_restrict_pointers();
        total_sum += test_various_steps();
        total_sum += test_complex_zero_arithmetic();
        total_sum += test_pointer_in_condition();
    }
    
    printf("Total checksum: %d\n", total_sum);
    printf("Tests completed.\n");
    
    return 0;
}
