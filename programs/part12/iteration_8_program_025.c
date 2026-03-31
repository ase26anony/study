/* test_auto_inc_dec.c - Program to trigger auto-inc-dec pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test_post_increment_zero_offset(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    for (; p < end; p++) {
        sum += p[0];  /* Zero offset access */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test_post_decrement_zero_offset(int *arr, int n) {
    int sum = 0;
    int *p = arr + n - 1;
    int *start = arr;
    
    for (; p >= start; p--) {
        sum += *(p + 0);  /* Zero offset via pointer arithmetic */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
void test_mixed_types() {
    char char_arr[ARRAY_SIZE];
    short short_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    long long_arr[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i % 65536);
        int_arr[i] = i;
        long_arr[i] = i * 2L;
    }
    
    /* Process each with zero-offset access */
    char *cp = char_arr;
    short *sp = short_arr;
    int *ip = int_arr;
    long *lp = long_arr;
    
    char char_sum = 0;
    short short_sum = 0;
    int int_sum = 0;
    long long_sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_sum += cp[0];      /* QImode access */
        cp++;
        
        short_sum += sp[0];     /* HImode access */
        sp++;
        
        int_sum += ip[0];       /* SImode access */
        ip++;
        
        long_sum += lp[0];      /* DImode access */
        lp++;
    }
    
    /* Use results to prevent optimization */
    printf("Mixed types checksum: %d %d %d %ld\n", 
           (int)char_sum, (int)short_sum, int_sum, long_sum);
}

/* Test 4: Volatile pointer with zero offset */
int test_volatile_zero_offset(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    
    for (int i = 0; i < n; i++) {
        sum += p[0];  /* Zero offset with volatile */
        p++;
    }
    
    return sum;
}

/* Test 5: Restrict pointer for aliasing guarantees */
int test_restrict_zero_offset(int *restrict arr, int n) {
    int sum = 0;
    int *restrict p = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *(p + 0);  /* Zero offset with restrict */
        p++;
    }
    
    return sum;
}

/* Test 6: Nested conditional with zero offset */
int test_nested_conditional(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            sum += p[0];  /* Zero offset in conditional */
        } else {
            sum -= p[0];  /* Same zero offset in else branch */
        }
        
        if (i % 3 == 0) {
            /* Nested conditional with pointer arithmetic */
            sum += *(p + 0) * 2;
        }
        
        p++;
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;   /* Offset 0 */
    int second;
    char third;
};

int test_struct_offset_zero(struct test_struct *arr, int n) {
    int sum = 0;
    struct test_struct *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Accessing first member is at offset 0 */
        sum += p->first;  /* Equivalent to p[0].first */
        p++;
    }
    
    return sum;
}

/* Test 8: Explicit zero cast as index */
int test_explicit_zero_cast(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Explicit zero cast to force const_int 0 pattern */
        sum += p[(int)(0)];
        p++;
    }
    
    return sum;
}

/* Test 9: Multiple induction variables */
int test_multiple_induction(int *arr, int n) {
    int sum = 0;
    int *p1 = arr;
    int *p2 = arr + n/2;
    
    for (int i = 0; i < n/2; i++) {
        sum += p1[0] + p2[0];  /* Two zero-offset accesses */
        p1++;
        p2--;
    }
    
    return sum;
}

/* Test 10: While loop with post-increment */
int test_while_zero_offset(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    while (p < end) {
        sum += *(p + 0);  /* Zero offset in while */
        p++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int test_to_run = -1;
    
    /* Parse command line to select test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Initialize test data */
    int data[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i;
    }
    
    struct test_struct struct_data[ARRAY_SIZE/4];
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        struct_data[i].first = i;
        struct_data[i].second = i * 2;
        struct_data[i].third = (char)(i % 256);
    }
    
    int result = 0;
    
    /* Run selected test or all tests */
    if (test_to_run == 1 || test_to_run == -1) {
        result += test_post_increment_zero_offset(data, ARRAY_SIZE);
    }
    if (test_to_run == 2 || test_to_run == -1) {
        result += test_post_decrement_zero_offset(data, ARRAY_SIZE);
    }
    if (test_to_run == 3 || test_to_run == -1) {
        test_mixed_types();
    }
    if (test_to_run == 4 || test_to_run == -1) {
        result += test_volatile_zero_offset(data, ARRAY_SIZE);
    }
    if (test_to_run == 5 || test_to_run == -1) {
        result += test_restrict_zero_offset(data, ARRAY_SIZE);
    }
    if (test_to_run == 6 || test_to_run == -1) {
        result += test_nested_conditional(data, ARRAY_SIZE);
    }
    if (test_to_run == 7 || test_to_run == -1) {
        result += test_struct_offset_zero(struct_data, ARRAY_SIZE/4);
    }
    if (test_to_run == 8 || test_to_run == -1) {
        result += test_explicit_zero_cast(data, ARRAY_SIZE);
    }
    if (test_to_run == 9 || test_to_run == -1) {
        result += test_multiple_induction(data, ARRAY_SIZE);
    }
    if (test_to_run == 10 || test_to_run == -1) {
        result += test_while_zero_offset(data, ARRAY_SIZE);
    }
    
    /* Print result to prevent optimization */
    printf("Final checksum: %d\n", result);
    
    return 0;
}
