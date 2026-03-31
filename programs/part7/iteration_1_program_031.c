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
    int result = 0;
    
    /* Force use of specific call-clobbered registers */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Multiple operations on register variables */
        a = a + i * 2;
        b = b - i / 2;
        c = c ^ (i * 3);
        
        /* Function call clobbers registers */
        external_func();
        
        /* More operations after call - values must be restored */
        result += a * b + c;
        
        /* Conditional to create basic block structure */
        if (i & 1) {
            a += returning_external();  /* Another call */
        } else {
            b -= global_counter;
        }
        
        /* Mix in another external call */
        another_external(c);
    }
    
    return result + a + b + c;
}

/* ===== SCENARIO 2: Volatile variables creating memory pressure ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int sum = 0;
    
    /* Complex expression with volatile across call */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        v3 = v3 ^ v2;
        
        /* Call in middle of expression */
        external_func();
        
        v4 = v4 - v3;
        v5 = v5 | v4;
        
        sum += v1 + v2 + v3 + v4 + v5;
        
        /* Create basic block ending with call then jump */
        if (v1 > 100) {
            another_external(v2);
            goto early_exit;  /* Creates jump at BB end */
        }
        
        v1 = v1 % 17;
    }
    
early_exit:
    return sum + v1 + v2;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    int r1, r2, r3;
    
    /* First call's result used in second call's setup */
    r1 = returning_external() + x;
    
    /* Complex expression with intermediate call */
    r2 = r1 * 2;
    another_external(r2);  /* Call clobbers registers */
    
    /* Result used immediately after */
    r3 = r2 + returning_external();  /* Another call */
    
    /* Switch statement with calls in cases */
    switch (x % 4) {
        case 0:
            external_func();
            r1 += 10;
            break;  /* Jump at BB end */
        case 1:
            r2 = returning_external();
            another_external(r2);
            r1 += 20;
            break;
        case 2:
            r3 = returning_external() * 2;
            external_func();
            r1 += 30;
            break;
        default:
            another_external(x);
            external_func();  /* Two calls in sequence */
            r1 += 40;
            break;
    }
    
    return r1 + r2 + r3;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    int a = 1, b = 2, c = 3;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First pass - do computation with calls */
        for (int i = 0; i < 5; ++i) {
            a = a * 2 + i;
            b = b - i * 3;
            
            /* Call that might longjmp */
            if (i == 3) {
                another_external(a);
                /* Potential longjmp point */
            }
            
            c = c ^ b;
            result += a + b + c;
            
            external_func();
        }
    } else {
        /* After longjmp - different computation */
        a = returning_external();
        b = a * 2;
        another_external(b);
        result = a + b;
    }
    
    return result;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int x) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = 0;
    int i = 0;
    
    /* Force values into registers */
    register int r1 __asm__ ("eax") = x;
    register int r2 __asm__ ("ecx") = x * 2;
    
start:
    if (i >= 4) goto end;
    
    /* Computed goto */
    goto *labels[i];
    
label0:
    r1 += 10;
    external_func();  /* Call in basic block with computed goto */
    r2 -= 5;
    i++;
    goto start;
    
label1:
    r1 *= 2;
    another_external(r1);
    r2 += returning_external();  /* Another call */
    i++;
    goto start;
    
label2:
    r1 = r1 ^ r2;
    external_func();
    /* Fall through to next case */
    
label3:
    r2 = r2 / 2;
    another_external(r2);
    result = r1 + r2;
    i++;
    goto start;
    
end:
    return result;
}

/* ===== Main function to drive all scenarios ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios to ensure code generation */
    total += scenario1();
    total += scenario2();
    total += scenario3(7);
    total += scenario4();
    total += scenario5(3);
    
    /* Add some more calls to increase pressure */
    for (int i = 0; i < 100; ++i) {
        external_func();
        global_counter++;
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
