/* test_auto_inc_dec.c - Program to trigger auto-inc-dec pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define SMALL_SIZE 16

/* Test 1: Basic zero-offset access with post-increment */
int test_basic_zero_offset(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* This should generate (mem (plus (reg) (const_int 0))) */
    while (p < end) {
        sum += p[0];  /* Zero offset access */
        p++;
    }
    return sum;
}

/* Test 2: Pointer arithmetic with explicit + 0 */
int test_explicit_zero_add(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += *(p + 0);  /* Explicit + 0 */
        p++;
    }
    return sum;
}

/* Test 3: Different data types for different memory modes */
long test_mixed_types(void) {
    char c_arr[SIZE];
    short s_arr[SIZE];
    int i_arr[SIZE];
    long l_arr[SIZE];
    long long ll_arr[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        c_arr[i] = (char)(i % 256);
        s_arr[i] = (short)(i % 65536);
        i_arr[i] = i;
        l_arr[i] = i * 2L;
        ll_arr[i] = i * 3LL;
    }
    
    long total = 0;
    
    /* Char access - QImode */
    char *cp = c_arr;
    for (int i = 0; i < SIZE; i++) {
        total += cp[0];  /* Zero offset */
        cp++;
    }
    
    /* Short access - HImode */
    short *sp = s_arr;
    for (int i = 0; i < SIZE; i++) {
        total += sp[0];  /* Zero offset */
        sp++;
    }
    
    /* Int access - SImode */
    int *ip = i_arr;
    for (int i = 0; i < SIZE; i++) {
        total += ip[0];  /* Zero offset */
        ip++;
    }
    
    /* Long access - DImode */
    long *lp = l_arr;
    for (int i = 0; i < SIZE; i++) {
        total += lp[0];  /* Zero offset */
        lp++;
    }
    
    return total;
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

/* Test 5: Restrict pointer for alias analysis */
int test_restrict_zero_offset(int *restrict arr, int n) {
    int sum = 0;
    int *restrict p = arr;
    
    for (int i = 0; i < n; i++) {
        sum += p[0];  /* Zero offset with restrict */
        p++;
    }
    return sum;
}

/* Test 6: Nested loops with conditional zero offset */
int test_nested_conditional(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Conditional zero offset access */
        if (i % 2 == 0) {
            sum += p[0];  /* Zero offset in if branch */
        } else {
            sum += p[0] * 2;  /* Same zero offset in else branch */
        }
        
        /* Sometimes skip increment to create more complex patterns */
        if (i % 3 != 0) {
            p++;
        }
    }
    return sum;
}

/* Test 7: Post-decrement with zero offset */
int test_post_decrement(int *arr, int n) {
    int sum = 0;
    int *p = arr + n - 1;
    
    for (int i = n - 1; i >= 0; i--) {
        sum += p[0];  /* Zero offset */
        p--;  /* Post-decrement */
    }
    return sum;
}

/* Test 8: Structure with first member at offset 0 */
struct first_member {
    int value;      /* Offset 0 */
    char data[32];
    double dbl;
};

int test_struct_offset_zero(struct first_member *arr, int n) {
    int sum = 0;
    struct first_member *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Accessing first member = offset 0 */
        sum += p->value;  /* Equivalent to p[0].value */
        p++;
    }
    return sum;
}

/* Test 9: Cast zero to pointer offset type */
int test_cast_zero_offset(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Cast zero to ptrdiff_t/ssize_t to force constant 0 */
        sum += p[(ptrdiff_t)0];  /* Cast zero to offset type */
        p++;
    }
    return sum;
}

/* Test 10: Complex loop with multiple zero-offset accesses */
int test_complex_multiple_accesses(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; i++) {
        /* Multiple zero-offset accesses in same iteration */
        int val1 = p1[0];  /* First zero offset */
        int val2 = p2[0];  /* Second zero offset */
        
        sum += val1 + val2;
        
        /* Different increment patterns */
        p1++;
        if (i % 2 == 0) {
            p2++;
        }
    }
    return sum;
}

/* Test 11: Array of pointers with zero offset dereference */
int test_pointer_array(int **ptr_arr, int n) {
    int sum = 0;
    int **p = ptr_arr;
    
    for (int i = 0; i < n; i++) {
        /* Dereference pointer with zero offset */
        if (p[0] != NULL) {  /* Zero offset access to pointer array */
            sum += *(p[0]);   /* Then dereference that pointer */
        }
        p++;
    }
    return sum;
}

/* Test 12: Loop with step size 2 */
int test_step_size_two(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i += 2) {
        sum += p[0];  /* Zero offset */
        p += 2;  /* Step by 2 */
    }
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    /* Initialize test arrays */
    int int_arr[SIZE];
    int int_arr2[SIZE];
    volatile int volatile_arr[SIZE];
    struct first_member struct_arr[SMALL_SIZE];
    int *ptr_arr[SMALL_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        int_arr2[i] = i * 2;
        volatile_arr[i] = i * 3;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        struct_arr[i].value = i * 10;
        ptr_arr[i] = &int_arr[i];
    }
    
    int total = 0;
    
    /* Run tests based on command line arguments or all by default */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "test1") || argc == 1) {
        total += test_basic_zero_offset(int_arr, SIZE);
    }
    
    if (run_all || strstr(argv[0], "test2") || argc == 1) {
        total += test_explicit_zero_add(int_arr, SIZE);
    }
    
    if (run_all || strstr(argv[0], "test3") || argc == 1) {
        total += test_mixed_types();
    }
    
    if (run_all || strstr(argv[0], "test4") || argc == 1) {
        total += test_volatile_zero_offset(volatile_arr, SIZE);
    }
    
    if (run_all || strstr(argv[0], "test5") || argc == 1) {
        total += test_restrict_zero_offset(int_arr, SIZE);
    }
    
    if (run_all || strstr(argv[0], "test6") || argc == 1) {
        total += test_nested_conditional(int_arr, SMALL_SIZE);
    }
    
    if (run_all || strstr(argv[0], "test7") || argc == 1) {
        total += test_post_decrement(int_arr, SMALL_SIZE);
    }
    
    if (run_all || strstr(argv[0], "test8") || argc == 1) {
        total += test_struct_offset_zero(struct_arr, SMALL_SIZE);
    }
    
    if (run_all || strstr(argv[0], "test9") || argc == 1) {
        total += test_cast_zero_offset(int_arr, SMALL_SIZE);
    }
    
    if (run_all || strstr(argv[0], "test10") || argc == 1) {
        total += test_complex_multiple_accesses(int_arr, int_arr2, SMALL_SIZE);
    }
    
    if (run_all || strstr(argv[0], "test11") || argc == 1) {
        total += test_pointer_array(ptr_arr, SMALL_SIZE);
    }
    
    if (run_all || strstr(argv[0], "test12") || argc == 1) {
        total += test_step_size_two(int_arr, SIZE);
    }
    
    /* Print result to prevent optimization removal */
    printf("Total checksum: %d\n", total);
    
    return 0;
}
