/* Test program for GCC auto-increment/decrement optimization */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -march=armv7-a -c test.c */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void use_value(int val);

/* Volatile variable to prevent constant propagation */
volatile int g_volatile_bound = 100;

/* Variant 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test_variant1(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Explicit zero offset - should generate *(p + 0) */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    /* Use result to prevent elimination */
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Variant 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test_variant2(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Separate increment with assignment */
        p += 1;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Variant 3: Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
int test_variant3(int *arr, int n) {
    struct ptr_wrapper wrapper;
    wrapper.current = arr;
    wrapper.end = arr + n;
    int sum = 0;
    
    while (wrapper.current < wrapper.end) {
        /* Access through structure pointer */
        int val = *(wrapper.current + 0);
        sum += val;
        /* Increment structure member */
        wrapper.current = wrapper.current + 1;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Variant 4: Store instead of load */
__attribute__((noinline))
void test_variant4(int *arr, int n, int value) {
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Decrement instead of increment */
        p = p + 1;
    }
    
    /* Ensure stores aren't eliminated */
    asm volatile("" : : "m"(*arr));
}

/* Variant 5: Different data type and increment */
__attribute__((noinline))
long test_variant5(long *arr, int n) {
    long *p = arr;
    long sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Different type, still zero offset */
        long val = *(p + 0);
        sum += val;
        /* Post-increment in separate statement */
        p = p + 1;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Variant 6: Char pointer with zero offset */
__attribute__((noinline))
int test_variant6(char *arr, int n) {
    char *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Char type, zero offset */
        char val = *(p + 0);
        sum += val;
        /* Increment */
        p++;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Variant 7: Double pointer with negative increment */
__attribute__((noinline))
double test_variant7(double *arr, int n) {
    double *p = arr + n - 1;
    double sum = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* Access with zero offset from end */
        double val = *(p + 0);
        sum += val;
        /* Decrement pointer */
        p = p - 1;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Main function that calls all variants */
int main(int argc, char **argv) {
    /* Use argc to make array size non-constant */
    int size = g_volatile_bound + argc;
    if (size < 10) size = 100;
    
    /* Dynamically allocate arrays */
    int *int_arr = malloc(size * sizeof(int));
    long *long_arr = malloc(size * sizeof(long));
    char *char_arr = malloc(size * sizeof(char));
    double *double_arr = malloc(size * sizeof(double));
    
    if (!int_arr || !long_arr || !char_arr || !double_arr) {
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        int_arr[i] = i * 3;
        long_arr[i] = i * 5L;
        char_arr[i] = (char)(i % 256);
        double_arr[i] = i * 2.5;
    }
    
    int total = 0;
    
    /* Call all test variants */
    total += test_variant1(int_arr, size);
    total += test_variant2(int_arr, size);
    total += test_variant3(int_arr, size);
    
    test_variant4(int_arr, size, 42);
    
    /* Cast result to int for total */
    total += (int)test_variant5(long_arr, size);
    total += test_variant6(char_arr, size);
    
    /* Double result affects total through side effect only */
    test_variant7(double_arr, size);
    
    /* Print result to prevent elimination */
    printf("Total: %d\n", total);
    
    /* Clean up */
    free(int_arr);
    free(long_arr);
    free(char_arr);
    free(double_arr);
    
    return 0;
}
