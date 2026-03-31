#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
__attribute__((noinline)) int get_input(int x) {
    return x ^ 0x55AA55AA;
}

__attribute__((noinline)) int barrier(int x) {
    volatile int v = x;
    return v;
}

/* Test function 1: MIPS target with simple jump pattern */
__attribute__((target("arch=mips32")))
int test_mips_delay_slot(int a, int b) {
    /* Initialize temporaries independent of jump condition */
    int temp1 = a + 100;
    int temp2 = b * 2;
    int temp3 = temp1 ^ temp2;
    int result = 0;
    
    /* Dynamic condition using input */
    if (a > b) {
        /* This should compile to a simple jump to label */
        goto target_label;
    }
    
    /* Some other code to create basic blocks */
    temp3 = temp3 + 1;
    result = temp3;
    goto end;
    
target_label:
    /* Safe, non-jump instruction immediately after label */
    /* Uses independent temporaries not involved in jump condition */
    temp1 = temp2 + 5;  /* Simple arithmetic operation */
    result = temp1;
    
end:
    return barrier(result);
}

/* Test function 2: SPARC target with different pattern */
__attribute__((target("arch=sparc")))
int test_sparc_delay_slot(int x, int y) {
    /* More temporaries to work with */
    int t1 = x & 0xFF;
    int t2 = y | 0x55;
    int t3 = t1 * 2;
    int t4 = t2 - 1;
    int result = 0;
    
    /* Another dynamic condition */
    volatile int cond = x;
    if ((cond & 1) == 0) {
        goto sparc_target;
    }
    
    /* Alternative path */
    t3 = t4 + t1;
    result = t3;
    goto sparc_end;
    
sparc_target:
    /* Safe instruction: bitwise operation on temporaries */
    t4 = t1 ^ t2;  /* Independent of jump condition variables */
    result = t4;
    
sparc_end:
    return barrier(result);
}

/* Test function 3: Generic pattern without arch attribute */
/* Rely on -march flag during compilation */
int test_generic_delay_slot(int p, int q) {
    /* Create many local variables to give scheduler options */
    int v1 = p + q;
    int v2 = p - q;
    int v3 = p * 3;
    int v4 = q * 7;
    int v5 = v1 & v2;
    int v6 = v3 | v4;
    int result = 0;
    
    /* Complex enough condition to not be optimized away */
    int sum = p + q;
    if ((sum % 3) == 0) {
        goto generic_target;
    }
    
    /* Other basic blocks */
    if (p > q) {
        v5 = v6 + v1;
        result = v5;
    } else {
        v6 = v5 - v2;
        result = v6;
    }
    goto generic_end;
    
generic_target:
    /* Multiple safe instructions in sequence */
    v3 = v4 + 1;      /* First instruction after label */
    /* Note: Only the first instruction should be considered for delay slot */
    result = v3;
    
generic_end:
    return barrier(result);
}

/* Test function 4: Nested control flow with jump to label */
__attribute__((target("arch=mips32")))
int test_nested_jump(int a, int b, int c) {
    int t1 = a * b;
    int t2 = b * c;
    int t3 = c * a;
    int result = 0;
    
    /* Outer condition */
    if (a > 0) {
        /* Inner condition for the target jump */
        if (b < c) {
            goto nested_target;
        }
        t1 = t2 + t3;
        result = t1;
    } else {
        t2 = t1 - t3;
        result = t2;
    }
    goto nested_end;
    
nested_target:
    /* Safe arithmetic on independent temporary */
    t3 = t1 + 10;  /* t1 defined before jump, but not in condition */
    result = t3;
    
nested_end:
    return barrier(result);
}

/* Test function 5: Loop with jump out to label */
int test_loop_jump(int iterations) {
    int i, acc = 0;
    int temp1 = iterations * 2;
    int temp2 = iterations + 100;
    
    for (i = 0; i < iterations; i++) {
        acc += i;
        /* Jump out of loop to label */
        if (acc > 1000) {
            goto loop_target;
        }
    }
    
    temp1 = temp2 - acc;
    return barrier(temp1);
    
loop_target:
    /* Instruction after label uses temp variable */
    temp2 = temp1 * 2;
    return barrier(temp2);
}

int main() {
    int result = 0;
    int i;
    
    /* Seed for pseudo-random but reproducible behavior */
    srand(42);
    
    /* Call test functions with various inputs */
    for (i = 0; i < 10; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        int c = rand() % 100;
        
        result ^= test_mips_delay_slot(a, b);
        result ^= test_sparc_delay_slot(b, c);
        result ^= test_generic_delay_slot(c, a);
        result ^= test_nested_jump(a, b, c);
        result ^= test_loop_jump(a % 10 + 1);
        
        /* Update global to prevent optimization */
        global_accumulator += result;
    }
    
    printf("Result checksum: %d\n", result);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return 0;
}
