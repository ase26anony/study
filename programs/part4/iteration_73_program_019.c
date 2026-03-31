#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int g_volatile_sink;
int g_a = 1, g_b = 2, g_c = 3, g_d = 4;
volatile int g_flag = 0;

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x + (g_flag ? 1 : 0);
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    *ptr += 1;
    g_volatile_sink = *ptr;
}

static int __attribute__((noinline, noipa)) check_condition(int a, int b) {
    return (a > b) ^ (g_flag & 1);
}

/* Test Case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_var_modification(int x, int y) {
    /* Test expression uses x and y */
    if (x > y && x != 0) {
        /* Modify x which appears in the test expression */
        x = x + 1;                    /* Direct modification */
        x = x * 2;                    /* Additional computation */
        g_a = x;                      /* Store to global to prevent DCE */
        g_volatile_sink = x;          /* Volatile sink */
    }
    g_volatile_sink = x + y;          /* Ensure value is used */
}

/* Test Case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_var_modification(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d)) {
        /* Modify both a and c which appear in the test expression */
        a = a + b;                    /* Modify 'a' from first part of condition */
        c = c - 1;                    /* Modify 'c' from second part of condition */
        int temp = a * c;             /* Additional computation using modified vars */
        g_volatile_sink = temp;       /* Volatile sink */
        
        /* More statements to flesh out the basic block */
        b = b ^ 0xFF;                 /* Modify other variable */
        g_volatile_sink = b;
    }
    /* Use results to prevent optimization */
    g_a = a; g_b = b; g_c = c; g_d = d;
}

/* Test Case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses *ptr */
    if (ptr != NULL && *ptr > threshold) {
        /* Modify *ptr - indirect modification of tested value */
        *ptr = *ptr + 5;              /* Modify the dereferenced value */
        *ptr = *ptr * 2;              /* Additional computation */
        modify(ptr);                  /* Call function that modifies it */
        g_volatile_sink = *ptr;
    }
}

/* Test Case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = get_value(10);
    int y = get_value(20);
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression uses loop-varying variables */
        if (x < y && i % 2 == 0) {
            /* Modify x which is used in the condition */
            x = x + i;                /* Loop-carried modification */
            y = y - 1;                /* Also modify y */
            g_volatile_sink = x * y;  /* Volatile sink with side effect */
            
            /* Additional statements */
            int z = x ^ y;
            g_volatile_sink = z;
        }
        /* Loop update with test expression variables */
        x = x + (i & 1);
        y = y + (i & 2);
    }
    g_a = x; g_b = y;
}

/* Test Case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int p, int q, int r) {
    /* Complex test expression with function call */
    if (check_condition(p, q) && (p + q) > r) {
        /* Modify p which appears in the test expression */
        p = p * 3;                    /* Direct modification */
        p = p | 0x0F;                 /* Bitwise operation */
        r = r + p;                    /* Modify other variable */
        
        /* Multiple statements to ensure basic block has content */
        volatile int local_sink = p;
        g_volatile_sink = r;
        local_sink = q * 2;
    }
    /* Ensure values are used */
    g_c = p + q + r;
}

/* Test Case 6: Modification through array indexing */
static void __attribute__((noinline, noipa)) test_array_modification(int idx, int limit) {
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Test expression uses array element */
    if (idx >= 0 && idx < 10 && arr[idx] < limit) {
        /* Modify arr[idx] which is implicitly tested */
        arr[idx] = arr[idx] * 2;      /* Modify tested array element */
        arr[idx] = arr[idx] + 1;      /* Additional modification */
        g_volatile_sink = arr[idx];   /* Volatile sink */
        
        /* Modify index variable too */
        idx = idx ^ 1;
        g_volatile_sink = idx;
    }
    /* Use array to prevent optimization */
    for (int i = 0; i < 10; i++) {
        g_volatile_sink += arr[i];
    }
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int x = get_value(5);
    int y = get_value(15);
    int z = get_value(25);
    
    /* Test 1: Single variable modification */
    test_single_var_modification(x, y);
    result += g_a;
    
    /* Test 2: Multiple variable modification */
    test_multi_var_modification(x, y, z, x + y);
    result += g_b;
    
    /* Test 3: Indirect modification */
    int value = 100;
    test_indirect_modification(&value, 50);
    result += value;
    
    /* Test 4: Loop-nested modification */
    test_loop_nested(10);
    result += g_a + g_b;
    
    /* Test 5: Complex condition */
    test_complex_condition(x, y, z);
    result += g_c;
    
    /* Test 6: Array modification */
    test_array_modification(3, 20);
    result += g_volatile_sink & 0xFF;
    
    /* Print checksum to ensure all code executed */
    printf("Result checksum: %d\n", result);
    
    /* Additional volatile operations to prevent dead code elimination */
    g_volatile_sink = result;
    
    return 0;
}
