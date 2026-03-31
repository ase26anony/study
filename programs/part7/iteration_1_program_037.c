/* caller_save_test.c - Test program to trigger specific caller-save optimization */
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

/* Dummy volatile function to prevent optimization */
volatile int dummy_volatile = 0;

/* Scenario 1: Explicit register variables with loop */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__("eax") = 1;
    register int b __asm__("ecx") = 2;
    register int c __asm__("edx") = 3;
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a += i * 2;
        b -= i * 3;
        c ^= i * 5;
        
        /* Function call clobbers registers - forces spills */
        external_func();
        
        /* Use values after call - forces reloads */
        sum += a + b + c;
        
        /* Additional arithmetic to create dense block */
        if (i & 1) {
            a += b;
            b ^= c;
            c -= a;
        } else {
            a -= c;
            b += a;
            c ^= b;
        }
    }
    
    /* Final computation using all register variables */
    return sum + a + b + c;
}

/* Scenario 2: Many volatile variables across calls */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int result = 0;
    
    /* Multiple calls with volatile usage in between */
    for (int i = 0; i < 8; ++i) {
        /* Complex expression with volatiles */
        v1 = v2 * v3 + v4 - v5;
        v2 = v3 ^ v4 | v5;
        v3 = v4 - v5 * v1;
        
        /* Call that forces spills */
        external_func();
        
        /* More volatile usage */
        v4 = v5 + v1 * v2;
        v5 = v1 ^ v2 & v3;
        
        /* Conditional that creates basic block structure */
        if (v1 > v2) {
            result += v3;
            external_func();  /* Another call in conditional path */
        } else {
            result -= v4;
            /* This creates a basic block ending with call then jump */
            if (v5 != 0) {
                external_func();
                goto skip;  /* Creates jump at end of block */
            }
        }
        
        result += v5;
        skip:
        dummy_volatile = i;
    }
    
    return result + v1 + v2 + v3 + v4 + v5;
}

/* Scenario 3: Nested calls with register pressure */
int __attribute__((noinline)) scenario3(int x) {
    /* Use explicit registers for call setup */
    register int arg1 __asm__("eax") = x;
    register int arg2 __asm__("ecx") = x * 2;
    register int arg3 __asm__("edx") = x * 3;
    int result = 0;
    
    /* Switch statement to create complex control flow */
    switch (x % 4) {
        case 0:
            /* Basic block with call at end */
            result = external_func_with_return();
            arg1 += result;
            external_func();  /* Call near block end */
            break;  /* Creates jump after call */
            
        case 1:
            /* Multiple calls in sequence */
            arg2 = external_func_with_return();
            external_func();
            arg3 = arg1 + arg2;
            external_func();
            result = arg3;
            break;
            
        case 2:
            /* Call in loop with register pressure */
            for (int i = 0; i < 5; ++i) {
                arg1 += i;
                external_func();  /* Call in loop body */
                arg2 -= arg1;
                if (arg2 > 0) {
                    external_func();
                    goto done;  /* Creates block ending with jump */
                }
                arg3 ^= arg2;
            }
            done:
            result = arg1 + arg2 + arg3;
            break;
            
        default:
            /* Dense block with multiple calls */
            arg1 = external_func_with_return();
            external_func();
            arg2 = external_func_with_return();
            external_func();
            result = arg1 * arg2;
            break;
    }
    
    return result;
}

/* Scenario 4: setjmp/longjmp with calls */
jmp_buf env;
int __attribute__((noinline)) scenario4(void) {
    volatile int counter = 0;
    register int a __asm__("eax") = 1;
    register int b __asm__("ecx") = 2;
    
    if (setjmp(env) == 0) {
        /* First execution path */
        for (int i = 0; i < 3; ++i) {
            a += i * 10;
            b -= i * 5;
            
            /* Function call that might need saves */
            external_func();
            
            counter += a + b;
            
            /* Conditional with goto label */
            if (counter > 100) {
                external_func();
                goto early_exit;  /* Creates jump at block end */
            }
        }
        early_exit:
        return counter + a + b;
    } else {
        /* longjmp target */
        a *= 2;
        b *= 3;
        external_func();
        return a - b;
    }
}

/* Helper for scenario 4 */
void trigger_longjmp(void) {
    longjmp(env, 1);
}

/* Scenario 5: Computed goto with calls */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    register int r1 __asm__("eax") = selector;
    register int r2 __asm__("ecx") = selector * 2;
    int result = 0;
    
    /* Complex pre-call computation */
    r1 = r1 * 3 + 7;
    r2 = r2 ^ 0x55;
    
    /* Function call before computed goto */
    external_func();
    
    /* Computed goto creates unusual control flow */
    goto *labels[selector % 4];
    
    label0:
        r1 += 10;
        external_func();  /* Call in goto target block */
        result = r1;
        goto end;
    
    label1:
        r2 -= 5;
        external_func();
        result = r2;
        /* Fall through to create block ending with call */
        external_func();
        goto end;
    
    label2:
        r1 ^= r2;
        external_func();
        result = r1 * r2;
        /* Conditional at block end */
        if (result > 0) {
            external_func();
            goto end;  /* Jump at block end after call */
        }
        result = -result;
        goto end;
    
    label3:
        external_func();
        result = r1 + r2;
        /* Multiple calls in sequence */
        external_func();
        external_func();
        /* End with return computation */
    
    end:
    return result;
}

/* Main function to run all scenarios */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run scenario 1 */
    total += scenario1();
    printf("Scenario 1 complete: %d\n", total);
    
    /* Run scenario 2 */
    total += scenario2();
    printf("Scenario 2 complete: %d\n", total);
    
    /* Run scenario 3 with different inputs */
    for (int i = 0; i < 5; ++i) {
        total += scenario3(i);
    }
    printf("Scenario 3 complete: %d\n", total);
    
    /* Run scenario 4 */
    total += scenario4();
    printf("Scenario 4 complete: %d\n", total);
    
    /* Trigger longjmp for coverage */
    trigger_longjmp();
    
    /* Run scenario 5 */
    for (int i = 0; i < 4; ++i) {
        total += scenario5(i);
    }
    printf("Scenario 5 complete: %d\n", total);
    
    printf("Final total: %d\n", total);
    return total != 0 ? 0 : 1;
}
