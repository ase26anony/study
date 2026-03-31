/* test_auto_inc_dec.c - Program to trigger auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Test 1: Basic post-increment with zero offset */
int test_basic_post_inc(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i;
    }
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    for (; ptr < end; ptr++) {
        sum += ptr[0];  /* Zero offset access */
    }
    
    /* Another variant with explicit + 0 */
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 0);  /* Explicit zero offset */
        ptr++;
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test_basic_post_dec(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int *ptr = arr + ARRAY_SIZE - 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[0];  /* Zero offset */
        ptr--;  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types for different memory modes */
long test_mixed_types(void) {
    char carr[ARRAY_SIZE];      /* QImode */
    short sarr[ARRAY_SIZE];     /* HImode */
    int iarr[ARRAY_SIZE];       /* SImode */
    long larr[ARRAY_SIZE];      /* DImode */
    long total = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        carr[i] = i & 0xFF;
        sarr[i] = i * 2;
        iarr[i] = i * 3;
        larr[i] = i * 4L;
    }
    
    /* Char access with zero offset */
    char *cptr = carr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += cptr[0];  /* QImode access */
        cptr++;
    }
    
    /* Short access with zero offset */
    short *sptr = sarr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += sptr[0];  /* HImode access */
        sptr++;
    }
    
    /* Int access with zero offset */
    int *iptr = iarr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += iptr[0];  /* SImode access */
        iptr++;
    }
    
    /* Long access with zero offset */
    long *lptr = larr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += lptr[0];  /* DImode access */
        lptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointers */
int test_volatile_access(void) {
    volatile int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i + 1;
    }
    
    volatile int *vptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[0];  /* Zero offset with volatile */
        vptr++;
    }
    
    return sum;
}

/* Test 5: Restrict pointers for alias analysis */
int test_restrict_pointers(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 5;
    }
    
    int *restrict rptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += rptr[0];  /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional access */
int test_nested_conditional(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Conditional access with zero offset */
        if (i % 2 == 0) {
            sum += ptr[0];  /* Access element 0 */
        } else {
            sum += ptr[1];  /* Access element 1 */
        }
        
        /* Multiple increments to create complex patterns */
        if (i % 3 == 0) {
            ptr += 1;
        } else if (i % 3 == 1) {
            ptr += 2;
        } else {
            ptr += 0;  /* Explicit zero increment */
        }
    }
    
    return sum;
}

/* Test 7: Structure with first member at offset 0 */
struct test_struct {
    int first;   /* Offset 0 */
    int second;
    char third;
};

int test_struct_first_member(void) {
    struct test_struct arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i].first = i * 10;
        arr[i].second = i * 20;
        arr[i].third = i & 0xFF;
    }
    
    struct test_struct *sptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Accessing first member is at offset 0 */
        sum += sptr->first;  /* Equivalent to sptr[0].first */
        sptr++;
    }
    
    return sum;
}

/* Test 8: Explicit cast of zero to pointer offset */
int test_explicit_zero_cast(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 7;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Explicit cast of zero to ptrdiff_t */
        sum += ptr[(ptrdiff_t)0];  /* Force const_int 0 in RTL */
        ptr++;
    }
    
    /* Another variant with size_t */
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[(size_t)0];  /* Different zero constant type */
        ptr++;
    }
    
    return sum;
}

/* Test 9: Multiple increments in loop */
int test_multiple_increments(void) {
    int arr[ARRAY_SIZE * 4];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE * 4; i++) {
        arr[i] = i;
    }
    
    /* Test different step sizes */
    for (int step = 1; step <= 4; step++) {
        int *ptr = arr;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            sum += ptr[0];  /* Zero offset */
            ptr += step;    /* Variable increment */
        }
    }
    
    return sum;
}

/* Test 10: Complex expression with zero offset */
int test_complex_zero_expr(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 11;
    }
    
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Complex expression that simplifies to + 0 */
        sum += ptr[i * 0];  /* Always zero offset */
        sum += ptr[0 * i];  /* Another zero offset variant */
        ptr++;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int test_num = 0;
    long total_result = 0;
    
    /* Use command line to select tests, ensuring all code is compiled */
    if (argc > 1) {
        test_num = atoi(argv[1]);
    }
    
    /* Run all tests or specific one based on argument */
    if (test_num == 0 || test_num == 1) {
        total_result += test_basic_post_inc();
        printf("Test 1 (basic post-inc): %ld\n", (long)test_basic_post_inc());
    }
    
    if (test_num == 0 || test_num == 2) {
        total_result += test_basic_post_dec();
        printf("Test 2 (basic post-dec): %ld\n", (long)test_basic_post_dec());
    }
    
    if (test_num == 0 || test_num == 3) {
        total_result += test_mixed_types();
        printf("Test 3 (mixed types): %ld\n", test_mixed_types());
    }
    
    if (test_num == 0 || test_num == 4) {
        total_result += test_volatile_access();
        printf("Test 4 (volatile): %ld\n", (long)test_volatile_access());
    }
    
    if (test_num == 0 || test_num == 5) {
        total_result += test_restrict_pointers();
        printf("Test 5 (restrict): %ld\n", (long)test_restrict_pointers());
    }
    
    if (test_num == 0 || test_num == 6) {
        total_result += test_nested_conditional();
        printf("Test 6 (nested conditional): %ld\n", (long)test_nested_conditional());
    }
    
    if (test_num == 0 || test_num == 7) {
        total_result += test_struct_first_member();
        printf("Test 7 (struct first member): %ld\n", (long)test_struct_first_member());
    }
    
    if (test_num == 0 || test_num == 8) {
        total_result += test_explicit_zero_cast();
        printf("Test 8 (explicit zero cast): %ld\n", (long)test_explicit_zero_cast());
    }
    
    if (test_num == 0 || test_num == 9) {
        total_result += test_multiple_increments();
        printf("Test 9 (multiple increments): %ld\n", (long)test_multiple_increments());
    }
    
    if (test_num == 0 || test_num == 10) {
        total_result += test_complex_zero_expr();
        printf("Test 10 (complex zero expr): %ld\n", (long)test_complex_zero_expr());
    }
    
    printf("Total checksum: %ld\n", total_result);
    
    /* Prevent dead code elimination */
    volatile int dummy = total_result;
    
    return 0;
}
