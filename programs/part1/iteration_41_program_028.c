/* Test program to trigger auto-increment/decrement optimization coverage
 * Specifically targets the uncovered block in find_inc_dec function
 * that handles simple register addressing with zero offset
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing patterns */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;  /* XEXP(x,0) is just a register, offset 0 */
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;  /* Another candidate for the uncovered block */
    
    return sum;
}

/* Test 2: Local pointer to global array */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Multiple simple register accesses */
    sum += p[0];  /* Simple register + 0 offset */
    sum += *p;    /* Another simple access */
    
    /* Mixed with offset access */
    sum += p[10];
    
    /* Loop with conditional simple access */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            sum += *p;  /* Simple access inside loop */
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Array traversal with multiple pointer variables */
int test_multiple_pointers(int *arr, int n) {
    int sum = 0;
    int *p1 = arr;
    int *p2 = arr + n/2;
    
    /* Simple accesses from different pointers */
    sum += *p1;  /* Should trigger uncovered block */
    sum += *p2;  /* Another candidate */
    
    /* Offset accesses */
    sum += p1[3];
    sum += p2[2];
    
    /* Two parallel loops with different patterns */
    for (int i = 0; i < n/2; i++) {
        sum += *p1++;
        sum += p2[i];
    }
    
    /* Final simple access */
    int *final_ptr = p1;
    sum += *final_ptr;
    
    return sum;
}

/* Test 4: Struct access with pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *ptr = data->values;
    
    /* Simple struct member access via pointer */
    sum += *ptr;  /* Simple register access */
    
    /* Loop through struct array */
    for (int i = 0; i < data->count && i < 20; i++) {
        sum += *ptr++;
    }
    
    /* Access with offset */
    sum += ptr[-1];
    
    /* Another simple access */
    int *simple = &data->values[5];
    sum += *simple;
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(int *regular, volatile int *volatile_ptr) {
    int sum = 0;
    
    /* Simple access through volatile pointer */
    sum += *volatile_ptr;
    
    /* Mixed with regular pointer */
    sum += *regular;
    sum += regular[5];
    
    /* Loop with both pointers */
    for (int i = 0; i < 10; i++) {
        sum += *volatile_ptr++;
        sum += regular[i];
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int local_arr[100];
    for (int i = 0; i < 100; i++) {
        local_arr[i] = i + 1;
        global_arr[i] = i * 2;
    }
    
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 20;
    
    volatile int volatile_arr[50];
    for (int i = 0; i < 50; i++) {
        volatile_arr[i] = i * 4;
    }
    
    /* Run all tests */
    result += test_simple_param(local_arr, 20);
    result += test_global_access();
    result += test_multiple_pointers(local_arr, 40);
    result += test_struct_access(&data);
    result += test_volatile_access(local_arr, volatile_arr);
    
    /* Print result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
