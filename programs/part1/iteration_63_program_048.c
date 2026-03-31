/* caller-save-test.c
 * 
 * A comprehensive test program designed to trigger the specific uncovered
 * instruction chain manipulation logic in GCC's caller-save pass.
 * 
 * Target uncovered lines in caller-save.cc (lines 905-913):
 *   SET_NEXT_INSN (prev) = NEXT_INSN (ins);
 *   SET_PREV_INSN (NEXT_INSN (ins)) = prev;
 *   SET_PREV_INSN (ins) = insn;
 *   SET_NEXT_INSN (ins) = NEXT_INSN (insn);
 *   SET_NEXT_INSN (insn) = ins;
 *   if (NEXT_INSN (ins))
 *     SET_PREV_INSN (NEXT_INSN (ins)) = ins;
 *   if (BB_END (bb) == insn)
 *     BB_END (bb) = ins;
 * 
 * Compilation recommendations:
 *   gcc -O3 -fschedule-insns2 -fno-gcse -fno-strict-aliasing -march=native \
 *       -fno-rename-registers -fno-sched-interblock caller-save-test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* ========== Helper Functions with Register Pressure ========== */

/* Force caller-save by clobbering many registers */
__attribute__((noinline, optimize("O3")))
static double complex_float_calc(double a, double b, double c, 
                                 double d, double e, double f) {
    /* Use all FP registers and force spills */
    volatile double v1 = a * b;
    volatile double v2 = c * d;
    volatile double v3 = e * f;
    
    /* Inline asm that clobbers FP registers */
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                 "xmm4", "xmm5", "xmm6", "xmm7", "memory");
    
    return v1 + v2 + v3 + a + b + c + d + e + f;
}

/* Function with many live variables across a call */
__attribute__((noinline))
static long long mixed_reg_pressure(int a, int b, int c, int d,
                                    float fa, float fb, float fc,
                                    double da, double db) {
    /* Create many live values */
    int t1 = a * b + c;
    int t2 = b * c + d;
    int t3 = c * d + a;
    float ft1 = fa * fb + fc;
    float ft2 = fb * fc + fa;
    double dt1 = da * db;
    double dt2 = da + db;
    
    /* Call that forces caller-save */
    double result = complex_float_calc(da, db, da*2, db*2, da*3, db*3);
    
    /* Use all values after call - forcing restores */
    return (long long)(t1 + t2 + t3) + 
           (long long)(ft1 + ft2) + 
           (long long)result +
           (long long)(dt1 + dt2);
}

/* Loop with function call and register pressure */
__attribute__((noinline, optimize("O3")))
static void loop_with_calls(int iterations) {
    volatile int accum_int = 0;
    volatile float accum_float = 0.0f;
    volatile double accum_double = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables across the call */
        int a = i * 2;
        int b = i * 3;
        int c = i * 5;
        float fa = i * 1.5f;
        float fb = i * 2.5f;
        double da = i * 1.7;
        double db = i * 3.3;
        
        /* Call that clobbers registers */
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                     "rsi", "rdi", "r8", "r9", "r10", "r11",
                     "xmm0", "xmm1", "xmm2", "xmm3", "memory");
        
        /* Use variables after asm - forcing restores */
        accum_int += a + b + c;
        accum_float += fa + fb;
        accum_double += da + db;
        
        /* Another call with different register usage */
        long long res = mixed_reg_pressure(a, b, c, i % 7, fa, fb, 0.0f, da, db);
        accum_int += (int)res;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(accum_int), "r"(accum_float), "r"(accum_double));
}

/* ========== Control Flow Manipulation ========== */

/* Function with complex control flow and calls */
__attribute__((noinline, optimize("O3")))
static int switch_with_calls(int value) {
    int result = 0;
    
    switch (value % 5) {
        case 0: {
            /* Many live variables */
            int a = value * 2, b = value * 3, c = value * 5;
            float f = value * 1.1f;
            double d = value * 2.2;
            
            /* Call that forces saves/restores */
            result = (int)complex_float_calc(d, d*2, d*3, d*4, f, f*2);
            
            /* Use variables after call */
            result += a + b + c;
            break;
        }
        case 1: {
            /* Different register pressure pattern */
            long long ll1 = value * 100LL;
            long long ll2 = value * 200LL;
            double d1 = value * 1.5;
            double d2 = value * 2.5;
            
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                         "xmm0", "xmm1", "xmm2", "xmm3", "memory");
            
            result = (int)(ll1 + ll2 + d1 + d2);
            break;
        }
        case 2: {
            /* Chain of calls */
            int t = value;
            for (int i = 0; i < 3; i++) {
                t += (int)mixed_reg_pressure(t, t+1, t+2, t+3,
                                             t*1.0f, t*2.0f, t*3.0f,
                                             t*1.0, t*2.0);
            }
            result = t;
            break;
        }
        case 3: {
            /* Mixed int/float with volatile */
            volatile int v1 = value;
            volatile float v2 = value * 1.234f;
            volatile double v3 = value * 2.345;
            
            /* Call between volatile accesses */
            result = (int)complex_float_calc(v3, v3*2, v3*3, v3*4, v2, v2*2);
            
            result += v1 + (int)v2;
            break;
        }
        case 4: {
            /* Deep expression with intermediate calls */
            double d = value;
            d = complex_float_calc(d, d+1, d+2, d+3, d+4, d+5);
            d += complex_float_calc(d, d*2, d*3, d*4, d*5, d*6);
            result = (int)d;
            break;
        }
    }
    
    return result;
}

