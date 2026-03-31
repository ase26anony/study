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

int __attribute__((noinline)) external_func_with_return(void) {
    return 42;
}

/* Volatile to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables in loop ===== */
int __attribute__((noinline)) scenario1_register_pressure(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__("eax") = 1;
    register int b __asm__("ecx") = 2;
    register int c __asm__("edx") = 3;
    register int d __asm__("esi") = 4;
    
    int result = 0;
    
    /* Complex loop with register pressure across call */
    for (int i = 0; i < 100; ++i) {
        /* Multiple operations on register variables */
        a = a + i * 2;
        b = b - i / 3;
        c = c ^ (i * 7);
        d = d | (i << 2);
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More operations after call - registers need to be restored */
        result += a + b - c * d;
        
        /* Conditional that creates basic block boundaries */
        if (i % 7 == 0) {
            a += external_func_with_return();
        }
        
        /* Additional arithmetic to increase register pressure */
        a = (a << 1) | (b & 0xFF);
        b = (b >> 2) + c;
        c = c * 3 - d;
        d = d ^ a;
    }
    
    return result + a + b + c + d;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int __attribute__((noinline)) scenario2_volatile_pressure(void) {
    volatile int v1 = 1234;
    volatile int v2 = 5678;
    volatile int v3 = 9012;
    volatile int v4 = 3456;
    
    int sum = 0;
    
    /* Multiple volatile accesses around calls */
    for (int i = 0; i < 50; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / 3 - i;
        
        external_func();
        
        v3 = v3 ^ v1;
        v4 = v4 | v2;
        
        /* Nested condition creating complex basic block */
        if (v1 > v2) {
            sum += v1 + external_func_with_return();
            if (v3 < v4) {
                sum -= v2;
                external_func();
            } else {
                sum += v3;
            }
        } else {
            sum += v4;
        }
        
        v1 = v1 + v3;
        v2 = v2 - v4;
    }
    
    return sum + v1 + v2 + v3 + v4;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3_nested_calls(void) {
    int x = 1, y = 2, z = 3, w = 4;
    
    /* Chain of operations where call arguments depend on 
       values in call-clobbered registers */
    for (int i = 0; i < 30; ++i) {
        /* Complex expression that uses many temporaries */
        x = (x * 3 + y) << 2;
        y = (y - z) * 5;
        
        /* Call with arguments - forces register allocation */
        global_counter += external_func_with_return() + x;
        
        z = z ^ y;
        w = w | x;
        
        /* Another call with different arguments */
        if (global_counter % 3 == 0) {
            external_func();
            x = x + w;
        } else {
            y = y + z;
        }
        
        /* Switch statement creating basic block with terminal jump */
        switch (i % 4) {
            case 0:
                x += 10;
                external_func();  /* Call before break */
                break;
            case 1:
                y -= 5;
                external_func();
                break;
            case 2:
                z *= 2;
                external_func();
                break;
            default:
                w /= 2;
                external_func();  /* Call at end of basic block before break */
                break;
        }
    }
    
    return x + y + z + w;
}

/* ===== SCENARIO 4: setjmp/longjmp with register pressure ===== */
int __attribute__((noinline)) scenario4_setjmp_pressure(void) {
    register int r1 __asm__("eax") = 100;
    register int r2 __asm__("ecx") = 200;
    
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        for (int i = 0; i < 20; ++i) {
            r1 = r1 * 2 + i;
            r2 = r2 - i * 3;
            
            /* Function call that might be saved/restored differently
               due to setjmp */
            external_func();
            
            result += r1 + r2;
            
            /* Conditional that might trigger longjmp */
            if (i == 15) {
                longjmp(jump_buffer, 1);
            }
            
            r1 = r1 ^ result;
            r2 = r2 | i;
        }
    } else {
        /* After longjmp */
        result += 1000;
    }
    
    return result + r1 + r2;
}

/* ===== SCENARIO 5: Computed goto with function call ===== */
int __attribute__((noinline)) scenario5_computed_goto(void) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    register int a __asm__("eax") = 5;
    register int b __asm__("ecx") = 10;
    int total = 0;
    
    for (int i = 0; i < 40; ++i) {
        a = a + i * 2;
        b = b - i;
        
        /* Computed goto creating unusual control flow */
        goto *labels[i % 4];
        
    label0:
        external_func();
        total += a;
        continue;
        
    label1:
        total += b;
        external_func();  /* Call in middle of block */
        a = a * 2;
        continue;
        
    label2:
        b = b / 2;
        external_func();
        total += a + b;
        continue;
        
    label3:
        external_func();
        if (total > 1000) {
            /* Conditional jump at end of block */
            total -= 100;
        }
        continue;
    }
    
    return total + a + b;
}

/* ===== Main function to drive all scenarios ===== */
int main(void) {
    int checksum = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios to ensure code generation */
    checksum += scenario1_register_pressure();
    checksum += scenario2_volatile_pressure();
    checksum += scenario3_nested_calls();
    checksum += scenario4_setjmp_pressure();
    checksum += scenario5_computed_goto();
    
    printf("Final checksum: %d\n", checksum);
    printf("(Note: The key coverage happens during compilation with -O2/-O3)\n");
    
    return 0;
}
