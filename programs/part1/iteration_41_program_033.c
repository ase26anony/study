/* Test program to trigger auto-increment/decrement optimization block
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const -c test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable various access patterns */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing */
int test_simple_mixed(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;  /* This creates mem_insn.reg1_is_const = true, reg1_val = 0 */
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access in the middle */
    sum += *base;  /* Another chance to hit the block */
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Simple register indirect from global pointer */
    sum += *p;  /* Should trigger the block */
    
    /* Mixed with offset */
    sum += p[10];
    
    /* Loop with increment */
    for (int i = 0; i < 20; i++) {
        sum += *p++;
    }
    
    /* Reset and simple access again */
    p = &global_arr[50];
    sum += *p;  /* Another chance */
    
    return sum;
}

/* Test 3: Conditional simple access inside loop */
int test_conditional_access(int *base, int n) {
    int sum = 0;
    int *simple_ptr = base;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing */
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
    sum += *simple_ptr;  /* Final chance */
    
    return sum;
}

/* Test 4: Multiple simple accesses in sequence */
int test_multiple_simple(int *p1, int *p2, int *p3) {
    int val = 0;
    
    /* Three consecutive simple register accesses */
    val += *p1;  /* Should trigger */
    val += *p2;  /* Should trigger */
    val += *p3;  /* Should trigger */
    
    /* Mix with offset */
    val += p1[1];
    val += p2[2];
    
    /* Back to simple */
    val += *p1;  /* Should trigger again */
    
    return val;
}

/* Test 5: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    
    /* Simple access to struct member */
    sum += d->count;  /* This may also create simple addressing */
    
    /* Pointer to array member */
    int *p = d->values;
    
    /* Simple register indirect */
    sum += *p;  /* Should trigger */
    
    /* Loop with increment */
    for (int i = 0; i < 10; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 6: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base) {
    int sum = 0;
    
    /* Simple volatile access */
    sum += *base;  /* Should trigger */
    
    /* Offset access */
    sum += base[3];
    
    /* Another simple */
    sum += *base;  /* Should trigger again */
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i;
    }
    
    /* Local arrays for testing */
    int local_arr1[50];
    int local_arr2[30];
    int local_arr3[40];
    
    for (int i = 0; i < 50; i++) local_arr1[i] = i * 2;
    for (int i = 0; i < 30; i++) local_arr2[i] = i * 3;
    for (int i = 0; i < 40; i++) local_arr3[i] = i * 4;
    
    struct Data data;
    for (int i = 0; i < 20; i++) data.values[i] = i * 5;
    data.count = 20;
    
    /* Run all tests to maximize coverage */
    result += test_simple_mixed(local_arr1, 20);
    result += test_global_access();
    result += test_conditional_access(local_arr2, 25);
    result += test_multiple_simple(local_arr1, local_arr2, local_arr3);
    result += test_struct_access(&data);
    result += test_volatile_access(local_arr1);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
