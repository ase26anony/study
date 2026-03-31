/* caller-save-test.c
 * Test program to trigger specific instruction list manipulation
 * in GCC's caller-save optimization pass.
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but not inlined */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int __attribute__((noinline)) returning_external(int x) {
    asm volatile("" : "+r"(x) : : "memory");
    return x + 1;
}

/* Scenario 1: Explicit register variables with loop */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__("eax") = 1;
    register int b __asm__("ecx") = 2;
    register int c __asm__("edx") = 3;
    register int d __asm__("esi") = 4;
    
    volatile int result = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 20; ++i) {
        /* Use all register variables before call */
        a = a + i * 2;
        b = b - i;
        c = c ^ (a + b);
        d = d * 2 + i;
        
        /* Function call clobbers registers */
        external_func();
        
        /* Use values after call - must be restored */
        a = a + 1;
        b = b - c;
        c = c ^ d;
        d = d + a;
        
        /* Another call with different register pressure */
        another_external(a + b + c + d);
        
        /* More arithmetic creating complex basic block */
        if (i % 3 == 0) {
            a = a * 2;
            external_func();
            b = b / 2;
        } else if (i % 3 == 1) {
            c = c + d;
            another_external(c);
            d = d - a;
        } else {
            external_func();
            a = a + b + c + d;
        }
        
        result += a + b + c + d;
    }
    
    return result;
}

/* Scenario 2: Many volatile variables forcing memory spills */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int sum = 0;
    
    /* Complex expression with function call in middle */
    for (int i = 0; i < 15; i++) {
        v1 = v1 + v2 * i;
        v2 = v2 - v3 / (i + 1);
        v3 = v3 ^ v4;
        v4 = v4 | v5;
        
        /* Call with many live volatiles */
        external_func();
        
        v5 = v5 + v6;
        v6 = v6 - v7;
        v7 = v7 * v8;
        v8 = v8 / (v9 + 1);
        
        /* Another call */
        another_external(v1 + v2);
        
        v9 = v9 ^ v10;
        v10 = v10 + v1;
        
        /* Conditional with call at end of basic block */
        if (v1 > v2) {
            v3 = returning_external(v3);
            /* Call at end of block before goto-like structure */
            external_func();
            goto update_sum;  /* Creates jump at BB end */
        } else if (v4 < v5) {
            v6 = returning_external(v6);
            another_external(v7);
            /* break-like control flow */
            if (v8 > 10) break;
        }
        
        v1 = v1 + 1;
        
    update_sum:
        sum += v1 + v2 + v3 + v4 + v5;
    }
    
    return sum;
}

/* Scenario 3: Nested calls with register dependencies */
int __attribute__((noinline)) scenario3(int x) {
    register int r1 __asm__("eax") = x;
    register int r2 __asm__("ecx") = x * 2;
    register int r3 __asm__("edx") = x * 3;
    
    /* Outer call setup in call-clobbered registers */
    r1 = r1 + 1;
    r2 = r2 - x;
    r3 = r3 * 2;
    
    /* Nested calls where inner call uses outer call's registers */
    int result = returning_external(r1);
    another_external(r2);
    
    /* More computation between calls */
    r1 = r1 + result;
    r2 = r2 - result;
    
    external_func();
    
    /* Switch statement with calls in cases */
    switch (x % 4) {
        case 0:
            r3 = returning_external(r1);
            external_func();  /* Call before break at BB end */
            break;
        case 1:
            another_external(r2);
            r1 = r1 * 2;
            external_func();
            /* Fall through */
        case 2:
            r3 = returning_external(r3);
            external_func();
            if (r1 > 10) {
                goto done;
            }
            break;
        default:
            external_func();
            r2 = returning_external(r1 + r2);
            /* Call at end before break */
            another_external(r3);
            break;
    }
    
    r1 = r1 + r2 + r3;
    
done:
    return r1 + r2 + r3;
}

/* Scenario 4: setjmp/longjmp with calls */
jmp_buf env;
int __attribute__((noinline)) scenario4(void) {
    volatile int a = 1, b = 2, c = 3;
    int sum = 0;
    
    if (setjmp(env) == 0) {
        /* First pass */
        for (int i = 0; i < 10; i++) {
            a = a + i;
            b = b * (i + 1);
            c = c - i;
            
            /* Call with live variables that might be longjmp'd */
            external_func();
            
            /* Complex condition */
            if (a > 100) {
                another_external(b);
                /* This creates interesting BB structure */
                goto update;
            }
            
            a = a + b + c;
            
        update:
            sum += a + b + c;
            
            /* Call near end of loop body */
            if (i % 2 == 0) {
                returning_external(a);
            }
        }
    } else {
        /* longjmp target */
        another_external(sum);
    }
    
    return sum;
}

/* Scenario 5: Computed goto with calls */
int __attribute__((noinline)) scenario5(int x) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    register int r1 __asm__("eax") = x;
    register int r2 __asm__("ecx") = x + 1;
    int result = 0;
    
    void *target = labels[x % 4];
    goto *target;
    
label0:
    r1 = r1 * 2;
    external_func();  /* Call in middle of block with computed goto later */
    r2 = r2 + r1;
    goto *labels[(x + 1) % 4];  /* Another computed goto */
    
label1:
    another_external(r1);
    r2 = returning_external(r2);
    external_func();
    if (r1 > r2) {
        goto end;
    }
    /* Fall through */
    
label2:
    r1 = r1 + r2;
    external_func();
    result = returning_external(r1);
    /* Call at end before jump to end */
    another_external(result);
    goto end;
    
label3:
    external_func();
    r2 = r2 * 3;
    another_external(r2);
    result = r1 + r2;
    /* end label creates BB end */
    
end:
    return result + r1 + r2;
}

/* Main function to drive all scenarios */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    total += scenario1();
    printf("Scenario 1 complete\n");
    
    total += scenario2();
    printf("Scenario 2 complete\n");
    
    total += scenario3(42);
    printf("Scenario 3 complete\n");
    
    total += scenario4();
    printf("Scenario 4 complete\n");
    
    total += scenario5(7);
    printf("Scenario 5 complete\n");
    
    printf("Total checksum: %d\n", total);
    
    /* Trigger longjmp for scenario 4 */
    longjmp(env, 1);
    
    return 0;
}
