/* caller-save-coverage.c
 * Designed to trigger GCC's caller-save pass instruction list manipulation
 * Specifically targets lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call boundary */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int __attribute__((noinline)) returning_external(void) {
    return 42;
}

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    int result = 0;
    
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use register variables before call */
        a = a + i * 2;
        b = b - i;
        c = c ^ (i + 1);
        
        /* Force spill/reload around this call */
        external_func();
        
        /* More operations after call requiring original values */
        result += a * b + c;
        
        /* Conditional that creates basic block structure */
        if (i & 1) {
            a += returning_external();  /* Another call */
        } else {
            b -= global_counter;  /* Volatile access */
        }
    }
    
    return result + a + b + c;
}

/* ===== SCENARIO 2: Many volatile variables across calls ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int sum = 0;
    
    /* Multiple volatile accesses interleaved with calls */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        
        /* Call in middle of volatile expression chain */
        external_func();
        
        v3 = v3 ^ v2;
        v4 = v4 | v3;
        
        /* Another call with volatile dependency */
        another_external(v4);
        
        v5 = v5 + v4 - v3;
        sum += v5;
        
        /* Conditional jump at end of basic block */
        if (v5 > 100) {
            goto early_exit;
        }
    }
    
early_exit:
    return sum + v1 + v2 + v3 + v4 + v5;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Force values into registers that will be clobbered */
    int a = x * 2;
    int b = x + 7;
    int c = x ^ 0xFF;
    
    /* Outer call setup uses registers */
    int outer = returning_external() + a;
    
    /* Inner call with arguments depending on outer call results */
    another_external(outer + b);
    
    /* More computation requiring original values */
    int d = a * b + c;
    
    /* Switch statement with call in default case */
    switch (x & 3) {
        case 0:
            d += 1;
            break;
        case 1:
            d += returning_external();
            break;
        default:
            /* Call at end of basic block before break */
            external_func();
            d += 3;
            break;  /* Creates jump at BB end */
    }
    
    return d + outer;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    int saved = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* Force registers to be live across setjmp */
        register int r1 __asm__ ("eax") = 100;
        register int r2 __asm__ ("ecx") = 200;
        
        /* Call that clobbers registers */
        external_func();
        
        /* Use registers after call */
        saved = r1 + r2;
        
        /* Another call */
        another_external(saved);
        
        /* Simulate longjmp - won't actually jump here */
        if (global_counter > 1000) {
            longjmp(jump_buffer, 1);
        }
    } else {
        /* longjmp target */
        saved = -1;
    }
    
    return saved;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int mode) {
    static const void* labels[] = { &&label0, &&label1, &&label2 };
    int result = 0;
    
    /* Force values into registers */
    int a = mode * 10;
    int b = mode + 20;
    
    /* Call before computed goto */
    external_func();
    
    /* Computed goto creates unusual control flow */
    goto *labels[mode % 3];
    
label0:
    result = a + b;
    another_external(result);
    break;
    
label1:
    result = a * b;
    /* Call near end of block */
    external_func();
    result += returning_external();
    break;
    
label2:
    result = a - b;
    /* Multiple calls in sequence */
    external_func();
    another_external(result);
    external_func();
    break;
    
    return result;
}

/* ===== SCENARIO 6: Complex basic block tail with calls ===== */
int __attribute__((noinline)) scenario6(int x) {
    int a = x, b = x * 2, c = x * 3;
    int result = 0;
    
    /* Create complex basic block ending with call+jump */
    for (int i = 0; i < 5; ++i) {
        a += i;
        b -= i;
        
        if (a > b) {
            /* Call followed by goto at BB end */
            result += returning_external();
            goto next_iteration;
        } else if (a < b) {
            /* Another call at BB end */
            external_func();
            result -= 1;
            continue;
        } else {
            /* Call with multiple register dependencies */
            c = a + b;
            another_external(c);
            result += c;
        }
        
        /* More operations */
        result += global_counter;
        
    next_iteration:
        c ^= result;
    }
    
    return result + a + b + c;
}

/* Main driver */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save coverage scenarios...\n");
    
    /* Run all scenarios */
    total += scenario1();
    total += scenario2();
    total += scenario3(5);
    total += scenario4();
    total += scenario5(2);
    total += scenario6(7);
    
    printf("Total checksum: %d\n", total);
    
    /* Verify with simple calculation */
    int verify = 0;
    for (int i = 0; i < 100; i++) {
        verify += i;
        if (i % 10 == 0) {
            external_func();  /* Extra calls for good measure */
        }
    }
    
    printf("Verification sum: %d\n", verify + total);
    return 0;
}
