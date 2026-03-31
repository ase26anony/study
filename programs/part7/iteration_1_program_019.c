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
    volatile int sink = 0;
    (void)sink;
}

int __attribute__((noinline)) external_func_with_return(void) {
    volatile int x = 42;
    return x;
}

/* Dummy volatile variables to prevent optimizations */
volatile int g_volatile_counter = 0;
volatile int g_volatile_flag = 1;

/* Scenario 1: Explicit register variables with loop */
int __attribute__((noinline)) scenario_register_vars(void) {
    int result = 0;
    
    /* Force use of specific call-clobbered registers */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    for (int i = 0; i < 10; ++i) {
        /* Complex arithmetic to create register pressure */
        a = a * i + 1;
        b = b - i * 2;
        c = c ^ (i + a);
        
        /* Mix with volatile to prevent optimization */
        g_volatile_counter++;
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More operations after call requiring original values */
        result += a + b - c;
        
        /* Conditional that might create basic block boundaries */
        if (i & 1) {
            a += external_func_with_return();
        } else {
            b -= g_volatile_flag;
        }
    }
    
    return result + a + b + c;
}

/* Scenario 2: Many volatile variables across calls */
int __attribute__((noinline)) scenario_volatile_vars(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int sum = 0;
    
    for (int i = 0; i < 5; ++i) {
        /* Use all volatiles in complex expression */
        v1 = v2 * v3 + v4;
        v5 = v6 - v7 * v8;
        v9 = v10 ^ (v1 + v5);
        
        /* Multiple function calls */
        external_func();
        
        /* Continue using volatiles */
        sum += v1 + v3 + v5 + v7 + v9;
        
        external_func_with_return();
        
        /* Create conditional basic block ending with call */
        if (v1 > v2) {
            v3 = external_func_with_return() + v4;
            /* This creates a basic block ending with function call */
            if (v5) {
                external_func();
                /* Followed by label and jump */
                goto update_and_continue;
            }
        }
        
        v2 = v3 + 1;
        
    update_and_continue:
        v4 = v5 * 2;
        
        /* Switch to create different basic block structure */
        switch (i % 3) {
            case 0:
                v6 = v7 + external_func_with_return();
                break;  /* Creates jump at end of basic block */
            case 1:
                external_func();
                v8 = v9 - 1;
                break;
            default:
                v10 = external_func_with_return() * 2;
                /* No break - falls through to increment */
        }
        
        v1++;
    }
    
    return sum + v1 + v2 + v3 + v4 + v5;
}

/* Scenario 3: Nested calls with register dependencies */
int __attribute__((noinline)) scenario_nested_calls(int param) {
    int x = param;
    int y = param * 2;
    int z = param + 7;
    
    /* Outer call setup in registers */
    x = x * 3 - 1;
    y = y + external_func_with_return();  /* First call */
    
    /* Inner call with arguments depending on outer results */
    for (int i = 0; i < 3; ++i) {
        /* Values in registers from outer call */
        int temp1 = x + y;
        int temp2 = z - i;
        
        /* Nested calls with register dependencies */
        external_func();
        temp1 = external_func_with_return() + temp2;
        
        /* Conditional that creates BB_END manipulation opportunities */
        if (temp1 > 100) {
            z = external_func_with_return();
            /* Basic block ends with call then goto */
            goto recalculation;
        } else if (temp1 < 50) {
            x = external_func_with_return();
            /* Another block ending with call */
        }
        
        y = temp1 + temp2;
        
    recalculation:
        z = z * 2 + 1;
        
        /* Computed goto to create unusual CFG */
        if (i == 1) {
            void* target = &&special_case;
            goto *target;
        }
        
        continue;
        
    special_case:
        external_func();
        x = y + z;
    }
    
    return x + y + z;
}

/* Scenario 4: setjmp/longjmp with calls */
jmp_buf jump_buffer;
int __attribute__((noinline)) scenario_setjmp(void) {
    volatile int preserved = 42;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        for (int i = 0; i < 4; ++i) {
            /* Live values across call that might need saving */
            int a = preserved + i;
            int b = a * 2;
            
            /* Function call that could trigger save/restore */
            external_func();
            
            /* Use values after call */
            result += a + b;
            
            /* Conditional with call at end */
            if (i == 2) {
                external_func_with_return();
                /* This creates BB_END == insn situation */
                goto update;
            }
            
            preserved += external_func_with_return();
            
        update:
            result *= 2;
        }
    } else {
        /* longjmp return */
        result = -1;
    }
    
    /* Simulate longjmp call */
    if (result > 100) {
        longjmp(jump_buffer, 1);
    }
    
    return result;
}

/* Scenario 5: Computed goto with calls in basic blocks */
int __attribute__((noinline)) scenario_computed_goto(void) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int values[4] = {0};
    int result = 0;
    
    for (int i = 0; i < 8; ++i) {
        /* Force values into registers */
        register int r1 __asm__ ("eax") = i * 2;
        register int r2 __asm__ ("ecx") = i * 3;
        register int r3 __asm__ ("edx") = i * 5;
        
        /* Use before call */
        values[i % 4] = r1 + r2 - r3;
        
        /* Function call clobbering registers */
        external_func();
        
        /* Use after call - requires restore */
        result += values[i % 4] + r1;
        
        /* Computed goto based on condition */
        int target = (r1 + r2) % 4;
        goto *labels[target];
        
    label0:
        r2 = external_func_with_return();
        /* Call at end of basic block */
        continue;
        
    label1:
        external_func();
        r3 = r1 * 2;
        continue;
        
    label2:
        r1 = external_func_with_return() + 1;
        /* Another call at block end */
        continue;
        
    label3:
        external_func();
        result += r2 + r3;
        continue;
    }
    
    return result;
}

/* Main function to drive all scenarios */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios */
    total += scenario_register_vars();
    total += scenario_volatile_vars();
    total += scenario_nested_calls(10);
    total += scenario_setjmp();
    total += scenario_computed_goto();
    
    printf("Total result: %d\n", total);
    printf("(This value is not meaningful - the goal is compile-time coverage)\n");
    
    return 0;
}
