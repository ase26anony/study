/* Test program to trigger auto-increment/decrement optimization coverage
 * Specifically targets the uncovered block in find_inc_dec function
 * where mem_insn is set up for simple register addressing with zero offset
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing global pointer access */
int global_array[100];

/* Test 1: Simple parameter access with mixed addressing patterns */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;  /* This is XEXP(x, 0) with no offset */
    
    /* Register + constant offset access */
    sum += base[5];
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access in the middle */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;  /* Another candidate for simple register addressing */
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_array[0];
    
    /* Multiple simple register accesses */
    sum += p[0];    /* Should be compiled as simple register indirect */
    sum += *p;      /* Another simple access */
    
    /* Mixed with offset access */
    sum += p[10];
    sum += p[20];
    
    /* Loop through global array */
    for (int i = 0; i < 50; i++) {
        sum += *p++;
    }
    
    /* Final simple access */
    p = &global_array[99];
    sum += *p;
    
    return sum;
}

/* Test 3: Struct access with pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *ptr = data->values;
    
    /* Simple register access to struct member */
    sum += data->count;  /* Might be simple register depending on struct layout */
    
    /* Simple register access through pointer */
    sum += *ptr;
    
    /* Loop with increment */
    for (int i = 0; i < 10; i++) {
        sum += *ptr++;
    }
    
    /* Conditional simple access */
    if (sum > 100) {
        int *temp = &data->values[5];
        sum += *temp;  /* Simple register access inside condition */
    }
    
    return sum;
}

/* Test 4: Multiple simple accesses in different contexts */
int test_mixed_simple(int *arr, int n) {
    int sum = 0;
    
    /* Array of pointers to force different addressing modes */
    int *ptrs[5];
    for (int i = 0; i < 5; i++) {
        ptrs[i] = arr + i * 3;
    }
    
    /* Access through each pointer - each could be simple register */
    for (int i = 0; i < 5; i++) {
        sum += *ptrs[i];  /* Simple register indirect through array element */
    }
    
    /* Nested loops with simple access */
    for (int i = 0; i < n; i++) {
        int *current = arr + i;
        for (int j = 0; j < 3; j++) {
            sum += *current;  /* Simple access in inner loop */
            current++;
        }
    }
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base) {
    int sum = 0;
    
    /* Volatile simple access */
    sum += *base;
    
    /* Non-volatile pointer derived from volatile */
    int *derived = (int *)base;
    sum += derived[0];  /* Simple register access */
    
    /* Loop with mixed volatile and non-volatile */
    for (int i = 0; i < 10; i++) {
        sum += *base;      /* Volatile simple access */
        sum += derived[i]; /* Non-volatile with offset */
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2;
    }
    
    /* Local array for testing */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 3;
    }
    
    /* Initialize struct */
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 5;
    }
    data.count = 42;
    
    /* Run all tests to generate various addressing patterns */
    result += test_simple_param(local_array, 20);
    result += test_global_access();
    result += test_struct_access(&data);
    result += test_mixed_simple(local_array, 10);
    result += test_volatile_access(local_array);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
