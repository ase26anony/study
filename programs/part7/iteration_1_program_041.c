/* caller_save_test.c - Test program for GCC caller-save optimization */
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
    return 42;
}

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    int result = 0;
    
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Arithmetic on register variables before call */
        a = a + i * 2;
        b = b - i;
        c = c ^ (i + 1);
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More arithmetic after call - values must be restored */
        result += a;
        result -= b;
        result ^= c;
        
        /* Conditional that creates basic block boundaries */
        if (i & 1) {
            a += 5;
            external_func();  /* Another call in conditional path */
            b -= 3;
        } else {
            c += 7;
        }
        
        /* Mix with global to prevent dead code elimination */
        global_counter++;
    }
    
    /* Final computation using all register variables */
    result = (a + b) * c;
    return result;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    
    int sum = 0;
    
    /* Multiple expressions with volatiles across calls */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        external_func();  /* Call between volatile accesses */
        v3 = v3 ^ v2;
        v4 = v4 - v3 * i;
        another_external(v4);
        v5 = v5 + v4 % 7;
        
        sum += v1 + v2 + v3 + v4 + v5;
        
        /* Conditional with call at end of basic block */
        if (sum > 100) {
            external_func();
            goto early_exit;  /* Creates jump at BB end */
        }
    }
    
early_exit:
    return sum;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Inner function call arguments depend on outer call setup */
    int a = x * 2;
    int b = x + 5;
    int c = x - 3;
    
    /* First call - setup in registers */
    another_external(a);
    
    /* Nested computation between calls */
    int d = a + b;
    external_func();  /* Middle call */
    
    /* Result depends on values that might be in call-clobbered regs */
    int e = d * c;
    another_external(e);
    
    /* Switch statement with calls in cases */
    int result = 0;
    switch (x % 4) {
        case 0:
            result = a + b;
            external_func();
            break;  /* Jump at end of BB */
        case 1:
            result = b - c;
            another_external(result);
            break;
        case 2:
            result = c * d;
            external_func();
            /* Fall through to create more complex CFG */
        case 3:
            result = e / (x + 1);
            another_external(result);
            break;
        default:
            result = returning_external();
            external_func();
            break;
    }
    
    return result;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    int saved = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        register int r1 __asm__ ("eax") = 100;
        register int r2 __asm__ ("ecx") = 200;
        
        /* Computation in registers */
        r1 = r1 * 2 + global_counter;
        r2 = r2 / 2 - global_counter;
        
        /* Call that might longjmp */
        external_func();
        
        /* These must be preserved across potential longjmp */
        saved = r1 + r2;
        
        /* Another call in same basic block */
        another_external(saved);
        
        /* Conditional at end */
        if (saved > 150) {
            external_func();
            /* BB ends with call then implicit fallthrough */
        }
    } else {
        /* After longjmp */
        saved = -1;
    }
    
    return saved;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int mode) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int result = 0;
    register int tmp __asm__ ("eax") = mode * 10;
    
    /* Jump to label based on mode */
    goto *labels[mode % 4];
    
label0:
    tmp += 5;
    external_func();  /* Call in middle of block */
    result = tmp * 2;
    goto end;
    
label1:
    tmp -= 3;
    another_external(tmp);
    result = tmp / 2;
    /* Fall through to label2 */
    
label2:
    tmp ^= 0xFF;
    external_func();
    result += tmp;
    if (result > 50) {
        external_func();  /* Call before block end */
        goto end;  /* Conditional jump at BB end */
    }
    result *= 2;
    goto end;
    
label3:
    tmp = returning_external();
    external_func();
    result = tmp + 100;
    /* Implicit fallthrough to end */
    
end:
    return result;
}

/* ===== Main function to drive everything ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios */
    total += scenario1();
    total += scenario2();
    total += scenario3(7);
    total += scenario4();
    total += scenario5(2);
    
    /* Add some more calls with different patterns */
    for (int i = 0; i < 5; ++i) {
        /* Mix of calls and computation */
        register int a __asm__ ("eax") = i * 10;
        register int b __asm__ ("ecx") = i * 20;
        
        a = a + b;
        external_func();
        b = b - a;
        another_external(b);
        
        total += a + b;
        
        /* Complex tail of basic block */
        if (i & 1) {
            external_func();
            total += 5;  /* Instruction after call before BB end */
        } else {
            total += returning_external();
            /* BB ends with call result */
        }
    }
    
    printf("Final result: %d\n", total);
    return total != 0 ? 0 : 1;
}
