/* auto-inc-dec-test.c
 * Designed to trigger GCC's auto-inc-dec pass for lines 1352-1358
 * where mem_insn.reg1_is_const = true and mem_insn.reg1_val = 0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test1_simple_postinc(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Loop with pointer arithmetic using +0 */
    int *ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end) {
        /* Force (mem (plus (reg) (const_int 0))) pattern */
        sum += ptr[0];          /* Zero offset access */
        sum += *(ptr + 0);      /* Alternative zero offset */
        ptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_postdec(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + SIZE - 1;
    int *start = arr;
    while (ptr >= start) {
        sum += ptr[0];          /* Zero offset */
        ptr--;                  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types for different memory modes */
long test3_mixed_types(void) {
    char c_arr[SIZE];
    short s_arr[SIZE];
    int i_arr[SIZE];
    long l_arr[SIZE];
    long total = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        c_arr[i] = (char)(i % 256);
        s_arr[i] = (short)(i * 3);
        i_arr[i] = i * 5;
        l_arr[i] = i * 7L;
    }
    
    /* Char access - QImode */
    char *c_ptr = c_arr;
    for (int i = 0; i < SIZE; i++) {
        total += c_ptr[0];      /* Zero offset */
        c_ptr++;
    }
    
    /* Short access - HImode */
    short *s_ptr = s_arr;
    for (int i = 0; i < SIZE; i++) {
        total += s_ptr[0];      /* Zero offset */
        s_ptr++;
    }
    
    /* Int access - SImode */
    int *i_ptr = i_arr;
    for (int i = 0; i < SIZE; i++) {
        total += i_ptr[0];      /* Zero offset */
        i_ptr++;
    }
    
    /* Long access - DImode */
    long *l_ptr = l_arr;
    for (int i = 0; i < SIZE; i++) {
        total += l_ptr[0];      /* Zero offset */
        l_ptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 11;
    }
    
    volatile int *vptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Force zero offset with volatile */
        sum += vptr[i + 0];     /* Zero offset with volatile */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for aliasing guarantees */
int test5_restrict_ptr(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *restrict rptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Multiple zero-offset accesses with restrict */
        sum += rptr[0];
        sum += *(rptr + 0);
        rptr += 2;  /* Step by 2 to test different increments */
    }
    
    return sum;
}

/* Test 6: Nested conditionals with zero offset */
int test6_conditional_access(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;
    }
    
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        if (i % 3 == 0) {
            sum += ptr[0];      /* Zero offset in conditional */
        } else if (i % 3 == 1) {
            sum -= ptr[0];      /* Same zero offset, different use */
        } else {
            sum += *(ptr + 0);  /* Alternative zero offset syntax */
        }
        
        if (i % 10 == 0) {
            /* Nested conditional */
            sum += ptr[0] * 2;
        }
        
        ptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;   /* At offset 0 */
    int second;
    char third;
};

int test7_struct_first_member(void) {
    struct test_struct arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)(i % 128);
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Access first member at offset 0 */
        sum += sptr->first;     /* Equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit zero cast as index */
int test8_explicit_zero_cast(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 13;
    }
    
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Force constant zero through cast */
        sum += ptr[(int)(0)];   /* Explicit zero cast */
        sum += ptr[0 + 0];      /* Constant expression zero */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple loops with different step sizes */
int test9_variable_steps(void) {
    int arr[SIZE * 4];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 4; i++) {
        arr[i] = i % 255;
    }
    
    /* Step size 1 */
    int *p1 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += p1[0];
        p1 += 1;
    }
    
    /* Step size 2 */
    int *p2 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += p2[0];
        p2 += 2;
    }
    
    /* Step size 4 */
    int *p4 = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += p4[0];
        p4 += 4;
    }
    
    /* Negative step */
    int *pn = arr + SIZE - 1;
    for (int i = 0; i < SIZE; i++) {
        sum += pn[0];
        pn -= 1;
    }
    
    return sum;
}

/* Test 10: Complex pointer arithmetic with zero */
int test10_complex_arithmetic(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 17;
    }
    
    int *ptr = arr;
    int index = 0;
    
    while (index < SIZE) {
        /* Complex expression that simplifies to +0 */
        int offset = (index * 0) + (0 * index) + 0;
        sum += ptr[offset];     /* Should be ptr[0] */
        
        /* Another variation */
        sum += *(ptr + (offset & 0));  /* offset & 0 = 0 */
        
        ptr++;
        index++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int test_to_run = -1;
    long total_result = 0;
    
    /* Use command line to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    if (test_to_run == 1 || test_to_run == -1) {
        total_result += test1_simple_postinc();
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == 2 || test_to_run == -1) {
        total_result += test2_postdec();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == 3 || test_to_run == -1) {
        total_result += test3_mixed_types();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == 4 || test_to_run == -1) {
        total_result += test4_volatile_access();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == 5 || test_to_run == -1) {
        total_result += test5_restrict_ptr();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == 6 || test_to_run == -1) {
        total_result += test6_conditional_access();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == 7 || test_to_run == -1) {
        total_result += test7_struct_first_member();
        printf("Test 7 completed\n");
    }
    
    if (test_to_run == 8 || test_to_run == -1) {
        total_result += test8_explicit_zero_cast();
        printf("Test 8 completed\n");
    }
    
    if (test_to_run == 9 || test_to_run == -1) {
        total_result += test9_variable_steps();
        printf("Test 9 completed\n");
    }
    
    if (test_to_run == 10 || test_to_run == -1) {
        total_result += test10_complex_arithmetic();
        printf("Test 10 completed\n");
    }
    
    /* Print checksum to prevent optimization removal */
    printf("Total checksum: %ld\n", total_result);
    
    return 0;
}
