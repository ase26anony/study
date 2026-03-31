/* caller-save-coverage.c
 * Designed to trigger GCC's caller-save optimization pass
 * Specifically targets the instruction list manipulation code
 * in caller-save.cc lines 905-913
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
volatile int volatile_global = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1_register_pressure(void) {
    int result = 0;
    
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Arithmetic on register variables */
        a = a + i * 2;
        b = b - i + 1;
        c = c ^ (i * 3);
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More arithmetic after call - values must be restored */
        a = a + volatile_global;
        b = b * (i + 1);
        c = c | 0xFF;
        
        /* Conditional that creates basic block boundaries */
        if (i & 1) {
            a += 5;
            external_func();
            b -= 3;
        } else {
            c ^= 0xAA;
            external_func();
            a <<= 1;
        }
        
        /* Use all values to keep them live */
        result += a + b + c;
    }
    
    return result;
}

/* ===== SCENARIO 2: Many volatile variables across calls ===== */
int __attribute__((noinline)) scenario2_volatile_pressure(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int sum = 0;
    
    /* Multiple expressions with volatile variables */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        v3 = v3 ^ v2;
        
        /* Call in middle of complex expression */
        external_func();
        
        v4 = v4 - v3 * i;
        v5 = v5 | (v4 & 0xF);
        
        /* Another call */
        if (v1 > v2) {
            external_func();
            v3 += external_func_with_return();
        }
        
        /* Force spill/reload around calls */
        sum += v1 + v2 + v3 + v4 + v5;
    }
    
    /* Basic block ending with call and jump */
    if (sum > 100) {
        external_func();
        goto done;
    } else {
        v1 = external_func_with_return();
        external_func();
    }
    
done:
    return sum + v1;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3_nested_calls(void) {
    int x = 1, y = 2, z = 3;
    
    /* Outer call setup in registers */
    for (int i = 0; i < 5; ++i) {
        x = x * 3 - i;
        y = y + x / 2;
        
        /* Inner call with arguments depending on outer values */
        int inner = external_func_with_return() + x;
        
        /* Another call that might clobber registers used for inner */
        external_func();
        
        z = z ^ inner;
        
        /* Switch statement to create complex control flow */
        switch (i % 3) {
            case 0:
                x += external_func_with_return();
                external_func();
                break;
            case 1:
                y -= external_func_with_return();
                external_func();
                break;
            default:
                z *= external_func_with_return();
                external_func();
                break;
        }
    }
    
    return x + y + z;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4_setjmp_pressure(void) {
    volatile int counter = 0;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        register int r1 __asm__ ("eax") = 100;
        register int r2 __asm__ ("ecx") = 200;
        
        for (int i = 0; i < 3; ++i) {
            r1 += i * 10;
            r2 -= i * 5;
            
            /* Call that might be affected by longjmp */
            external_func();
            
            result += r1 + r2;
            counter++;
            
            if (counter > 5) {
                /* Simulate longjmp - never reached but affects analysis */
                volatile_global = 1;
            }
        }
    } else {
        /* After longjmp - different path */
        external_func();
        result = external_func_with_return();
    }
    
    /* Basic block with call at end */
    if (result > 0) {
        external_func();
        goto exit_label;
    }
    
    result += 1000;
    
exit_label:
    return result;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5_computed_goto(void) {
    static void* labels[] = { &&label1, &&label2, &&label3 };
    int value = 0;
    int i = 0;
    
    /* Force spill before computed goto */
    register int r1 __asm__ ("eax") = 10;
    register int r2 __asm__ ("ecx") = 20;
    
label_start:
    r1 += i;
    r2 -= i;
    
    /* Function call in block with computed goto */
    external_func();
    
    /* Use computed goto */
    goto *labels[i % 3];
    
label1:
    value += r1 * 2;
    external_func();
    i++;
    if (i < 3) goto label_start;
    goto done;
    
label2:
    value += r2 / 2;
    external_func();
    i++;
    if (i < 3) goto label_start;
    goto done;
    
label3:
    value += r1 + r2;
    external_func();
    i++;
    if (i < 3) goto label_start;
    
done:
    return value;
}

/* ===== Main function to drive all scenarios ===== */
int main(void) {
    int total = 0;
    
    printf("Running caller-save coverage scenarios...\n");
    
    /* Run all scenarios to ensure code generation */
    total += scenario1_register_pressure();
    total += scenario2_volatile_pressure();
    total += scenario3_nested_calls();
    total += scenario4_setjmp_pressure();
    total += scenario5_computed_goto();
    
    printf("Total result: %d\n", total);
    printf("(This value is not meaningful - coverage is compile-time)\n");
    
    /* Use result to prevent dead code elimination */
    volatile int prevent_opt = total;
    return prevent_opt > 0 ? 0 : 1;
}
