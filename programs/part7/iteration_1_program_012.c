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

/* Volatile variables to prevent optimization */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile int vol_var3 = 3;

/* Jump buffer for setjmp/longjmp test */
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    int result = 0;
    
    /* Force use of specific call-clobbered registers */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use register variables before call */
        a = a + i + vol_var1;
        b = b - i * vol_var2;
        c = c ^ (i + vol_var3);
        
        /* Function call clobbers registers - forces save/restore */
        external_func();
        
        /* Use register variables after call */
        result += a + b + c;
        
        /* Conditional to create basic block structure */
        if (i & 1) {
            a += external_func_with_return();
        } else {
            b -= external_func_with_return();
        }
        
        /* More arithmetic to create dense instruction sequence */
        c = (c * 3) / 2;
    }
    
    return result + a + b + c;
}

/* ===== SCENARIO 2: Many volatile variables across calls ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = vol_var1;
    volatile int v2 = vol_var2;
    volatile int v3 = vol_var3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    volatile int v6 = 6;
    
    int sum = 0;
    
    /* Multiple calls with volatile variables live across them */
    for (int i = 0; i < 5; ++i) {
        /* Complex expression with volatiles */
        v1 = v2 + v3 * i;
        v4 = v5 - v6 / (i + 1);
        
        /* Call that forces spills */
        external_func();
        
        /* Use volatiles after call */
        sum += v1 + v4;
        
        /* Conditional with call at end of basic block */
        if (v1 > v4) {
            v2 = external_func_with_return();
            /* This creates a basic block ending with a call */
            goto update_vars;  /* Creates jump at end of block */
        }
        
        v3 = v4 + v5;
        
    update_vars:
        v5 = v6 + i;
        v6 = v1 - i;
    }
    
    return sum + v1 + v2 + v3 + v4 + v5 + v6;
}

/* ===== SCENARIO 3: Nested function calls ===== */
int __attribute__((noinline)) helper3(int x, int y) {
    return x * y + external_func_with_return();
}

int __attribute__((noinline)) scenario3(void) {
    int total = 0;
    
    /* Use call-clobbered registers for nested call setup */
    register int r1 __asm__ ("eax") = 10;
    register int r2 __asm__ ("ecx") = 20;
    register int r3 __asm__ ("edx") = 30;
    
    for (int i = 0; i < 8; ++i) {
        /* Setup arguments in registers */
        r1 = r1 + i;
        r2 = r2 - i;
        
        /* Nested call - inner call uses registers from outer setup */
        int temp = helper3(r1, r2);
        
        /* Another call that clobbers registers */
        external_func();
        
        /* Use results */
        r3 = r3 + temp;
        total += r3;
        
        /* Switch statement to create complex control flow */
        switch (i % 3) {
            case 0:
                r1 = external_func_with_return();
                break;  /* Creates jump at end of basic block */
            case 1:
                r2 = helper3(r1, r3);
                external_func();
                break;
            default:
                r3 = r1 + r2;
                external_func();
                /* Fall through to update */
        }
        
        /* More operations */
        r1 = r1 ^ r2;
        r2 = r2 | r3;
    }
    
    return total + r1 + r2 + r3;
}

/* ===== SCENARIO 4: setjmp/longjmp with function calls ===== */
int __attribute__((noinline)) scenario4(void) {
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        register int j1 __asm__ ("eax") = 100;
        register int j2 __asm__ ("ecx") = 200;
        
        /* Use registers before call */
        j1 = j1 + vol_var1;
        j2 = j2 - vol_var2;
        
        /* Function call - registers must be saved conservatively
           because longjmp could restore them */
        external_func();
        
        /* Use registers after call */
        result = j1 * j2;
        
        /* Simulate error condition */
        if (vol_var3 > 2) {
            longjmp(jump_buffer, 1);
        }
        
        /* More operations that won't be reached if longjmp is taken */
        j1 = j1 + external_func_with_return();
        result += j1;
    } else {
        /* After longjmp */
        result = 999;
    }
    
    return result;
}

/* ===== SCENARIO 5: Computed goto with function call ===== */
int __attribute__((noinline)) scenario5(void) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    register int g1 __asm__ ("eax") = 5;
    register int g2 __asm__ ("ecx") = 10;
    int total = 0;
    int index = 0;
    
    for (int i = 0; i < 12; ++i) {
        /* Update index for computed goto */
        index = (g1 + i) % 4;
        
        /* Function call before computed goto */
        g1 = g1 + external_func_with_return();
        g2 = g2 - i;
        
        /* Computed goto - creates unusual control flow */
        goto *labels[index];
        
    label0:
        total += g1;
        external_func();
        /* Continue loop */
        continue;
        
    label1:
        total += g2;
        g1 = g1 * 2;
        external_func();
        /* Conditional break at end of block */
        if (total > 100) break;
        continue;
        
    label2:
        total += g1 + g2;
        g2 = external_func_with_return();
        /* Fall through to label3 */
        
    label3:
        total -= 5;
        external_func();
        /* Loop continues */
    }
    
    return total + g1 + g2;
}

/* ===== Main function to execute all scenarios ===== */
int main(void) {
    int checksum = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios and accumulate results */
    checksum += scenario1();
    printf("Scenario 1 completed\n");
    
    checksum += scenario2();
    printf("Scenario 2 completed\n");
    
    checksum += scenario3();
    printf("Scenario 3 completed\n");
    
    checksum += scenario4();
    printf("Scenario 4 completed\n");
    
    checksum += scenario5();
    printf("Scenario 5 completed\n");
    
    printf("Final checksum: %d\n", checksum);
    
    /* Use the result to prevent dead code elimination */
    if (checksum > 0) {
        return 0;
    } else {
        return 1;
    }
}
