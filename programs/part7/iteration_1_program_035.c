/* test_caller_save.c - Program to trigger GCC's caller-save pass list manipulation */
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

int __attribute__((noinline)) returns_value(void) {
    return 42;
}

/* Global jump buffer for setjmp/longjmp scenario */
jmp_buf jump_buffer;

/* ========== SCENARIO 1: Explicit register variables with loop ========== */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a += i;
        b -= i;
        c *= (i + 1);
        d ^= i;
        
        /* Function call clobbers registers - forces save/restore */
        external_func();
        
        /* Use values after call - they must be restored */
        sum += a + b + c + d;
        
        /* Conditional that creates basic block boundaries */
        if (i % 3 == 0) {
            a += returns_value();  /* Another call */
        } else if (i % 3 == 1) {
            b -= returns_value();
        } else {
            external_func();
        }
    }
    
    /* Final computation to ensure values are used */
    return sum + a + b + c + d;
}

/* ========== SCENARIO 2: Volatile variables forcing memory spills ========== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    
    int result = 0;
    
    /* Multiple calls with volatile usage in between */
    for (int i = 0; i < 8; i++) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        
        external_func();  /* Call 1 */
        
        v3 = v3 ^ v2;
        v4 = v4 | v3;
        
        another_external(v4);  /* Call 2 with argument */
        
        v5 = v5 & v4;
        result += v1 + v2 + v3 + v4 + v5;
        
        /* Conditional at end of basic block */
        if (result > 1000) {
            external_func();
            goto early_exit;
        }
    }
    
early_exit:
    return result;
}

/* ========== SCENARIO 3: Nested calls with register dependencies ========== */
int __attribute__((noinline)) scenario3(int x) {
    /* Arguments in registers get clobbered by inner calls */
    int a = x * 2;
    int b = x + 7;
    int c = x - 3;
    
    /* Outer call setup in registers */
    int r1 = returns_value() + a;  /* First call */
    
    /* Inner call with dependency on outer call result */
    another_external(r1 + b);      /* Second call - clobbers registers */
    
    /* More computation requiring original values */
    int r2 = returns_value() + c;  /* Third call */
    
    /* Complex control flow with calls at block ends */
    switch (x % 4) {
        case 0:
            external_func();
            break;  /* Creates jump at BB end */
        case 1:
            another_external(a);
            break;
        case 2:
            external_func();
            /* Fall through to create different BB structure */
        default:
            another_external(b);
            external_func();  /* Call right before block end */
            break;
    }
    
    return r1 + r2 + a + b + c;
}

/* ========== SCENARIO 4: setjmp/longjmp with calls ========== */
int __attribute__((noinline)) scenario4(void) {
    register int r1 __asm__ ("eax") = 100;
    register int r2 __asm__ ("ecx") = 200;
    int normal_path = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* Normal path - values in registers must be preserved */
        for (int i = 0; i < 5; i++) {
            r1 += i * 2;
            r2 -= i;
            
            /* Call that might trigger save/restore */
            external_func();
            
            normal_path += r1 * r2;
            
            /* Conditional with call at end */
            if (i == 3) {
                another_external(r1);
                /* Potential BB_END manipulation here */
            }
        }
    } else {
        /* longjmp target - different register usage */
        r1 = returns_value();
        r2 = returns_value() * 2;
    }
    
    /* Use computed goto to create unusual control flow */
    void* labels[] = { &&label1, &&label2, &&label3 };
    
    goto *labels[normal_path % 3];
    
label1:
    external_func();
    return r1;
    
label2:
    another_external(r2);
    external_func();  /* Call before block end */
    return r2;
    
label3:
    return r1 + r2;
}

/* ========== SCENARIO 5: Computed goto with calls ========== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* jump_table[] = { &&case0, &&case1, &&case2, &&case3 };
    
    register int a __asm__ ("eax") = selector * 10;
    register int b __asm__ ("ecx") = selector * 20;
    int result = 0;
    
    /* Force values to be live across the indirect jump */
    a += 5;
    b -= 3;
    
    if (selector >= 0 && selector < 4) {
        goto *jump_table[selector];
    }
    
case0:
    external_func();
    result = a + b;
    goto end;
    
case1:
    /* Call in middle of block with computation after */
    another_external(a);
    result = b - a;
    external_func();  /* Another call before block end */
    goto end;
    
case2:
    result = returns_value();
    /* Multiple calls in sequence */
    external_func();
    another_external(result);
    /* No explicit goto - falls through to case3 */
    
case3:
    external_func();
    result = a * b;
    /* Block ends with return */

end:
    return result;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios to ensure code generation */
    total += scenario1();
    total += scenario2();
    total += scenario3(10);
    total += scenario4();
    total += scenario5(2);
    
    printf("Total checksum: %d\n", total);
    printf("(If this compiles and runs, the caller-save pass was engaged)\n");
    
    return 0;
}
