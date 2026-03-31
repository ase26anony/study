/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p detection in then_bb */

#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* To prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;  /* Non-trivial computation */
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = (*x * 3 + 1) & 0xFFFF;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) && ((a ^ b) & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses 'a' */
    if (a > b && glob_c != 0) {
        /* Modify 'a' which appears in test expression */
        a = a + glob_d;  // Direct modification
        a = a * 2;       // Additional computation
        sink += a;       // Prevent dead code elimination
        glob_c = a & 0xFF;  // Also modify global used in condition
    }
    sink += a + b;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_modification(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d) || (glob_a != glob_b)) {
        /* Modify multiple variables from the condition */
        a = a * 3 + 1;      // Modifies 'a' from (a > b)
        b = b ^ 0x1234;     // Modifies 'b' from (a > b)
        c = get_value(c);   // Modifies 'c' from (c < d)
        d = d + glob_a;     // Modifies 'd' from (c < d)
        
        /* Additional statements to flesh out the basic block */
        int temp = a * b;
        temp = temp ^ c ^ d;
        sink += temp;
        
        /* Modify globals too */
        glob_a = (glob_a + 1) & 0xFF;
        glob_b = (glob_b - 1) & 0xFF;
    }
    sink += a + b + c + d;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses pointer-derived value */
    if (ptr && *ptr > threshold && glob_c < 100) {
        /* Modify through pointer - affects *ptr used in condition */
        *ptr = *ptr * 2 + 1;
        
        /* Additional operations */
        modify(ptr);
        sink += *ptr;
        
        /* Also modify global */
        glob_c = *ptr & 0x7F;
    }
    if (ptr) sink += *ptr;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses variables modified in loop */
        if (a > b && (a + b) < 1000) {
            /* Modify variables used in condition */
            a = a + i;          // Modifies 'a' from (a > b)
            b = b - (i & 1);    // Modifies 'b' from (a > b)
            
            /* Additional computation */
            int prod = a * b;
            sum += prod & 0xFF;
            
            /* Function call that might affect condition */
            modify(&a);
        }
        
        /* Loop-carried dependency */
        a = a ^ (b << 1);
        b = b + (i & 3);
    }
    
    sink += sum + a + b;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int x, int y) {
    /* Condition with function call */
    if (check_cond(x, y) || (glob_c > glob_d)) {
        /* Modify variables used in condition */
        x = x * 5 - 3;      // Affects check_cond(x, y) if called again
        y = y ^ x;          // Affects check_cond(x, y)
        
        /* Multiple statements in then block */
        int t1 = x << 2;
        int t2 = y >> 1;
        sink += t1 + t2;
        
        /* Modify globals from condition */
        glob_c = glob_c + x;
        glob_d = glob_d - y;
    }
    sink += x + y;
}

/* Test 6: Volatile accesses in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    volatile int local_a = glob_a;
    volatile int local_b = glob_b;
    
    /* Condition uses volatile locals */
    if (local_a > local_b || local_a < 0) {
        /* Modify the volatile variables used in condition */
        local_a = local_a * 2;      // This should be detected
        local_b = local_b / 2;      // This too
        
        /* Additional non-volatile computation */
        int normal = local_a + local_b;
        normal = normal ^ 0xAA;
        sink += normal;
        
        /* Write back to globals */
        glob_a = local_a;
        glob_b = local_b;
    }
}

/* Main function that exercises all test cases */
int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int arg1 = get_value(100);
    int arg2 = get_value(200);
    int arg3 = get_value(300);
    int arg4 = get_value(400);
    
    /* Test 1: Single modification */
    test_single_modification(arg1, arg2);
    result += sink & 1;
    
    /* Test 2: Multiple modifications */
    test_multi_modification(arg1, arg2, arg3, arg4);
    result += sink & 2;
    
    /* Test 3: Indirect modification */
    int data = 50;
    int *ptr = &data;
    test_indirect_modification(ptr, 40);
    result += data & 4;
    
    /* Test 4: Loop nested */
    test_loop_nested(10);
    result += sink & 8;
    
    /* Test 5: Complex condition */
    test_complex_condition(arg1, arg2);
    result += sink & 16;
    
    /* Test 6: Volatile mix */
    test_volatile_mix();
    result += sink & 32;
    
    /* Additional test: Direct modification in minimal case */
    {
        int x = glob_a;
        int y = glob_b;
        
        /* Simple condition with modification */
        if (x > y) {
            x = x + 1;      // Direct modification of test variable
            y = y * 2;      // Additional modification
            sink += x * y;
        }
        result += x + y;
    }
    
    printf("Test result checksum: %d\n", result & 0xFF);
    printf("Global state: a=%d, b=%d, c=%d, d=%d\n", 
           glob_a, glob_b, glob_c, glob_d);
    
    return 0;
}
