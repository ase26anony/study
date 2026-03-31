/* caller-save-test.c
 * Test program to trigger specific instruction list manipulation
 * in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc)
 */

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

/* Global jump buffer for setjmp/longjmp test */
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline, optimize("O2"))) 
test_scenario1(void) {
    int sum = 0;
    
    /* Force use of call-clobbered registers on x86-64 */
    register int64_t a __asm__("rax") = 1;
    register int64_t b __asm__("rcx") = 2;
    register int64_t c __asm__("rdx") = 3;
    register int64_t d __asm__("rsi") = 4;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 20; ++i) {
        /* Use all register variables before call */
        a = a + i * 2;
        b = b - i;
        c = c ^ (a + b);
        d = d * 2 + i;
        
        /* Function call clobbers registers */
        external_func();
        
        /* More operations after call - values must be restored */
        a = a + c;
        b = b - d;
        c = c ^ (b >> 2);
        d = d + a * 3;
        
        /* Conditional that creates basic block boundaries */
        if (i % 3 == 0) {
            external_func();
            sum += a;
        } else if (i % 3 == 1) {
            sum += b;
        } else {
            external_func();
            sum += c + d;
        }
    }
    
    /* Final computation using all register variables */
    return sum + a + b + c + d;
}

/* ===== SCENARIO 2: Many volatile variables across calls ===== */
int __attribute__((noinline, optimize("O3"))) 
test_scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int result = 0;
    
    /* Multiple expressions with volatile variables across calls */
    for (int i = 0; i < 15; i++) {
        v1 = v1 + v2;
        v3 = v4 - v5;
        v6 = v7 * v8;
        
        /* Call in middle of expression chain */
        external_func();
        
        v2 = v9 / (v10 + 1);
        v4 = v6 ^ v3;
        v5 = v1 | v2;
        
        /* Another call */
        int ret = external_func_with_return();
        
        v7 = v8 + ret;
        v8 = v9 - v4;
        v9 = v10 * v5;
        v10 = v1 + v6;
        
        /* Conditional with call at end of basic block */
        if (v1 > v2) {
            result += v3;
            external_func();  /* Call near BB end */
            /* No explicit jump here - fall through */
        } else {
            result -= v4;
            /* Basic block ends with break in switch below */
        }
    }
    
    /* Switch to create specific BB structure with calls near BB_END */
    switch (result % 4) {
        case 0:
            v1 = volatile_func(v2);
            external_func();
            break;  /* Creates jump at BB end */
            
        case 1:
            v2 = volatile_func(v3);
            result += v2;
            external_func();
            /* Fall through to create different BB structure */
            
        case 2:
            v3 = volatile_func(v4);
            external_func();
            goto label1;
            
        default:
            v4 = volatile_func(v5);
            external_func();
            result = v4;
            /* BB ends with return */
            return result;
    }
    
label1:
    return result + v1 + v10;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline, optimize("O2")))
test_scenario3(int x) {
    /* Chain of computations where call arguments depend on 
       values in call-clobbered registers */
    int a = x * 2;
    int b = a + 5;
    
    /* First call - result in register */
    int c = external_func_with_return() + a;
    
    /* Computation using result */
    int d = c * 3 - b;
    
    /* Nested call pattern */
    for (int i = 0; i < 8; i++) {
        /* Values must be preserved across both calls */
        a = a + i;
        b = b - external_func_with_return();  /* Call in expression */
        
        external_func();  /* Another call */
        
        c = c + a * b;
        d = d ^ (c >> 3);
        
        /* Complex condition with call at end */
        if (d > 100) {
            a = volatile_func(b);
            external_func();
            /* BB may end with implicit fallthrough to loop increment */
        } else if (d < 50) {
            b = volatile_func(c);
            external_func();
            continue;  /* Different BB end */
        } else {
            c = volatile_func(d);
            external_func();
            break;  /* Another BB end type */
        }
    }
    
    /* Computed goto to create unusual CFG */
    void* labels[] = { &&end, &&calc, &&ret };
    
    if (a > b) {
        goto *labels[0];
    } else if (a < b) {
        goto *labels[1];
    } else {
        goto *labels[2];
    }
    
calc:
    a = a + external_func_with_return();
    external_func();
    /* Fall through */
    
end:
    return a + b + c + d;
    
ret:
    external_func();
    return x;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline, optimize("O3")))
test_scenario4(int x) {
    volatile int a = x, b = x * 2, c = x * 3;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        for (int i = 0; i < 10; i++) {
            a = a + i;
            b = b * 2 - i;
            
            /* Call with live values in registers */
            external_func();
            
            c = c ^ (a + b);
            result += c;
            
            /* Conditional call near BB end */
            if (i == 5) {
                external_func();
                /* BB ends with longjmp */
                longjmp(jump_buffer, 1);
            }
        }
    } else {
        /* After longjmp */
        a = volatile_func(b);
        external_func();
        result += a * 2;
    }
    
    /* Switch with calls in cases */
    switch (result % 3) {
        case 0:
            a = external_func_with_return();
            external_func();
            result += a;
            break;
            
        case 1:
            b = volatile_func(c);
            external_func();
            result += b;
            /* Fall through */
            
        default:
            c = external_func_with_return();
            external_func();
            result += c;
            /* BB ends with return */
    }
    
    return result;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline, optimize("O2")))
test_scenario5(int init) {
    static const void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    
    register int r1 __asm__("rax") = init;
    register int r2 __asm__("rcx") = init * 2;
    register int r3 __asm__("rdx") = init * 3;
    
    int counter = 0;
    
L0:
    r1 = r1 + 1;
    external_func();
    if (counter++ > 20) goto L4;
    
L1:
    r2 = r2 - r1;
    external_func();
    goto *labels[r1 % 5];
    
L2:
    r3 = r3 ^ r2;
    external_func();
    if (r3 > 100) goto L0;
    
L3:
    r1 = r1 * 2;
    external_func();
    goto *labels[(r2 + counter) % 5];
    
L4:
    /* Final basic block with call and computation */
    external_func();
    return r1 + r2 + r3 + counter;
}

/* ===== Main function to run all tests ===== */
int main(void) {
    int result = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios */
    result += test_scenario1();
    printf("Scenario 1 complete\n");
    
    result += test_scenario2();
    printf("Scenario 2 complete\n");
    
    result += test_scenario3(10);
    printf("Scenario 3 complete\n");
    
    result += test_scenario4(5);
    printf("Scenario 4 complete\n");
    
    result += test_scenario5(3);
    printf("Scenario 5 complete\n");
    
    printf("Final result: %d\n", result);
    printf("(This value is arbitrary; the goal is compile-time coverage)\n");
    
    return 0;
}
