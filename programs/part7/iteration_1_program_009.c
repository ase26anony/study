/* caller_save_coverage.c
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
    volatile int sink = 0;
    (void)sink;
}

int __attribute__((noinline)) external_func_with_return(void) {
    return 42;
}

/* Dummy volatile function to prevent optimization */
volatile int global_counter = 0;

/* Scenario 1: Explicit register variables with loop */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__("eax") = 1;
    register int b __asm__("ecx") = 2;
    register int c __asm__("edx") = 3;
    register int d __asm__("esi") = 4;
    
    int result = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 20; ++i) {
        /* Create register pressure before call */
        a = a + i * 3;
        b = b - i * 2;
        c = c ^ i;
        d = d | (i << 2);
        
        /* Mix arithmetic to create dense instruction sequence */
        a = (a * 2) + (b / 3);
        b = (b << 1) | (c & 0xFF);
        c = c + d - i;
        d = d ^ a ^ b;
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More operations after call requiring reloads */
        result += a + b - c + d;
        
        /* Conditional that might create branch at BB end */
        if (i & 1) {
            a = a * 3;
            b = b + 5;
        } else {
            c = c - 2;
            d = d ^ 0xAAAA;
        }
        
        /* Another call with different register usage */
        if (i == 10) {
            external_func();
            /* This creates a basic block ending with call+jump */
            goto loop_end;
        }
    }
loop_end:
    
    return result + a + b + c + d;
}

/* Scenario 2: Volatile variables forcing memory spills */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int result = 0;
    
    /* Complex expression with volatiles across call */
    for (int i = 0; i < 15; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        v3 = v3 ^ v2;
        v4 = v4 | v3;
        v5 = v5 & 0xFFFF;
        
        /* Call in middle of complex expression */
        external_func();
        
        /* Continue using volatiles */
        result += v1 + v2 * v3 - v4 / (v5 + 1);
        
        /* Switch statement to create complex BB structure */
        switch (i % 4) {
            case 0:
                v1 += external_func_with_return();
                break;  /* Creates jump at BB end */
            case 1:
                v2 -= external_func_with_return();
                break;
            case 2:
                v3 *= external_func_with_return();
                /* Fall through to create different BB structure */
            case 3:
                v4 ^= external_func_with_return();
                external_func();
                break;
        }
    }
    
    return result;
}

/* Scenario 3: Nested calls with register dependencies */
int __attribute__((noinline)) scenario3(int x) {
    int a = x * 2;
    int b = x + 5;
    int c = x ^ 0x1234;
    
    /* Outer call setup in registers */
    a = a * 3 - 1;
    b = b + a / 2;
    
    /* Nested call pattern */
    int temp = external_func_with_return();  /* First call */
    
    /* Use result in register for next call setup */
    a = a + temp;
    b = b - temp;
    
    /* Another call with arguments depending on clobbered regs */
    external_func();  /* Clobbers registers */
    
    /* More computation requiring reloads */
    c = c + a * b;
    
    /* Complex tail with conditional goto */
    if (c > 1000) {
        external_func();
        goto done;  /* Creates BB ending with call then jump */
    } else if (c < 0) {
        a = external_func_with_return();
        b = a * 2;
    } else {
        external_func();
        a = b + c;
    }
    
done:
    return a + b + c;
}

/* Scenario 4: setjmp/longjmp with calls */
jmp_buf env;
int __attribute__((noinline)) scenario4(void) {
    volatile int x = 0, y = 0, z = 0;
    
    if (setjmp(env) == 0) {
        /* First time through */
        x = 1;
        y = 2;
        z = 3;
        
        /* Function call that might be saved across longjmp */
        external_func();
        
        x = x * 2;
        y = y + external_func_with_return();  /* Call in expression */
        z = z - 1;
        
        /* Simulate longjmp - compiler must be conservative */
        if (global_counter++ > 5) {
            longjmp(env, 1);
        }
    } else {
        /* After longjmp */
        x = 10;
        external_func();
        y = 20;
    }
    
    /* More calls in different paths */
    if (x > y) {
        external_func();
        z = z * 3;
    } else {
        z = external_func_with_return();
    }
    
    return x + y + z;
}

/* Scenario 5: Computed goto with function calls */
int __attribute__((noinline)) scenario5(void) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int i = 0;
    int result = 0;
    
    register int r1 __asm__("eax") = 1;
    register int r2 __asm__("ecx") = 2;
    
label0:
    r1 = r1 * 3 + i;
    external_func();  /* Call in basic block with computed goto */
    i++;
    goto *labels[i % 4];
    
label1:
    r2 = r2 - r1;
    external_func();
    result += r1;
    i++;
    if (i < 10) goto *labels[(i + 1) % 4];
    else goto end;
    
label2:
    r1 = r1 ^ 0xABCD;
    external_func();
    result += r2;
    i++;
    goto *labels[(i * 2) % 4];
    
label3:
    r2 = r2 | r1;
    external_func();
    result += r1 + r2;
    i++;
    if (i < 8) goto *labels[i % 4];
    
end:
    return result;
}

/* Main function to drive all scenarios */
int main(void) {
    int sum = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios */
    sum += scenario1();
    printf("Scenario 1 complete\n");
    
    sum += scenario2();
    printf("Scenario 2 complete\n");
    
    sum += scenario3(42);
    printf("Scenario 3 complete\n");
    
    sum += scenario4();
    printf("Scenario 4 complete\n");
    
    sum += scenario5();
    printf("Scenario 5 complete\n");
    
    printf("Final checksum: %d\n", sum);
    
    return 0;
}
