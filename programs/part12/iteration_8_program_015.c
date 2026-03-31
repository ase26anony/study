/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for lines 1352-1358
 * Specifically targets (mem (plus (reg) (const_int 0))) patterns
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
        arr[i] = i % 100;
    }
    
    /* Loop with pointer + 0 access pattern */
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE;
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    while (ptr < end) {
        sum += *(ptr + 0);  /* Zero offset access */
        ptr++;              /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Different data types with zero offset */
int test2_mixed_types(void) {
    char c_arr[ARRAY_SIZE];
    short s_arr[ARRAY_SIZE];
    int i_arr[ARRAY_SIZE];
    long l_arr[ARRAY_SIZE];
    
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        c_arr[i] = (char)(i % 128);
        s_arr[i] = (short)(i % 1000);
        i_arr[i] = i;
        l_arr[i] = i * 2L;
    }
    
    /* Char pointer loop */
    char *cptr = c_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += cptr[0];  /* Zero offset - QImode */
        cptr++;
    }
    
    /* Short pointer loop */
    short *sptr = s_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += sptr[0];  /* Zero offset - HImode */
        sptr++;
    }
    
    /* Int pointer loop */
    int *iptr = i_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += iptr[0];  /* Zero offset - SImode */
        iptr++;
    }
    
    /* Long pointer loop */
    long *lptr = l_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += (int)lptr[0];  /* Zero offset - DImode */
        lptr++;
    }
    
    return sum;
}

/* Test 3: Volatile pointers with zero offset */
int test3_volatile_access(void) {
    volatile int v_arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        v_arr[i] = i % 50;
    }
    
    /* Volatile pointer with restrict to help optimization */
    volatile int * restrict vptr = v_arr;
    
    /* Loop with zero offset access */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple zero-offset accesses to create pattern */
        sum += vptr[i + 0];      /* Array index with +0 */
        sum += *(vptr + i + 0);  /* Pointer arithmetic with +0 */
    }
    
    /* Another loop with post-increment */
    vptr = v_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr[0];  /* Direct zero offset */
        vptr++;          /* Post-increment */
    }
    
    return sum;
}

/* Test 4: Complex control flow with zero offset */
int test4_complex_flow(void) {
    int arr[ARRAY_SIZE * 2];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        arr[i] = i % 75;
    }
    
    int *ptr = arr;
    int *end = arr + ARRAY_SIZE * 2;
    
    /* Nested loops with conditional zero-offset access */
    while (ptr < end) {
        /* Outer loop with zero offset */
        sum += ptr[0];
        
        /* Inner loop with different step */
        int *inner_ptr = ptr;
        for (int j = 0; j < 4 && inner_ptr < end; j++) {
            if (j % 2 == 0) {
                sum += inner_ptr[0];  /* Zero offset in conditional */
            } else {
                sum += inner_ptr[1];
            }
            inner_ptr += 2;  /* Different step size */
        }
        
        /* Conditional zero offset access */
        if (sum % 2 == 0) {
            sum += *(ptr + 0);  /* Another zero offset pattern */
        }
        
        ptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 5: Post-decrement patterns */
int test5_post_decrement(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i % 60;
    }
    
    /* Post-decrement loop */
    int *ptr = arr + ARRAY_SIZE - 1;
    
    /* This should also generate (plus (reg) (const_int 0)) patterns */
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
        sum += ptr[0];  /* Zero offset */
        ptr--;          /* Post-decrement */
    }
    
    /* Another variant with while loop */
    ptr = arr + ARRAY_SIZE;
    while (ptr > arr) {
        ptr--;          /* Pre-decrement then access */
        sum += ptr[0];  /* Zero offset after decrement */
    }
    
    return sum;
}

/* Test 6: Structure access with zero offset */
struct test_struct {
    int first;   /* Offset 0 */
    int second;
    char third;
};

int test6_struct_access(void) {
    struct test_struct s_arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        s_arr[i].first = i;
        s_arr[i].second = i * 2;
        s_arr[i].third = (char)(i % 128);
    }
    
    /* Access first member (offset 0) */
    struct test_struct *sptr = s_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Accessing first member is at offset 0 */
        sum += sptr->first;  /* This is offset 0 */
        sum += sptr[0].first; /* Another zero offset pattern */
        sptr++;
    }
    
    return sum;
}

/* Test 7: Cast zero to pointer offset */
int test7_cast_zero_offset(void) {
    int arr[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i % 80;
    }
    
    /* Explicit cast of zero to ptrdiff_t */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Cast zero to different integer types used as offsets */
        sum += ptr[(ptrdiff_t)0];      /* ptrdiff_t zero */
        sum += ptr[(int)0];            /* int zero */
        sum += ptr[(unsigned int)0];   /* unsigned zero */
        sum += ptr[(long)0];           /* long zero */
        ptr++;
    }
    
    return sum;
}

/* Test 8: Multiple zero offsets in same expression */
int test8_multiple_zero_offsets(void) {
    int arr1[ARRAY_SIZE];
    int arr2[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i % 90;
        arr2[i] = i % 70;
    }
    
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    /* Multiple pointers with zero offsets in same loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple (plus (reg) (const_int 0)) patterns */
        int val1 = ptr1[0 + 0];      /* Double zero */
        int val2 = *(ptr2 + 0);      /* Pointer + 0 */
        sum += val1 + val2;
        
        /* More complex expression with zero offsets */
        sum += (ptr1[0] * 2) + (ptr2[0] / 2);
        
        ptr1++;
        ptr2++;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    int test_to_run = -1;
    
    /* Use command line argument to select test or run all */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    if (test_to_run == 1 || test_to_run == -1) {
        total_sum += test1_basic_zero_offset();
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == 2 || test_to_run == -1) {
        total_sum += test2_mixed_types();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == 3 || test_to_run == -1) {
        total_sum += test3_volatile_access();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == 4 || test_to_run == -1) {
        total_sum += test4_complex_flow();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == 5 || test_to_run == -1) {
        total_sum += test5_post_decrement();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == 6 || test_to_run == -1) {
        total_sum += test6_struct_access();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == 7 || test_to_run == -1) {
        total_sum += test7_cast_zero_offset();
        printf("Test 7 completed\n");
    }
    
    if (test_to_run == 8 || test_to_run == -1) {
        total_sum += test8_multiple_zero_offsets();
        printf("Test 8 completed\n");
    }
    
    /* Print checksum to prevent optimization and verify correctness */
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
