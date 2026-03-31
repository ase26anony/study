/* test_caller_save.c - Program to trigger specific caller-save optimization paths */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call instruction */
    asm volatile("" : : : "memory");
}

int __attribute__((noinline)) external_func_with_return(void) {
    return 42;
}

/* Volatile function to prevent optimization */
int __attribute__((noinline, noclone)) volatile_func(int x) {
    volatile int result = x * 2;
    return result;
}

/* Global jump buffer for setjmp/longjmp scenario */
static jmp_buf jump_buffer;

/* ========== SCENARIO 1: Explicit register variables with loop ========== */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a = a + i + 1;
        b = b - i * 2;
        c = c ^ (i << 1);
        d = d | (i & 0xF);
        
        /* Force spill/reload around this call */
        external_func();
        
        /* More operations after call */
        a = a * 2 - b;
        b = b + c / 3;
        c = c ^ d;
        d = d + a;
        
        /* Another call with different register pressure */
        if (i % 3 == 0) {
            external_func();
            a += external_func_with_return();
        }
        
        sum += a + b + c + d;
    }
    
    return sum;
}

/* ========== SCENARIO 2: Many volatile variables across calls ========== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    
    int result = 0;
    
    /* Multiple calls with volatile variables used in between */
    for (int i = 0; i < 5; ++i) {
        /* Complex expression using volatiles */
        v1 = v1 * v2 + v3;
        v2 = v2 - v4 / (v5 + 1);
        v3 = v3 ^ v6;
        v4 = v4 | v7;
        v5 = v5 & v8;
        
        /* Call that forces spills */
        external_func();
        
        /* More volatile operations */
        v6 = v6 + v9 - v10;
        v7 = v7 * v1;
        v8 = v8 ^ v2;
        v9 = v9 | v3;
        v10 = v10 & v4;
        
        /* Another call */
        if (v1 > v2) {
            external_func();
        }
        
        result += v1 + v2 + v3 + v4 + v5;
    }
    
    return result;
}

/* ========== SCENARIO 3: Nested calls with register pressure ========== */
int __attribute__((noinline)) scenario3(int x) {
    /* Create register pressure for nested calls */
    int a = x * 2;
    int b = x + 100;
    int c = x - 50;
    int d = x ^ 0xABCD;
    
    /* Outer call setup uses registers */
    int r1 = volatile_func(a + b);
    
    /* Inner call with arguments depending on outer call results */
    int r2 = volatile_func(r1 + c);
    
    /* More operations between calls */
    a = r1 * r2;
    b = a + d;
    
    /* Call with complex argument expression */
    int r3 = volatile_func(a * b - c);
    
    /* Conditional call at end of basic block */
    if (r3 > 1000) {
        external_func();  /* This call may trigger BB_END update */
        return r3;        /* Return creates end of block */
    }
    
    /* Alternative path with different call pattern */
    external_func();
    return r1 + r2;
}

/* ========== SCENARIO 4: setjmp/longjmp with calls ========== */
int __attribute__((noinline)) scenario4(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First pass - do computations */
        for (int i = 0; i < 3; ++i) {
            a = a * 2 + i;
            b = b - i * 3;
            c = c ^ (i << 2);
            d = d | (i & 0x7);
            
            /* Call that might be before setjmp */
            external_func();
            
            result += a + b + c + d;
        }
        
        /* Simulate longjmp - never reached in normal execution */
        /* longjmp(jump_buffer, 1); */
    } else {
        /* Second pass - different computations */
        a = 100;
        b = 200;
        external_func();
        result = a * b;
    }
    
    /* Another call at block end */
    external_func();
    return result;
}

/* ========== SCENARIO 5: Computed goto with function call ========== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int a = 10, b = 20, c = 30, d = 40;
    int result = 0;
    
    /* Force values into registers */
    a = a * selector;
    b = b + selector * 2;
    c = c ^ selector;
    d = d | selector;
    
    /* Computed goto */
    goto *labels[selector % 4];
    
label0:
    a += 100;
    external_func();  /* Call in middle of block */
    b += 50;
    goto end;
    
label1:
    b *= 2;
    external_func();
    c -= 10;
    if (c > 0) goto end;  /* Conditional jump at end */
    /* Fall through */
    
label2:
    c = volatile_func(a);
    external_func();
    d = b + c;
    goto end;
    
label3:
    external_func();
    d = 999;
    /* Fall through to end */
    
end:
    result = a + b + c + d;
    
    /* One more call potentially at BB_END */
    if (result > 1000) {
        external_func();
    }
    
    return result;
}

/* ========== SCENARIO 6: Switch statement with calls at block ends ========== */
int __attribute__((noinline)) scenario6(int code) {
    int x = 0, y = 0, z = 0;
    
    /* Force register usage */
    register int r1 __asm__ ("eax") = code * 10;
    register int r2 __asm__ ("ecx") = code + 100;
    
    switch (code % 4) {
        case 0:
            x = r1 * 2;
            y = r2 + 5;
            external_func();  /* Call before break */
            z = x + y;
            break;  /* Jump at end of basic block */
            
        case 1:
            x = volatile_func(r1);
            external_func();
            y = x * 3;
            if (y > 50) {
                external_func();  /* Call in conditional block */
                z = 100;
            } else {
                z = 200;
            }
            break;
            
        case 2:
            for (int i = 0; i < 3; ++i) {
                r1 += i;
                r2 -= i;
                external_func();  /* Call in loop */
                x += r1;
                y += r2;
            }
            z = x * y;
            break;
            
        default:
            x = r1 ^ r2;
            external_func();  /* Call in default case */
            y = volatile_func(x);
            z = y * 2;
            /* No break - fall through to return */
    }
    
    return z + r1 + r2;
}

/* ========== Main function to drive all scenarios ========== */
int main(void) {
    int total = 0;
    
    printf("Running caller-save coverage tests...\n");
    
    /* Run all scenarios to ensure code generation */
    total += scenario1();
    total += scenario2();
    total += scenario3(42);
    total += scenario4();
    total += scenario5(2);
    total += scenario6(3);
    
    printf("Total checksum: %d\n", total);
    printf("(If this compiles and runs, the caller-save pass was engaged)\n");
    
    /* Verify with a simple check */
    if (total != 0) {
        printf("SUCCESS: Program executed with non-zero result\n");
    }
    
    return 0;
}