/* ========== setjmp/longjmp Testing ========== */

static jmp_buf env;
static volatile int setjmp_counter = 0;

__attribute__((noinline, optimize("O0")))  /* O0 to prevent reordering */
static int function_with_setjmp(int arg) {
    int a = arg * 2;
    int b = arg * 3;
    float f = arg * 1.5f;
    double d = arg * 2.5;
    
    if (setjmp(env) == 0) {
        /* Many live variables across longjmp */
        a += 10;
        b += 20;
        f += 1.0f;
        d += 2.0;
        
        /* Call that might trigger caller-save */
        double result = complex_float_calc(d, d+1, d+2, d+3, f, f+1);
        
        return a + b + (int)f + (int)d + (int)result;
    } else {
        /* Return path after longjmp */
        return a + b + (int)f + (int)d + 1000;
    }
}

/* ========== External Function Declarations ========== */

/* External function to force PLT call */
extern void external_asm_clobber(void) __asm__("external_asm_clobber");

/* Mock external function */
__attribute__((noinline))
static void external_asm_clobber_impl(void) {
    /* Clobber many registers */
    asm volatile("" : : : 
                 "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                 "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                 "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                 "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                 "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

void external_asm_clobber(void) {
    external_asm_clobber_impl();
}

/* ========== Main Test Orchestrator ========== */

int main(void) {
    int checksum = 0;
    
    printf("Starting caller-save stress test...\n");
    
    /* Test 1: Mixed register pressure with function calls */
    printf("Test 1: Mixed register pressure...\n");
    for (int i = 0; i < 10; i++) {
        long long result = mixed_reg_pressure(
            i, i*2, i*3, i*4,
            i*1.1f, i*2.2f, i*3.3f,
            i*1.5, i*2.5
        );
        checksum += (int)result;
    }
    
    /* Test 2: Loop with calls and register pressure */
    printf("Test 2: Loop with calls...\n");
    loop_with_calls(5);
    checksum += 12345;  /* Add constant to prevent elimination */
    
    /* Test 3: Switch with complex control flow */
    printf("Test 3: Switch with calls...\n");
    for (int i = 0; i < 15; i++) {
        checksum += switch_with_calls(i);
    }
    
    /* Test 4: External function calls */
    printf("Test 4: External calls...\n");
    {
        int a = 1, b = 2, c = 3, d = 4;
        float fa = 1.5f, fb = 2.5f;
        double da = 1.7, db = 3.3;
        
        external_asm_clobber();
        
        /* Use variables after external call */
        checksum += a + b + c + d + (int)fa + (int)fb + (int)da + (int)db;
        
        external_asm_clobber();
        
        checksum += a * b * c * d;
    }
    
    /* Test 5: setjmp/longjmp */
    printf("Test 5: setjmp/longjmp...\n");
    int sj_result = function_with_setjmp(42);
    checksum += sj_result;
    
    /* Trigger longjmp from different context */
    setjmp_counter++;
    if (setjmp_counter == 1) {
        longjmp(env, 1);
    }
    
    /* Test 6: Unreachable code pattern */
    printf("Test 6: Unreachable pattern...\n");
    {
        int x = 100, y = 200, z = 300;
        float fx = 100.5f;
        double dx = 200.5;
        
        checksum += x + y + z;
        
        /* Call with many live registers */
        double result = complex_float_calc(dx, dx*2, dx*3, dx*4, fx, fx*2);
        checksum += (int)result;
        
        /* Potentially unreachable to affect block analysis */
        if (checksum > 1000000) {
            __builtin_unreachable();
        }
    }
    
    /* Final validation */
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    /* Use checksum to prevent dead code elimination */
    if (checksum == 0) {
        printf("ERROR: All code was eliminated!\n");
        return 1;
    }
    
    return 0;
}
