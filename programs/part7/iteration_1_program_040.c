/* caller_save_coverage.c
 * Designed to trigger GCC's caller-save pass instruction list manipulation
 * Compile with: gcc -O2 -fno-omit-frame-pointer -c caller_save_coverage.c
 * Or: gcc -O3 -fschedule-insns -freg-struct-return -c caller_save_coverage.c
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External function declarations to force calls */
void external_func(void) __attribute__((noinline));
int external_func_with_return(void) __attribute__((noinline));
void clobbering_func(void) __attribute__((noinline));

/* Define them to avoid linkage errors */
void external_func(void) {
    /* Empty but non-inlineable */
    asm volatile("" : : : "memory");
}

int external_func_with_return(void) {
    return 42;
}

void clobbering_func(void) {
    /* Clobber many registers */
    asm volatile("" : : : "eax", "ecx", "edx", "esi", "edi", "ebx", "memory");
}

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables in loop ===== */
int __attribute__((noinline)) scenario1_register_pressure(void) {
    /* Force use of specific call-clobbered registers */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 20; ++i) {
        /* Use all register variables before call */
        a += i * 2;
        b -= i;
        c = a ^ b;
        d = c + i;
        
        /* Function call clobbers registers - forces save/restore */
        external_func();
        
        /* Use values after call - they must be restored */
        sum += a + b + c + d;
        
        /* Conditional to create basic block boundaries */
        if (i & 1) {
            a *= 2;
            b /= 2;
        } else {
            c += d;
            d -= a;
        }
        
        /* Another call with different register pressure */
        if (i % 3 == 0) {
            clobbering_func();
            sum += external_func_with_return();
        }
    }
    
    return sum + a + b + c + d;
}

/* ===== SCENARIO 2: Volatile variables across calls ===== */
int __attribute__((noinline)) scenario2_volatile_pressure(void) {
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    
    int result = 0;
    
    /* Multiple basic blocks with calls at ends */
    for (int i = 0; i < 15; i++) {
        v1 += i;
        v2 -= i;
        
        if (v1 > v2) {
            /* Call at end of basic block before jump */
            external_func();
            result += v1 * v2;
            /* goto-like control flow */
            if (v3 > 10) goto compute;
        } else {
            v3 *= 2;
            v4 /= 2;
            external_func_with_return();
            result += v3 - v4;
        }
        
        v5 = v1 + v2 + v3 + v4;
        clobbering_func();
        
    compute:
        result += v5;
        
        /* Switch to create complex basic block ends */
        switch (i % 4) {
            case 0:
                v1 += external_func_with_return();
                break;  /* Creates jump at BB end */
            case 1:
                v2 -= external_func_with_return();
                break;
            case 2:
                external_func();
                v3 *= 3;
                break;
            default:
                clobbering_func();
                v4 = v5;
                break;
        }
    }
    
    return result + v1 + v2 + v3 + v4 + v5;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3_nested_calls(void) {
    int x = 1, y = 2, z = 3, w = 4;
    
    /* Chain of computations with calls in between */
    for (int i = 0; i < 10; i++) {
        /* Setup in registers */
        x = i * 2;
        y = i + 5;
        
        /* Call that might clobber registers used in next call's args */
        external_func();
        
        /* Use results after call */
        z = x + y;
        
        /* Another call with register-based arguments */
        w = z + external_func_with_return();
        
        /* Complex condition with call at end */
        if (w > 20) {
            x = external_func_with_return() * 2;
            y = x + 5;
            /* Call at end of BB before implicit jump to loop end */
            clobbering_func();
        } else {
            y = external_func_with_return() / 2;
            x = y - 3;
            external_func();
        }
        
        /* Label for computed goto in next scenario */
        if (i == 5) {
            z = w * 2;
        }
    }
    
    return x + y + z + w;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4_setjmp_pressure(void) {
    volatile int saved = 0;
    int a = 1, b = 2, c = 3;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        a = 10;
        b = 20;
        c = 30;
        
        /* Call that might be interrupted */
        external_func();
        
        /* Use values after call - conservative save/restore needed */
        saved = a + b + c;
        
        /* Another call before potential longjmp */
        clobbering_func();
        
        /* Simulate error - would longjmp in real scenario */
        if (global_counter++ > 100) {
            /* longjmp(jump_buffer, 1); */
        }
    } else {
        /* After longjmp */
        a = 100;
        b = 200;
        external_func();
        c = 300;
    }
    
    /* Mix of calls and computations */
    for (int i = 0; i < 5; i++) {
        a += external_func_with_return();
        if (i & 1) {
            b -= clobbering_func(), external_func();
        } else {
            c *= 2;
            external_func();
        }
    }
    
    return a + b + c + saved;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5_computed_goto(void) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int values[4] = {0};
    int index = 0;
    int result = 0;
    
    /* Initialize */
    for (int i = 0; i < 4; i++) {
        values[i] = i * 10 + 1;
    }
    
    /* Loop with computed goto */
    for (int i = 0; i < 20; i++) {
        index = i % 4;
        
        /* Call before goto */
        external_func();
        
        goto *labels[index];
        
    label0:
        values[0] += external_func_with_return();
        result += values[0];
        continue;
        
    label1:
        clobbering_func();
        values[1] *= 2;
        result += values[1];
        continue;
        
    label2:
        values[2] -= 5;
        external_func();
        result += values[2];
        continue;
        
    label3:
        values[3] = external_func_with_return() + values[0];
        result += values[3];
        continue;
    }
    
    return result;
}

/* ===== Main function to drive everything ===== */
int main(void) {
    int total = 0;
    
    printf("Starting caller-save coverage test...\n");
    
    /* Run all scenarios */
    total += scenario1_register_pressure();
    total += scenario2_volatile_pressure();
    total += scenario3_nested_calls();
    total += scenario4_setjmp_pressure();
    total += scenario5_computed_goto();
    
    printf("Total result: %d\n", total);
    printf("(This value is arbitrary - main goal is compile-time coverage)\n");
    
    return 0;
}
