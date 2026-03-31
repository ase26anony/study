/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p detection in then_bb */

#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink;  /* To prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x + (rand() & 1);  /* Non-deterministic but safe */
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = *x + 1;
    sink = *x;  /* Side effect */
}

static int __attribute__((noinline, noipa)) cond_check(int a, int b) {
    return a != b;
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* Test expression uses a and b */
    if (a > b && glob_c != 0) {
        /* Modify variable 'a' used in test expression */
        a = a + 1;                     /* Direct modification */
        b = b * 2;                     /* Additional statement */
        glob_c = glob_c - 1;           /* Modify global from test */
        sink = a + b + glob_c;         /* Prevent elimination */
    }
    
    /* Use results to prevent dead code */
    volatile int result = a + b;
    sink = result;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int x = get_value(glob_a);
    int y = get_value(glob_b);
    int z = glob_c;
    int w = glob_d;
    
    /* Compound conditional using multiple variables */
    if ((x > y) || (z < w) || (x + z > 100)) {
        /* Modify multiple variables from test expression */
        x = x | 0xFF;                  /* Bitwise operation */
        y = y ^ 0xAA;                  /* Another operation */
        z = z * 3;                     /* Arithmetic */
        w = w >> 1;                    /* Shift operation */
        
        /* Additional non-debug instructions */
        int temp = x + y;
        temp = temp * z;
        sink = temp + w;
    }
    
    /* Ensure variables are used */
    sink = x + y + z + w;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(void) {
    int value = glob_a;
    int *ptr = &value;
    int threshold = glob_b;
    
    /* Test expression uses value */
    if (value > threshold && ptr != NULL) {
        /* Indirect modification through pointer */
        *ptr = 42;                     /* Modifies 'value' indirectly */
        
        /* Additional operations to create more instructions */
        int calc = *ptr * 2;
        calc = calc + threshold;
        sink = calc;
        
        /* Another assignment */
        value = value / 2;             /* Direct modification too */
    }
    
    sink = value;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int counter = glob_a;
    int limit = glob_b;
    int accumulator = 0;
    
    for (int i = 0; i < 10; i++) {
        /* Test expression uses counter and limit */
        if (counter < limit && glob_c > 0) {
            /* Modify test expression variable */
            counter = counter + i;     /* Loop-carried modification */
            
            /* Additional statements */
            accumulator = accumulator + counter;
            glob_c = glob_c - 1;       /* Modify global from test */
            
            /* Function call with side effect */
            modify(&counter);
        }
        
        /* Loop update with test variable */
        limit = limit - (i & 1);
        sink = accumulator;
    }
    
    sink = counter + accumulator;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_cond(void) {
    int a = get_value(glob_a);
    int b = get_value(glob_b);
    int c = glob_c;
    
    /* Complex test with function call */
    if (cond_check(a, b) && (c > 0 || a < 100)) {
        /* Modify all variables from test expression */
        a = a * 2 + 1;                 /* Complex arithmetic */
        b = b - c;                     /* Using another test var */
        c = c ^ 0x55;                  /* Bitwise operation */
        
        /* Sequence of assignments */
        int t1 = a << 2;
        int t2 = b >> 1;
        int t3 = c & 0xF;
        sink = t1 + t2 + t3;
        
        /* Another modification */
        a = a % 17;                    /* Additional mod */
    }
    
    /* Force use of results */
    volatile int result = a * 100 + b * 10 + c;
    sink = result;
}

/* Test 6: Multiple basic blocks within then */
static void __attribute__((noinline, noipa)) test_multi_bb_in_then(void) {
    int x = glob_a;
    int y = glob_b;
    
    if (x > 0 && y > 0) {
        /* First modification */
        x = x * 2;
        sink = x;
        
        /* Conditional inside then (creates more BBs) */
        if (x > y) {
            y = y + x;                 /* Modify another test var */
            x = x / 2;                 /* Modify again */
        }
        
        /* More modifications */
        x = x | 0x1;
        y = y & 0x7F;
        
        /* Function call that might modify */
        modify(&x);
    }
    
    sink = x + y;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Run each test multiple times with different global values */
    for (int i = 0; i < 3; i++) {
        glob_a = 10 + i;
        glob_b = 20 - i;
        glob_c = 30 + i * 2;
        glob_d = 40 - i * 2;
        
        test_single_var_mod();
        checksum += sink;
        
        test_multi_var_mod();
        checksum += sink;
        
        test_indirect_mod();
        checksum += sink;
        
        test_loop_nested();
        checksum += sink;
        
        test_complex_cond();
        checksum += sink;
        
        test_multi_bb_in_then();
        checksum += sink;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
