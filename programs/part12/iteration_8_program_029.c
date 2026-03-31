/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass with (plus (reg) (const_int 0)) patterns
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
    
    /* Loop with pointer + 0 access pattern */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    while (ptr < end) {
        sum += *(ptr + 0);  /* Zero offset access */
        ptr++;
    }
    
    return sum;
}

/* Test 2: Different data types with zero offset */
int test2_mixed_types(void) {
    char carr[ARRAY_SIZE];
    short sarr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    long larr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        carr[i] = (char)(i & 0xFF);
        sarr[i] = (short)i;
        iarr[i] = i;
        larr[i] = i;
    }
    
    /* Char pointer loop with zero offset */
    char *cptr = carr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += cptr[0 + i];  /* Zero offset in array index */
        cptr++;
    }
    
    /* Short pointer loop */
    short *sptr = sarr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr[0];  /* Direct zero offset */
        sptr = sptr + 1;  /* Different increment pattern */
    }
    
    /* Int pointer with post-increment in condition */
    int *iptr = iarr;
    int *iend = iarr + ARRAY_SIZE;
    while (iptr != iend) {
        sum += *(iptr + 0);  /* Zero offset with pointer arithmetic */
        iptr += 1;
    }
    
    /* Long pointer with complex zero offset */
    long *lptr = larr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += (int)lptr[(int)(0)];  /* Cast zero to int */
        lptr++;
    }
    
    return sum;
}

/* Test 3: Volatile pointers with zero offset */
int test3_volatile_access(void) {
    static int arr[ARRAY_SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[i + 0];  /* Zero offset with volatile */
        /* The volatile qualifier may affect optimization but still generates the pattern */
    }
    
    /* Another volatile pattern */
    volatile int *vptr2 = arr;
    int *end = arr + ARRAY_SIZE;
    while (vptr2 < (volatile int *)end) {
        sum += *(vptr2 + 0);  /* Pointer arithmetic with zero */
        vptr2++;
    }
    
    return sum;
}

/* Test 4: Restrict pointers for alias analysis */
int test4_restrict_pointers(void) {
    int arr[ARRAY_SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 3;
    }
    
    /* Restrict allows aggressive optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];  /* Zero offset with restrict */
        rptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 5: Nested loops and conditional access */
int test5_complex_patterns(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i % 100;
    }
    
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE * 2;
    
    /* Nested loop structure */
    for (int outer = 0; outer < 10; outer++) {
        int *inner_ptr = ptr + outer * (ARRAY_SIZE / 10);
        int *inner_end = inner_ptr + (ARRAY_SIZE / 10);
        
        while (inner_ptr < inner_end) {
            /* Conditional zero-offset access */
            if (*inner_ptr > 50) {
                sum += *(inner_ptr + 0);  /* Zero offset in true branch */
            } else {
                sum += inner_ptr[0];      /* Zero offset in false branch */
            }
            
            /* Mixed increment patterns */
            if (outer % 2 == 0) {
                inner_ptr += 1;
            } else {
                inner_ptr = inner_ptr + 1;
            }
        }
    }
    
    /* Another pattern: pointer in loop update */
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; ptr++, i++) {
        /* Access with zero offset using array notation */
        sum += ptr[0 + 0];  /* Double zero offset */
    }
    
    return sum;
}

/* Test 6: Structure access with zero offset (first field) */
struct test_struct {
    int first_field;  /* Offset 0 */
    int second_field;
    char third_field;
};

int test6_struct_access(void) {
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first_field = i;
        arr[i].second_field = i * 2;
        arr[i].third_field = (char)i;
    }
    
    /* Structure pointer with -> access (offset 0 for first field) */
    struct test_struct *sptr = arr;
    struct test_struct *send = arr + ARRAY_SIZE;
    
    while (sptr < send) {
        sum += sptr->first_field;  /* This compiles to offset 0 */
        sptr++;
    }
    
    /* Alternative: pointer arithmetic with zero */
    sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += (*(sptr + 0)).first_field;  /* Explicit zero offset */
        sptr++;
    }
    
    return sum;
}

/* Test 7: Post-decrement patterns */
int test7_decrement_patterns(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 4;
    }
    
    /* Reverse traversal with post-decrement */
    int *ptr = arr + ARRAY_SIZE - 1;
    int *start = arr;
    
    while (ptr >= start) {
        sum += *(ptr + 0);  /* Zero offset with decrement */
        ptr--;  /* Post-decrement */
    }
    
    /* Another decrement pattern */
    ptr = arr + ARRAY_SIZE;
    do {
        ptr--;
        sum += ptr[0];  /* Zero offset */
    } while (ptr > arr);
    
    return sum;
}

/* Test 8: Multiple increments in same loop */
int test8_multiple_increments(void) {
    int arr1[ARRAY_SIZE];
    int arr2[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    int *p1 = arr1;
    int *p2 = arr2;
    int *end1 = arr1 + ARRAY_SIZE;
    
    /* Two pointers incrementing with zero-offset access */
    while (p1 < end1) {
        sum += *(p1 + 0);  /* Zero offset for p1 */
        sum += p2[0];       /* Zero offset for p2 */
        p1++;
        p2++;
    }
    
    return sum;
}

/* Test 9: Function pointer parameters */
static int helper_with_zero_offset(int *restrict ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Zero offset in helper function */
        ptr++;
    }
    return sum;
}

int test9_function_calls(void) {
    int arr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 5;
    }
    
    /* Call helper multiple times with different patterns */
    int sum = 0;
    sum += helper_with_zero_offset(arr, ARRAY_SIZE);
    sum += helper_with_zero_offset(arr + ARRAY_SIZE/2, ARRAY_SIZE/2);
    
    return sum;
}

/* Test 10: Mixed step sizes */
int test10_variable_steps(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i % 64;
    }
    
    /* Different step sizes in different loops */
    for (int step = 1; step <= 4; step *= 2) {
        int *ptr = arr;
        int *end = arr + ARRAY_SIZE;
        
        while (ptr < end) {
            sum += *(ptr + 0);  /* Zero offset */
            ptr += step;  /* Variable step */
        }
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    int test_to_run = -1;
    
    /* Parse command line to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    if (test_to_run == -1 || test_to_run == 1) {
        total_sum += test1_basic_zero_offset();
    }
    if (test_to_run == -1 || test_to_run == 2) {
        total_sum += test2_mixed_types();
    }
    if (test_to_run == -1 || test_to_run == 3) {
        total_sum += test3_volatile_access();
    }
    if (test_to_run == -1 || test_to_run == 4) {
        total_sum += test4_restrict_pointers();
    }
    if (test_to_run == -1 || test_to_run == 5) {
        total_sum += test5_complex_patterns();
    }
    if (test_to_run == -1 || test_to_run == 6) {
        total_sum += test6_struct_access();
    }
    if (test_to_run == -1 || test_to_run == 7) {
        total_sum += test7_decrement_patterns();
    }
    if (test_to_run == -1 || test_to_run == 8) {
        total_sum += test8_multiple_increments();
    }
    if (test_to_run == -1 || test_to_run == 9) {
        total_sum += test9_function_calls();
    }
    if (test_to_run == -1 || test_to_run == 10) {
        total_sum += test10_variable_steps();
    }
    
    /* Print checksum to prevent optimization removal */
    printf("Checksum: %d\n", total_sum);
    
    return 0;
}
