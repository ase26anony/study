/* caller-save-test.c
 * Test program to trigger specific instruction list manipulation
 * in GCC's caller-save optimization pass.
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inlineable */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int __attribute__((noinline)) returning_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
    return x + 1;
}

/* Scenario 1: Explicit register variables with loop */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__("eax") = 1;
    register int b __asm__("ecx") = 2;
    register int c __asm__("edx") = 3;
    register int d __asm__("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a = a + i;
        b = b - i;
        c = c * (i + 1);
        d = d ^ i;
        
        /* Function call clobbers registers */
        external_func();
        
        /* Use values after call - must be restored */
        sum += a + b + c + d;
        
        /* Conditional that creates basic block boundaries */
        if (i % 3 == 0) {
            a += returning_external(b);
        } else if (i % 3 == 1) {
            b -= returning_external(c);
        } else {
            c ^= returning_external(d);
        }
    }
    
    return sum + a + b + c + d;
}

/* Scenario 2: Volatile variables forcing memory spills */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1234;
    volatile int v2 = 5678;
    volatile int v3 = 9012;
    volatile int v4 = 3456;
    
    int result = 0;
    
    /* Multiple calls with volatile usage in between */
    for (int j = 0; j < 8; ++j) {
        /* Complex expression with volatiles */
        v1 = v1 * 3 + j;
        v2 = v2 / 2 - j;
        
        /* Call that forces spills */
        another_external(v1);
        
        /* More volatile operations */
        v3 = v3 ^ v1;
        v4 = v4 | v2;
        
        /* Another call */
        external_func();
        
        /* Final computation */
        result += v1 + v2 + v3 + v4;
        
        /* Conditional jump at end of basic block */
        if (v1 > 10000) {
            result += returning_external(v2);
            goto done_label;
        }
    }
    
done_label:
    return result;
}

/* Scenario 3: Nested calls with register dependencies */
int __attribute__((noinline)) scenario3(int x) {
    /* Values that will be in registers */
    int a = x * 2;
    int b = x + 100;
    int c = x - 50;
    int d = x ^ 0xFF;
    
    /* Outer call setup uses registers */
    int r1 = returning_external(a);
    
    /* Inner call depends on outer result AND other values */
    int r2 = returning_external(r1 + b);
    
    /* Another call with complex argument expression */
    int r3 = returning_external(r2 * c - d);
    
    /* Switch statement to create interesting BB structure */
    switch (x % 4) {
        case 0:
            external_func();
            a += r1;
            break;  /* Creates jump at BB end */
        case 1:
            b += r2;
            external_func();
            break;
        case 2:
            external_func();
            c += r3;
            /* Fall through */
        default:
            d += returning_external(a + b + c);
            external_func();
            break;
    }
    
    return a + b + c + d + r1 + r2 + r3;
}

/* Scenario 4: setjmp/longjmp with calls */
jmp_buf env;
int __attribute__((noinline)) scenario4(int x) {
    int a = x;
    int b = x * 2;
    int c = x * 3;
    
    if (setjmp(env) == 0) {
        /* First path: use values, make call */
        a += 100;
        external_func();
        b += returning_external(a);
        
        /* Complex condition with call at end */
        if (a > b) {
            c = returning_external(c);
            goto compute;
        } else {
            another_external(b);
            /* Basic block ends with call then goto */
        }
    } else {
        /* Second path: different register usage */
        a -= 50;
        external_func();
        b ^= 0xAAAA;
    }
    
compute:
    return a + b + c;
}

/* Scenario 5: Computed goto with calls */
int __attribute__((noinline)) scenario5(int x) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int a = x;
    int b = x + 1;
    int c = x + 2;
    
    /* Use computed goto */
    goto *labels[x % 4];
    
label0:
    a += 10;
    external_func();  /* Call in middle of block */
    b += 20;
    goto end;
    
label1:
    b += 30;
    another_external(a);
    c += 40;
    /* Fall through to label2 */
    
label2:
    external_func();
    a = returning_external(b);
    goto end;
    
label3:
    c += 50;
    external_func();
    a += returning_external(c);
    /* No explicit goto - falls through to end */
    
end:
    return a + b + c;
}

/* Main function to drive everything */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    total += scenario1();
    printf("After scenario1: %d\n", total);
    
    total += scenario2();
    printf("After scenario2: %d\n", total);
    
    total += scenario3(42);
    printf("After scenario3: %d\n", total);
    
    total += scenario4(100);
    printf("After scenario4: %d\n", total);
    
    total += scenario5(7);
    printf("After scenario5: %d\n", total);
    
    printf("Final total: %d\n", total);
    
    return 0;
}
