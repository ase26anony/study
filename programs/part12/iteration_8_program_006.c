/* test_auto_inc_dec.c - Program to trigger auto-inc-dec pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define SMALL_SIZE 16

/* Test 1: Simple post-increment with zero offset */
int test_simple_post_inc(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Pattern: ptr[i + 0] with post-increment */
    int *ptr = arr;
    int *end = arr + SIZE;
    while (ptr < end) {
        sum += ptr[0];          /* Zero offset access */
        ptr++;                  /* Post-increment */
    }
    
    return sum;
}

/* Test 2: Post-decrement with zero offset */
int test_simple_post_dec(void) {
    int arr[SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    /* Pattern: *(ptr + 0) with post-decrement */
    int *ptr = arr + SIZE - 1;
    int *start = arr;
    while (ptr >= start) {
        sum += *(ptr + 0);      /* Zero offset via pointer arithmetic */
        ptr--;                  /* Post-decrement */
    }
    
    return sum;
}

/* Test 3: Mixed data types with zero offset */
long test_mixed_types(void) {
    char c_arr[SIZE];
    short s_arr[SIZE];
    int i_arr[SIZE];
    long l_arr[SMALL_SIZE];
    long total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        c_arr[i] = (char)(i % 128);
        s_arr[i] = (short)(i * 2);
        i_arr[i] = i * 3;
    }
    for (int i = 0; i < SMALL_SIZE; i++) {
        l_arr[i] = i * 1000L;
    }
    
    /* Char array with zero offset */
    char *c_ptr = c_arr;
    for (int i = 0; i < SIZE; i++) {
        total += c_ptr[0];      /* QImode access */
        c_ptr++;
    }
    
    /* Short array with zero offset */
    short *s_ptr = s_arr;
    for (int i = 0; i < SIZE; i++) {
        total += s_ptr[0];      /* HImode access */
        s_ptr++;
    }
    
    /* Int array with zero offset */
    int *i_ptr = i_arr;
    for (int i = 0; i < SIZE; i++) {
        total += i_ptr[0];      /* SImode access */
        i_ptr++;
    }
    
    /* Long array with zero offset */
    long *l_ptr = l_arr;
    for (int i = 0; i < SMALL_SIZE; i++) {
        total += l_ptr[0];      /* DImode access */
        l_ptr++;
    }
    
    return total;
}

/* Test 4: Volatile pointer with zero offset */
int test_volatile_access(void) {
    static int arr[SIZE];
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 5;
    }
    
    /* Volatile access with zero offset */
    for (int i = 0; i < SIZE; i++) {
        sum += vptr[i + 0];     /* Zero offset with volatile */
    }
    
    /* Another pattern with restrict */
    int *restrict rptr = arr;
    for (int i = 0; i < SIZE; i++) {
        sum += *(rptr + 0);     /* Zero offset with restrict */
        rptr++;
    }
    
    return sum;
}

/* Test 5: Nested loops and conditional access */
int test_complex_patterns(void) {
    int arr[SIZE * 2];
    int sum = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE * 2; i++) {
        arr[i] = i % 100;
    }
    
    /* Nested loops with zero offset */
    for (int outer = 0; outer < 4; outer++) {
        int *ptr = arr + outer * (SIZE / 2);
        int *end = ptr + (SIZE / 2);
        
        while (ptr < end) {
            /* Conditional access with zero offset */
            if (*ptr > 50) {
                sum += ptr[0];          /* Zero offset in true branch */
            } else {
                sum -= *(ptr + 0);      /* Zero offset in false branch */
            }
            
            /* Mixed step sizes */
            if (outer % 2 == 0) {
                ptr += 1;               /* Step 1 */
            } else {
                ptr += 2;               /* Step 2 */
            }
        }
    }
    
    return sum;
}

/* Test 6: Structure access with zero offset */
struct test_struct {
    int first;      /* Offset 0 */
    int second;
    char third;
};

int test_struct_access(void) {
    struct test_struct arr[SMALL_SIZE];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < SMALL_SIZE; i++) {
        arr[i].first = i * 10;
        arr[i].second = i * 20;
        arr[i].third = (char)i;
    }
    
    /* Access first member (offset 0) */
    struct test_struct *sptr = arr;
    for (int i = 0; i < SMALL_SIZE; i++) {
        sum += sptr->first;     /* Accesses member at offset 0 */
        sptr++;
    }
    
    /* Cast zero to index */
    sptr = arr;
    for (int i = 0; i < SMALL_SIZE; i++) {
        sum += sptr[(int)(0)].first;    /* Zero offset via cast */
        sptr++;
    }
    
    return sum;
}

/* Test 7: Array of pointers with zero offset */
int test_pointer_array(void) {
    int data[SIZE];
    int *ptr_arr[SMALL_SIZE];
    int sum = 0;
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i * 7;
    }
    
    /* Set up pointer array */
    for (int i = 0; i < SMALL_SIZE; i++) {
        ptr_arr[i] = &data[i * (SIZE / SMALL_SIZE)];
    }
    
    /* Access through pointer array with zero offset */
    for (int i = 0; i < SMALL_SIZE; i++) {
        int **pptr = &ptr_arr[i];
        sum += (*pptr)[0];      /* Double indirection with zero offset */
    }
    
    return sum;
}

/* Test 8: Function pointer chasing with zero offset */
typedef int (*func_ptr_t)(int);

int func1(int x) { return x + 1; }
int func2(int x) { return x * 2; }
int func3(int x) { return x - 3; }

int test_function_pointers(void) {
    func_ptr_t funcs[] = {func1, func2, func3};
    int results[3] = {0};
    int sum = 0;
    
    /* Call functions and store results with zero offset */
    for (int i = 0; i < 3; i++) {
        results[i] = funcs[i](i * 10);
    }
    
    /* Access results with zero offset pattern */
    int *rptr = results;
    for (int i = 0; i < 3; i++) {
        sum += rptr[0];         /* Zero offset access */
        rptr++;
    }
    
    return sum;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int test_to_run = 0;
    long total_result = 0;
    
    /* Use command line to select test, ensuring all code is compiled */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run all tests regardless to ensure compilation */
    switch (test_to_run) {
        case 1:
            total_result = test_simple_post_inc();
            break;
        case 2:
            total_result = test_simple_post_dec();
            break;
        case 3:
            total_result = test_mixed_types();
            break;
        case 4:
            total_result = test_volatile_access();
            break;
        case 5:
            total_result = test_complex_patterns();
            break;
        case 6:
            total_result = test_struct_access();
            break;
        case 7:
            total_result = test_pointer_array();
            break;
        case 8:
            total_result = test_function_pointers();
            break;
        default:
            /* Run all tests */
            total_result += test_simple_post_inc();
            total_result += test_simple_post_dec();
            total_result += test_mixed_types();
            total_result += test_volatile_access();
            total_result += test_complex_patterns();
            total_result += test_struct_access();
            total_result += test_pointer_array();
            total_result += test_function_pointers();
            break;
    }
    
    /* Print result to prevent optimization */
    printf("Result: %ld\n", total_result);
    
    return 0;
}
