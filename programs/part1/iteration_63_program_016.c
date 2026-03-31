/* caller-save-test.c
 * 
 * A comprehensive test program designed to trigger the specific instruction
 * reordering logic in GCC's caller-save restoration placement.
 * Target: caller-save.cc lines 905-913
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* ========== Helper Functions with Register Pressure ========== */

/* Function that clobbers many caller-saved registers via inline asm */
__attribute__((noinline, optimize("O3")))
void clobber_caller_saved_registers() {
    /* Clobber multiple general purpose and floating point registers */
    asm volatile (
        "# Clobber caller-saved registers\n\t"
        "mov $0x12345678, %%eax\n\t"
        "mov $0x9ABCDEF0, %%ebx\n\t"
        "mov $0x11111111, %%ecx\n\t"
        "mov $0x22222222, %%edx\n\t"
        "mov $0x33333333, %%esi\n\t"
        "mov $0x44444444, %%edi\n\t"
        :
        : 
        : "eax", "ebx", "ecx", "edx", "esi", "edi", 
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "memory"
    );
}

/* Function with many live variables across a call */
__attribute__((noinline))
int function_with_many_live_vars(int a, int b, int c, int d, int e, int f) {
    /* Create many local variables that must survive across the call */
    volatile int v1 = a * 2;
    volatile int v2 = b * 3;
    volatile int v3 = c * 4;
    volatile int v4 = d * 5;
    volatile int v5 = e * 6;
    volatile int v6 = f * 7;
    volatile int v7 = a + b;
    volatile int v8 = c + d;
    volatile int v9 = e + f;
    volatile int v10 = a * b;
    
    /* Mix float and int operations to engage different register banks */
    volatile float f1 = a * 1.5f;
    volatile float f2 = b * 2.5f;
    volatile float f3 = c * 3.5f;
    volatile double d1 = d * 1.7;
    volatile double d2 = e * 2.7;
    volatile long long ll1 = (long long)a * b;
    volatile long long ll2 = (long long)c * d;
    
    /* Call that forces caller-save */
    clobber_caller_saved_registers();
    
    /* Use all variables after the call - they must be restored */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
           (int)f1 + (int)f2 + (int)f3 + (int)d1 + (int)d2 + 
           (int)ll1 + (int)ll2;
}

/* ========== Loop with Caller-Save Pressure ========== */

__attribute__((noinline, optimize("O3")))
int loop_with_caller_save_pressure(int iterations) {
    int sum = 0;
    
    /* Many loop-carried variables that must survive across each call */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    float fa = 1.1f, fb = 2.2f, fc = 3.3f;
    double da = 1.11, db = 2.22;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex computation using all variables */
        a = a * 3 + i;
        b = b * 5 - i;
        c = c * 7 + a;
        d = d * 11 - b;
        e = e * 13 + c;
        f = f * 17 - d;
        
        fa = fa * 1.5f + i;
        fb = fb * 2.5f - i;
        fc = fc * 3.5f + fa;
        
        da = da * 1.7 + i;
        db = db * 2.7 - i;
        
        /* Function call that clobbers registers - forces save/restore */
        clobber_caller_saved_registers();
        
        /* Use all variables after call - they must be restored */
        sum += a + b + c + d + e + f + 
               (int)fa + (int)fb + (int)fc + 
               (int)da + (int)db;
    }
    
    return sum;
}

/* ========== Conditional Control Flow ========== */

__attribute__((noinline))
int conditional_caller_save_test(int mode) {
    int result = 0;
    
    /* Variables that must survive across calls in different branches */
    volatile int x1 = 100, x2 = 200, x3 = 300, x4 = 400;
    volatile float y1 = 1.23f, y2 = 4.56f;
    volatile double z1 = 7.89, z2 = 10.11;
    
    switch (mode) {
        case 0:
            /* Branch 0: computation then call */
            x1 = x1 * 2;
            x2 = x2 * 3;
            y1 = y1 * 1.5f;
            z1 = z1 * 2.0;
            
            clobber_caller_saved_registers();
            
            result = x1 + x2 + (int)y1 + (int)z1;
            break;
            
        case 1:
            /* Branch 1: call then computation */
            clobber_caller_saved_registers();
            
            x3 = x3 * 4;
            x4 = x4 * 5;
            y2 = y2 * 2.5f;
            z2 = z2 * 3.0;
            
            result = x3 + x4 + (int)y2 + (int)z2;
            break;
            
        case 2:
            /* Branch 2: call in the middle of computation */
            x1 = x1 * 2;
            y1 = y1 * 1.5f;
            
            clobber_caller_saved_registers();
            
            x2 = x2 * 3;
            z1 = z1 * 2.0;
            
            result = x1 + x2 + (int)y1 + (int)z1;
            break;
            
        default:
            /* Multiple calls in sequence */
            x1 = x1 * 2;
            clobber_caller_saved_registers();
            
            x2 = x2 * 3;
            clobber_caller_saved_registers();
            
            x3 = x3 * 4;
            result = x1 + x2 + x3;
            break;
    }
    
    return result;
}

/* ========== setjmp/longjmp Test ========== */

static jmp_buf jump_buffer;
static int jmp_value = 0;

