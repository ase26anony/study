#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g1 = 10;
volatile int g2 = 20;
volatile int g3 = 30;
volatile int g4 = 40;
static volatile int counter = 0;

/* Function with side effects for condition testing */
__attribute__((noinline)) 
int side_effect_func(void) {
    static int state = 0;
    return state++ % 3;
}

/* Test function 1: Simple modification of condition variable in then block */
__attribute__((noinline))
void test1_modify_in_then(void) {
    volatile int a = g1;
    volatile int b = g2;
    volatile int result = 0;
    
    /* Loop to create basic block context */
    for (int i = 0; i < 100; i++) {
        /* Condition where 'a' is tested */
        if (a > b) {
            /* MODIFICATION IN THEN BLOCK HEADER: 
               'a' is modified here, which is part of the condition */
            a = b + i;  // This should trigger modified_in_p check
            result += a * 2;
        } else {
            result += b;
        }
        
        /* Additional operations to prevent simplification */
        b += (i % 3);
    }
    
    g1 = a;
    g2 = b;
}

/* Test function 2: Compound condition with multiple modifications */
__attribute__((noinline))
void test2_compound_condition(void) {
    volatile int x = g1;
    volatile int y = g2;
    volatile int z = g3;
    
    for (int i = 0; i < 50; i++) {
        /* Complex condition using multiple variables */
        if (x != 0 && y < z && (x + y) > 10) {
            /* Multiple modifications of condition variables */
            x = y;      // 'x' used in condition (x != 0)
            y = z + i;  // 'y' used in condition (y < z)
            z = x * 2;  // 'z' used in condition (y < z)
            
            /* Additional non-trivial operations */
            if (i % 5 == 0) {
                x += 1;
            }
        } else {
            z = x + y;
        }
        
        /* Loop-dependent modification */
        x += (i % 7);
    }
    
    g1 = x;
    g2 = y;
    g3 = z;
}

/* Test function 3: Function call in condition with modification */
__attribute__((noinline))
void test3_func_in_condition(void) {
    volatile int a = g1;
    volatile int b = g2;
    
    for (int i = 0; i < 75; i++) {
        /* Function call in condition */
        if (side_effect_func() > 0 && a < b) {
            /* Modify variables that could affect future function calls */
            counter++;  // side_effect_func uses static state
            a = b + counter;  // 'a' used in condition
            
            /* Additional instructions in header */
            b = a - 1;
            a = b * 2;
        } else {
            b = a + i;
        }
        
        /* Ensure loop has side effects */
        a += (i % 4);
    }
    
    g1 = a;
    g2 = b;
}

/* Test function 4: Nested conditions with modifications */
__attribute__((noinline))
void test4_nested_modifications(void) {
    volatile int p = g1;
    volatile int q = g2;
    volatile int r = g3;
    volatile int s = g4;
    
    int iterations = 60;
    while (iterations-- > 0) {
        /* Multi-part condition */
        if ((p > q) || (r < s && p != 0)) {
            /* Multiple assignments to condition variables */
            if (p > q) {
                p = q - 1;  // 'p' used in condition
                q = r;      // 'q' used in condition
            }
            
            if (r < s) {
                r = s + p;  // 'r' used in condition
                s = p / 2;  // 's' used in condition
            }
            
            /* More operations in the header */
            p += 2;
            q -= 1;
        } else {
            r = p + q + s;
        }
        
        /* Loop variation */
        s += (iterations % 6);
    }
    
    g1 = p;
    g2 = q;
    g3 = r;
    g4 = s;
}

/* Test function 5: Pointer modification affecting condition */
__attribute__((noinline))
void test5_pointer_modification(void) {
    volatile int arr[4] = {g1, g2, g3, g4};
    volatile int *ptr1 = &arr[0];
    volatile int *ptr2 = &arr[2];
    
    for (int i = 0; i < 40; i++) {
        /* Condition using pointer dereferences */
        if (*ptr1 > *ptr2 && ptr1 != ptr2) {
            /* Modify through pointers - affects dereferenced values */
            *ptr1 = *ptr2 + i;  // Affects *ptr1 in condition
            ptr1 = &arr[i % 4]; // Changes what ptr1 points to
            
            /* Additional header instructions */
            *ptr2 += 1;
            arr[1] = *ptr1;
        } else {
            *ptr2 = *ptr1 - i;
        }
        
        /* Loop body with array access */
        arr[i % 4] += i;
    }
    
    g1 = arr[0];
    g2 = arr[1];
    g3 = arr[2];
    g4 = arr[3];
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to introduce runtime variability */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with some variability */
    g1 = 10 + (seed % 5);
    g2 = 20 + (seed % 7);
    g3 = 30 + (seed % 11);
    g4 = 40 + (seed % 13);
    counter = seed % 100;
    
    /* Run all test functions */
    test1_modify_in_then();
    test2_compound_condition();
    test3_func_in_condition();
    test4_nested_modifications();
    test5_pointer_modification();
    
    /* Compute and print result to ensure side effects are observable */
    int result = g1 + g2 * 2 + g3 * 3 + g4 * 4 + counter;
    printf("Result: %d (g1=%d, g2=%d, g3=%d, g4=%d, counter=%d)\n", 
           result, g1, g2, g3, g4, counter);
    
    return 0;
}
