/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test1_post_increment(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Access with ptr[0] pattern */
    int *ptr = arr;
    int *end = arr + SIZE;
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    while (ptr < end) {
        sum += ptr[0];  /* Zero offset access */
        ptr++;          /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_post_decrement(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + SIZE - 1;
    int *start = arr;
    
    while (ptr >= start) {
        sum += *(ptr + 0);  /* Explicit + 0 offset */
        ptr--;              /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types to test various modes */
long test3_mixed_types(void) {
    char c_arr[SIZE];
    short s_arr[SIZE];
    int i_arr[SIZE];
    long l_arr[SIZE];
    long total = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        c_arr[i] = i & 0xFF;
        s_arr[i] = i * 2;
        i_arr[i] = i * 3;
        l_arr[i] = i * 4L;
    }
    
    /* Char access - QImode */
    char *c_ptr = c_arr;
    for (int i = 0; i < SIZE; i++) {
        total += c_ptr[0];  /* Zero offset */
        c_ptr++;
    }
    
    /* Short access - HImode */
    short *s_ptr = s_arr;
    for (int i = 0; i < SIZE; i++) {
        total += s_ptr[0];  /* Zero offset */
        s_ptr++;
    }
    
    /* Int access - SImode */
    int *i_ptr = i_arr;
    for (int i = 0; i < SIZE; i++) {
        total += i_ptr[0];  /* Zero offset */
        i_ptr++;
    }
    
    /* Long access - DImode */
    long *l_ptr = l_arr;
    for (int i = 0; i < SIZE; i++) {
        total += l_ptr[0];  /* Zero offset */
        l_ptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    int arr[SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < SIZE; i++) {
        sum += vptr[i + 0];  /* Explicit i + 0 */
    }
    
    /* Also test with restrict */
    int *restrict rptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += rptr[0];  /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 5: Nested loops and conditional access */
int test5_complex_patterns(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i % 100;
    }
    
    int *ptr = arr;
    
    /* Nested loop with conditional zero-offset access */
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < SIZE / 10; inner++) {
            if (inner % 2 == 0) {
                /* Access with zero offset in one branch */
                sum += ptr[0];
            } else {
                /* Access with non-zero offset in other branch */
                sum += ptr[1];
            }
        }
        ptr += SIZE / 10;
    }
    
    /* Another pattern: while loop with post-increment in condition */
    ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end && *((int*)((char*)ptr + 0)) != 0) {
        /* Cast to char* and back to force address computation */
        sum += *ptr;
        ptr++;
    }
    
    return sum;
}

/* Test 6: Structure with first member at offset 0 */
struct test_struct {
    int first;  /* At offset 0 */
    int second;
    char third;
};

int test6_struct_access(void) {
    struct test_struct arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = i & 0xFF;
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Access first member (offset 0) */
        sum += sptr->first;  /* Equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 7: Array of pointers with zero index */
int test7_pointer_array(void) {
    int data[SIZE];
    int *ptr_array[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        data[i] = i * 3;
        ptr_array[i] = &data[i];
    }
    
    int sum = 0;
    int **pptr = ptr_array;
    
    for (int i = 0; i < SIZE; i++) {
        /* Dereference with zero offset */
        sum += *(*pptr + 0);  /* *pptr[0] with explicit + 0 */
        pptr++;
    }
    
    return sum;
}

/* Test 8: Different step sizes */
int test8_variable_steps(void) {
    int arr[SIZE * 4];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 4; i++) {
        arr[i] = i % 256;
    }
    
    /* Step size 1 */
    int *ptr1 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += ptr1[0];
        ptr1 += 1;
    }
    
    /* Step size 2 */
    int *ptr2 = arr;
    for (int i = 0; i < SIZE; i += 2) {
        sum += ptr2[0];
        ptr2 += 2;
    }
    
    /* Step size 4 */
    int *ptr4 = arr;
    for (int i = 0; i < SIZE; i += 4) {
        sum += ptr4[0];
        ptr4 += 4;
    }
    
    return sum;
}

/* Test 9: Function pointer parameter with zero offset */
static void process_element(int *elem, int *sum) {
    *sum += elem[0];  /* Zero offset access */
}

int test9_function_calls(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 5;
    }
    
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        process_element(ptr + 0, &sum);  /* Pass ptr + 0 */
        ptr++;
    }
    
    return sum;
}

/* Test 10: Mixed zero and non-zero offsets in same loop */
int test10_mixed_offsets(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    
    /* Alternate between zero and non-zero offsets */
    for (int i = 0; i < SIZE; i++) {
        if (i % 3 == 0) {
            sum += ptr[0];      /* Zero offset */
        } else if (i % 3 == 1) {
            sum += ptr[1];      /* Offset 1 */
        } else {
            sum += *(ptr + 0);  /* Another zero offset variant */
        }
        ptr++;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    long total_result = 0;
    
    /* Use command line to select which test to run */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run all tests if no specific test requested */
    if (test_to_run <= 0 || test_to_run == 1) {
        int r1 = test1_post_increment();
        printf("Test 1 result: %d\n", r1);
        total_result += r1;
    }
    
    if (test_to_run <= 0 || test_to_run == 2) {
        int r2 = test2_post_decrement();
        printf("Test 2 result: %d\n", r2);
        total_result += r2;
    }
    
    if (test_to_run <= 0 || test_to_run == 3) {
        long r3 = test3_mixed_types();
        printf("Test 3 result: %ld\n", r3);
        total_result += r3;
    }
    
    if (test_to_run <= 0 || test_to_run == 4) {
        int r4 = test4_volatile_access();
        printf("Test 4 result: %d\n", r4);
        total_result += r4;
    }
    
    if (test_to_run <= 0 || test_to_run == 5) {
        int r5 = test5_complex_patterns();
        printf("Test 5 result: %d\n", r5);
        total_result += r5;
    }
    
    if (test_to_run <= 0 || test_to_run == 6) {
        int r6 = test6_struct_access();
        printf("Test 6 result: %d\n", r6);
        total_result += r6;
    }
    
    if (test_to_run <= 0 || test_to_run == 7) {
        int r7 = test7_pointer_array();
        printf("Test 7 result: %d\n", r7);
        total_result += r7;
    }
    
    if (test_to_run <= 0 || test_to_run == 8) {
        int r8 = test8_variable_steps();
        printf("Test 8 result: %d\n", r8);
        total_result += r8;
    }
    
    if (test_to_run <= 0 || test_to_run == 9) {
        int r9 = test9_function_calls();
        printf("Test 9 result: %d\n", r9);
        total_result += r9;
    }
    
    if (test_to_run <= 0 || test_to_run == 10) {
        int r10 = test10_mixed_offsets();
        printf("Test 10 result: %d\n", r10);
        total_result += r10;
    }
    
    printf("Total checksum: %ld\n", total_result);
    
    return 0;
}