__attribute__((noinline))
void function_with_setjmp_call(int *value) {
    /* Variables that must be saved across longjmp */
    volatile int a = 10, b = 20, c = 30;
    volatile float f = 3.14f;
    volatile double d = 2.71828;
    
    if (setjmp(jump_buffer) == 0) {
        /* First call: do computation */
        a = a * 2 + *value;
        b = b * 3 - *value;
        f = f * 1.5f;
        d = d * 2.0;
        
        /* Call that clobbers registers */
        clobber_caller_saved_registers();
        
        *value = a + b + (int)f + (int)d;
    } else {
        /* After longjmp: use the variables */
        *value += a + b + (int)f + (int)d;
    }
}

/* ========== Complex Expression Chains ========== */

__attribute__((noinline, optimize("O3")))
int complex_expression_chain(int start) {
    /* Create a long dependency chain */
    int a = start;
    int b = a * 2 + 1;
    int c = b * 3 - 2;
    int d = c * 4 + 3;
    int e = d * 5 - 4;
    int f = e * 6 + 5;
    int g = f * 7 - 6;
    int h = g * 8 + 7;
    int i = h * 9 - 8;
    int j = i * 10 + 9;
    
    /* Call in the middle of the chain */
    clobber_caller_saved_registers();
    
    /* Continue the chain after the call */
    int k = j * 11 - 10;
    int l = k * 12 + 11;
    int m = l * 13 - 12;
    int n = m * 14 + 13;
    int o = n * 15 - 14;
    int p = o * 16 + 15;
    int q = p * 17 - 16;
    int r = q * 18 + 17;
    int s = r * 19 - 18;
    int t = s * 20 + 19;
    
    return t;
}

/* ========== External Function Declaration ========== */

/* Declare as extern to prevent inlining analysis */
extern int external_helper(int, int, int, int, int, int);

/* Mock implementation for linking */
int external_helper(int a, int b, int c, int d, int e, int f) {
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6;
}

/* ========== Main Test Orchestrator ========== */

int main() {
    int total_checksum = 0;
    
    printf("Starting caller-save restoration placement tests...\n");
    
    /* Test 1: Many live variables across a call */
    printf("Test 1: Function with many live variables...\n");
    int result1 = function_with_many_live_vars(1, 2, 3, 4, 5, 6);
    total_checksum += result1;
    printf("  Result: %d\n", result1);
    
    /* Test 2: Loop with caller-save pressure */
    printf("Test 2: Loop with caller-save pressure...\n");
    int result2 = loop_with_caller_save_pressure(10);
    total_checksum += result2;
    printf("  Result: %d\n", result2);
    
    /* Test 3: Conditional control flow */
    printf("Test 3: Conditional caller-save test...\n");
    for (int mode = 0; mode < 4; mode++) {
        int result3 = conditional_caller_save_test(mode);
        total_checksum += result3;
        printf("  Mode %d: %d\n", mode, result3);
    }
    
    /* Test 4: Complex expression chain */
    printf("Test 4: Complex expression chain...\n");
    int result4 = complex_expression_chain(1);
    total_checksum += result4;
    printf("  Result: %d\n", result4);
    
    /* Test 5: External function calls with register pressure */
    printf("Test 5: External function calls...\n");
    int ext_sum = 0;
    for (int i = 0; i < 5; i++) {
        /* Create many variables that must survive across external call */
        int a = i * 10 + 1;
        int b = i * 10 + 2;
        int c = i * 10 + 3;
        int d = i * 10 + 4;
        int e = i * 10 + 5;
        int f = i * 10 + 6;
        
        float fa = i * 1.1f;
        float fb = i * 2.2f;
        
        /* External call - compiler doesn't know what it clobbers */
        int ext_result = external_helper(a, b, c, d, e, f);
        
        /* Use variables after call - may need restoration */
        ext_sum += ext_result + (int)fa + (int)fb;
        
        /* Additional clobber to force more save/restore */
        clobber_caller_saved_registers();
    }
    total_checksum += ext_sum;
    printf("  External calls sum: %d\n", ext_sum);
    
    /* Test 6: setjmp/longjmp test */
    printf("Test 6: setjmp/longjmp test...\n");
    int jmp_test_val = 42;
    function_with_setjmp_call(&jmp_test_val);
    
    /* Trigger longjmp */
    jmp_value = 100;
    longjmp(jump_buffer, 1);
    
    total_checksum += jmp_test_val;
    printf("  setjmp result: %d\n", jmp_test_val);
    
    /* Test 7: Mixed operations in basic block with unreachable */
    printf("Test 7: Mixed operations with unreachable...\n");
    {
        int x = 1000;
        float y = 3.14159f;
        double z = 2.71828;
        
        /* Complex mixed computation */
        x = x * 2 + (int)(y * 100.0f);
        z = z * 3.0 + x;
        y = y * 1.5f + (float)z;
        
        clobber_caller_saved_registers();
        
        x = x + (int)y + (int)z;
        
        /* Call that might affect block termination analysis */
        external_helper(x, x/2, x/3, x/4, x/5, x/6);
        
        /* Potentially affect block layout */
        if (x > 10000) {
            __builtin_unreachable();
        }
        
        total_checksum += x;
        printf("  Mixed ops result: %d\n", x);
    }
    
    printf("\nTotal checksum: %d\n", total_checksum);
    printf("All tests completed.\n");
    
    return total_checksum == 0 ? 1 : 0; /* Non-zero return if all zero (unlikely) */
}
