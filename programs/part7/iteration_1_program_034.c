/* caller_save_coverage.c
 * Designed to trigger GCC's caller-save pass instruction list manipulation
 * Specifically targets lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call instruction */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int __attribute__((noinline)) returns_value(void) {
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
        
        /* Function call clobbers registers */
        external_func();
        
        /* More operations after call - values must be restored */
        result += a * b + c;
        
        /* Conditional that creates basic block structure */
        if (i & 1) {
            a += returns_value();  /* Another call! */
        }
    }
    
    /* Final computation using register variables */
    return result + a + b + c;
}

/* ===== SCENARIO 2: Many volatile variables ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int sum = 0;
    
    /* Multiple volatile operations across calls */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 2) + v1;
        
        /* Call in middle of computation */
        external_func();
        
        v3 = v3 - v2;
        v4 = v4 ^ v3;
        
        /* Another call with different function */
        another_external(v4);
        
        v5 = v5 + v4 * v3;
        sum += v5;
        
        /* Conditional jump at end of basic block */
        if (v5 > 100) {
            goto early_exit;
        }
    }
    
early_exit:
    return sum + v1 + v2 + v3 + v4 + v5;
}

/* ===== SCENARIO 3: Nested function calls ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Outer call setup in call-clobbered registers */
    register int arg1 __asm__ ("eax") = x * 2;
    register int arg2 __asm__ ("ecx") = x + 5;
    register int arg3 __asm__ ("edx") = x ^ 0xFF;
    
    /* First call - clobbers registers */
    external_func();
    
    /* Use results immediately for nested call */
    int temp = arg1 + arg2;
    
    /* Nested call with arguments depending on register values */
    another_external(temp + arg3);
    
    /* More computation after nested call */
    for (int i = 0; i < 3; ++i) {
        arg1 += i;
        external_func();  /* Call at end of loop body */
        
        /* Switch statement to create complex CFG */
        switch (i) {
            case 0:
                arg2 += arg1;
                break;
            case 1:
                arg3 += arg2;
                /* Function call right before break */
                external_func();
                break;
            default:
                arg1 += returns_value();  /* Call in default case */
                break;
        }
    }
    
    return arg1 + arg2 + arg3;
}

/* ===== SCENARIO 4: setjmp/longjmp interaction ===== */
int __attribute__((noinline)) scenario4(void) {
    register int preserved1 __asm__ ("eax") = 100;
    register int preserved2 __asm__ ("ecx") = 200;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        for (int i = 0; i < 5; ++i) {
            preserved1 += i * 10;
            preserved2 -= i * 5;
            
            /* Call that might longjmp */
            external_func();
            
            /* Values must be preserved across potential longjmp */
            result += preserved1 * preserved2;
            
            /* Conditional with call at end */
            if (i == 3) {
                global_counter++;
                another_external(result);
            }
        }
    } else {
        /* After longjmp */
        result = preserved1 + preserved2;
    }
    
    return result;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    register int r1 __asm__ ("eax") = selector;
    register int r2 __asm__ ("ecx") = selector * 2;
    int total = 0;
    
    /* Computed goto creates unusual control flow */
    goto *labels[selector & 3];
    
label0:
    r1 += 10;
    external_func();  /* Call before other label */
    goto join;
    
label1:
    r2 -= 20;
    /* Call followed by label at BB end */
    another_external(r2);
    goto join;
    
label2:
    r1 *= 2;
    r2 /= 2;
    external_func();
    /* Fall through to label3 */
    
label3:
    total = returns_value();  /* Call */
    r1 += total;
    /* No goto - falls through to join */
    
join:
    /* Multiple calls in sequence */
    for (int i = 0; i < 2; ++i) {
        r1 += i;
        external_func();
        r2 -= i;
        another_external(r2);
        
        /* Complex tail of basic block */
        if (r1 > r2) {
            total += r1;
            external_func();  /* Call at BB end before jump */
            goto done;
        }
    }
    
done:
    return total + r1 + r2;
}

/* ===== Main driver ===== */
int main(void) {
    int checksum = 0;
    
    printf("Testing caller-save coverage scenarios...\n");
    
    /* Run all scenarios */
    checksum += scenario1();
    checksum += scenario2();
    checksum += scenario3(5);
    checksum += scenario4();
    checksum += scenario5(2);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify with a simple calculation */
    int verify = 0;
    for (int i = 0; i < 100; ++i) {
        verify += i;
        if (i % 10 == 0) {
            external_func();  /* Extra calls for good measure */
        }
    }
    
    printf("Verification sum: %d\n", verify + checksum);
    
    return 0;
}
