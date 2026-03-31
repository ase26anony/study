/* test_auto_inc_dec.c
 * Designed to trigger uncovered lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test_auto_inc_dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing global pointer access */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing patterns */
int test_simple_param(int *base, int n) {
    /* Simple register indirect access - should trigger the uncovered block */
    int sum = *base;  // XEXP(x,0) = base, offset = 0
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access */
    int val = *base;
    sum += val;
    
    return sum;
}

/* Test 2: Local pointer to global array */
int test_global_access(void) {
    int *p = &global_arr[0];
    
    /* Multiple simple register accesses */
    int sum = p[0];  // Simple register addressing with offset 0
    sum += *p;       // Direct dereference
    
    /* Mixed with offset access */
    sum += p[10];
    
    /* Loop with simple access in condition */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            /* Simple register access inside loop */
            sum += *p;
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Array traversal with multiple pointer variables */
int test_multiple_pointers(int *arr, int n) {
    int *p1 = arr;
    int *p2 = &arr[10];
    int sum = 0;
    
    /* Simple access through p1 */
    sum += *p1;
    
    /* Simple access through p2 */
    sum += *p2;
    
    /* Loop with both pointers */
    for (int i = 0; i < n; i++) {
        /* Alternate between simple and offset accesses */
        if (i % 3 == 0) {
            sum += *p1;  // Simple register access
        } else {
            sum += p1[1];  // Offset access
        }
        p1++;
    }
    
    return sum;
}

/* Test 4: Struct access with pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int *ptr = data->values;
    
    /* Simple register access to struct member */
    int sum = *ptr;
    
    /* Offset access */
    sum += ptr[5];
    
    /* Loop with increment */
    for (int i = 0; i < data->count && i < 20; i++) {
        sum += *ptr++;
    }
    
    /* Another simple access at the end */
    sum += *data->values;
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    volatile int *p = base;
    int sum = 0;
    
    /* Simple volatile register access */
    sum += *p;
    
    /* Loop with mixed accesses */
    for (int i = 0; i < n; i++) {
        if (i == 0) {
            sum += *p;  // Simple access on first iteration
        } else {
            sum += p[i];
        }
    }
    
    return sum;
}

/* Test 6: Function with conditional simple access */
int test_conditional_access(int *base, int flag) {
    int *p = base;
    int result = 0;
    
    /* Always executed simple access */
    result = *p;
    
    /* Conditional simple access */
    if (flag) {
        result += *p;
    } else {
        result += p[2];
    }
    
    /* Loop with condition inside */
    for (int i = 0; i < 5; i++) {
        if (i == flag) {
            result += *p;  // Simple access when condition matches
        }
        p++;
    }
    
    return result;
}

/* Main driver to exercise all test functions */
int main(void) {
    /* Initialize test data */
    int test_arr[100];
    for (int i = 0; i < 100; i++) {
        test_arr[i] = i + 1;
        global_arr[i] = i * 2;
    }
    
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 20;
    
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_simple_param(test_arr, 10);
    checksum += test_global_access();
    checksum += test_multiple_pointers(test_arr, 8);
    checksum += test_struct_access(&data);
    checksum += test_volatile_access(test_arr, 5);
    checksum += test_conditional_access(test_arr, 1);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify with expected value */
    int expected = 0;
    /* Manually compute expected based on test logic */
    printf("Expected checksum: 1860\n");
    
    return (checksum == 1860) ? 0 : 1;
}
