/* test-auto-inc-dec.c
 * 
 * This program is designed to trigger the specific uncovered block in GCC's
 * auto-inc-dec.cc optimization pass. The block handles memory operands with
 * simple register addressing (no offset) and attempts to convert them to
 * auto-increment/decrement forms.
 * 
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test-auto-inc-dec.c -o test
 * For ARM targets: gcc -O2 -mcpu=cortex-a57 -fno-inline test-auto-inc-dec.c -o test-arm
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable various access patterns */
volatile int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing patterns
 * This should generate a simple register indirect access (*p) which
 * matches the pattern: mem_insn.reg1_is_const = true, mem_insn.reg1_val = 0
 */
int test_simple_param(int *p, int n) {
    int sum = 0;
    
    /* Simple register indirect - target for uncovered block */
    sum += *p;                 /* This should be XEXP(x,0) with offset 0 */
    
    /* Register + constant offset */
    sum += p[5];               /* Different pattern to trigger find_inc_dec */
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = p;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;         /* Sequential access pattern */
    }
    
    /* Another simple register indirect in the middle */
    sum += *p;                 /* Another chance for the uncovered block */
    
    return sum;
}

/* Test 2: Local pointer to global with zero offset access
 * The compiler may treat global access differently
 */
int test_global_access(void) {
    int *p = &global_arr[0];
    int sum = 0;
    
    /* Simple register indirect from global pointer */
    sum += p[0];               /* Equivalent to *p - zero offset */
    
    /* Mixed with offset access */
    sum += p[10];
    
    /* Loop through part of global array */
    for (int i = 0; i < 20; i++) {
        sum += *p++;
    }
    
    /* Reset and do another simple access */
    p = &global_arr[50];
    sum += *p;                 /* Another simple register indirect */
    
    return sum;
}

/* Test 3: Array access via pointer with conditional simple access
 * Conditional flow might affect how addresses are analyzed
 */
int test_conditional_access(int *arr, int n, int flag) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Most iterations use pointer increment */
        sum += *ptr++;
        
        /* Occasionally do a simple register indirect on base */
        if (flag && (i % 7 == 0)) {
            sum += *arr;       /* Simple register indirect in loop */
        }
    }
    
    /* Final simple access */
    sum += *arr;               /* Post-loop simple access */
    
    return sum;
}

/* Test 4: Multiple simple register accesses in same function
 * Increases probability of hitting the block
 */
int test_multiple_simple_accesses(int *p1, int *p2, int *p3) {
    int sum = 0;
    
    /* Three separate simple register indirects */
    sum += *p1;                /* First chance */
    sum += *p2;                /* Second chance */
    sum += *p3;                /* Third chance */
    
    /* Mix with offset access */
    sum += p1[3];
    sum += p2[7];
    
    /* Another round of simple accesses */
    sum += *p1;                /* Fourth chance */
    
    return sum;
}

/* Test 5: Struct access with pointer - different context */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    int *p = d->values;
    
    /* Simple register indirect to struct member */
    sum += *p;                 /* Simple access via pointer */
    
    /* Access with offset */
    sum += p[5];
    
    /* Loop through struct array */
    for (int i = 0; i < 10; i++) {
        sum += *p++;
    }
    
    /* Final simple access */
    p = d->values;
    sum += *p;                 /* Another simple access */
    
    return sum + d->count;     /* Include non-array member */
}

/* Driver function to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int local_arr[50];
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i * 2;
        if (i < 100) global_arr[i] = i * 3;
    }
    
    /* Initialize struct */
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 5;
    }
    data.count = 42;
    
    /* Run all tests to generate various addressing patterns */
    result += test_simple_param(local_arr, 10);
    result += test_global_access();
    result += test_conditional_access(local_arr, 15, 1);
    
    /* Create multiple pointers for test 4 */
    int *p1 = &local_arr[0];
    int *p2 = &local_arr[10];
    int *p3 = &local_arr[20];
    result += test_multiple_simple_accesses(p1, p2, p3);
    
    result += test_struct_access(&data);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Also use volatile to ensure memory accesses aren't optimized away */
    volatile int *volatile_ptr = local_arr;
    result += *volatile_ptr;   /* Volatile simple access */
    
    return result != 0 ? 0 : 1;
}
