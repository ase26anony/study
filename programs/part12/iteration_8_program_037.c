/* auto-inc-dec-test.c
 * Program designed to trigger GCC's auto-increment/decrement optimization
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define ITERATIONS 100

/* Test 1: Simple post-increment with zero offset */
int test1_post_increment_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Loop with pointer arithmetic using +0 */
    int *ptr = arr;
    int *end = arr + SIZE;
    
    /* This should generate (mem (plus (reg) (const_int 0))) patterns */
    while (ptr < end) {
        /* Multiple zero-offset accesses to increase chances */
        sum += *(ptr + 0);      /* Explicit +0 */
        sum += ptr[0];          /* Array index 0 */
        sum += *ptr;            /* Direct dereference */
        ptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_post_decrement_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + SIZE - 1;
    int *start = arr;
    
    /* Decrementing loop with zero offset */
    while (ptr >= start) {
        sum += *(ptr + 0);      /* +0 offset */
        sum += ptr[0];          /* index 0 */
        ptr--;                  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
long test3_mixed_types_zero_offset(void) {
    char char_arr[SIZE];
    short short_arr[SIZE];
    int int_arr[SIZE];
    long long_arr[SIZE];
    long total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        char_arr[i] = i % 256;
        short_arr[i] = i * 2;
        int_arr[i] = i * 3;
        long_arr[i] = i * 4;
    }
    
    /* Char pointer loop */
    char *cptr = char_arr;
    char *cend = char_arr + SIZE;
    while (cptr < cend) {
        total += *(cptr + 0);   /* QImode access with +0 */
        cptr++;
    }
    
    /* Short pointer loop */
    short *sptr = short_arr;
    short *send = short_arr + SIZE;
    while (sptr < send) {
        total += *(sptr + 0);   /* HImode access with +0 */
        sptr++;
    }
    
    /* Int pointer loop */
    int *iptr = int_arr;
    int *iend = int_arr + SIZE;
    while (iptr < iend) {
        total += *(iptr + 0);   /* SImode access with +0 */
        iptr++;
    }
    
    /* Long pointer loop */
    long *lptr = long_arr;
    long *lend = long_arr + SIZE;
    while (lptr < lend) {
        total += *(lptr + 0);   /* DImode access with +0 */
        lptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    volatile int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    volatile int *vptr = arr;
    volatile int *vend = arr + SIZE;
    
    /* Volatile access with zero offset */
    while (vptr < vend) {
        sum += *(vptr + 0);     /* +0 offset with volatile */
        vptr++;
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    int *restrict rptr = arr;
    int *restrict rend = arr + SIZE;
    
    /* Restrict pointer with zero offset */
    while (rptr < rend) {
        sum += *(rptr + 0);     /* +0 offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset */
int test6_nested_conditional_zero_offset(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i % 100;
    }
    
    int *ptr = arr;
    
    /* Outer loop */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Inner loop with conditional access */
        int *inner_ptr = ptr;
        for (int j = 0; j < SIZE; j++) {
            if (j % 2 == 0) {
                /* Even indices use +0 offset */
                sum += *(inner_ptr + 0);
            } else {
                /* Odd indices use direct access */
                sum += *inner_ptr;
            }
            inner_ptr++;
        }
        ptr += (i % 10);  /* Variable pointer advancement */
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;   /* At offset 0 */
    int second;
    char third;
};

int test7_struct_zero_offset(void) {
    struct test_struct arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = i % 256;
    }
    
    struct test_struct *sptr = arr;
    struct test_struct *send = arr + SIZE;
    
    /* Access first member (at offset 0) */
    while (sptr < send) {
        /* These should generate (plus (reg) (const_int 0)) patterns */
        sum += sptr->first;     /* Field access at offset 0 */
        sum += (*(sptr + 0)).first;  /* Explicit +0 */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Complex pointer arithmetic with zero in loop */
int test8_complex_zero_arithmetic(void) {
    int arr[SIZE * 3];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 3; i++) {
        arr[i] = i;
    }
    
    /* Multiple pointer variables with different step sizes */
    int *ptr1 = arr;
    int *ptr2 = arr + SIZE;
    int *ptr3 = arr + SIZE * 2;
    
    for (int i = 0; i < SIZE; i++) {
        /* All using +0 offset in different ways */
        sum += ptr1[0 + 0];          /* Double zero */
        sum += *(ptr2 + (int)(0));   /* Cast zero */
        sum += *(ptr3 + 0);          /* Simple +0 */
        
        ptr1 += 1;
        ptr2 += 2;  /* Different step size */
        ptr3 += 3;  /* Different step size */
    }
    
    return sum;
}

/* Test 9: Function pointer arguments with zero offset */
void process_array(int *restrict dst, const int *restrict src, int n) {
    for (int i = 0; i < n; i++) {
        /* Zero offset in function context */
        dst[i] = *(src + 0) + i;
        src++;
    }
}

int test9_function_call_zero_offset(void) {
    int src[SIZE];
    int dst[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3;
    }
    
    process_array(dst, src, SIZE);
    
    for (int i = 0; i < SIZE; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* Test 10: Switch statement with different offset patterns */
int test10_switch_zero_offset(void) {
    int arr[SIZE * 4];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 4; i++) {
        arr[i] = i % 50;
    }
    
    int *ptr = arr;
    
    for (int i = 0; i < SIZE; i++) {
        switch (i % 4) {
            case 0:
                sum += *(ptr + 0);      /* +0 offset */
                break;
            case 1:
                sum += ptr[0];          /* index 0 */
                break;
            case 2:
                sum += *ptr;            /* direct */
                break;
            case 3:
                sum += *(ptr + 0) * 2;  /* +0 with operation */
                break;
        }
        ptr++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Run tests based on command line arguments or all by default */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "test1") || (argc > 1 && strcmp(argv[1], "1") == 0)) {
        total += test1_post_increment_zero_offset();
    }
    
    if (run_all || strstr(argv[0], "test2") || (argc > 1 && strcmp(argv[1], "2") == 0)) {
        total += test2_post_decrement_zero_offset();
    }
    
    if (run_all || strstr(argv[0], "test3") || (argc > 1 && strcmp(argv[1], "3") == 0)) {
        total += test3_mixed_types_zero_offset();
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && strcmp(argv[1], "4") == 0)) {
        total += test4_volatile_zero_offset();
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && strcmp(argv[1], "5") == 0)) {
        total += test5_restrict_zero_offset();
    }
    
    if (run_all || strstr(argv[0], "test6") || (argc > 1 && strcmp(argv[1], "6") == 0)) {
        total += test6_nested_conditional_zero_offset();
    }
    
    if (run_all || strstr(argv[0], "test7") || (argc > 1 && strcmp(argv[1], "7") == 0)) {
        total += test7_struct_zero_offset();
    }
    
    if (run_all || strstr(argv[0], "test8") || (argc > 1 && strcmp(argv[1], "8") == 0)) {
        total += test8_complex_zero_arithmetic();
    }
    
    if (run_all || strstr(argv[0], "test9") || (argc > 1 && strcmp(argv[1], "9") == 0)) {
        total += test9_function_call_zero_offset();
    }
    
    if (run_all || strstr(argv[0], "test10") || (argc > 1 && strcmp(argv[1], "10") == 0)) {
        total += test10_switch_zero_offset();
    }
    
    /* Print result to prevent optimization removal */
    printf("Total checksum: %d\n", total);
    
    return 0;
}
