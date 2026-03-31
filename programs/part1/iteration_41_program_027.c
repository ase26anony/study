/* test_auto_inc_dec.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec.cc
 * Lines 1352-1358: Simple register addressing pattern detection
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to ensure memory accesses are real */
volatile int global_array[256];

/* Test 1: Simple parameter access with mixed patterns */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;                    /* Line 1: Simple reg addressing */
    
    /* Register + constant offset */
    sum += base[5];                  /* Line 2: Reg + const offset */
    
    /* Loop with pointer increment */
    for (int i = 0; i < n; i++) {
        sum += *base++;              /* Line 3: Auto-increment pattern */
    }
    
    /* Another simple register access */
    int *simple_ptr = base;
    sum += *simple_ptr;              /* Line 4: Another simple reg access */
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = (int*)&global_array[0];
    
    /* Multiple simple register accesses */
    sum += p[0];                     /* Line 5: p[0] is simple reg */
    sum += *p;                       /* Line 6: *p is simple reg */
    
    /* Mixed with offset */
    sum += p[10];
    sum += p[20];
    
    /* Loop with simple access inside */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sum += *p;               /* Line 7: Simple access in loop */
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Array traversal with different pointer usages */
int test_array_traversal(int *arr, int size) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = &arr[size/2];
    
    /* Simple access through ptr1 */
    sum += *ptr1;                    /* Line 8: Simple reg */
    
    /* Simple access through ptr2 */
    sum += *ptr2;                    /* Line 9: Simple reg */
    
    /* Traverse first half */
    for (int i = 0; i < size/2; i++) {
        sum += *ptr1++;
    }
    
    /* Traverse second half with simple access check */
    for (int i = 0; i < size/2; i++) {
        sum += *ptr2;               /* Line 10: Simple reg in loop */
        ptr2++;
    }
    
    return sum;
}

/* Test 4: Conditional simple accesses */
int test_conditional_access(int *base, int n) {
    int sum = 0;
    int *current = base;
    
    for (int i = 0; i < n; i++) {
        if (i == 0) {
            sum += *current;        /* Line 11: Simple reg in condition */
        } else if (i == n-1) {
            sum += *current;        /* Line 12: Simple reg in condition */
        } else {
            sum += current[i];
        }
    }
    
    /* Post-loop simple access */
    sum += *current;                /* Line 13: Simple reg after loop */
    
    return sum;
}

/* Test 5: Struct access with simple pointer */
struct Data {
    int values[16];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *ptr = data->values;
    
    /* Simple struct member access */
    sum += *ptr;                    /* Line 14: Simple reg from struct */
    sum += ptr[0];                  /* Line 15: Another simple reg */
    
    /* Loop through struct array */
    for (int i = 0; i < data->count && i < 16; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Local test array */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2 + 5;
    }
    
    /* Struct for testing */
    struct Data data;
    for (int i = 0; i < 16; i++) {
        data.values[i] = i * 7 + 3;
    }
    data.count = 16;
    
    /* Run all tests */
    result += test_simple_param(local_array, 20);
    result += test_global_access();
    result += test_array_traversal(local_array, 100);
    result += test_conditional_access(local_array, 50);
    result += test_struct_access(&data);
    
    /* Use volatile to prevent optimization */
    volatile int final_result = result;
    
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
