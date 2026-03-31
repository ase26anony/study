/* Test program to trigger auto-increment/decrement optimization coverage
 * Specifically targets the uncovered block in find_inc_dec function
 * that handles simple register addressing modes with zero offset.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing global pointer access */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing patterns */
int test_simple_mixed(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;  /* XEXP(x,0) is just the register 'base' */
    
    /* Register + constant offset */
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

/* Test 2: Local pointer to array with zero offset access */
int test_local_array(void) {
    int local_arr[50];
    
    /* Initialize array */
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i * 2;
    }
    
    int sum = 0;
    int *p = local_arr;  /* Simple pointer to array start */
    
    /* Multiple simple register accesses */
    sum += p[0];    /* Should be compiled as simple register + 0 offset */
    sum += *p;      /* Direct dereference - another simple register access */
    
    /* Mixed with offset access */
    sum += p[10];
    
    /* Loop with auto-increment pattern */
    int *q = p;
    for (int i = 0; i < 10; i++) {
        sum += *q++;
    }
    
    return sum;
}

/* Test 3: Global array access via local pointer */
int test_global_access(void) {
    int *gp = &global_arr[0];
    int sum = 0;
    
    /* Simple register access to global */
    sum += *gp;      /* Should trigger simple register addressing */
    
    /* Offset access */
    sum += gp[20];
    
    /* Loop through global with pointer */
    for (int i = 0; i < 25; i++) {
        sum += *gp++;
    }
    
    return sum;
}

/* Test 4: Conditional simple access inside loop */
int test_conditional_simple(int *base, int n) {
    int sum = 0;
    int *ptr = base;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing in most iterations */
        sum += ptr[i];
        
        /* But occasionally use simple register access */
        if (i % 7 == 0) {
            int *simple = ptr + i;
            sum += *simple;  /* Simple register access inside loop */
        }
    }
    
    /* Final simple access */
    sum += *ptr;
    
    return sum;
}

/* Test 5: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    
    /* Simple access to struct member via pointer */
    sum += d->count;  /* This involves address calculation but may simplify */
    
    /* Get pointer to array inside struct */
    int *arr_ptr = d->values;
    
    /* Simple register access to struct member array */
    sum += *arr_ptr;      /* Simple register + 0 offset */
    sum += arr_ptr[0];    /* Same as above, different syntax */
    
    /* Offset access */
    sum += arr_ptr[10];
    
    return sum;
}

/* Test 6: Volatile pointer to ensure no dead code elimination */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    
    /* Simple volatile register access */
    sum += *base;
    
    /* Loop with volatile pointer */
    volatile int *vptr = base;
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    
    /* Another simple access */
    volatile int *simple_vol = base + 5;
    sum += *simple_vol;
    
    return sum;
}

/* Main driver to call all tests and ensure code generation */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int test_array[100];
    for (int i = 0; i < 100; i++) {
        test_array[i] = i + 1;
        global_arr[i] = i * 3;
    }
    
    /* Initialize struct */
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 5;
    }
    data.count = 42;
    
    /* Run all tests to generate various addressing patterns */
    result += test_simple_mixed(test_array, 20);
    result += test_local_array();
    result += test_global_access();
    result += test_conditional_simple(test_array, 30);
    result += test_struct_access(&data);
    result += test_volatile_access(test_array, 15);
    
    /* Print result to prevent optimization */
    printf("Final checksum: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
