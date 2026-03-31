/* caller-save-test.c
 * Test program to trigger specific instruction list manipulation
 * in GCC's caller-save optimization pass.
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call instruction */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int __attribute__((noinline)) returning_external(void) {
    return 42;
}

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of specific call-clobbered registers */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a = a + i * 2;
        b = b - i;
        c = c ^ (i + 1);
        d = d * (i % 3 + 1);
        
        /* Function call clobbers registers */
        external_func();
        
        /* Use values after call - must be restored */
        sum += a + b + c + d;
        
        /* More arithmetic to create dense block */
        if (i & 1) {
            a += b;
            b -= c;
        } else {
            c ^= d;
            d += a;
        }
        
        /* Another call with different register pressure */
        another_external(sum);
    }
    
    /* Final computation using all registers */
    return a + b * 2 + c * 3 + d * 4 + sum;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int result = 0;
    
    /* Multiple expressions with volatiles across calls */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        v3 = v3 ^ v2;
        
        /* Call with live volatile values */
        external_func();
        
        v4 = v4 + v3 - v2;
        v5 = v5 * (v4 % 7 + 1);
        
        /* Another call */
        another_external(v5);
        
        /* Complex conditional at block end */
        if (v1 > v2) {
            result += v1 - v2;
            /* Call near block end with goto-like structure */
            if (v3 < v4) {
                external_func();
                goto compute;  /* Creates jump at block end */
            }
        }
        
        v1 += returning_external();
    }
    
compute:
    return v1 + v2 + v3 + v4 + v5 + result;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Values computed in registers used as call arguments */
    int a = x * 2;
    int b = x + 7;
    int c = x ^ 0x55;
    
    /* Outer call setup uses registers */
    int r1 = returning_external() + a;
    
    /* Nested call pattern */
    for (int i = 0; i < 5; ++i) {
        /* Compute values in registers */
        int t1 = a * i + b;
        int t2 = b * i - c;
        int t3 = c ^ i + a;
        
        /* Call with register arguments */
        another_external(t1);
        
        /* Inner call that clobbers registers used above */
        external_func();
        
        /* Use values after nested calls */
        a = t1 + t2;
        b = t2 - t3;
        c = t3 ^ t1;
        
        /* Switch statement to create complex block ends */
        switch (i % 3) {
            case 0:
                a += returning_external();
                break;
            case 1:
                external_func();
                /* Call followed by break at block end */
                break;
            default:
                another_external(c);
                /* Multiple instructions after call before block end */
                a = a * 2;
                b = b / 2;
                break;
        }
    }
    
    return a + b + c + r1;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First pass: compute with calls */
        for (int i = 0; i < 4; ++i) {
            a = a * (i + 1) + b;
            b = b - i + c;
            
            /* Call that might be affected by longjmp */
            external_func();
            
            c = c ^ d + a;
            d = d + b * c;
            
            /* Another call */
            another_external(d);
            
            result += a + b + c + d;
            
            /* Conditional with call at end */
            if (i == 2) {
                external_func();
                /* Simulate complex block end */
                goto update;
            }
        }
    } else {
        /* longjmp return path */
        a += 100;
        b += 200;
    }
    
update:
    return a + b + c + d + result;
}

/* ===== SCENARIO 5: Computed goto with function calls ===== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int a = 5, b = 6, c = 7, d = 8;
    int* ptrs[] = { &a, &b, &c, &d };
    
    /* Force values into registers */
    register int r1 __asm__ ("eax") = a * 2;
    register int r2 __asm__ ("ecx") = b + 3;
    
    /* Computed goto */
    goto *labels[selector % 4];
    
label0:
    r1 += 10;
    external_func();  /* Call in middle of block */
    r2 -= 5;
    goto end;
    
label1:
    r1 *= 2;
    another_external(r1);
    r2 = r2 ^ r1;
    /* Call near block end with following jump */
    external_func();
    goto end;
    
label2:
    r1 = r1 + r2;
    external_func();
    r2 = r1 * 2;
    /* Multiple calls in sequence */
    another_external(r2);
    external_func();
    goto end;
    
label3:
    r1 = returning_external();
    r2 = r1 + 10;
    external_func();
    /* Fall through to end */
    
end:
    return r1 + r2 + a + b + c + d;
}

/* ===== Main function to execute all scenarios ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Execute all scenarios */
    total += scenario1();
    total += scenario2();
    total += scenario3(10);
    total += scenario4();
    total += scenario5(2);
    
    /* Also test longjmp path */
    longjmp(jump_buffer, 1);
    
    printf("Total checksum: %d\n", total);
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
