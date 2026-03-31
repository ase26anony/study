/* caller_save_test.c - Test program to trigger specific caller-save optimization logic */
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call boundary */
    volatile int sink = 0;
    (void)sink;
}

int __attribute__((noinline)) external_func_with_return(void) {
    return 42;
}

/* Global jump buffer for setjmp/longjmp test */
static jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) test_register_pressure_loop(void) {
    /* Force use of specific call-clobbered registers */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a += i * 2;
        b -= i;
        c = a ^ b;
        d = c + i;
        
        /* Function call clobbers registers - forces save/restore */
        external_func();
        
        /* Use values after call - must be restored */
        sum += a + b + c + d;
        
        /* Conditional to create basic block structure */
        if (i % 3 == 0) {
            a += external_func_with_return();
        }
    }
    
    /* Mix of operations to create dense instruction sequence */
    volatile int barrier = sum;
    return barrier + a - b + c * d;
}

/* ===== SCENARIO 2: Many volatile variables across calls ===== */
int __attribute__((noinline)) test_volatile_pressure(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    
    /* Multiple expressions using volatiles before call */
    int result = v1 * v2 + v3 - v4 / (v5 + 1);
    result += v6 * v7 - v8 + v9 % (v10 + 1);
    
    /* Call that forces spills */
    external_func();
    
    /* More expressions after call */
    result += v1 + v2 * v3 - v4 / v5;
    result += v6 - v7 + v8 * v9 % v10;
    
    /* Another call with different volatile usage */
    v1 = external_func_with_return();
    external_func();
    
    /* Final computation */
    return result + v1 + v2 + v3 + v4 + v5;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) test_nested_call_pressure(void) {
    int x = 1, y = 2, z = 3;
    
    /* Outer call setup uses registers */
    x = x * 2 + y;
    y = y + z * 3;
    
    /* Function call - arguments depend on register values */
    external_func();
    
    /* Inner call sequence with data dependencies */
    for (int i = 0; i < 5; i++) {
        /* Values in registers from previous iteration */
        int temp = x + y + z;
        
        /* Call with register-dependent computation */
        external_func();
        
        /* Update using result (forces save/restore around call) */
        x = temp + i;
        y = x * 2;
        z = y - temp;
        
        /* Conditional jump at end of basic block */
        if (i == 3) {
            goto special_label;
        }
        
        /* Continue normal flow */
        x += 1;
    }
    
    return x + y + z;

special_label:
    /* Different basic block ending with call and jump */
    external_func();
    return 1000;  /* Creates jump to function return */
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) test_setjmp_pressure(void) {
    volatile int counter = 0;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        register int r1 __asm__ ("eax") = 100;
        register int r2 __asm__ ("ecx") = 200;
        
        /* Use registers before call */
        result = r1 + r2;
        
        /* Call that might longjmp */
        external_func();
        
        /* Code that may not be reached if longjmp happens */
        result += r1 * r2;
        
        /* Another call */
        external_func_with_return();
    } else {
        /* After longjmp */
        result = 999;
    }
    
    /* Force register usage after setjmp context */
    volatile int sink = result;
    for (int i = 0; i < 3; i++) {
        sink += i;
        external_func();
    }
    
    return sink;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) test_computed_goto_pressure(void) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int value = 0;
    int index = 0;
    
    /* Use explicit register variables */
    register int acc __asm__ ("ebx") = 0;
    
label_start:
    /* Switch-like structure with computed goto */
    index = (index + 1) % 4;
    
    /* Function call before goto */
    external_func();
    
    /* Computed goto - creates unusual control flow */
    goto *labels[index];
    
label0:
    acc += 1;
    external_func();  /* Call near block end */
    value += acc;
    goto label_start;
    
label1:
    acc += 2;
    /* Call then conditional */
    if (acc > 10) {
        external_func();
        return value;
    }
    external_func();
    value += acc;
    goto label_start;
    
label2:
    acc += 3;
    external_func();
    value += acc;
    if (value > 50) {
        return value;
    }
    goto label_start;
    
label3:
    acc += 4;
    external_func();
    value += acc;
    goto label_start;
}

/* ===== SCENARIO 6: Complex basic block ending with call and jump ===== */
int __attribute__((noinline)) test_complex_block_end(void) {
    int x = 0, y = 0, z = 0;
    
    /* Create register pressure */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    
    for (int i = 0; i < 20; i++) {
        /* Multiple basic blocks within loop */
        if (i % 2 == 0) {
            a += i;
            b -= i;
            
            /* Function call in middle of block */
            external_func();
            
            /* More operations */
            x += a;
            y += b;
            
            /* Conditional at end of block */
            if (x > y) {
                z += external_func_with_return();
                /* Block ends with call then jump */
                continue;
            }
        } else {
            a *= 2;
            b /= 2;
            
            /* Another call */
            external_func();
            
            x -= a;
            y -= b;
            
            /* Switch-like structure */
            switch (i % 3) {
                case 0:
                    z += a;
                    external_func();  /* Call before break */
                    break;
                case 1:
                    z += b;
                    external_func();  /* Call before break */
                    break;
                default:
                    z += a + b;
                    external_func();  /* Call before break */
                    break;
            }
        }
        
        /* Loop update with call */
        a += 1;
        b += 1;
        external_func();
    }
    
    return x + y + z + a + b;
}

/* ===== Main function to drive all tests ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all test scenarios */
    total += test_register_pressure_loop();
    total += test_volatile_pressure();
    total += test_nested_call_pressure();
    total += test_setjmp_pressure();
    total += test_computed_goto_pressure();
    total += test_complex_block_end();
    
    printf("Total checksum: %d\n", total);
    
    /* Verify with a simple check */
    if (total != 0) {
        printf("All tests executed successfully.\n");
    }
    
    return 0;
}
