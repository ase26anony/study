#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_modifier = 1;

/* Function prototypes with noinline attribute */
__attribute__((noinline)) 
int test_simple_modification(volatile int a, volatile int b, volatile int c);
__attribute__((noinline))
int test_compound_condition(volatile int x, volatile int y, volatile int z);
__attribute__((noinline))
int test_loop_with_modification(volatile int start, volatile int limit);
__attribute__((noinline))
int test_function_call_side_effect(void);
__attribute__((noinline))
int test_nested_conditions(volatile int p, volatile int q, volatile int r, volatile int s);

/* Non-pure function for condition testing */
__attribute__((noinline))
int get_value(void) {
    return global_counter;
}

/* Function that modifies global state */
__attribute__((noinline))
void modify_global(void) {
    global_counter += global_modifier;
}

/* Test 1: Simple modification of condition variable in then block */
__attribute__((noinline))
int test_simple_modification(volatile int a, volatile int b, volatile int c) {
    int result = 0;
    
    /* Loop to create basic block context */
    for (int i = 0; i < 10; i++) {
        /* Condition where 'a' is tested and modified in then block */
        if (a > b && c < 100) {
            /* This modifies 'a' which is part of the condition */
            a = b + c;  // MODIFICATION IN THEN BLOCK HEADER
            result += a;
            /* Additional instructions to ensure block has content */
            c++;
            b--;
        } else {
            result -= 1;
        }
        
        /* Mix in some other operations */
        if (b < c) {
            b = a + i;
        }
    }
    
    return result;
}

/* Test 2: Compound condition with multiple modifications */
__attribute__((noinline))
int test_compound_condition(volatile int x, volatile int y, volatile int z) {
    int sum = 0;
    
    /* Multiple conditions with modifications */
    if (x != 0 && y < z && (x + y) > 10) {
        /* Modify x which is used in the compound condition */
        x = y * 2;  // MODIFICATION IN THEN BLOCK HEADER
        /* Also modify y which is also in the condition */
        y = z - x;  // ANOTHER MODIFICATION
        sum = x + y + z;
        
        /* Add more instructions to ensure block header has content */
        z++;
        if (z > 50) {
            z = 50;
        }
    } else {
        sum = x - y;
    }
    
    /* Another if with different pattern */
    if (z > 20 || x < y) {
        x = z;
        sum += x;
    }
    
    return sum;
}

/* Test 3: Loop with condition variable modification */
__attribute__((noinline))
int test_loop_with_modification(volatile int start, volatile int limit) {
    volatile int counter = start;
    int total = 0;
    
    while (counter < limit) {
        /* Condition where counter is tested and modified */
        if (counter > 0 && counter < (limit / 2)) {
            /* Modify counter which is part of the condition */
            counter += 2;  // MODIFICATION IN THEN BLOCK HEADER
            total += counter;
            
            /* Additional operations */
            if (total > 100) {
                total = 100;
            }
        } else {
            counter++;
            total -= 1;
        }
        
        /* Break condition to prevent infinite loops */
        if (counter > 1000) break;
    }
    
    return total;
}

/* Test 4: Function call in condition with side effects */
__attribute__((noinline))
int test_function_call_side_effect(void) {
    int result = 0;
    
    for (int i = 0; i < 5; i++) {
        /* Function call in condition */
        if (get_value() > 0 && global_modifier < 10) {
            /* Modify global state that get_value() reads */
            modify_global();  // MODIFIES global_counter
            result += global_counter;
            
            /* Also modify global_modifier used in condition */
            global_modifier++;  // ANOTHER MODIFICATION IN THEN BLOCK HEADER
            
            /* More instructions */
            if (result > 50) {
                result = 50;
            }
        } else {
            result -= global_counter;
        }
        
        /* Alternate path */
        if (global_counter % 2 == 0) {
            global_modifier--;
        }
    }
    
    return result;
}

/* Test 5: Complex nested conditions */
__attribute__((noinline))
int test_nested_conditions(volatile int p, volatile int q, volatile int r, volatile int s) {
    int value = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < 3; outer++) {
        /* Inner loop */
        for (int inner = 0; inner < 4; inner++) {
            /* Complex condition with multiple variables */
            if ((p > q || r < s) && (p + r) > (q + s) && p != 0) {
                /* Modify p which is used in the complex condition */
                p = q + r;  // MODIFICATION IN THEN BLOCK HEADER
                
                /* Also modify r which is also in the condition */
                r = s - p;  // ANOTHER MODIFICATION
                
                value += p * r;
                
                /* Nested if inside then block */
                if (value > 1000) {
                    value = 1000;
                    s = p;
                }
            } else {
                q = p + 1;
                value -= q;
            }
            
            /* Additional control flow */
            switch (inner % 3) {
                case 0: s++; break;
                case 1: q--; break;
                case 2: r += 2; break;
            }
        }
        
        /* Modify variables between iterations */
        p += outer;
        q -= outer;
    }
    
    return value;
}

int main(int argc, char *argv[]) {
    int total_result = 0;
    
    /* Use command line arguments to introduce runtime variability */
    volatile int base = (argc > 1) ? atoi(argv[1]) : 10;
    volatile int modifier = (argc > 2) ? atoi(argv[2]) : 5;
    
    printf("Starting if-conversion test with base=%d, modifier=%d\n", base, modifier);
    
    /* Test 1: Simple modification */
    printf("Test 1 - Simple modification: ");
    int r1 = test_simple_modification(base, modifier, base + modifier);
    printf("result = %d\n", r1);
    total_result += r1;
    
    /* Test 2: Compound condition */
    printf("Test 2 - Compound condition: ");
    int r2 = test_compound_condition(base * 2, modifier, base - modifier);
    printf("result = %d\n", r2);
    total_result += r2;
    
    /* Test 3: Loop with modification */
    printf("Test 3 - Loop with modification: ");
    int r3 = test_loop_with_modification(base % 10, base);
    printf("result = %d\n", r3);
    total_result += r3;
    
    /* Reset globals */
    global_counter = base;
    global_modifier = modifier;
    
    /* Test 4: Function call side effects */
    printf("Test 4 - Function call side effects: ");
    int r4 = test_function_call_side_effect();
    printf("result = %d (global_counter=%d, global_modifier=%d)\n", 
           r4, global_counter, global_modifier);
    total_result += r4;
    
    /* Test 5: Nested conditions */
    printf("Test 5 - Nested conditions: ");
    int r5 = test_nested_conditions(base, modifier, base + 1, modifier + 1);
    printf("result = %d\n", r5);
    total_result += r5;
    
    printf("Total result: %d\n", total_result);
    
    /* Read from stdin to prevent dead code elimination */
    if (argc > 3 && argv[3][0] == 'i') {
        int dummy;
        printf("Enter a number: ");
        scanf("%d", &dummy);
        total_result += dummy;
    }
    
    return total_result != 0 ? 0 : 1;
}
