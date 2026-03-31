/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583 checking test_expr modifications */

#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* Sink for results to prevent optimization */

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;  /* Non-trivial computation */
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr = (*ptr * 3) + 7;
}

static int __attribute__((noinline, noipa)) check_condition(int a, int b) {
    return (a > b) ^ (a < b * 2);
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* Test expression uses a and b */
    if (a > b && glob_c != 0) {
        /* Modify variable 'a' used in test expression */
        a = a + 1;                    /* Direct modification */
        a = a * 2;                    /* Additional computation */
        b = b - 1;                    /* Modify other test variable */
        sink += a + b;                /* Use result to prevent elimination */
    }
    
    /* Additional code to create more complex CFG */
    for (int i = 0; i < 3; i++) {
        if (a < b * 2) {
            a = get_value(a);
            sink += a;
        }
    }
}

/* Test case 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multiple_modifications(void) {
    int x = glob_a;
    int y = glob_b;
    int z = glob_c;
    
    /* Compound conditional using multiple variables */
    if ((x > y) || (z < glob_d) || (x + y > z)) {
        /* Modify all variables used in test expression */
        x = x * 3 + 1;                /* Modify x */
        y = y / 2;                    /* Modify y */
        z = z ^ 0x12345678;           /* Modify z with bitwise op */
        
        /* Additional statements to flesh out basic block */
        int temp = x | y;
        temp = temp & z;
        sink += temp;
        
        /* Function call that might modify through pointer */
        modify(&x);
    }
    
    /* Loop with conditional to preserve structure */
    volatile int counter = 5;
    while (counter-- > 0) {
        if (x != y) {
            x = x + y;
            sink += x;
        }
    }
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(void) {
    int base = glob_a;
    int *ptr = &base;
    int threshold = glob_b;
    
    /* Test expression uses variable that ptr is derived from */
    if (base > threshold && ptr != NULL) {
        /* Indirect modification through pointer */
        *ptr = 42;                    /* Modifies 'base' indirectly */
        base = base * 2;              /* Direct modification */
        
        /* Additional arithmetic */
        int result = (*ptr) << 2;
        result = result | 0xF;
        sink += result;
    }
    
    /* Nested conditionals for complex CFG */
    if (base < 100) {
        modify(ptr);
        sink += *ptr;
    }
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int a = glob_a;
    int b = glob_b;
    int accum = 0;
    
    /* Loop creates context for if-conversion analysis */
    for (int i = 0; i < 10; i++) {
        /* Test expression using loop-varying variables */
        if (a > b && (i % 3) != 0) {
            /* Modify test expression variables */
            a = a + i;                /* Loop-carried modification */
            b = b - (i & 1);          /* Another modification */
            
            /* Multiple statements in then block */
            accum += a * b;
            accum = accum ^ (a << 3);
            sink += accum;
        }
        
        /* Additional loop body to prevent simplification */
        a = get_value(a);
        b = check_condition(b, i);
    }
    
    /* Final sink update */
    sink += a + b + accum;
}

/* Test case 5: Complex expression with function calls in condition */
static void __attribute__((noinline, noipa)) test_complex_condition(void) {
    int val1 = glob_a;
    int val2 = glob_b;
    int val3 = glob_c;
    
    /* Complex condition with function call */
    if ((val1 > get_value(val2)) && 
        (check_condition(val3, val1) != 0) &&
        (val2 < val3 * 2)) {
        
        /* Modify variables used in various parts of condition */
        val1 = val1 * 2 + 5;          /* Used in first part */
        val2 = val2 ^ val1;           /* Used in third part */
        val3 = (val3 << 1) | 1;       /* Used in second and third parts */
        
        /* Sequence of operations */
        int tmp = val1 + val2;
        tmp = tmp * val3;
        tmp = tmp >> 2;
        sink += tmp;
        
        /* Another modification */
        val1 = val1 % 100;
    }
    
    /* Post-conditional code */
    volatile int v = 0;
    for (int i = 0; i < 4; i++) {
        v += val1 + val2 + val3;
    }
    sink += v;
}

/* Test case 6: Multiple basic blocks within then block */
static void __attribute__((noinline, noipa)) test_nested_in_then(void) {
    int x = glob_a;
    int y = glob_b;
    
    if (x != y && x > 0) {
        /* First modification */
        x = x * 3;
        sink += x;
        
        /* Nested if inside then block */
        if (y < 100) {
            y = y + x;
            x = x ^ y;
        }
        
        /* Another modification after nested block */
        x = x >> 1;
        y = y << 1;
        
        /* Final computations */
        sink += x * y;
    }
    
    /* Loop to preserve structure */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += x + y + i;
    }
    sink += sum;
}

int main(void) {
    printf("Starting if-conversion test patterns...\n");
    
    /* Execute all test cases */
    test_single_modification();
    printf("Test 1 complete, sink = %d\n", sink);
    
    test_multiple_modifications();
    printf("Test 2 complete, sink = %d\n", sink);
    
    test_indirect_modification();
    printf("Test 3 complete, sink = %d\n", sink);
    
    test_loop_nested();
    printf("Test 4 complete, sink = %d\n", sink);
    
    test_complex_condition();
    printf("Test 5 complete, sink = %d\n", sink);
    
    test_nested_in_then();
    printf("Test 6 complete, sink = %d\n", sink);
    
    /* Final checksum */
    int checksum = sink ^ glob_a ^ glob_b ^ glob_c ^ glob_d;
    printf("Final checksum: 0x%08X\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
