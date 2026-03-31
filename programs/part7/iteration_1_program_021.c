/* caller_save_coverage.c
 * Designed to trigger GCC's caller-save pass instruction list manipulation
 * Compile with: gcc -O2 -fno-omit-frame-pointer -c caller_save_coverage.c
 * Or: gcc -O3 -fschedule-insns -freg-struct-return -c caller_save_coverage.c
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

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ========== SCENARIO 1: Explicit register variables in loop ========== */
int __attribute__((noinline)) scenario1_register_pressure(void) {
    /* Force use of specific call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 100; ++i) {
        /* Use all register variables before call */
        a = a * 2 + i;
        b = b / 2 - i;
        c = c ^ (a + b);
        d = d & (c | i);
        
        /* Function call clobbers registers - forces save/restore */
        external_func();
        
        /* More operations after call - values must be restored */
        sum += a + b + c + d;
        
        /* Conditional that creates basic block boundaries */
        if (i & 1) {
            a += external_func_with_return();
        } else {
            b -= external_func_with_return();
        }
        
        /* Another call with different register pressure */
        if (i % 3 == 0) {
            external_func();
            c = c * 2;
        }
    }
    
    return sum + a + b + c + d;
}

/* ========== SCENARIO 2: Volatile variables and complex expressions ========== */
int __attribute__((noinline)) scenario2_volatile_pressure(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int result = 0;
    
    /* Multiple volatile accesses around calls */
    for (int i = 0; i < 50; ++i) {
        /* Complex expression with volatiles */
        v1 = v1 * v2 + v3 - v4 / (v5 + 1);
        v2 = v2 ^ v1 | v3;
        
        /* Call that forces spills */
        external_func();
        
        /* More volatile operations */
        v3 = v3 + v1 * v2;
        v4 = v4 - v3 / v2;
        
        /* Conditional call at end of basic block */
        if (v1 > v2) {
            result += external_func_with_return();
            /* This creates a basic block ending with call+result usage */
            goto update;
        } else {
            result -= external_func_with_return();
        }
        
    update:
        v5 = v5 + i;
        
        /* Switch to create different basic block structure */
        switch (i % 4) {
            case 0:
                v1 += 1;
                external_func();  /* Call before break */
                break;  /* Jump at end of basic block */
            case 1:
                v2 -= 1;
                external_func();
                break;
            default:
                v3 *= 2;
                external_func();  /* Call in default case before break */
                break;
        }
    }
    
    return result + v1 + v2 + v3 + v4 + v5;
}

/* ========== SCENARIO 3: Nested calls with register dependencies ========== */
int __attribute__((noinline)) scenario3_nested_calls(void) {
    int x = 1, y = 2, z = 3;
    
    /* Outer call setup uses registers that inner call clobbers */
    for (int i = 0; i < 30; ++i) {
        /* Compute values in registers */
        x = x * 3 + i;
        y = y * 2 - i;
        z = z ^ (x + y);
        
        /* Nested call pattern */
        if (x > y) {
            /* First call */
            external_func();
            
            /* Use results in second call's arguments */
            int temp = external_func_with_return() + x;
            
            /* Second call with dependency */
            if (temp > 0) {
                external_func();
                z += temp;
            }
            
            /* Label for computed goto */
            if (i == 15) {
                goto special_case;
            }
        }
        
        /* Another basic block ending with call */
        if (z % 2 == 0) {
            y += external_func_with_return();
            /* This should create BB_END == call insn */
            goto loop_end;
        }
        
    special_case:
        x = x / 2;
        
    loop_end:
        continue;
    }
    
    /* Computed goto to create unusual CFG */
    void* labels[] = { &&label1, &&label2, &&label3 };
    
    for (int i = 0; i < 3; ++i) {
        goto *labels[i];
        
    label1:
        external_func();
        x++;
        continue;
        
    label2:
        external_func();
        y++;
        continue;
        
    label3:
        external_func();
        z++;
        continue;
    }
    
    return x + y + z;
}

/* ========== SCENARIO 4: setjmp/longjmp with calls ========== */
int __attribute__((noinline)) scenario4_setjmp_pressure(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        for (int i = 0; i < 20; ++i) {
            /* Live values across potential longjmp */
            a = a * 2 + i;
            b = b / 2 - i;
            
            /* Function call - registers must be saved conservatively */
            external_func();
            
            c = c ^ (a + b);
            d = d & (c | i);
            
            result += a + b + c + d;
            
            /* Conditional with call at end */
            if (i == 10) {
                external_func();
                /* Basic block ends with call */
                goto checkpoint;
            }
        }
    } else {
        /* After longjmp */
        result += 1000;
    }
    
checkpoint:
    /* More operations with calls */
    for (int j = 0; j < 5; ++j) {
        a += external_func_with_return();
        b -= external_func_with_return();
        
        /* Switch with calls in cases */
        switch (j % 3) {
            case 0:
                external_func();
                c++;
                break;
            case 1:
                external_func();
                d++;
                break;
            case 2:
                external_func();
                result++;
                break;
        }
    }
    
    /* Simulate longjmp */
    if (global_counter++ < 1) {
        longjmp(jump_buffer, 1);
    }
    
    return result + a + b + c + d;
}

/* ========== SCENARIO 5: Computed goto with calls in basic blocks ========== */
int __attribute__((noinline)) scenario5_computed_goto(void) {
    static void* targets[] = { &&block1, &&block2, &&block3, &&block4 };
    
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4;
    int sum = 0;
    
    for (int i = 0; i < 40; ++i) {
        /* Modify register-like values */
        r1 = r1 * 3 + i;
        r2 = r2 * 2 - i;
        r3 = r3 ^ r1;
        r4 = r4 & r2;
        
        /* Computed goto creates complex CFG */
        goto *targets[i % 4];
        
    block1:
        /* Call in middle of block with following operations */
        external_func();
        r1 += 1;
        sum += r1;
        /* Fall through */
        
    block2:
        external_func();
        r2 -= 1;
        sum += r2;
        if (i % 2 == 0) {
            goto block4;
        }
        /* Continue */
        
    block3:
        external_func();
        r3 *= 2;
        sum += r3;
        /* Another call at what might be BB_END */
        if (r3 > 10) {
            external_func();
            /* This creates: call -> label -> jump */
            goto block_end;
        }
        r4 /= 2;
        
    block4:
        external_func();
        r4 += 3;
        sum += r4;
        
    block_end:
        continue;
    }
    
    return sum + r1 + r2 + r3 + r4;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int total = 0;
    
    printf("Starting caller-save coverage test...\n");
    
    /* Call all scenarios to ensure compilation */
    total += scenario1_register_pressure();
    total += scenario2_volatile_pressure();
    total += scenario3_nested_calls();
    total += scenario4_setjmp_pressure();
    total += scenario5_computed_goto();
    
    printf("Total checksum: %d\n", total);
    printf("If this prints, the compiled code executed successfully.\n");
    
    return total != 0 ? 0 : 1;
}
