#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int a = 5, b = 10, c = 15, d = 20;
volatile int x = 0, y = 1, z = 2;
volatile int counter = 0;
volatile int limit = 100;
volatile int global_var = 0;

/* Function prototypes with noinline attribute */
__attribute__((noinline)) void test1(void);
__attribute__((noinline)) void test2(void);
__attribute__((noinline)) void test3(void);
__attribute__((noinline)) void test4(void);
__attribute__((noinline)) int side_effect_func(void);

/* Function with side effects used in conditions */
__attribute__((noinline)) int side_effect_func(void) {
    return global_var++;
}

/* Test 1: Simple modification of condition variable in then block */
__attribute__((noinline)) void test1(void) {
    for (int i = 0; i < 100; i++) {
        /* The condition uses 'a' and 'b' */
        if (a > b) {
            /* This modifies 'a' which is used in the condition */
            a = b;  // This should trigger modified_in_p check
            x = a + b;
        } else {
            b = a + 1;
        }
        
        /* Add more instructions to ensure basic block has content */
        c = a + b;
        d = c * 2;
    }
}

/* Test 2: Compound condition with multiple modifications */
__attribute__((noinline)) void test2(void) {
    volatile int local_a = a, local_b = b, local_c = c;
    
    for (int i = 0; i < 50; i++) {
        /* Compound condition using multiple variables */
        if (local_a != 0 && local_b < local_c) {
            /* Modify variables used in the condition */
            local_b = local_a;  // Modifies variable used in condition
            local_a = 0;        // Also modifies variable used in condition
            y = local_b * 2;
            
            /* Add more non-trivial instructions */
            z = local_a + local_b + local_c;
        }
        
        /* Ensure loop has side effects */
        local_c++;
        local_a = i % 10;
    }
    
    a = local_a;
    b = local_b;
    c = local_c;
}

/* Test 3: Static variable modified in then block */
__attribute__((noinline)) void test3(void) {
    static volatile int static_counter = 0;
    
    for (int i = 0; i < 75; i++) {
        /* Condition uses static variable */
        if (static_counter < limit) {
            /* Modify the variable used in condition */
            static_counter++;  // Direct modification
            counter = static_counter;
            
            /* Additional instructions in the header */
            x = static_counter * 2;
            y = static_counter + 1;
        }
        
        /* Loop with computation */
        limit = (limit + 1) % 200;
    }
}

/* Test 4: Function call in condition with modification in then block */
__attribute__((noinline)) void test4(void) {
    volatile int result1, result2;
    
    for (int i = 0; i < 25; i++) {
        /* Function call in condition */
        result1 = side_effect_func();
        result2 = side_effect_func();
        
        if (result1 > 0 && result2 < 10) {
            /* Modify global variable that side_effect_func reads */
            global_var += 5;  // This affects future calls to side_effect_func
            a = global_var;
            b = result1 + result2;
        }
        
        /* Additional loop body */
        c = i * global_var;
    }
}

/* Test 5: Nested conditions with modifications */
__attribute__((noinline)) void test5(void) {
    volatile int m = a, n = b, p = c;
    
    for (int i = 0; i < 30; i++) {
        if (m > n) {
            if (p < m) {
                /* Modify 'm' which is used in both outer and inner conditions */
                m = p;  // This modification should be detected
                n = m + 1;
            }
            p = m + n;
        }
        
        /* Ensure the loop progresses */
        m += i;
        n -= i % 3;
    }
    
    a = m;
    b = n;
    c = p;
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to introduce runtime variability */
    if (argc > 1) {
        a = atoi(argv[1]);
        b = atoi(argv[2]) + 1;
    }
    
    printf("Starting tests...\n");
    
    /* Run all test functions */
    test1();
    printf("After test1: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d);
    
    test2();
    printf("After test2: a=%d, b=%d, c=%d, x=%d, y=%d, z=%d\n", a, b, c, x, y, z);
    
    test3();
    printf("After test3: counter=%d, limit=%d\n", counter, limit);
    
    test4();
    printf("After test4: global_var=%d, a=%d, b=%d, c=%d\n", global_var, a, b, c);
    
    test5();
    printf("After test5: a=%d, b=%d, c=%d\n", a, b, c);
    
    /* Compute and print final result */
    int final_result = a + b + c + x + y + z + counter + global_var;
    printf("Final result: %d\n", final_result);
    
    return final_result % 256;
}
