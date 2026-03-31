/* test_auto_inc_dec.c
 * Designed to trigger uncovered lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test_auto_inc_dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable multiple access patterns */
int global_arr[256];

/* Test 1: Simple parameter access with mixed patterns */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;                     /* Line 1: Simple register addressing */
    
    /* Register + constant offset */
    sum += base[5];                   /* Line 2: Offset addressing */
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;                /* Line 3: Auto-increment pattern */
    }
    
    /* Another simple register access */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;               /* Line 4: Another simple register */
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Multiple simple register accesses */
    sum += p[0];                      /* Line 1: p[0] is *(p + 0) */
    sum += *p;                        /* Line 2: Direct dereference */
    
    /* Mixed with offset */
    sum += p[10];
    sum += p[20];
    
    /* Loop with conditional simple access */
    for (int i = 0; i < 50; i++) {
        if (i % 5 == 0) {
            sum += *p;                /* Line 3: Simple access in loop */
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Array traversal with multiple pointer variables */
int test_multiple_pointers(int *arr, int size) {
    int sum = 0;
    int *p1 = arr;
    int *p2 = arr + size/2;
    
    /* Simple accesses from different pointers */
    sum += *p1;                       /* Line 1: Simple from p1 */
    sum += *p2;                       /* Line 2: Simple from p2 */
    
    /* Offset accesses */
    sum += p1[3];
    sum += p2[3];
    
    /* Two loops with different patterns */
    for (int i = 0; i < size/2; i++) {
        sum += *p1++;                 /* Auto-increment */
        sum += p2[i];                 /* Indexed access */
    }
    
    /* Final simple access */
    int *final_ptr = p1;
    sum += *final_ptr;                /* Line 3: Simple from final_ptr */
    
    return sum;
}

/* Test 4: Struct access with pointer */
struct Data {
    int values[32];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *ptr = data->values;
    
    /* Simple struct member access */
    sum += data->count;               /* Different pattern */
    
    /* Simple pointer access to array inside struct */
    sum += *ptr;                      /* Line 1: Simple register */
    sum += ptr[0];                    /* Line 2: Same as above */
    
    /* Loop through struct array */
    for (int i = 0; i < 16; i++) {
        sum += *ptr++;
    }
    
    /* Reset and simple access */
    ptr = data->values + 8;
    sum += *ptr;                      /* Line 3: Simple after reset */
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    
    /* Volatile simple access */
    sum += *base;                     /* Line 1: Volatile simple */
    
    /* Non-volatile pointer derived from volatile */
    int *p = (int*)base;
    sum += *p;                        /* Line 2: Simple from derived */
    
    /* Mixed access patterns */
    for (int i = 0; i < n; i++) {
        sum += p[i];                  /* Indexed */
        if (i % 3 == 0) {
            sum += *p;                /* Line 3: Simple in loop */
        }
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_arr[i] = i;
    }
    
    /* Test array for parameter tests */
    int test_arr[100];
    for (int i = 0; i < 100; i++) {
        test_arr[i] = i * 2;
    }
    
    /* Struct for testing */
    struct Data data;
    for (int i = 0; i < 32; i++) {
        data.values[i] = i * 3;
    }
    data.count = 32;
    
    /* Run all tests */
    result += test_simple_param(test_arr, 20);
    result += test_global_access();
    result += test_multiple_pointers(test_arr, 100);
    result += test_struct_access(&data);
    result += test_volatile_access(test_arr, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Additional forced execution with different alignments */
    int *aligned_ptr = (int*)(((size_t)test_arr + 15) & ~15);
    if (aligned_ptr < test_arr + 100) {
        result += *aligned_ptr;  /* One more simple access */
    }
    
    return result != 0 ? 0 : 1;
}
