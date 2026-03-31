#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int a = 5, b = 10, c = 15, d = 20;
volatile int counter = 0;
volatile int limit = 100;
volatile int x = 0, y = 1, z = 2;

/* Global variable for function side effects */
volatile int global_mod = 0;

/* Function with side effect - not pure */
int __attribute__((noinline)) side_effect_func(void) {
    return global_mod++;
}

/* Test 1: Simple modification in then block */
void __attribute__((noinline)) test1_modify_in_then(void) {
    for (int i = 0; i < 100; i++) {
        /* The condition uses 'a' and 'b' */
        if (a > b) {
            /* This modifies 'a' which is used in the condition */
            a = b + 1;  // Modifies condition variable in then block
            /* Add more instructions to ensure header has content */
            c = a * 2;
            d = c + b;
        } else {
            b = a + 1;
        }
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 10 == 0) {
            x = y + z;
        }
    }
}

/* Test 2: Compound condition with modification */
void __attribute__((noinline)) test2_compound_condition(void) {
    volatile int local_a = a;
    volatile int local_b = b;
    volatile int local_c = c;
    
    for (int i = 0; i < 50; i++) {
        /* Complex condition with multiple variables */
        if (local_a > 0 && local_b < local_c && local_c != d) {
            /* Modify variables used in the condition */
            local_a = local_b;  // Modifies 'local_a' from condition
            local_b = local_c + 1;  // Modifies 'local_b' from condition
            /* Add non-trivial computation */
            local_c = (local_a * local_b) / 2;
            
            /* More instructions for header analysis */
            global_mod += local_a;
            x = local_b - local_a;
        } else {
            local_c = local_a + local_b;
        }
        
        /* Loop variant to prevent dead code elimination */
        if (i & 1) {
            local_a++;
        } else {
            local_b--;
        }
    }
    
    /* Write back to globals */
    a = local_a;
    b = local_b;
    c = local_c;
}

/* Test 3: Static variable modification in then block */
void __attribute__((noinline)) test3_static_modification(void) {
    static volatile int static_counter = 0;
    volatile int threshold = 50;
    
    while (static_counter < limit) {
        /* Condition uses static_counter */
        if (static_counter < threshold && a > 0) {
            /* Modify static_counter which is in the condition */
            static_counter++;  // Direct modification of condition variable
            
            /* Additional modifications */
            a = a - 1;
            b = b + static_counter;
            
            /* Complex enough to avoid simplification */
            for (int j = 0; j < 3; j++) {
                c = c + (j * static_counter);
            }
        } else {
            threshold = threshold / 2;
            static_counter = static_counter > 10 ? static_counter - 5 : 0;
        }
        
        /* Break condition to prevent infinite loop */
        if (static_counter > 200) break;
    }
    
    counter = static_counter;
}

/* Test 4: Function call in condition with side effects */
void __attribute__((noinline)) test4_function_in_condition(void) {
    volatile int result1, result2;
    
    for (int i = 0; i < 30; i++) {
        /* Function call in condition - creates complex RTL */
        result1 = side_effect_func();
        result2 = side_effect_func();
        
        if (result1 > 0 && result2 < 10) {
            /* Modify global variable that side_effect_func reads */
            global_mod = global_mod * 2 + 1;  // Affects future calls
            
            /* Also modify other condition-related variables */
            a = result1 + result2;
            b = global_mod - a;
            
            /* Multiple instructions in header */
            c = a | b;
            d = c ^ global_mod;
            x = y << 2;
        } else {
            global_mod = global_mod / 2;
            y = z + 1;
        }
        
        /* Loop-dependent modification */
        z = (z + i) & 0xFF;
    }
}

/* Test 5: Nested conditions with modifications */
void __attribute__((noinline)) test5_nested_modifications(void) {
    volatile int p = a, q = b, r = c;
    
    /* Outer loop */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with condition */
        for (int inner = 0; inner < 20; inner++) {
            /* Condition with multiple variables */
            if ((p < q) || (r > 0 && p != 0)) {
                /* Modify p which is used in the condition */
                p = q + inner;  // Modification in then block
                
                /* Chain of modifications */
                q = r - p;
                r = p * q;
                
                /* Additional computations */
                if (inner % 3 == 0) {
                    p = p + 1;  // Another modification of condition variable
                }
                
                /* More header instructions */
                global_mod += p;
                x = q ^ r;
            }
            
            /* Loop variant */
            r = (r + inner) % 100;
        }
        
        /* Outer loop modification */
        p = (p + outer) % 50;
    }
    
    a = p;
    b = q;
    c = r;
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to introduce runtime variability */
    if (argc > 1) {
        a = atoi(argv[1]);
        b = atoi(argv[2]) + 1;
        c = atoi(argv[3]) + 2;
    }
    
    printf("Starting values: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d);
    printf("counter=%d, limit=%d, global_mod=%d\n", counter, limit, global_mod);
    
    /* Run all test functions */
    test1_modify_in_then();
    printf("After test1: a=%d, b=%d, c=%d\n", a, b, c);
    
    test2_compound_condition();
    printf("After test2: a=%d, b=%d, c=%d\n", a, b, c);
    
    test3_static_modification();
    printf("After test3: counter=%d, a=%d, b=%d\n", counter, a, b);
    
    test4_function_in_condition();
    printf("After test4: global_mod=%d, a=%d, b=%d\n", global_mod, a, b);
    
    test5_nested_modifications();
    printf("After test5: a=%d, b=%d, c=%d\n", a, b, c);
    
    /* Compute and print final result */
    int result = a + b + c + d + counter + global_mod + x + y + z;
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
