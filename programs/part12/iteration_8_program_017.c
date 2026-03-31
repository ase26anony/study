/* auto-inc-dec-test.c - Test program to trigger auto-inc-dec pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define ITERS 100

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
        /* Force (plus (reg) (const_int 0)) pattern */
        sum += ptr[0];          /* Zero offset array access */
        sum += *(ptr + 0);      /* Another zero offset pattern */
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
        /* Multiple zero-offset patterns */
        sum += ptr[0];
        sum += *(ptr + 0);
        ptr--;                  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Different data types for different memory modes */
void test3_mixed_types(void) {
    char carr[SIZE];
    short sarr[SIZE];
    int iarr[SIZE];
    long larr[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        carr[i] = (char)(i & 0xFF);
        sarr[i] = (short)(i * 2);
        iarr[i] = i * 3;
        larr[i] = i * 4L;
    }
    
    /* Char pointer loop - QImode */
    char *cptr = carr;
    for (int i = 0; i < SIZE; i++) {
        /* Force zero offset with cast */
        char val = cptr[(int)(0)];
        cptr++;
    }
    
    /* Short pointer loop - HImode */
    short *sptr = sarr;
    for (int i = 0; i < SIZE; i++) {
        short val = sptr[0];
        sptr++;
    }
    
    /* Int pointer loop - SImode */
    int *iptr = iarr;
    for (int i = 0; i < SIZE; i++) {
        int val = iptr[0];
        iptr++;
    }
    
    /* Long pointer loop - DImode */
    long *lptr = larr;
    for (int i = 0; i < SIZE; i++) {
        long val = lptr[0];
        lptr++;
    }
}

/* Test 4: Volatile pointers with zero offset */
int test4_volatile_access(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Volatile pointer - may create different RTL patterns */
    volatile int *vptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Zero offset with volatile */
        sum += vptr[i + 0];
        vptr++;  /* Post-increment */
    }
    
    return sum;
}

/* Test 5: Restrict pointers for alias analysis */
int test5_restrict_pointers(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Restrict gives stronger aliasing guarantees */
    int *restrict rptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += rptr[0];  /* Zero offset */
        rptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with conditional zero-offset access */
int test6_nested_conditional(void) {
    int arr[SIZE][4];
    int sum = 0;
    
    /* Initialize 2D array */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i][j] = i * 4 + j;
        }
    }
    
    /* Nested loop with conditional access */
    for (int i = 0; i < SIZE; i++) {
        int *row = arr[i];
        for (int j = 0; j < 4; j++) {
            if (j % 2 == 0) {
                /* Zero offset in conditional path */
                sum += row[0];
            } else {
                sum += row[1];
            }
            row++;  /* Post-increment */
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

int test7_struct_first_member(void) {
    struct test_struct arr[SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        arr[i].first = i;
        arr[i].second = i * 2;
        arr[i].third = (char)(i & 0xFF);
    }
    
    /* Access first member (offset 0) in loop */
    struct test_struct *sptr = arr;
    for (int i = 0; i < SIZE; i++) {
        /* sptr->first is at offset 0 */
        sum += sptr->first;
        sptr++;
    }
    
    return sum;
}

/* Test 8: Multiple induction variables with different steps */
int test8_multiple_induction(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i;
    }
    
    /* Multiple pointers with different update patterns */
    int *ptr1 = arr;
    int *ptr2 = arr;
    for (int i = 0; i < SIZE; i++) {
        /* Both use zero offset */
        sum += ptr1[0];
        sum += ptr2[0];
        
        ptr1 += 1;  /* Step by 1 */
        ptr2 += 2;  /* Step by 2 */
    }
    
    return sum;
}

/* Test 9: Complex expression with zero offset */
int test9_complex_zero_offset(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    int *ptr = arr;
    int index = 0;
    while (index < SIZE) {
        /* Complex expression that simplifies to +0 */
        int offset = (index * 0) + (0 * index) + 0;
        sum += ptr[offset];
        
        ptr++;
        index++;
    }
    
    return sum;
}

/* Test 10: Function pointer array with zero offset */
typedef int (*func_ptr_t)(void);
int func1(void) { return 1; }
int func2(void) { return 2; }
int func3(void) { return 3; }

int test10_function_pointers(void) {
    func_ptr_t funcs[3] = {func1, func2, func3};
    int sum = 0;
    
    func_ptr_t *fptr = funcs;
    for (int i = 0; i < 3; i++) {
        /* Zero offset function pointer access */
        sum += fptr[0]();
        fptr++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Run tests based on command line arguments */
    if (argc == 1) {
        /* Run all tests if no arguments */
        total += test1_simple_postinc();
        total += test2_postdec();
        test3_mixed_types();
        total += test4_volatile_access();
        total += test5_restrict_pointers();
        total += test6_nested_conditional();
        total += test7_struct_first_member();
        total += test8_multiple_induction();
        total += test9_complex_zero_offset();
        total += test10_function_pointers();
    } else {
        /* Run specific tests based on arguments */
        for (int i = 1; i < argc; i++) {
            int test_num = atoi(argv[i]);
            switch (test_num) {
                case 1: total += test1_simple_postinc(); break;
                case 2: total += test2_postdec(); break;
                case 3: test3_mixed_types(); break;
                case 4: total += test4_volatile_access(); break;
                case 5: total += test5_restrict_pointers(); break;
                case 6: total += test6_nested_conditional(); break;
                case 7: total += test7_struct_first_member(); break;
                case 8: total += test8_multiple_induction(); break;
                case 9: total += test9_complex_zero_offset(); break;
                case 10: total += test10_function_pointers(); break;
                default: printf("Unknown test: %d\n", test_num);
            }
        }
    }
    
    printf("Checksum: %d\n", total);
    return 0;
}
