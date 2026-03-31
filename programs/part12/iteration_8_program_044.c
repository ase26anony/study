/* auto-inc-dec-test.c
 * Designed to trigger GCC's auto-increment/decrement pass
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Basic pointer arithmetic with zero offset in loops */
int test1_basic_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* Access with ptr + 0 pattern */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* This should generate (mem (plus (reg) (const_int 0))) */
        sum += *(ptr + 0);  /* Zero offset addition */
        ptr++;  /* Post-increment to create auto-inc pattern */
    }
    
    return sum;
}

/* Test 2: Array indexing with zero in expression */
int test2_array_index_zero(void) {
    short sarr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sarr[i] = (short)(i % 256);
    }
    
    short *sptr = sarr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Access with i + 0 pattern */
        sum += sptr[i + 0];  /* Zero offset in index */
        sptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
void test3_mixed_types(void) {
    char carr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    long larr[ARRAY_SIZE];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        carr[i] = (char)(i % 128);
        iarr[i] = i * 2;
        larr[i] = i * 1000L;
    }
    
    /* Char access with zero offset */
    char *cptr = carr;
    int char_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_sum += cptr[0];  /* First element access = offset 0 */
        cptr++;
    }
    
    /* Int access with explicit zero offset */
    int *iptr = iarr;
    int int_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_sum += *(iptr + 0);  /* Explicit zero offset */
        iptr++;
    }
    
    /* Long access with casted zero offset */
    long *lptr = larr;
    long long_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        long zero = 0;
        long_sum += lptr[(int)zero];  /* Cast zero to int offset */
        lptr++;
    }
    
    printf("Mixed types: char=%d, int=%d, long=%ld\n", 
           char_sum, int_sum, long_sum);
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    volatile int varr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        varr[i] = i * 3;
    }
    
    volatile int *vptr = varr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Volatile access with zero offset */
        sum += vptr[0 + 0];  /* Double zero for emphasis */
        vptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for aliasing guarantees */
int test5_restrict_zero_offset(void) {
    int rarr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        rarr[i] = i * 4;
    }
    
    int *restrict rptr = rarr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Restrict allows aggressive optimization */
        sum += rptr[(0)];  /* Zero in parentheses */
        rptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset */
int test6_nested_conditional(void) {
    int narr[ARRAY_SIZE][4];
    int sum = 0;
    
    /* Initialize 2D array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < 4; j++) {
            narr[i][j] = i * 4 + j;
        }
    }
    
    /* Nested loop with conditional access */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int *row_ptr = narr[i];
        for (int j = 0; j < 4; j++) {
            if (j % 2 == 0) {
                /* Access with zero offset in conditional */
                sum += row_ptr[0];  /* Always access first element */
            } else {
                sum += row_ptr[j];
            }
        }
        /* Pointer arithmetic that might be optimized */
        row_ptr += 0;  /* Explicit no-op pointer addition */
    }
    
    return sum;
}

/* Test 7: Post-decrement with zero offset */
int test7_post_decrement(void) {
    int darr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        darr[i] = i * 5;
    }
    
    int *dptr = &darr[ARRAY_SIZE - 1];
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
        /* Post-decrement pattern */
        sum += dptr[0];  /* Zero offset access */
        dptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 8: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* At offset 0 */
    int second;
    char third;
};

int test8_struct_offset_zero(void) {
    struct test_struct sarr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sarr[i].first = i;
        sarr[i].second = i * 2;
        sarr[i].third = (char)(i % 128);
    }
    
    struct test_struct *sptr = sarr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Accessing first member = offset 0 */
        sum += sptr->first;  /* This compiles to offset 0 */
        sptr++;
    }
    
    return sum;
}

/* Test 9: Pointer arithmetic with computed zero */
int test9_computed_zero(void) {
    int carr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        carr[i] = i * 6;
    }
    
    int *cptr = carr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Compute zero in a way that's not optimized away */
        int zero = i - i;  /* Always zero */
        sum += cptr[zero];  /* Use computed zero as offset */
        cptr++;
    }
    
    return sum;
}

/* Test 10: Multiple increments in same loop */
int test10_multiple_increments(void) {
    int marr1[ARRAY_SIZE];
    int marr2[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        marr1[i] = i;
        marr2[i] = i * 7;
    }
    
    int *mptr1 = marr1;
    int *mptr2 = marr2;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple zero-offset accesses */
        sum += mptr1[0];  /* First array, zero offset */
        sum += mptr2[0];  /* Second array, zero offset */
        
        /* Multiple increments */
        mptr1++;
        mptr2++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    printf("Running auto-inc-dec coverage tests...\n");
    
    /* Run tests based on command line or all by default */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "test1")) {
        total_sum += test1_basic_zero_offset();
    }
    if (run_all || strstr(argv[0], "test2")) {
        total_sum += test2_array_index_zero();
    }
    if (run_all || strstr(argv[0], "test3")) {
        test3_mixed_types();
    }
    if (run_all || strstr(argv[0], "test4")) {
        total_sum += test4_volatile_zero_offset();
    }
    if (run_all || strstr(argv[0], "test5")) {
        total_sum += test5_restrict_zero_offset();
    }
    if (run_all || strstr(argv[0], "test6")) {
        total_sum += test6_nested_conditional();
    }
    if (run_all || strstr(argv[0], "test7")) {
        total_sum += test7_post_decrement();
    }
    if (run_all || strstr(argv[0], "test8")) {
        total_sum += test8_struct_offset_zero();
    }
    if (run_all || strstr(argv[0], "test9")) {
        total_sum += test9_computed_zero();
    }
    if (run_all || strstr(argv[0], "test10")) {
        total_sum += test10_multiple_increments();
    }
    
    printf("Total checksum: %d\n", total_sum);
    printf("Note: Compile with -O2 -fdump-rtl-auto-inc-dec to see pass activity\n");
    
    return 0;
}
