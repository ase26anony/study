/* caller_save_coverage.c
 * Designed to trigger GCC's caller-save pass instruction list manipulation
 * Specifically targets lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call instruction */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    int sum = 0;
    
    /* Complex loop with register pressure across call */
    for (int i = 0; i < 10; ++i) {
        /* Use registers before call - must be preserved */
        a = a + i * 2;
        b = b - i;
        c = c ^ (i + 1);
        
        /* Function call clobbers registers */
        external_func();
        
        /* Use registers after call - requires reload */
        sum += a + b + c;
        
        /* More arithmetic to create dense block */
        if (i & 1) {
            a += global_counter;
        } else {
            b -= global_counter;
        }
        
        /* Another call with different register usage */
        another_external(c);
        
        /* Conditional at end of basic block */
        if (sum > 100) {
            sum = sum / 2;
        }
    }
    
    /* Force use of all registers at end */
    return sum + a + b + c;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    int result = 0;
    
    /* Multiple volatile operations across calls */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        
        /* Call in middle of volatile sequence */
        external_func();
        
        v3 = v3 ^ v2;
        v4 = v4 | v3;
        
        /* Another call */
        another_external(v4);
        
        result += v1 + v2 + v3 + v4;
        
        /* Switch statement to create complex CFG */
        switch (i % 3) {
            case 0:
                v1 += 5;
                break;  /* Creates jump at BB end */
            case 1:
                v2 -= 3;
                external_func();  /* Call before break */
                break;
            default:
                v3 *= 2;
                another_external(v3);
                break;  /* Call then jump at BB end */
        }
    }
    
    return result;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Nested calls where inner call args depend on outer call setup */
    int a = x * 2;
    int b = x + 5;
    
    /* First call setup in registers */
    int r1 = a + b;
    external_func();  /* Clobbers registers */
    
    /* Result used immediately for next call */
    int r2 = r1 * 3;
    another_external(r2);
    
    /* Complex conditional with calls at different points */
    if (r2 > 50) {
        register int t1 __asm__ ("eax") = r2;
        register int t2 __asm__ ("ecx") = a;
        
        t1 = t1 >> 2;
        external_func();
        t2 = t2 << 1;
        
        /* Call at end of conditional block */
        another_external(t1 + t2);
        
        return t1 + t2;
    } else {
        /* Different path with its own calls */
        external_func();
        another_external(b);
        return b;
    }
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    int saved = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        register int j1 __asm__ ("eax") = 42;
        register int j2 __asm__ ("ecx") = 73;
        
        /* Use registers, then call */
        j1 = j1 * 2;
        external_func();  /* May be clobbered */
        j2 = j2 + j1;
        
        saved = j1 + j2;
        
        /* Call before potential longjmp */
        another_external(saved);
        
        /* Simulate error - triggers longjmp */
        if (global_counter++ > 5) {
            longjmp(jump_buffer, 1);
        }
    } else {
        /* After longjmp - registers may need restoring */
        external_func();
        saved = 100;
    }
    
    return saved;
}

/* ===== SCENARIO 5: Computed goto with function call ===== */
int __attribute__((noinline)) scenario5(int x) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = 0;
    
    /* Force switch to be compiled as computed goto */
    if (x >= 0 && x < 4) {
        goto *labels[x];
    }
    
label0:
    {
        register int r1 __asm__ ("eax") = 10;
        external_func();
        result = r1;
        goto end;
    }
    
label1:
    {
        register int r1 __asm__ ("eax") = 20;
        register int r2 __asm__ ("ecx") = 30;
        r1 = r1 + x;
        external_func();  /* Call in middle of block */
        r2 = r2 - x;
        another_external(r2);
        result = r1 + r2;
        goto end;
    }
    
label2:
    external_func();
    result = 40;
    goto end;
    
label3:
    result = 50;
    another_external(result);
    /* Fall through to end */
    
end:
    return result;
}

/* ===== Main driver ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save coverage scenarios...\n");
    
    /* Run all scenarios to ensure compilation */
    total += scenario1();
    total += scenario2();
    total += scenario3(10);
    total += scenario4();
    total += scenario5(1);
    
    /* Mix of calls to create varied patterns */
    for (int i = 0; i < 3; i++) {
        total += scenario1() % 100;
        total += scenario2() % 100;
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
