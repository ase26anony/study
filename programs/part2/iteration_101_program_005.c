#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE volatile

/* Global variables to create side effects */
VOLATILE int global_modifier = 0;
static int static_counter = 0;

/* Function with side effects used in conditions */
NOINLINE int side_effect_func(void) {
    return global_modifier++;
}

/* Test function 1: Simple modification in then block */
NOINLINE void test_simple_modification(void) {
    VOLATILE int a = 10, b = 5, c = 0;
    
    /* Loop context to create interesting block structure */
    for (int i = 0; i < 100; i++) {
        /* Condition where 'a' is tested and modified in then block */
        if (a > b) {           /* test_expr: a > b */
            a = b;             /* MODIFIES 'a' which is in test_expr */
            c = a + b;         /* Additional instruction in header */
            b++;               /* Modify another variable */
        } else {
            b = a;
        }
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 10 == 0) {
            global_modifier += i;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Test1: a=%d, b=%d, c=%d\n", a, b, c);
}

/* Test function 2: Compound condition with multiple modifications */
NOINLINE void test_compound_condition(void) {
    VOLATILE int x = 100, y = 50, z = 75;
    VOLATILE int flag = 1;
    
    /* Complex condition with multiple variables */
    for (int i = 0; i < 50; i++) {
        /* Compound condition: x != 0 && y < z */
        if (x != 0 && y < z) {  /* test_expr involves x, y, z */
            x = y;              /* MODIFIES 'x' which is in test_expr */
            y = z + 1;          /* MODIFIES 'y' which is in test_expr */
            z = x * 2;          /* MODIFIES 'z' which is in test_expr */
            /* Multiple instructions in header before any control flow */
            flag = !flag;
            global_modifier += x;
        } else {
            z = x + y;
        }
        
        /* Introduce variability */
        if (side_effect_func() % 3 == 0) {
            x += 1;
        }
    }
    
    printf("Test2: x=%d, y=%d, z=%d, flag=%d\n", x, y, z, flag);
}

/* Test function 3: Static variable modification in then block */
NOINLINE void test_static_modification(void) {
    VOLATILE int limit = 100;
    
    for (int i = 0; i < 200; i++) {
        /* Condition uses static variable */
        if (static_counter < limit) {  /* test_expr: static_counter < limit */
            static_counter++;          /* MODIFIES static_counter in test_expr */
            limit--;                   /* MODIFIES limit in test_expr */
            /* Additional non-label, non-note instructions */
            int temp = static_counter * 2;
            global_modifier ^= temp;
        } else {
            static_counter = 0;
            limit = 100;
        }
        
        /* Function call in condition with side effects */
        if (side_effect_func() > 50) {
            limit += 10;
        }
    }
    
    printf("Test3: static_counter=%d, limit=%d\n", static_counter, limit);
}

/* Test function 4: Nested conditions and pointer modifications */
NOINLINE void test_pointer_modification(void) {
    VOLATILE int data[4] = {10, 20, 30, 40};
    VOLATILE int *ptr1 = &data[0];
    VOLATILE int *ptr2 = &data[2];
    VOLATILE int sum = 0;
    
    for (int i = 0; i < 100; i++) {
        /* Condition involving pointer comparisons */
        if (ptr1 < ptr2 && *ptr1 < *ptr2) {  /* Complex test_expr */
            *ptr1 = *ptr2;    /* MODIFIES *ptr1 which is in test_expr */
            ptr1++;           /* MODIFIES ptr1 which is in test_expr */
            sum += *ptr2;
            data[3] = sum;    /* Additional modification */
        } else {
            ptr2--;
            sum -= *ptr1;
        }
        
        /* Reset pointers periodically */
        if (i % 25 == 0) {
            ptr1 = &data[0];
            ptr2 = &data[2];
        }
    }
    
    printf("Test4: sum=%d, data[0]=%d, data[3]=%d\n", sum, data[0], data[3]);
}

/* Test function 5: Function call in condition with then-block modification */
NOINLINE void test_function_call_condition(void) {
    VOLATILE int base = 1000;
    
    for (int i = 0; i < 75; i++) {
        /* Function call in condition - func reads global_modifier */
        if (side_effect_func() < base) {  /* test_expr involves function call */
            global_modifier = base;       /* MODIFIES global var used by func */
            base -= 50;                   /* MODIFIES base in test_expr */
            /* Ensure multiple instructions in header */
            int calc = global_modifier * 2 + base;
            static_counter += calc % 7;
        } else {
            base += 25;
        }
        
        /* Additional condition to create more blocks */
        if (i % 15 == 0) {
            base = 1000;
        }
    }
    
    printf("Test5: base=%d, global_modifier=%d\n", base, global_modifier);
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to introduce runtime variability */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
        global_modifier = seed;
    }
    
    printf("Starting IF-CONVERSION test (seed=%d)\n", seed);
    
    /* Call all test functions to exercise different patterns */
    test_simple_modification();
    test_compound_condition();
    test_static_modification();
    test_pointer_modification();
    test_function_call_condition();
    
    /* Final computation with observable result */
    int result = global_modifier + static_counter;
    printf("Final result: %d\n", result);
    
    return result % 256;
}
