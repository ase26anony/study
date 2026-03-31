/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583 checking for modifications to test_expr */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* To consume results and prevent elimination */

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;  /* Non-trivial computation */
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = (*x * 3 + 1) & 0x7FFFFFFF;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) ^ (a & 1);
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* if (a > b) { a = a + 1; ... } */
    if (a > b) {
        /* Multiple statements to create a non-trivial basic block */
        a = a + 1;                    /* Modifies test expression variable */
        sink = a * b;                 /* Use result to prevent elimination */
        a = a ^ 0x12345678;           /* Another modification */
        glob_a = a;                   /* Store to volatile global */
    }
    sink += a;  /* Ensure value is used */
}

/* Test case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* if (a > b && c != d) { modify both a and c } */
    if (a > b && c != d) {
        /* Modify both variables used in the condition */
        a = a * 2 + 1;                /* Modifies first test variable */
        c = c | 0xFF;                 /* Modifies second test variable */
        
        /* Additional statements to flesh out the basic block */
        int temp = a + c;
        sink = temp;
        modify(&a);                   /* Function call that modifies a */
        glob_b = b ^ c;               /* Use other variables */
    }
    sink += a + c;
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* if (ptr && *ptr < threshold) { *ptr = threshold; ... } */
    if (ptr && *ptr < threshold) {
        *ptr = threshold;             /* Indirect modification through pointer */
        *ptr += 5;                    /* Another modification */
        
        /* Additional operations */
        int val = *ptr;
        sink = val * 2;
        modify(ptr);                  /* Function modifies through pointer */
        glob_c = *ptr;
    }
    if (ptr) sink += *ptr;
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* if (x < y && i % 2 == 0) { modify x } */
        if (x < y && (i & 1) == 0) {
            x = x + y;                /* Modifies test expression variable */
            x = x ^ get_value(i);     /* Complex modification */
            sink = x;
            
            /* Multiple statements */
            y = y - 1;                /* Also modifies other test variable */
            glob_d = x + y;
        } else {
            y = y + 2;
        }
        
        /* Loop-carried dependency */
        x = x ^ (i * 3);
    }
    
    sink += x + y;
    glob_a = x;
    glob_b = y;
}

/* Test case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b, int c) {
    /* if (check_cond(a, b) && a + c > 100) { modify a } */
    if (check_cond(a, b) && a + c > 100) {
        a = a * 3;                    /* Modifies test expression variable */
        a = a | 0x0F0F0F0F;           /* Bitwise operation */
        
        /* Sequence of operations */
        int t1 = a + b;
        int t2 = c * 2;
        sink = t1 ^ t2;
        
        /* Call that might affect condition variables */
        modify(&a);
        modify(&b);
    }
    
    sink += a + b + c;
}

/* Test case 6: Modification in both then and else blocks */
static void __attribute__((noinline, noipa)) test_both_branches_modify(int a, int b) {
    /* if (a != b) { modify a } else { modify b } */
    if (a != b) {
        a = a + b;                    /* Modifies test variable in then block */
        a = a << 3;                   /* Additional modification */
        sink = a;
        glob_a = a * 2;
    } else {
        b = b - a;                    /* Modifies test variable in else block */
        b = b >> 2;
        sink = b;
        glob_b = b / 3;
    }
    
    sink += a * b;
}

/* Main function that exercises all test cases */
int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int var1 = get_value(1);
    int var2 = get_value(2);
    int var3 = get_value(3);
    int var4 = get_value(4);
    
    /* Test 1: Single modification */
    test_single_modification(var1, var2);
    result += sink;
    
    /* Test 2: Multiple modifications */
    test_multiple_modifications(var1, var2, var3, var4);
    result += sink;
    
    /* Test 3: Indirect modification */
    int local_var = 50;
    test_indirect_modification(&local_var, 100);
    result += sink + local_var;
    
    /* Test 4: Loop nested */
    test_loop_nested(10);
    result += sink;
    
    /* Test 5: Complex condition */
    test_complex_condition(var1, var2, var3);
    result += sink;
    
    /* Test 6: Both branches modify */
    test_both_branches_modify(var1, var2);
    result += sink;
    
    /* Additional test with volatile to ensure side effects */
    volatile int vol_a = 100;
    volatile int vol_b = 200;
    if (vol_a < vol_b) {
        vol_a = vol_a + vol_b;  /* Modification of volatile test variable */
        sink = vol_a;
        /* Multiple operations */
        vol_b = vol_b / 2;
        vol_a = vol_a ^ vol_b;
    }
    result += vol_a + vol_b;
    
    printf("Result checksum: %d\n", result);
    return 0;
}
