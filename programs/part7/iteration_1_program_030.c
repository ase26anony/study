/* caller-save-test.c
 * Program designed to trigger specific instruction list manipulation
 * in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc)
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but not inlined */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    /* Clobber multiple registers */
    asm volatile("" : : "r"(x) : "eax", "ecx", "edx", "esi", "edi", "memory");
}

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of specific call-clobbered registers */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    int result = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 20; ++i) {
        /* Create register pressure before call */
        a = a * 2 + i;
        b = b - i * 3;
        c = c ^ (a + b);
        d = d + (c >> 2);
        
        /* Function call clobbers our registers */
        external_func();
        
        /* Use values after call - must be restored */
        result += a + b - c + d;
        
        /* More arithmetic to create dense block */
        if (i & 1) {
            a += result;
            b -= result;
        } else {
            c ^= result;
            d |= result;
        }
        
        /* Another call with different clobber pattern */
        another_external(result);
        
        /* Final computation at block end */
        result = (result * 13) ^ (a + b + c + d);
    }
    
    return result;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1234;
    volatile int v2 = 5678;
    volatile int v3 = 9012;
    volatile int v4 = 3456;
    int sum = 0;
    
    /* Multiple basic blocks with calls at ends */
    for (int j = 0; j < 15; ++j) {
        /* Block with computation then call at end */
        int temp = v1 * v2 - v3 / (v4 + 1);
        
        /* Call at what could be BB_END */
        external_func();
        
        /* This creates a new block after the call */
        if (temp > 1000) {
            v1 = temp;
            /* Another call in conditional block */
            another_external(v1);
            /* goto-like control flow */
            if (v1 & 1) goto update;
        }
        
        v2 = temp + v3;
        /* Call before block end with following jump */
        external_func();
        
    update:
        v3 = v2 ^ v1;
        v4 = v3 + j;
        sum += v4;
        
        /* Switch to create complex block ends */
        switch (j % 4) {
            case 0:
                v1 += sum;
                external_func();  /* Call before break */
                break;
            case 1:
                v2 -= sum;
                another_external(v2);
                break;
            case 2:
                v3 *= sum;
                external_func();
                /* Fall through */
            default:
                v4 /= (sum | 1);
                another_external(v4);
                /* break is implicit at end */
        }
    }
    
    return sum;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Arguments in registers that get clobbered */
    int a = x * 2;
    int b = x + 1;
    int c = x ^ 0x55;
    
    /* Outer call setup uses registers */
    int outer = a + b + c;
    
    /* Nested call pattern */
    for (int i = 0; i < 10; ++i) {
        /* Compute arguments in registers */
        int arg1 = outer + i;
        int arg2 = outer - i * 2;
        
        /* First call clobbers registers */
        external_func();
        
        /* Use results immediately for next call */
        arg1 = arg1 * 3 + 1;
        arg2 = arg2 / 2 | 0x1;
        
        /* Second call with register arguments */
        another_external(arg1 + arg2);
        
        /* Complex tail of basic block */
        if (arg1 > arg2) {
            outer = arg1 - arg2;
            external_func();  /* Call at potential BB_END */
            /* Label after call */
            if (outer & 1) goto compute;
        } else {
            outer = arg2 - arg1;
            another_external(outer);
        }
        
    compute:
        c = outer ^ b;
        a = b + c;
        b = c - a;
    }
    
    return a + b + c;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    volatile int saved = 0;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        register int r1 __asm__ ("eax") = 100;
        register int r2 __asm__ ("ecx") = 200;
        
        /* Do computation in registers */
        for (int i = 0; i < 5; ++i) {
            r1 = r1 * 2 + i;
            r2 = r2 - i * 3;
            
            /* Call that might longjmp */
            external_func();
            
            /* These must be saved across call due to setjmp */
            result += r1 + r2;
            saved = r1 * r2;
            
            /* Another call */
            another_external(result);
        }
    } else {
        /* After longjmp */
        result = 999;
    }
    
    /* Simulate longjmp call */
    if (global_counter++ < 3) {
        longjmp(jump_buffer, 1);
    }
    
    return result;
}

/* ===== SCENARIO 5: Computed goto with function calls ===== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int value = 0;
    
    /* Force register usage */
    register int x __asm__ ("edx") = selector * 2;
    register int y __asm__ ("esi") = selector + 100;
    
    /* Jump to computed label */
    goto *labels[selector % 4];
    
label0:
    x = x + y;
    external_func();  /* Call in middle of block */
    y = y - x;
    /* Fall through */
    
label1:
    value = x * y;
    another_external(value);
    if (value > 1000) goto label3;
    /* else fall through */
    
label2:
    x = value ^ y;
    external_func();
    y = x + value;
    goto end;
    
label3:
    y = value | x;
    another_external(y);
    x = y * 2;
    /* Fall through to end */
    
end:
    return x + y + value;
}

/* ===== Main function to drive everything ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios to ensure code generation */
    total += scenario1();
    total += scenario2();
    total += scenario3(42);
    total += scenario4();
    total += scenario5(2);
    
    printf("Result checksum: %d\n", total);
    printf("(If this compiles and runs, the caller-save pass was engaged)\n");
    
    return 0;
}
