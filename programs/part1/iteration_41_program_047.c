/* Test program to trigger auto-increment/decrement optimization block
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const -c test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable multiple access patterns */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed patterns */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - target for uncovered block */
    sum += *base;  /* This should generate mem: (reg) addressing */
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access in the middle */
    if (n > 0) {
        sum += *base;  /* Another candidate for simple register addressing */
    }
    
    return sum;
}

/* Test 2: Local pointer to global with zero offset */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];  /* Simple pointer to array start */
    
    /* Multiple simple register accesses */
    sum += p[0];  /* Should be (reg) with offset 0 */
    sum += *p;    /* Another simple (reg) access */
    
    /* Mixed with offset access */
    sum += p[10];
    
    /* Loop with mixed addressing */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sum += *p;  /* Simple register access inside loop */
        } else {
            sum += p[i];  /* Register + offset */
        }
    }
    
    return sum;
}

/* Test 3: Pointer arithmetic with zero offset check */
int test_pointer_arithmetic(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Force compiler to consider different addressing modes */
    for (int i = 0; i < n; i++) {
        /* Conditional simple register access */
        if (i == 0) {
            sum += *ptr;  /* Simple (reg) addressing */
        }
        
        /* Normal sequential access */
        sum += *ptr++;
        
        /* Reset to simple register occasionally */
        if (i % 5 == 0) {
            ptr = arr;  /* Back to base */
            sum += *ptr;  /* Simple register again */
            ptr = arr + i + 1;
        }
    }
    
    return sum;
}

/* Test 4: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    int *p = d->values;
    
    /* Simple register access to struct member */
    sum += *p;  /* p points to values[0] */
    
    /* Loop through struct array */
    for (int i = 0; i < d->count && i < 20; i++) {
        sum += *p++;
    }
    
    /* Return to simple register */
    p = d->values;
    sum += *p;
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    
    /* Volatile simple register access */
    sum += *base;
    
    /* Loop with volatile pointer */
    volatile int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* One more simple access */
    sum += *base;
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int local_arr[50];
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i + 1;
        if (i < 100) global_arr[i] = i * 2;
    }
    
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 20;
    
    /* Run all tests to generate various addressing patterns */
    result += test_simple_param(local_arr, 20);
    result += test_global_access();
    result += test_pointer_arithmetic(local_arr, 30);
    result += test_struct_access(&data);
    result += test_volatile_access(local_arr, 10);
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
