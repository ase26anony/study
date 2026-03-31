/* caller-save-test.c
 * 
 * A comprehensive test program designed to trigger the specific instruction
 * reordering logic in GCC's caller-save pass (lines 905-913 of caller-save.cc).
 * This code creates scenarios where caller-save restorations are inserted
 * and then manipulated within basic block instruction chains.
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* ========== Helper Functions with Register Pressure ========== */

/* Force caller-save by clobbering many caller-saved registers */
__attribute__((noinline))
void clobber_caller_saved_registers() {
    /* Clobber multiple caller-saved registers across different register classes */
    asm volatile(
        "# Clobber general purpose registers\n\t"
        "mov $0, %%rax\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        "# Clobber floating point/SSE registers\n\t"
        "pxor %%xmm0, %%xmm0\n\t"
        "pxor %%xmm1, %%xmm1\n\t"
        "pxor %%xmm2, %%xmm2\n\t"
        "pxor %%xmm3, %%xmm3\n\t"
        "pxor %%xmm4, %%xmm4\n\t"
        "pxor %%xmm5, %%xmm5\n\t"
        "pxor %%xmm6, %%xmm6\n\t"
        "pxor %%xmm7, %%xmm7\n\t"
        "pxor %%xmm8, %%xmm8\n\t"
        "pxor %%xmm9, %%xmm9\n\t"
        "pxor %%xmm10, %%xmm10\n\t"
        "pxor %%xmm11, %%xmm11\n\t"
        "pxor %%xmm12, %%xmm12\n\t"
        "pxor %%xmm13, %%xmm13\n\t"
        "pxor %%xmm14, %%xmm14\n\t"
        "pxor %%xmm15, %%xmm15"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
    );
}

/* External function declaration to force call instruction */
extern int external_helper(int, double, long long);

/* Function with many live variables across a call */
__attribute__((noinline, optimize("O3")))
int test_many_live_variables(int seed) {
    /* Declare many variables of different types to use all register classes */
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6 = seed * 6;
    int v7 = seed * 7;
    int v8 = seed * 8;
    
    double f1 = seed * 1.1;
    double f2 = seed * 2.2;
    double f3 = seed * 3.3;
    double f4 = seed * 4.4;
    
    long long ll1 = seed * 100LL;
    long long ll2 = seed * 200LL;
    long long ll3 = seed * 300LL;
    
    /* Use all variables in computations before call */
    int sum_int = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    double sum_double = f1 + f2 + f3 + f4;
    long long sum_ll = ll1 + ll2 + ll3;
    
    /* Function call that clobbers caller-saved registers */
    clobber_caller_saved_registers();
    
    /* Use all variables again after call - forces save/restore */
    sum_int += v1 * v2 - v3 / (v4 ? v4 : 1) + v5 % (v6 ? v6 : 1) + v7 - v8;
    sum_double += f1 * f2 - f3 / f4;
    sum_ll += ll1 * ll2 - ll3;
    
    /* Another call with different register usage pattern */
    int ext_result = external_helper(sum_int, sum_double, sum_ll);
    
    /* Final computation mixing all types */
    return (int)(sum_int + (int)sum_double + (int)(sum_ll >> 32) + ext_result);
}

/* ========== Loop with Caller-Save Pressure ========== */

__attribute__((noinline, optimize("O2")))
long test_loop_with_calls(int iterations) {
    long accumulator = 0;
    volatile int loop_var = 7;  /* Prevent optimization */
    
    for (int i = 0; i < iterations; i++) {
        /* Loop-invariant calculations that need registers */
        int a = i * 3;
        int b = i * 5;
        int c = i * 7;
        int d = i * 11;
        double x = i * 1.414;
        double y = i * 2.718;
        
        /* These must survive across the function call */
        int pre_call_sum = a + b + c + d;
        double pre_call_product = x * y;
        
        /* Call that clobbers registers */
        clobber_caller_saved_registers();
        
        /* Use the values after call - forces restoration */
        accumulator += pre_call_sum * (int)pre_call_product;
        
        /* Another computation with different register set */
        int e = a * b;
        int f = c * d;
        double z = x / y;
        
        /* Second call in same iteration */
        asm volatile(
            "# Clobber specific registers\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rdx\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1"
            :
            :
            : "rax", "rdx", "xmm0", "xmm1", "memory"
        );
        
        /* More uses after second call */
        accumulator += e + f + (int)z;
        
        /* Conditional that creates basic block boundaries */
        if (i % 2 == 0) {
            accumulator += loop_var;
        } else {
            accumulator -= loop_var;
        }
    }
    
    return accumulator;
}

/* ========== Switch Statement with Multiple Edges ========== */

__attribute__((noinline))
int switch_helper_1(int x, double y) {
    asm volatile("" : : : "rax", "rcx", "xmm0", "xmm1", "memory");
    return (int)(x * y);
}

__attribute__((noinline))
int switch_helper_2(int x, double y) {
    asm volatile("" : : : "rdx", "rsi", "xmm2", "xmm3", "memory");
    return (int)(x / y);
}

__attribute__((noinline))
int switch_helper_3(int x, double y) {
    asm volatile("" : : : "rdi", "r8", "xmm4", "xmm5", "memory");
    return (int)(x + y);
}

