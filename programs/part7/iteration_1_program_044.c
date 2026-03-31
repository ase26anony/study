/* test_caller_save.c - Program to trigger GCC caller-save pass instruction list manipulation */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void external_func(void) __attribute__((noinline));
void another_external(int x) __attribute__((noinline));
int third_external(int a, int b) __attribute__((noinline));

/* Dummy implementations to satisfy linker */
void external_func(void) {
    /* Empty but prevents inlining */
    asm volatile("" : : : "memory");
}

void another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int third_external(int a, int b) {
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    return a + b;
}

/* Global to prevent optimization */
volatile int global_counter = 0;

/* SCENARIO 1: Explicit register variables with loop */
/* Force use of call-clobbered registers on x86: eax, ecx, edx */
int __attribute__((noinline)) scenario1_register_pressure(void) {
    register int a __asm__("eax") = 1;
    register int b __asm__("ecx") = 2;
    register int c __asm__("edx") = 3;
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 100; ++i) {
        /* Use registers before call */
        a = a + i * 2;
        b = b - i / 3;
        c = c ^ (i & 0xFF);
        
        /* Function call clobbers registers - forces save/restore */
        external_func();
        
        /* More operations after call - values must be restored */
        a = a | 0x55;
        b = b & 0xAA;
        c = c + 1;
        
        /* Conditional to create basic block structure */
        if (i % 7 == 0) {
            sum += a + b + c;
        }
    }
    
    /* Final computation to use all values */
    return sum + a + b + c;
}

/* SCENARIO 2: Many volatile variables across calls */
int __attribute__((noinline)) scenario2_volatile_pressure(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int result = 0;
    
    /* Multiple calls with volatile usage in between */
    v1 = v1 * 2 + 1;
    v2 = v2 / 2 - 1;
    external_func();  /* Call 1 */
    
    v3 = v3 | v1;
    v4 = v4 & v2;
    another_external(v3);  /* Call 2 */
    
    v5 = v5 ^ v4;
    v1 = v1 + v5;
    result = third_external(v1, v2);  /* Call 3 */
    
    /* Complex tail with conditional and call */
    if (v3 > 100) {
        v4 = v4 * 2;
        external_func();  /* Call at end of basic block */
        goto done;
    } else {
        v4 = v4 / 2;
    }
    
    result += v4;
    
done:
    /* Force BB_END update by having label at end */
    return result + v1 + v2 + v3 + v5;
}

/* SCENARIO 3: Nested calls with register dependencies */
int __attribute__((noinline)) scenario3_nested_calls(void) {
    int x = 1, y = 2, z = 3;
    
    /* Outer call setup in registers, then inner call */
    for (int i = 0; i < 50; i++) {
        x = x * 3 + i;
        y = y / 2 - i;
        
        /* Nested call pattern */
        int temp = third_external(x, y);  /* Uses x,y in regs */
        
        /* Another call immediately after */
        z = third_external(temp, z);  /* Result must be in reg for next call */
        
        /* Conditional jump at end of block */
        if (z > 1000) {
            external_func();
            break;  /* Creates jump at BB_END */
        }
        
        x = z + 1;
        y = z - 1;
    }
    
    return x + y + z;
}

/* SCENARIO 4: setjmp/longjmp with calls */
jmp_buf env;

int __attribute__((noinline)) scenario4_setjmp_flow(void) {
    volatile int a = 10, b = 20, c = 30;
    int ret = setjmp(env);
    
    if (ret == 0) {
        /* First time through */
        a = a * 2;
        b = b / 2;
        
        /* Function call that might be saved across setjmp */
        external_func();
        
        c = a + b;
        
        /* Another call before potential longjmp */
        another_external(c);
        
        /* Force save/restore around this call */
        a = third_external(a, b);
        
        /* Simulate error - would longjmp in real code */
        if (c > 100) {
            /* In real code: longjmp(env, 1); */
            /* For compilation, just create the control flow */
            goto error_handler;
        }
    } else {
        /* longjmp return path */
        a = a + 100;
        b = b + 200;
    }
    
    return a + b + c;

error_handler:
    /* Separate basic block with call at end */
    external_func();
    return -1;  /* BB_END is return */
}

/* SCENARIO 5: Computed goto with calls */
int __attribute__((noinline)) scenario5_computed_goto(void) {
    static void* targets[] = { &&label1, &&label2, &&label3 };
    volatile int index = global_counter % 3;
    int x = 0, y = 0, z = 0;
    
    /* Force different registers to be live */
    x = 100;
    y = 200;
    z = 300;
    
    /* Computed goto */
    goto *targets[index];
    
label1:
    x = x * 2;
    external_func();  /* Call in middle of block */
    y = y + x;
    goto end;
    
label2:
    y = y / 2;
    another_external(y);
    z = z - y;
    /* Fall through to label3 */
    
label3:
    z = z | 0xFF;
    third_external(x, z);  /* Another call */
    x = x ^ z;
    
end:
    /* Final computation with all values */
    return x + y + z;
}

/* SCENARIO 6: Switch statement with calls at block ends */
int __attribute__((noinline)) scenario6_switch_calls(void) {
    int val = global_counter;
    int a = 1, b = 2, c = 3, d = 4;
    
    switch (val % 4) {
        case 0:
            a = a * 3;
            b = b + a;
            external_func();  /* Call then break - creates specific BB_END pattern */
            break;
            
        case 1:
            c = c | 0xAA;
            d = d & 0x55;
            another_external(c + d);
            /* No break, fall through */
            
        case 2:
            a = a ^ d;
            b = b | c;
            third_external(a, b);
            external_func();  /* Multiple calls in sequence */
            break;
            
        default:
            d = d * 2;
            external_func();  /* Call at end of default case */
            /* BB_END should point to call, then update when save inserted */
            break;
    }
    
    return a + b + c + d;
}

/* Main function to run all scenarios */
int main(void) {
    int result = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios to ensure code generation */
    result += scenario1_register_pressure();
    result += scenario2_volatile_pressure();
    result += scenario3_nested_calls();
    result += scenario4_setjmp_flow();
    result += scenario5_computed_goto();
    result += scenario6_switch_calls();
    
    printf("Final result: %d\n", result);
    printf("(This value is arbitrary - main goal is compile-time coverage)\n");
    
    return 0;
}
