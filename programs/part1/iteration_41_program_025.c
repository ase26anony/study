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
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;  /* Line 1: mem_insn.reg0 = XEXP(x,0), reg1_is_const = true, reg1_val = 0 */
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access in the middle */
    sum += *base;  /* Line 2: Another chance to trigger the block */
    
    return sum;
}

/* Test 2: Local pointer to global array */
int test_global_access(void) {
    int *p = &global_arr[0];
    int sum = 0;
    
    /* Simple register access to global */
    sum += *p;  /* Should trigger the block */
    
    /* Mixed access patterns */
    sum += p[10];
    sum += p[20];
    
    /* Simple access again */
    sum += *p;
    
    return sum;
}

/* Test 3: Array traversal with conditional simple access */
int test_conditional_access(int *arr, int n) {
    int sum = 0;
    int *simple_ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing in loop */
        sum += arr[i];
        
        /* Conditional simple register access */
        if (i % 3 == 0) {
            sum += *simple_ptr;  /* Should trigger the block when condition is true */
        }
        
        /* Another addressing mode */
        sum += *(arr + i);
    }
    
    /* Final simple access */
    sum += *arr;
    
    return sum;
}

/* Test 4: Multiple simple accesses in different contexts */
int test_multiple_simple(int *p1, int *p2, int *p3) {
    int sum = 0;
    
    /* Three different simple register accesses */
    sum += *p1;  /* Should trigger */
    sum += *p2;  /* Should trigger */
    sum += *p3;  /* Should trigger */
    
    /* Add some offset accesses for variety */
    sum += p1[2];
    sum += p2[4];
    sum += p3[6];
    
    /* More simple accesses */
    sum += *p1;
    
    return sum;
}

/* Test 5: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    
    /* Simple access to struct member */
    sum += data->count;  /* This might be decomposed to *(&data->count) */
    
    /* Access array through pointer */
    int *ptr = data->values;
    sum += *ptr;  /* Simple register access */
    
    /* Loop with mixed access */
    for (int i = 0; i < 10; i++) {
        sum += ptr[i];
        if (i == 5) {
            sum += *ptr;  /* Simple access inside loop */
        }
    }
    
    return sum;
}

/* Test 6: Volatile pointer to ensure no dead code elimination */
int test_volatile_access(volatile int *base) {
    int sum = 0;
    
    /* Simple access through volatile pointer */
    sum += *base;
    
    /* Mixed access */
    sum += base[3];
    sum += base[7];
    
    /* Another simple access */
    sum += *base;
    
    return sum;
}

/* Main driver to call all test functions */
int main(void) {
    /* Initialize test arrays */
    int local_arr[50];
    int arr2[30];
    int arr3[40];
    
    for (int i = 0; i < 50; i++) local_arr[i] = i;
    for (int i = 0; i < 30; i++) arr2[i] = i * 2;
    for (int i = 0; i < 40; i++) arr3[i] = i * 3;
    for (int i = 0; i < 100; i++) global_arr[i] = i;
    
    struct Data data;
    for (int i = 0; i < 20; i++) data.values[i] = i * 5;
    data.count = 20;
    
    volatile int volatile_val = 42;
    
    /* Call all test functions with various parameters */
    int total = 0;
    
    total += test_simple_param(local_arr, 10);
    total += test_global_access();
    total += test_conditional_access(local_arr, 15);
    total += test_multiple_simple(local_arr, arr2, arr3);
    total += test_struct_access(&data);
    total += test_volatile_access(&volatile_val);
    
    /* Also test with NULL checks to add more control flow */
    if (local_arr) {
        total += *local_arr;  /* Simple access in conditional */
    }
    
    printf("Total checksum: %d\n", total);
    
    /* Return non-zero if total is non-zero (should always be true) */
    return total != 0 ? 0 : 1;
}