__attribute__((optimize("O3")))
int test_switch_with_calls(int mode, int base) {
    int result = base;
    double fp_val = base * 1.5;
    
    /* Variables that must survive across switch cases */
    int preserved1 = base * 2;
    int preserved2 = base * 3;
    double preserved_fp = base * 2.5;
    
    switch (mode % 4) {
        case 0:
            /* Complex computation before call */
            preserved1 = preserved1 * preserved2 + (int)preserved_fp;
            result = switch_helper_1(preserved1, preserved_fp);
            /* Use values after call */
            result += preserved1 + (int)preserved_fp;
            break;
            
        case 1:
            preserved2 = preserved1 - preserved2 * (int)preserved_fp;
            result = switch_helper_2(preserved2, preserved_fp * 2.0);
            result += preserved2 * (int)(preserved_fp * 3.0);
            break;
            
        case 2:
            preserved_fp = preserved_fp * (preserved1 + preserved2);
            result = switch_helper_3((int)preserved_fp, fp_val);
            result += (int)preserved_fp * 2;
            break;
            
        case 3:
            /* Multiple calls in one case */
            preserved1 = switch_helper_1(preserved1, preserved_fp);
            clobber_caller_saved_registers();
            preserved2 = switch_helper_2(preserved2, preserved_fp);
            result = preserved1 + preserved2;
            break;
            
        default:
            __builtin_unreachable();
    }
    
    /* Common code after switch with more register usage */
    double final_fp = result * 1.618;
    asm volatile(
        "# Final clobber\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        "pxor %%xmm10, %%xmm10\n\t"
        "pxor %%xmm11, %%xmm11"
        :
        :
        : "r10", "r11", "xmm10", "xmm11", "memory"
    );
    
    return result + (int)final_fp;
}

/* ========== setjmp/longjmp Test ========== */

static jmp_buf jump_buffer;

__attribute__((noinline))
void function_with_setjmp(int* ptr, double* fptr) {
    /* Variables that need to be saved across longjmp */
    int local1 = *ptr * 2;
    int local2 = *ptr * 3;
    double local_fp = *fptr * 1.5;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        *ptr = local1 + local2;
        *fptr = local_fp * 2.0;
        
        /* Call that clobbers registers before longjmp */
        clobber_caller_saved_registers();
        
        /* longjmp will restore registers from jump_buffer */
        longjmp(jump_buffer, 1);
    } else {
        /* After longjmp - original values should be restored */
        *ptr += local1 * local2;
        *fptr += local_fp;
    }
}

/* ========== Mixed Float/Int Operations ========== */

__attribute__((noinline, optimize("O3")))
float test_mixed_operations(int base) {
    /* Mix float and int operations to use different register banks */
    float f1 = base * 1.1f;
    float f2 = base * 2.2f;
    float f3 = base * 3.3f;
    float f4 = base * 4.4f;
    
    int i1 = base * 5;
    int i2 = base * 6;
    int i3 = base * 7;
    int i4 = base * 8;
    
    /* Computation using both float and int registers */
    float float_sum = f1 + f2 + f3 + f4;
    int int_sum = i1 + i2 + i3 + i4;
    
    /* Call that clobbers both register types */
    asm volatile(
        "# Clobber mixed registers\n\t"
        "mov $0, %%eax\n\t"
        "mov $0, %%ecx\n\t"
        "pxor %%xmm0, %%xmm0\n\t"
        "pxor %%xmm1, %%xmm1"
        :
        :
        : "eax", "ecx", "xmm0", "xmm1", "memory"
    );
    
    /* More mixed operations after call */
    float_sum += (float)int_sum * 0.5f;
    float_sum = float_sum * f1 - f2 / f3 + f4;
    
    /* Another call */
    clobber_caller_saved_registers();
    
    /* Final mixed computation */
    return float_sum * (float)(i1 * i2 - i3 / (i4 ? i4 : 1));
}

/* ========== Main Test Orchestrator ========== */

int main() {
    printf("Starting caller-save instruction chain test...\n");
    
    int checksum = 0;
    
    /* Test 1: Many live variables across calls */
    printf("Test 1: Many live variables...\n");
    int result1 = test_many_live_variables(42);
    checksum += result1;
    printf("  Result: %d\n", result1);
    
    /* Test 2: Loop with caller-save pressure */
    printf("Test 2: Loop with calls...\n");
    long result2 = test_loop_with_calls(10);
    checksum += (int)result2;
    printf("  Result: %ld\n", result2);
    
    /* Test 3: Switch with multiple control flow edges */
    printf("Test 3: Switch statement with calls...\n");
    for (int i = 0; i < 4; i++) {
        int result3 = test_switch_with_calls(i, 100);
        checksum += result3;
        printf("  Mode %d: %d\n", i, result3);
    }
    
    /* Test 4: setjmp/longjmp */
    printf("Test 4: setjmp/longjmp...\n");
    int jmp_int = 10;
    double jmp_double = 20.0;
    function_with_setjmp(&jmp_int, &jmp_double);
    checksum += jmp_int + (int)jmp_double;
    printf("  Results: int=%d, double=%.2f\n", jmp_int, jmp_double);
    
    /* Test 5: Mixed float/int operations */
    printf("Test 5: Mixed float/int operations...\n");
    float result5 = test_mixed_operations(25);
    checksum += (int)result5;
    printf("  Result: %.2f\n", result5);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal checksum: %d\n", checksum);
    
    /* Validate results aren't obviously wrong */
    if (checksum < 0) {
        printf("ERROR: Negative checksum\n");
        return 1;
    }
    
    return 0;
}

/* External function definition */
int external_helper(int a, double b, long long c) {
    /* Simple computation that uses its arguments */
    return (int)(a + b + (double)(c >> 16));
}
