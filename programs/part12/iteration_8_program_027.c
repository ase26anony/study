/* auto-inc-dec-test.c - Test program to cover auto-inc-dec pass lines 1352-1358 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test1_post_increment_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Access with ptr + 0 pattern */
    int *ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += *(ptr + 0);  /* This should generate plus with const_int 0 */
        ptr++;  /* Post-increment for find_inc to match */
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
    while (ptr >= start) {
        sum += ptr[0];  /* Array index 0 should generate const_int 0 */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
void test3_mixed_types_zero_offset(void) {
    char char_arr[SIZE];
    short short_arr[SIZE];
    int int_arr[SIZE];
    long long_arr[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        char_arr[i] = i & 0xFF;
        short_arr[i] = i * 2;
        int_arr[i] = i * 3;
        long_arr[i] = i * 4L;
    }
    
    /* Char access with zero offset */
    char *cptr = char_arr;
    char *cend = char_arr + SIZE;
    int char_sum = 0;
    while (cptr < cend) {
        char_sum += cptr[0];  /* QImode access */
        cptr++;
    }
    
    /* Short access with zero offset */
    short *sptr = short_arr;
    short *send = short_arr + SIZE;
    int short_sum = 0;
    while (sptr < send) {
        short_sum += *(sptr + 0);  /* HImode access with explicit + 0 */
        sptr++;
    }
    
    /* Int access with zero offset */
    int *iptr = int_arr;
    int *iend = int_arr + SIZE;
    int int_sum = 0;
    while (iptr < iend) {
        int_sum += iptr[(int)(0)];  /* Cast zero to force const_int 0 */
        iptr++;
    }
    
    /* Long access with zero offset */
    long *lptr = long_arr;
    long *lend = long_arr + SIZE;
    long long_sum = 0;
    while (lptr < lend) {
        long_sum += *(lptr + 0);  /* DImode access */
        lptr++;
    }
    
    printf("Mixed types sums: char=%d, short=%d, int=%d, long=%ld\n",
           char_sum, short_sum, int_sum, long_sum);
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_zero_offset(void) {
    int arr[SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 5;
    }
    
    for (int i = 0; i < SIZE; i++) {
        /* Volatile access with zero offset */
        sum += vptr[i + 0];  /* Force plus pattern with volatile */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for alias analysis */
int test5_restrict_zero_offset(void) {
    int arr[SIZE];
    int *restrict rptr = arr;
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 7;
    }
    
    for (int i = 0; i < SIZE; i++) {
        /* Restrict allows aggressive optimization */
        sum += rptr[0];  /* Always offset 0 */
        rptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero offset */
int test6_nested_conditional_zero_offset(void) {
    int arr[SIZE][4];
    int sum = 0;
    
    /* Initialize 2D array */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Nested loop with conditional access */
    for (int i = 0; i < SIZE; i++) {
        int *row_ptr = arr[i];
        for (int j = 0; j < 4; j++) {
            if (j % 2 == 0) {
                /* Access with zero offset in conditional */
                sum += *(row_ptr + 0);  /* Even columns use offset 0 */
            } else {
                sum += row_ptr[1];  /* Odd columns use offset 1 */
            }
            row_ptr++;  /* Pointer increment in inner loop */
        }
    }
    
    return sum;
}

/* Test 7: Structure with first field at offset 0 */
struct test_struct {
    int first;  /* Offset 0 */
    int second;
    char third;
};

int test7_struct_first_field(void) {
    struct test_struct arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i * 2;
        arr[i].second = i * 3;
        arr[i].third = i & 0xFF;
    }
    
    struct test_struct *sptr = arr;
    struct test_struct *send = arr + SIZE;
    while (sptr < send) {
        /* Access first field (offset 0) */
        sum += sptr->first;  /* Should generate memory access with offset 0 */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Different step sizes */
int test8_various_step_sizes(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i;
    }
    
    /* Step size 1 */
    int *ptr1 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += ptr1[0];
        ptr1 += 1;  /* Step 1 */
    }
    
    /* Step size 2 */
    int *ptr2 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += *(ptr2 + 0);
        ptr2 += 2;  /* Step 2 */
    }
    
    /* Step size -1 (reverse) */
    int *ptr3 = arr + SIZE - 1;
    for (int i = 0; i < SIZE; i++) {
        sum += ptr3[0];
        ptr3 -= 1;  /* Step -1 */
    }
    
    return sum;
}

/* Test 9: Complex expression with zero offset */
int test9_complex_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 11;
    }
    
    int *ptr = arr;
    int *end = arr + SIZE;
    
    /* Complex loop with multiple zero-offset accesses */
    while (ptr < end) {
        int temp = 0;
        
        /* Multiple accesses with zero offset */
        temp += ptr[0];
        temp += *(ptr + 0);
        temp += *(0 + ptr);  /* Commutative form */
        
        sum += temp;
        ptr++;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    printf("Auto-inc-dec test program\n");
    
    /* Run tests based on command line or all tests */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "test1") || argc > 1) {
        total_sum += test1_post_increment_zero_offset();
        printf("Test1 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test2") || argc > 1) {
        total_sum += test2_post_decrement_zero_offset();
        printf("Test2 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test3") || argc > 1) {
        test3_mixed_types_zero_offset();
        printf("Test3 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test4") || argc > 1) {
        total_sum += test4_volatile_zero_offset();
        printf("Test4 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test5") || argc > 1) {
        total_sum += test5_restrict_zero_offset();
        printf("Test5 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test6") || argc > 1) {
        total_sum += test6_nested_conditional_zero_offset();
        printf("Test6 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test7") || argc > 1) {
        total_sum += test7_struct_first_field();
        printf("Test7 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test8") || argc > 1) {
        total_sum += test8_various_step_sizes();
        printf("Test8 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test9") || argc > 1) {
        total_sum += test9_complex_zero_offset();
        printf("Test9 completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    /* Use result to prevent dead code elimination */
    if (total_sum > 0) {
        return 0;
    } else {
        return 1;
    }
}
