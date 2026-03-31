/* test_auto_inc_dec.c
 * Designed to trigger uncovered lines 1352-1358 in gcc/auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test_auto_inc_dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to ensure memory accesses aren't optimized away */
volatile int global_array[100] = {0};

/* Test 1: Simple register indirect access with mixed patterns */
int test_simple_register(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
    sum += *base;                    /* Line 1: Simple register access */
    
    /* Register + constant offset */
    sum += base[5];                  /* Line 2: Offset access */
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;               /* Line 3: Auto-increment pattern */
    }
    
    /* Another simple register access after loop */
    sum += *base;                    /* Line 4: Another simple access */
    
    return sum;
}

/* Test 2: Function parameter used directly */
int load_param(int *p) {
    /* Multiple simple register accesses */
    int val1 = *p;                   /* Simple register indirect */
    int val2 = *p;                   /* Another simple access */
    return val1 + val2;
}

/* Test 3: Global array access via local pointer */
int test_global() {
    int *p = &global_array[0];
    int sum = 0;
    
    /* Simple register indirect from global */
    sum += *p;                       /* Simple register access */
    
    /* Mixed with offset */
    sum += p[10];                    /* Offset access */
    
    /* Another simple access */
    sum += *p;                       /* Another simple access */
    
    return sum;
}

/* Test 4: Conditional simple access inside loop */
int test_conditional(int *base, int n) {
    int sum = 0;
    int *simple_ptr = base;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing in loop */
        sum += base[i * 2];          /* Scaled index */
        
        /* Conditional simple register access */
        if (i % 3 == 0) {
            sum += *simple_ptr;      /* Simple register inside condition */
        }
        
        /* Pointer arithmetic */
        simple_ptr++;
    }
    
    /* Final simple access */
    sum += *base;                    /* Simple register after loop */
    
    return sum;
}

/* Test 5: Array of pointers with simple dereference */
int test_pointer_array(int **ptr_array, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Simple register indirect through array element */
        if (ptr_array[i]) {
            sum += *ptr_array[i];    /* Simple register access */
        }
    }
    
    return sum;
}

/* Test 6: Struct access with simple pointer */
struct Data {
    int values[10];
    int count;
};

int test_struct(struct Data *data) {
    int sum = 0;
    
    /* Simple register indirect to struct member */
    sum += data->count;              /* Simple access via pointer */
    
    /* Access array within struct */
    int *p = data->values;
    sum += *p;                       /* Simple register indirect */
    sum += p[5];                     /* Offset access */
    
    return sum;
}

/* Test 7: Multiple simple accesses in sequence */
int test_sequence(int *p1, int *p2, int *p3) {
    /* Sequence of simple register accesses */
    int val1 = *p1;                  /* Simple access 1 */
    int val2 = *p2;                  /* Simple access 2 */
    int val3 = *p3;                  /* Simple access 3 */
    
    /* Mixed with offset */
    val1 += p1[2];                   /* Offset access */
    
    return val1 + val2 + val3;
}

/* Main driver to ensure all code is executed */
int main() {
    int result = 0;
    
    /* Initialize test data */
    int test_array[100];
    for (int i = 0; i < 100; i++) {
        test_array[i] = i;
        global_array[i] = i * 2;
    }
    
    /* Initialize pointer array */
    int *ptr_array[10];
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = &test_array[i * 5];
    }
    
    /* Initialize struct */
    struct Data data;
    for (int i = 0; i < 10; i++) {
        data.values[i] = i * 3;
    }
    data.count = 42;
    
    /* Run all tests to generate various addressing patterns */
    result += test_simple_register(test_array, 10);
    result += load_param(&test_array[20]);
    result += test_global();
    result += test_conditional(test_array, 15);
    result += test_pointer_array(ptr_array, 10);
    result += test_struct(&data);
    
    /* Additional pointers for sequence test */
    result += test_sequence(&test_array[0], &test_array[30], &test_array[60]);
    
    /* Add more variations */
    volatile int *volatile_ptr = test_array;
    result += *volatile_ptr;         /* Volatile simple access */
    
    /* Loop with mixed simple and complex accesses */
    int *loop_ptr = test_array;
    for (int i = 0; i < 5; i++) {
        result += *loop_ptr;         /* Simple in loop */
        result += loop_ptr[i];       /* Indexed in loop */
        loop_ptr += 2;
    }
    
    printf("Result: %d\n", result);
    
    /* Return non-zero to prevent optimization */
    return result != 0 ? 0 : 1;
}
