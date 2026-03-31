/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Force clobbering of caller-saved registers */
#if defined(__x86_64__) || defined(__i386__)
    /* x86/x86_64 caller-saved registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11",
        "xmm0", "xmm1", "xmm2", "xmm3",
        "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15",
        "memory");
#elif defined(__aarch64__)
    /* ARM64 caller-saved registers */
    asm volatile("" : : : 
        "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
        "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
        "x16", "x17", "x18",
        "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
        "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
        "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
        "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
        "memory");
#else
    /* Generic memory clobber */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, optimize("no-ipa-ra")))
int caller_function(int param1, int param2) {
    /* Declare many local variables to create register pressure */
    register int a = param1 * 2;
    register int b = param2 + 3;
    register int c = a ^ b;
    register int d = param1 - param2;
    register int e = a * b + c;
    register int f = d << 2;
    register int g = e ^ f;
    register int h = a + b + c + d;
    register int i = param1 * param2;
    register int j = i - a + b;
    
    /* Additional variables for more pressure */
    int k = j * 3;
    int l = k ^ 0xABCD;
    int m = l + h;
    int n = m * g;
    int o = n >> 1;
    int p = o ^ f;
    
    /* Memory barrier to force values to registers */
    asm volatile("" : : : "memory");
    
    /* Complex computation before call */
    a = b * c + d - e;
    b = f ^ g | h;
    c = i + j - k;
    d = l * m / (n + 1);
    
    /* Conditional call based on volatile flag */
    if (global_flag) {
        /* Additional computation right before call */
        e = (a & b) | (c ^ d);
        f = g + h - i;
        
        /* Memory barrier before call */
        asm volatile("" : : : "memory");
        
        /* The call that will trigger caller-save */
        callee_function();
        
        /* Memory barrier after call */
        asm volatile("" : : : "memory");
        
        /* Computation after call using same variables */
        g = j * k + l;
        h = m ^ n ^ o;
    } else {
        /* Alternative path without call */
        g = a * b - c;
        h = d + e + f;
    }
    
    /* More computations ensuring variables stay live */
    i = (g & h) | (a ^ b);
    j = c + d - e;
    k = f * g / (h + 1);
    l = i ^ j ^ k;
    m = a + b + c + d + e + f;
    n = g * h - i * j;
    o = k | l | m | n;
    p = o ^ 0xDEADBEEF;
    
    /* Complex return value using all variables */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

/* Another caller to create different call patterns */
__attribute__((noipa, noinline))
int caller_function2(int x) {
    int v1 = x * 3;
    int v2 = x + 5;
    int v3 = v1 ^ v2;
    int v4 = x << 2;
    int v5 = v3 + v4;
    int v6 = v5 * 2;
    
    asm volatile("" : : : "memory");
    
    if (global_flag & 1) {
        v1 = v2 * v3;
        v2 = v4 ^ v5;
        callee_function();
        v3 = v6 + v1;
        v4 = v2 * v3;
    }
    
    v5 = v1 + v2 + v3 + v4;
    v6 = v5 ^ x;
    
    return v1 + v2 + v3 + v4 + v5 + v6;
}

int main(void) {
    int result1, result2;
    
    /* Vary the flag to create different paths */
    for (int i = 0; i < 10; i++) {
        global_flag = i & 1;
        
        /* Call with different parameters to prevent constant propagation */
        result1 = caller_function(i, i + 1);
        result2 = caller_function2(i * 2);
        
        /* Use results to prevent dead code elimination */
        printf("Iteration %d: results = %d, %d\n", i, result1, result2);
        
        /* Additional memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Force compiler to keep both functions */
    if (result1 > 1000) {
        callee_function();
    }
    
    return 0;
}
