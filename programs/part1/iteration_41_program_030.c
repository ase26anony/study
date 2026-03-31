/* test_auto_inc_dec.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec.cc
 * Lines 1352-1358: Simple register addressing with zero offset
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing global pointer access */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;                    /* Line: mem_insn.reg1_is_const = true; mem_insn.reg1_val = 0; */
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;              /* Another candidate for simple register addressing */
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Simple register access to global */
    sum += p[0];                     /* Should use simple register addressing */
    
    /* Mixed accesses */
    sum += p[10];
    sum += p[20];
    
    /* Loop through global array */
    for (int i = 0; i < 50; i++) {
        sum += *p++;
    }
    
    /* Reset and simple access again */
    p = &global_arr[30];
    sum += *p;                       /* Simple register indirect */
    
    return sum;
}

/* Test 3: Array with conditional simple access */
int test_conditional_access(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Conditional simple register access */
        if (i % 3 == 0) {
            sum += *ptr;             /* Simple register access inside loop */
        } else {
            sum += ptr[i % 5];       /* Offset access */
        }
        
        /* Pointer increment pattern */
        if (i % 2 == 0) {
            sum += *ptr++;
        }
    }
    
    /* Final simple access */
    int *final_ptr = arr + n/2;
    sum += *final_ptr;               /* Another simple register access */
    
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
    sum += *p;                       /* Should be simple register addressing */
    
    /* Access with offset */
    sum += p[5];
    sum += p[10];
    
    /* Loop through struct array */
    for (int i = 0; i < d->count && i < 20; i++) {
        sum += *p++;
    }
    
    /* Reset and simple access */
    p = &d->values[15];
    sum += *p;                       /* Simple register indirect */
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    volatile int *vptr = base;
    
    /* Simple volatile register access */
    sum += *vptr;                    /* Should still use simple addressing */
    
    /* Mixed volatile accesses */
    sum += vptr[3];
    
    /* Loop with volatile */
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    
    /* Another simple volatile access */
    vptr = base + 7;
    sum += *vptr;
    
    return sum;
}

/* Test 6: Multiple simple accesses in sequence */
int test_multiple_simple(int *a, int *b, int *c) {
    int sum = 0;
    
    /* Sequence of simple register accesses */
    sum += *a;                       /* Simple */
    sum += *b;                       /* Simple */
    sum += *c;                       /* Simple */
    
    /* Add some offset accesses */
    sum += a[2];
    sum += b[3];
    sum += c[4];
    
    /* More simple accesses */
    int *tmp = a + 8;
    sum += *tmp;                     /* Simple */
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test arrays */
    int arr1[100];
    int arr2[50];
    struct Data data = {.count = 15};
    
    for (int i = 0; i < 100; i++) {
        arr1[i] = i;
        global_arr[i] = i * 2;
        if (i < 20) data.values[i] = i * 3;
    }
    
    for (int i = 0; i < 50; i++) {
        arr2[i] = i * 4;
    }
    
    /* Run all tests to generate various addressing patterns */
    result += test_simple_param(arr1, 20);
    result += test_global_access();
    result += test_conditional_access(arr2, 30);
    result += test_struct_access(&data);
    result += test_volatile_access(arr1, 10);
    result += test_multiple_simple(arr1, arr2, data.values);
    
    /* Additional test: nested function calls */
    for (int i = 0; i < 5; i++) {
        int *ptr = arr1 + i * 5;
        result += *ptr;              /* Simple access in loop */
    }
    
    printf("Final checksum: %d\n", result);
    
    /* Return non-zero to indicate execution */
    return result != 0 ? 0 : 1;
}
