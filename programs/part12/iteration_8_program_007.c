/* auto-inc-dec-test.c
 * Test program to trigger auto-increment/decrement optimization
 * with zero-offset memory accesses in various contexts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024

/* Test 1: Simple post-increment with zero offset */
int test1_simple_post_inc(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += ptr[0];          /* Zero offset access */
        ptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test2_simple_post_dec(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = &arr[SIZE - 1];
    for (int i = 0; i < SIZE; i++) {
        sum += ptr[0];          /* Zero offset */
        ptr--;                  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
void test3_mixed_types(void) {
    char carr[SIZE];
    short sarr[SIZE];
    int iarr[SIZE];
    long larr[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        carr[i] = (char)(i % 256);
        sarr[i] = (short)(i % 65536);
        iarr[i] = i;
        larr[i] = i * 1000L;
    }
    
    /* Char access with zero offset */
    char *cptr = carr;
    int csum = 0;
    for (int i = 0; i < SIZE; i++) {
        csum += cptr[0];        /* QImode access */
        cptr++;
    }
    
    /* Short access with zero offset */
    short *sptr = sarr;
    int ssum = 0;
    for (int i = 0; i < SIZE; i++) {
        ssum += sptr[0];        /* HImode access */
        sptr++;
    }
    
    /* Int access with zero offset */
    int *iptr = iarr;
    int isum = 0;
    for (int i = 0; i < SIZE; i++) {
        isum += iptr[0];        /* SImode access */
        iptr++;
    }
    
    /* Long access with zero offset */
    long *lptr = larr;
    long lsum = 0;
    for (int i = 0; i < SIZE; i++) {
        lsum += lptr[0];        /* DImode access */
        lptr++;
    }
    
    printf("Mixed types: c=%d s=%d i=%d l=%ld\n", csum, ssum, isum, lsum);
}

/* Test 4: Volatile pointer with zero offset */
int test4_volatile_access(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    volatile int *vptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += vptr[0];         /* Zero offset with volatile */
        vptr++;                 /* Should still attempt auto-inc */
    }
    
    return sum;
}

/* Test 5: Restrict pointer for aliasing guarantee */
int test5_restrict_pointer(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    int *restrict rptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += rptr[0];         /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Explicit zero offset arithmetic */
int test6_explicit_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Multiple ways to express zero offset */
        sum += *(ptr + 0);      /* Explicit plus zero */
        sum += ptr[0 + 0];      /* Double zero offset */
        sum += *(0 + ptr);      /* Commutative zero addition */
        ptr++;
    }
    
    return sum;
}

/* Test 7: Nested loops with conditional zero offset */
int test7_nested_conditional(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i % 100;
    }
    
    int *ptr = arr;
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < SIZE / 10; inner++) {
            if (inner % 2 == 0) {
                sum += ptr[0];  /* Zero offset in conditional */
            } else {
                sum += ptr[1];  /* Non-zero offset */
            }
        }
        ptr += SIZE / 10;       /* Step pointer */
    }
    
    return sum;
}

/* Test 8: Structure with first member at offset 0 */
struct test_struct {
    int first;      /* At offset 0 */
    int second;
    char third;
};

int test8_struct_first_member(void) {
    struct test_struct arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)(i % 256);
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += sptr->first;     /* Accesses member at offset 0 */
        sptr++;
    }
    
    return sum;
}

/* Test 9: Pointer arithmetic with cast zero */
int test9_cast_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Cast zero to different types to force (const_int 0) */
        sum += ptr[(int)(0)];
        sum += ptr[(unsigned)(0)];
        sum += ptr[(long)(0)];
        sum += ptr[(size_t)(0)];
        ptr++;
    }
    
    return sum;
}

/* Test 10: Complex loop with multiple zero-offset accesses */
int test10_complex_pattern(void) {
    int src[SIZE];
    int dst[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        src[i] = i;
        dst[i] = 0;
    }
    
    int *sptr = src;
    int *dptr = dst;
    
    /* Copy with zero-offset accesses */
    for (int i = 0; i < SIZE; i++) {
        dptr[0] = sptr[0];      /* Zero offset on both sides */
        sum += dptr[0];
        sptr++;
        dptr++;
    }
    
    /* Verify copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            return -1;
        }
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    int test_to_run = -1;
    
    /* Use command line to select specific test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run all tests or specific one */
    if (test_to_run == -1 || test_to_run == 1) {
        total_sum += test1_simple_post_inc();
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 2) {
        total_sum += test2_simple_post_dec();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 3) {
        test3_mixed_types();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 4) {
        total_sum += test4_volatile_access();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 5) {
        total_sum += test5_restrict_pointer();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 6) {
        total_sum += test6_explicit_zero_offset();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 7) {
        total_sum += test7_nested_conditional();
        printf("Test 7 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 8) {
        total_sum += test8_struct_first_member();
        printf("Test 8 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 9) {
        total_sum += test9_cast_zero_offset();
        printf("Test 9 completed\n");
    }
    
    if (test_to_run == -1 || test_to_run == 10) {
        total_sum += test10_complex_pattern();
        printf("Test 10 completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
