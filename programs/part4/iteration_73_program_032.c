/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p check in then_bb */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* To consume results and prevent elimination */

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() & 0xFF;
}

static void __attribute__((noinline, noipa)) use_value(int val) {
    sink ^= val;  /* Side effect to prevent elimination */
}

static int __attribute__((noinline, noipa)) cond_check(int a, int b) {
    return (a > b) ^ (sink & 1);
}

static int __attribute__((noinline, noipa)) modify(int x) {
    return x + (sink & 3) - 1;
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses 'a' */
    if (a > b) {
        /* Modify the test expression variable 'a' */
        a = a + 1;                     /* Direct modification */
        a = a * 2;                     /* Another modification */
        int temp = a ^ b;              /* Use both variables */
        use_value(temp);
        a = modify(a);                 /* Function call modification */
    }
    use_value(a + b);
}

/* Test case 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound test expression */
    if ((a > b) && (c != d)) {
        /* Modify multiple test expression variables */
        a = a + b;                     /* Modify 'a' */
        b = b - 1;                     /* Modify 'b' */
        c = c * 2;                     /* Modify 'c' */
        d = d ^ 0x55;                  /* Modify 'd' */
        
        /* Additional non-debug instructions */
        int sum = a + b + c + d;
        use_value(sum);
        
        /* More arithmetic using test variables */
        a = (a << 2) | (b & 0xF);
        c = modify(c);
    }
    use_value(a ^ b ^ c ^ d);
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses pointer and value */
    if (ptr != NULL && *ptr > threshold) {
        /* Indirect modification of what was tested */
        *ptr = *ptr + 1;               /* Modify through pointer */
        *ptr = *ptr * 2;               /* Another modification */
        
        /* Also modify threshold */
        threshold = threshold - 5;
        
        /* Additional computation */
        int val = *ptr ^ threshold;
        use_value(val);
        
        *ptr = modify(*ptr);
    }
    if (ptr) use_value(*ptr + threshold);
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    int accum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression with loop-variant variables */
        if (a > b && (i % 3) != 0) {
            /* Modify test expression variables */
            a = a + i;                 /* Loop-dependent modification */
            b = b - (i & 1);           /* Another modification */
            
            /* Multiple statements in then block */
            accum += a * b;
            a = (a << 1) | (b & 0xF);
            b = modify(b);
            
            use_value(accum);
        } else {
            /* Else block to preserve control flow */
            a = a ^ b;
            b = b + 1;
        }
        
        /* Loop-carried dependency */
        accum = accum ^ (a + b);
    }
    
    use_value(accum);
    glob_a = a;  /* Write back to volatile global */
    glob_b = b;
}

/* Test case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int x, int y, int z) {
    /* Complex test expression with function call */
    if (cond_check(x, y) && (z > 0)) {
        /* Modify variables from test expression */
        x = x + y;                     /* Modify 'x' */
        y = y - z;                     /* Modify 'y' */
        z = z * 2;                     /* Modify 'z' */
        
        /* Sequence of assignments */
        int t1 = x ^ y;
        int t2 = y | z;
        int t3 = t1 & t2;
        
        x = t3 + x;
        y = modify(y);
        z = z << 2;
        
        use_value(x + y + z);
    }
    use_value(x * y * z);
}

/* Test case 6: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    volatile int local_a = glob_a;
    volatile int local_b = glob_b;
    
    /* Test expression uses volatile variables */
    if (local_a > local_b && glob_c != 0) {
        /* Modify variables used in test */
        local_a = local_a + glob_c;    /* Modify local_a */
        glob_c = glob_c - 1;           /* Modify glob_c (used in test) */
        
        /* Multiple arithmetic operations */
        local_b = local_b * 2;
        int diff = local_a - local_b;
        
        local_a = diff ^ local_a;
        local_b = modify(local_b);
        
        use_value(diff);
    }
    
    /* Force volatile writes */
    glob_a = local_a;
    glob_b = local_b;
}

int main(void) {
    int result = 0;
    
    /* Initialize random seed for variability */
    srand(42);
    
    /* Test 1: Single modification */
    test_single_modification(get_value(), get_value());
    result ^= sink;
    
    /* Test 2: Multiple modifications */
    test_multiple_modifications(
        get_value(), get_value(),
        get_value(), get_value()
    );
    result ^= sink;
    
    /* Test 3: Indirect modification */
    int data = get_value();
    test_indirect_modification(&data, 50);
    result ^= sink ^ data;
    
    /* Test 4: Loop-nested */
    test_loop_nested(10);
    result ^= sink;
    
    /* Test 5: Complex condition */
    test_complex_condition(
        get_value(), get_value(), get_value()
    );
    result ^= sink;
    
    /* Test 6: Volatile mix */
    test_volatile_mix();
    result ^= sink;
    
    /* Additional edge case: modification in both branches */
    {
        int a = get_value();
        int b = get_value();
        
        if (a > b) {
            a = a + b;      /* Modify test variable in then */
            b = b * 2;
            use_value(a);
        } else {
            a = a - b;      /* Also modify in else */
            use_value(b);
        }
        result ^= a ^ b;
    }
    
    printf("Result checksum: %d\n", result & 0xFF);
    return 0;
}
