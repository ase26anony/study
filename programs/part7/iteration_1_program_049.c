/* test_caller_save.c - Program to trigger GCC caller-save pass instruction list manipulation */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void external_func(void) __attribute__((noinline));
void another_external(int x) __attribute__((noinline));
int returns_value(void) __attribute__((noinline));

/* Dummy implementations to satisfy linker */
void external_func(void) {
    volatile int sink = 0;
    (void)sink;
}

void another_external(int x) {
    volatile int sink = x;
    (void)sink;
}

int returns_value(void) {
    return 42;
}

/* Global jump buffer for setjmp/longjmp scenario */
static jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
__attribute__((noinline, optimize("O2")))
int scenario1_register_pressure(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    volatile int result = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a = a + i * 2;
        b = b - i;
        c = c ^ (i + 1);
        d = d | (i << 2);
        
        /* Function call clobbers registers - forces save/restore */
        external_func();
        
        /* More operations after call - values must be restored */
        a = a + b;
        b = b - c;
        c = c ^ d;
        d = d & a;
        
        /* Another call with different register pressure */
        another_external(i);
        
        /* Accumulate result */
        result += a + b + c + d;
    }
    
    return result;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
__attribute__((noinline, optimize("O3")))
int scenario2_volatile_pressure(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int sum = 0;
    
    /* Multiple volatile operations around calls */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        v3 = v3 ^ v2;
        
        /* Call with volatile values live */
        external_func();
        
        v4 = v4 | v3;
        v5 = v5 & v4;
        
        /* Another call */
        another_external(v1);
        
        v1 = v1 + v5;
        v2 = v2 - v4;
        
        /* Conditional that creates basic block boundary */
        if (i & 1) {
            v3 = v3 * 2;
            external_func();  /* Call at end of basic block */
            /* BB_END might be updated here */
        } else {
            v3 = v3 / 2;
        }
        
        sum += v1 + v2 + v3 + v4 + v5;
    }
    
    return sum;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
__attribute__((noinline, optimize("O2")))
int scenario3_nested_calls(void) {
    int x = 0;
    
    /* Outer call setup in registers */
    for (int i = 0; i < 5; ++i) {
        int a = i * 10;
        int b = i * 20;
        int c = i * 30;
        
        /* First call - clobbers registers */
        another_external(a);
        
        /* Inner call with arguments depending on previous values */
        x += returns_value() + b;
        
        /* More computation */
        c = c + a + b;
        
        /* Switch statement creating complex basic block structure */
        switch (i % 3) {
            case 0:
                external_func();
                x += 1;
                break;  /* Creates jump at end of basic block */
            case 1:
                x += returns_value();
                external_func();
                x += 2;
                break;
            default:
                x += 3;
                another_external(c);
                x += 4;
                break;
        }
    }
    
    return x;
}

/* ===== SCENARIO 4: setjmp/longjmp with register uncertainty ===== */
__attribute__((noinline, optimize("O2")))
int scenario4_setjmp_pressure(void) {
    volatile int counter = 0;
    register int r1 __asm__ ("eax") = 100;
    register int r2 __asm__ ("ecx") = 200;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        for (int i = 0; i < 3; ++i) {
            r1 += i * 10;
            r2 -= i * 5;
            
            /* Call that might trigger save/restore due to setjmp */
            external_func();
            
            r1 = r1 ^ r2;
            r2 = r2 | r1;
            
            counter += r1 + r2;
            
            /* Simulate longjmp on last iteration */
            if (i == 2) {
                longjmp(jump_buffer, 1);
            }
        }
    } else {
        /* After longjmp */
        r1 = 50;
        r2 = 150;
        external_func();
        counter += r1 * r2;
    }
    
    return counter;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
__attribute__((noinline, optimize("O3")))
int scenario5_computed_goto(void) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = 0;
    int i = 0;
    
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    
label_start:
    a = a + i;
    b = b - i;
    
    /* Function call in middle of computed goto logic */
    external_func();
    
    /* Computed goto based on i */
    goto *labels[i % 4];
    
label0:
    result += a * 2;
    another_external(result);
    i++;
    if (i < 8) goto label_start;
    goto done;
    
label1:
    result += b * 3;
    external_func();
    i++;
    if (i < 8) goto label_start;
    goto done;
    
label2:
    result += a + b;
    another_external(a);
    i++;
    if (i < 8) goto label_start;
    goto done;
    
label3:
    result += a - b;
    external_func();
    i++;
    if (i < 8) goto label_start;
    /* Fall through to done */
    
done:
    return result;
}

/* ===== Main function to execute all scenarios ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios and accumulate results */
    total += scenario1_register_pressure();
    printf("Scenario 1 completed: %d\n", total);
    
    total += scenario2_volatile_pressure();
    printf("Scenario 2 completed: %d\n", total);
    
    total += scenario3_nested_calls();
    printf("Scenario 3 completed: %d\n", total);
    
    total += scenario4_setjmp_pressure();
    printf("Scenario 4 completed: %d\n", total);
    
    total += scenario5_computed_goto();
    printf("Scenario 5 completed: %d\n", total);
    
    printf("Final total: %d\n", total);
    
    /* Return non-zero if any scenario returned 0 (unlikely) */
    return total == 0 ? 1 : 0;
}
