/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_use(int val);
extern void dummy_store(int *ptr);

/* Prevent inlining to keep patterns intact */
#define NOINLINE __attribute__((noinline))

/* Variant 1: Using *(p + 0) and p++ */
NOINLINE int test_variant1(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    while (p < end) {
        /* Explicit zero offset access - must not fold to *p */
        int val = *(p + 0);  /* This should create mem_insn with reg1_val = 0 */
        sum += val;
        /* Separate increment statement */
        p++;  /* This should be found by find_inc() */
    }
    
    /* Use result to prevent elimination */
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Variant 2: Using p[0] and p += 1 */
NOINLINE int test_variant2(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Array access with index 0 */
        sum += p[0];  /* Should be equivalent to *(p + 0) */
        /* Separate increment */
        p += 1;  /* Constant increment by 1 */
    }
    
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Variant 3: Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

NOINLINE int test_variant3(int *arr, int n) {
    struct ptr_wrapper w = {arr, arr + n};
    int sum = 0;
    
    while (w.current < w.end) {
        /* Access through structure member with zero offset */
        int val = *(w.current + 0);
        sum += val;
        /* Increment the pointer in the structure */
        w.current++;  /* Post-increment */
    }
    
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Variant 4: Store instead of load */
NOINLINE void test_variant4(int *arr, int n, int value) {
    int *p = arr;
    int *end = arr + n;
    
    while (p < end) {
        /* Store with zero offset */
        *(p + 0) = value;  /* Store operation with reg1_val = 0 */
        /* Decrement variant */
        p--;  /* Negative increment */
        /* Adjust to avoid infinite loop in this example */
        break;
    }
    
    /* For proper testing, we'd need a different loop structure */
    /* This is simplified to show the pattern */
    asm volatile("" : : "r"(p) : "memory");
}

/* Variant 5: More complex zero offset expression */
NOINLINE int test_variant5(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *limit = arr + n;
    
    /* Use volatile to prevent constant propagation */
    volatile int *volatile_ptr = arr;
    
    while (p < limit) {
        /* Complex-looking but still zero offset */
        int val = *(p + (0));  /* Parentheses around 0 */
        sum += val;
        
        /* Increment with assignment */
        p = p + 1;  /* Different syntax for increment */
    }
    
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Variant 6: Loop with if condition that doesn't prevent pattern */
NOINLINE int test_variant6(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int count = n;
    
    while (count-- > 0) {
        /* Access with zero offset */
        if (p != NULL) {  /* Condition that doesn't affect the pattern */
            sum += *(p + 0);
        }
        /* Post-increment */
        ++p;  /* Pre-increment variant */
    }
    
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Main function to drive tests */
int main(int argc, char **argv) {
    /* Use argc to make array size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate array dynamically to avoid constant folding */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < size; i++) {
        array[i] = i * 3 + 1;  /* Non-constant pattern */
    }
    
    int total = 0;
    
    /* Call all test variants */
    total += test_variant1(array, size);
    total += test_variant2(array, size);
    total += test_variant3(array, size);
    
    /* For store variant, we need a different array */
    int *array2 = (int *)malloc(size * sizeof(int));
    if (array2) {
        test_variant4(array2, size, 42);
        free(array2);
    }
    
    total += test_variant5(array, size);
    total += test_variant6(array, size);
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %d\n", total);
    
    free(array);
    return 0;
}
