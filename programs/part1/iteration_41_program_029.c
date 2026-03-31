/* Test program to trigger auto-increment/decrement optimization block
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const -o test test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_arr[100];

/* Test 1: Simple parameter access with mixed addressing */
int test_simple_mixed(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
    sum += *base;
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Simple register indirect from global */
    sum += *p;
    
    /* Mixed access patterns */
    sum += p[0];  /* Another simple access */
    sum += p[20]; /* Offset access */
    
    /* Loop through global */
    for (int i = 0; i < 50; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 3: Conditional simple access inside loop */
int test_conditional_access(int *base, int n) {
    int sum = 0;
    int *ptr = base;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing in loop */
        sum += ptr[i];
        
        /* Conditional simple register access */
        if (i % 3 == 0) {
            int *simple = &ptr[i];
            sum += *simple;  /* Should trigger simple register case */
        }
    }
    
    /* Final simple access */
    sum += *ptr;
    
    return sum;
}

/* Test 4: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    
    /* Simple access to struct member */
    sum += d->count;
    
    /* Pointer to array inside struct */
    int *p = d->values;
    
    /* Simple register indirect */
    sum += *p;
    
    /* Mixed access patterns */
    sum += p[0];  /* Another simple */
    sum += p[10]; /* Offset */
    
    /* Loop with increment */
    for (int i = 0; i < 10; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 5: Volatile pointer to force generation */
int test_volatile_access(int *regular, volatile int *volatile_ptr) {
    int sum = 0;
    
    /* Simple access through volatile - compiler can't optimize away */
    sum += *volatile_ptr;
    
    /* Mixed with regular pointer */
    sum += *regular;
    sum += regular[5];
    
    /* Loop with both */
    for (int i = 0; i < 10; i++) {
        sum += *regular++;
        sum += *volatile_ptr;
    }
    
    return sum;
}

/* Test 6: Nested pointer indirection */
int test_nested_pointers(int **ptr_ptr) {
    int sum = 0;
    
    /* Dereference to get simple pointer */
    int *simple = *ptr_ptr;
    
    /* Now use simple register indirect */
    sum += *simple;
    
    /* Also use the double indirect */
    sum += **ptr_ptr;
    
    /* Loop with the simple pointer */
    for (int i = 0; i < 5; i++) {
        sum += *simple++;
    }
    
    return sum;
}

/* Driver function */
int main(void) {
    /* Initialize test data */
    int local_arr[100];
    struct Data data;
    int *ptr_to_local = local_arr;
    volatile int volatile_var = 42;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        local_arr[i] = i;
        global_arr[i] = i * 2;
        if (i < 20) data.values[i] = i * 3;
    }
    data.count = 100;
    
    int total_sum = 0;
    
    /* Run all tests to trigger different compilation paths */
    total_sum += test_simple_mixed(local_arr, 20);
    total_sum += test_global_access();
    total_sum += test_conditional_access(local_arr, 30);
    total_sum += test_struct_access(&data);
    total_sum += test_volatile_access(local_arr, &volatile_var);
    total_sum += test_nested_pointers(&ptr_to_local);
    
    /* Use result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    
    return total_sum != 0 ? 0 : 1;
}
