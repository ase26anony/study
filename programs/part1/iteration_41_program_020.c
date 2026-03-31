/* Test program to trigger auto-increment/decrement optimization coverage
 * Specifically targets the uncovered block in find_inc_dec function
 * where mem_insn is set up for simple register addressing with zero offset
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing global pointer access */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing patterns */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;  /* This is XEXP(x, 0) with no offset */
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access after loop */
    sum += *base;
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Simple register indirect from global pointer */
    sum += *p;  /* Should trigger the block */
    
    /* Mixed offsets */
    sum += p[10];
    sum += p[20];
    
    /* Loop with increment */
    for (int i = 0; i < 50; i++) {
        sum += *p++;
    }
    
    /* Reset and simple access again */
    p = &global_arr[0];
    sum += *p;
    
    return sum;
}

/* Test 3: Local array with volatile pointer to prevent optimization */
int test_volatile_access(int n) {
    volatile int *volatile_ptr;
    int local_arr[100];
    int sum = 0;
    
    /* Initialize local array */
    for (int i = 0; i < 100; i++) {
        local_arr[i] = i;
    }
    
    volatile_ptr = local_arr;
    
    /* Simple volatile register access */
    sum += *volatile_ptr;  /* Should trigger the block */
    
    /* Mixed pattern */
    sum += volatile_ptr[15];
    
    /* Loop with increment */
    for (int i = 0; i < n; i++) {
        sum += *volatile_ptr++;
    }
    
    return sum;
}

/* Test 4: Conditional simple access inside loop */
int test_conditional_access(int *base, int n) {
    int sum = 0;
    int *simple_ptr = base;
    int *inc_ptr = base;
    
    for (int i = 0; i < n; i++) {
        /* Conditional simple register access */
        if (i % 3 == 0) {
            sum += *simple_ptr;  /* Should trigger the block */
        }
        
        /* Regular increment access */
        sum += *inc_ptr++;
    }
    
    /* Final simple access */
    sum += *base;
    
    return sum;
}

/* Test 5: Struct access with simple pointer */
struct Data {
    int values[50];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *p = data->values;
    
    /* Simple struct member access via pointer */
    sum += *p;  /* Should trigger the block */
    
    /* Access with offset */
    sum += p[10];
    sum += p[data->count % 20];
    
    /* Loop through struct array */
    for (int i = 0; i < data->count && i < 50; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 6: Multiple simple accesses in same function */
int test_multiple_simple(int *p1, int *p2, int *p3) {
    int sum = 0;
    
    /* Three separate simple register accesses */
    sum += *p1;  /* Should trigger the block */
    sum += *p2;  /* Should trigger the block */
    sum += *p3;  /* Should trigger the block */
    
    /* Add some offset accesses */
    sum += p1[5];
    sum += p2[10];
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int test_arr[100];
    int result = 0;
    
    /* Initialize test arrays */
    for (int i = 0; i < 100; i++) {
        test_arr[i] = i + 1;
        global_arr[i] = i * 2;
    }
    
    /* Initialize struct */
    struct Data data;
    for (int i = 0; i < 50; i++) {
        data.values[i] = i * 3;
    }
    data.count = 25;
    
    /* Run all tests to generate various addressing patterns */
    result += test_simple_param(test_arr, 20);
    result += test_global_access();
    result += test_volatile_access(15);
    result += test_conditional_access(test_arr, 30);
    result += test_struct_access(&data);
    
    /* Test with multiple pointers */
    int *p1 = &test_arr[0];
    int *p2 = &test_arr[20];
    int *p3 = &test_arr[40];
    result += test_multiple_simple(p1, p2, p3);
    
    /* Additional loop with mixed patterns */
    int *base_ptr = test_arr;
    for (int i = 0; i < 10; i++) {
        /* Alternate between simple and offset access */
        if (i % 2 == 0) {
            result += *base_ptr;  /* Simple register access */
        } else {
            result += base_ptr[i];
        }
        base_ptr++;
    }
    
    printf("Final result: %d\n", result);
    return result != 0 ? 0 : 1;
}
