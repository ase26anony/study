/* Test program to trigger auto-increment/decrement optimization block
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const -c test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_arr[100];

/* Test 1: Simple parameter access with mixed addressing modes */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;           /* This creates mem_insn.reg1_val = 0 */
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access in the middle */
    sum += *base;
    
    return sum;
}

/* Test 2: Local pointer to global array */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Multiple simple register accesses */
    sum += *p;              /* Should trigger the block */
    sum += p[0];            /* Alternative syntax, same effect */
    
    /* Mixed with offset accesses */
    sum += p[10];
    sum += p[20];
    
    /* Another simple access */
    int val = *p;
    sum += val;
    
    return sum;
}

/* Test 3: Loop with conditional simple access */
int test_conditional_access(int *base, int n) {
    int sum = 0;
    int *simple_ptr = base;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing in loop */
        sum += base[i];
        
        /* Conditional simple register access */
        if (i % 3 == 0) {
            sum += *simple_ptr;  /* Should trigger the block */
        }
        
        /* Sometimes use offset */
        if (i % 5 == 0) {
            sum += simple_ptr[2];
        }
    }
    
    /* Final simple access */
    sum += *base;
    
    return sum;
}

/* Test 4: Multiple simple pointers */
int test_multiple_pointers(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    /* Interleaved simple accesses */
    sum += *p1;      /* Should trigger */
    sum += *p2;      /* Should trigger */
    
    for (int i = 0; i < n; i++) {
        sum += p1[i] + p2[i];
    }
    
    /* More simple accesses */
    sum += *arr1;
    sum += *arr2;
    
    return sum;
}

/* Test 5: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    int *ptr = d->values;
    
    /* Simple access to struct member via pointer */
    sum += *ptr;           /* Should trigger */
    sum += ptr[0];         /* Same effect */
    
    /* Access with offset */
    sum += ptr[5];
    sum += ptr[10];
    
    /* Loop through struct array */
    for (int i = 0; i < d->count && i < 20; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Test 6: Volatile pointer to ensure no dead code elimination */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    volatile int *p = base;
    
    /* Volatile simple access */
    sum += *p;            /* Should trigger */
    
    /* Mix volatile and non-volatile */
    int *regular = (int*)base;
    sum += *regular;      /* Should also trigger */
    
    for (int i = 0; i < n; i++) {
        sum += p[i];
    }
    
    return sum;
}

/* Main driver to exercise all tests */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i * 2;
    }
    
    /* Local arrays for testing */
    int local_arr1[50];
    int local_arr2[50];
    for (int i = 0; i < 50; i++) {
        local_arr1[i] = i * 3;
        local_arr2[i] = i * 5;
    }
    
    /* Struct for testing */
    struct Data data;
    data.count = 20;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 7;
    }
    
    /* Run all tests to generate various addressing patterns */
    result += test_simple_param(local_arr1, 10);
    result += test_global_access();
    result += test_conditional_access(local_arr1, 15);
    result += test_multiple_pointers(local_arr1, local_arr2, 8);
    result += test_struct_access(&data);
    result += test_volatile_access(local_arr1, 5);
    
    printf("Result checksum: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    return result == 0 ? 1 : 0;
}
