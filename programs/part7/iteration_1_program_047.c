/* caller-save-coverage.c
 * Designed to trigger specific instruction list manipulation in GCC's caller-save pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -c caller-save-coverage.c
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

int __attribute__((noinline)) returning_external(void) {
    volatile int r = 42;
    return r;
}

/* Global jmp_buf for setjmp/longjmp scenario */
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    volatile int result = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 20; ++i) {
        /* Arithmetic on register variables */
        a = a + i * 2;
        b = b - i;
        c = c ^ (i + 1);
        d = d * (i % 5 + 1);
        
        /* Function call clobbers registers */
        external_func();
        
        /* More arithmetic after call - values must be restored */
        a = a + 1;
        b = b * 2;
        c = c - i;
        d = d ^ 0xFF;
        
        /* Conditional that might affect BB structure */
        if (i & 1) {
            result += a;
        } else {
            result += b;
        }
        
        /* Another call with different register usage */
        another_external(c);
    }
    
    /* Final computation using all register variables */
    return result + a + b + c + d;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1234;
    volatile int v2 = 5678;
    volatile int v3 = 9012;
    volatile int v4 = 3456;
    volatile int v5 = 7890;
    
    int sum = 0;
    
    /* Multiple basic blocks with calls in between */
    for (int i = 0; i < 15; i++) {
        /* Complex expression with volatiles */
        v1 = v1 * 3 + v2;
        v2 = v2 / 2 + v3;
        
        /* Call that forces spills */
        external_func();
        
        v3 = v3 ^ v4;
        v4 = v4 - v5;
        
        /* Another call */
        int ret = returning_external();
        v5 = v5 + ret;
        
        /* Conditional at end of BB */
        if (v1 > v2) {
            sum += v1;
            /* Call near BB end with following jump */
            another_external(v3);
            goto update;  /* Creates jump at BB end */
        } else {
            sum += v2;
        }
        
        /* Label for goto */
        update:
        v1 = v1 ^ sum;
    }
    
    /* Switch statement creating complex BB structure */
    switch (sum % 4) {
        case 0:
            v1 += external_func(), 1;  /* Comma operator with call */
            break;
        case 1:
            v2 += returning_external();
            /* Fall through */
        case 2:
            external_func();
            v3 *= 2;
            break;
        default:
            /* Call in default case with break at end */
            another_external(v4);
            v4 = v5;
            break;  /* Jump at BB end */
    }
    
    return v1 + v2 + v3 + v4 + v5 + sum;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Values that will be in registers across nested calls */
    int a = x * 2;
    int b = x + 100;
    int c = x ^ 0xABCD;
    
    /* Outer call setup in registers */
    register int r1 __asm__ ("eax") = a;
    register int r2 __asm__ ("ecx") = b;
    
    /* First call - clobbers registers */
    external_func();
    
    /* Use results in second call's arguments */
    r1 = r1 + c;
    r2 = r2 - a;
    
    /* Nested call sequence */
    another_external(r1);
    
    /* More computation */
    r1 = r1 * 3;
    r2 = r2 ^ 0xFF;
    
    /* Another call */
    int ret = returning_external();
    
    /* Complex conditional BB ending with call */
    if (r1 > r2) {
        r1 = r1 + ret;
        external_func();  /* Call near BB end */
        goto finish;
    } else if (r1 < r2) {
        r2 = r2 - ret;
        another_external(r1);
        /* No goto here - fall through */
    } else {
        r1 = r1 ^ r2;
        ret = returning_external();
        external_func();
    }
    
    finish:
    return r1 + r2 + ret;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    volatile int x = 0;
    volatile int y = 0;
    volatile int z = 0;
    
    /* setjmp forces conservative register saving */
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        x = 1;
        y = 2;
        z = 3;
        
        /* Call with live registers */
        external_func();
        
        x = x * 2;
        y = y + 5;
        
        /* Another call */
        another_external(x);
        
        z = z - 1;
        
        /* Simulate longjmp - won't actually jump here in normal execution */
        /* But compiler doesn't know that */
    } else {
        /* longjmp target */
        x = 100;
    }
    
    /* More calls in different paths */
    if (x > 10) {
        y = returning_external();
        external_func();
        goto done;
    } else {
        z = returning_external();
    }
    
    done:
    /* Call at end of BB before return */
    another_external(y + z);
    return x + y + z;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    volatile int a = 10;
    volatile int b = 20;
    volatile int c = 30;
    
    /* Force values into registers */
    register int r1 __asm__ ("eax") = a + selector;
    register int r2 __asm__ ("ecx") = b - selector;
    
    /* Call before computed goto */
    external_func();
    
    /* Computed goto - creates unusual CFG */
    goto *labels[selector % 4];
    
    label0:
        r1 = r1 * 2;
        external_func();  /* Call in middle of block */
        r2 = r2 + 5;
        goto end;
    
    label1:
        r1 = r1 + 10;
        another_external(r1);
        r2 = r2 - 3;
        /* Fall through to label2 */
    
    label2:
        r1 = returning_external();
        external_func();
        r2 = r2 * 2;
        goto end;
    
    label3:
        r1 = r1 ^ 0xFF;
        external_func();
        r2 = returning_external();
        /* Fall through to end */
    
    end:
    /* Final call at BB end */
    another_external(r1 + r2);
    return r1 + r2 + a + b + c;
}

/* ===== Main function to drive everything ===== */
int main(void) {
    int total = 0;
    
    printf("Starting caller-save coverage test...\n");
    
    /* Run all scenarios */
    total += scenario1();
    total += scenario2();
    total += scenario3(42);
    total += scenario4();
    total += scenario5(2);
    
    printf("Result: %d\n", total);
    printf("If you see this, the program compiled and ran successfully.\n");
    printf("The key coverage happens during GCC compilation with -O2 or -O3.\n");
    
    return 0;
}
