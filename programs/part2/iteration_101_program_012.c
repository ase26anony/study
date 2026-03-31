#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and ensure basic blocks are preserved */
#define NOINLINE __attribute__((noinline))

/* Global volatile variables to prevent constant folding */
volatile int g_cond_a = 0;
volatile int g_cond_b = 0;
volatile int g_cond_c = 0;
volatile int g_cond_d = 0;
static volatile int s_counter = 0;

/* Function with side effects for condition tests */
NOINLINE int side_effect_func(void) {
    static int call_count = 0;
    return ++call_count;
}

/* Pattern 1: Simple modification of condition variable in then block */
NOINLINE void test_pattern1(void) {
    volatile int a = g_cond_a;
    volatile int b = g_cond_b;
    
    /* Loop to create interesting block structure */
    for (int i = 0; i < 10; i++) {
        /* Condition uses a and b */
        if (a > b) {
            /* MODIFIES condition variable 'a' in then block header */
            a = b + i;  // This should trigger modified_in_p check
            /* Additional instructions to create non-trivial block */
            g_cond_c = a * 2;
            g_cond_d = b / 2;
        }
        /* Prevent loop unrolling from simplifying too much */
        if (i % 3 == 0) {
            b++;
        }
    }
    
    g_cond_a = a;
    g_cond_b = b;
}

/* Pattern 2: Compound condition with multiple modifications */
NOINLINE void test_pattern2(void) {
    volatile int x = g_cond_a;
    volatile int y = g_cond_b;
    volatile int z = g_cond_c;
    
    for (int i = 0; i < 8; i++) {
        /* Complex compound condition */
        if (x != 0 && y < z && (x + y) > 5) {
            /* Multiple modifications of condition variables */
            x = y;      // Modifies 'x' used in (x != 0) and (x + y) > 5
            y = z + i;  // Modifies 'y' used in (y < z) and (x + y) > 5
            /* Additional instruction to ensure block has content */
            z = x * y;
        }
        
        /* Alternate path to create control flow */
        if (i % 2 == 0) {
            x += 2;
        } else {
            z -= 1;
        }
    }
    
    g_cond_a = x;
    g_cond_b = y;
    g_cond_c = z;
}

/* Pattern 3: Static variable modified in then block */
NOINLINE void test_pattern3(void) {
    volatile int limit = g_cond_d;
    
    for (int i = 0; i < 12; i++) {
        /* Condition uses static variable */
        if (s_counter < limit) {
            /* Modifies the static variable used in condition */
            s_counter++;  // Direct modification of condition variable
            /* Additional computation */
            g_cond_a += s_counter;
            g_cond_b -= i;
        }
        
        /* Vary the limit to prevent optimization */
        if (i % 4 == 0) {
            limit += side_effect_func();
        }
    }
    
    g_cond_d = limit;
}

/* Pattern 4: Function call in condition with side effects */
NOINLINE void test_pattern4(void) {
    volatile int base = g_cond_a;
    
    for (int i = 0; i < 6; i++) {
        /* Function call in condition */
        if (side_effect_func() > 2 && base > 0) {
            /* Modify variable that affects future function calls */
            base = -base;  // Changes future condition evaluations
            /* Global side effect */
            g_cond_c = side_effect_func() * 2;
        }
        
        /* Ensure loop has multiple iterations */
        base += i;
    }
    
    g_cond_a = base;
}

/* Pattern 5: Nested conditions with modifications */
NOINLINE void test_pattern5(void) {
    volatile int p = g_cond_b;
    volatile int q = g_cond_c;
    volatile int r = g_cond_d;
    
    int iterations = 5 + (g_cond_a % 3);
    for (int i = 0; i < iterations; i++) {
        /* Outer condition */
        if (p > q) {
            /* Inner condition creating complex block structure */
            if (q < r) {
                /* Modify variables from outer condition */
                p = q + r;  // Modifies 'p' from outer condition
                q = p * 2;  // Modifies 'q' from both conditions
            }
            /* Additional modification in outer then block */
            r += p;  // Modifies 'r' from inner condition
        }
        
        /* Loop variant to prevent dead code elimination */
        if (i % 2) {
            p += side_effect_func();
        }
    }
    
    g_cond_b = p;
    g_cond_c = q;
    g_cond_d = r;
}

int main(int argc, char *argv[]) {
    /* Initialize with non-constant values */
    g_cond_a = (argc > 1) ? atoi(argv[1]) : 5;
    g_cond_b = (argc > 2) ? atoi(argv[2]) : 10;
    g_cond_c = (argc > 3) ? atoi(argv[3]) : 15;
    g_cond_d = (argc > 4) ? atoi(argv[4]) : 20;
    
    printf("Initial values: a=%d, b=%d, c=%d, d=%d\n", 
           g_cond_a, g_cond_b, g_cond_c, g_cond_d);
    
    /* Execute all test patterns */
    test_pattern1();
    printf("After pattern1: a=%d, b=%d, c=%d, d=%d\n", 
           g_cond_a, g_cond_b, g_cond_c, g_cond_d);
    
    test_pattern2();
    printf("After pattern2: a=%d, b=%d, c=%d, d=%d\n", 
           g_cond_a, g_cond_b, g_cond_c, g_cond_d);
    
    test_pattern3();
    printf("After pattern3: a=%d, b=%d, c=%d, d=%d, counter=%d\n", 
           g_cond_a, g_cond_b, g_cond_c, g_cond_d, s_counter);
    
    test_pattern4();
    printf("After pattern4: a=%d, b=%d, c=%d, d=%d\n", 
           g_cond_a, g_cond_b, g_cond_c, g_cond_d);
    
    test_pattern5();
    printf("After pattern5: a=%d, b=%d, c=%d, d=%d\n", 
           g_cond_a, g_cond_b, g_cond_c, g_cond_d);
    
    /* Compute and print final result */
    int result = g_cond_a + g_cond_b * 2 + g_cond_c * 3 + g_cond_d * 4 + s_counter;
    printf("Final result: %d\n", result);
    
    return result % 100;
}
