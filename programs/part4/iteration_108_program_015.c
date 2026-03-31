/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

volatile int global_seed;

/* Function that clobbers many registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
#elif __i386__
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi", "memory"
#elif __riscv
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "memory"
#else
#define CLOBBER_LIST "memory"
#endif

void __attribute__((noinline, noclone)) clobber_callee(int *p1, int *p2, int *p3) {
    /* Opaque assembly to clobber registers */
    asm volatile ("" : : "r"(p1), "r"(p2), "r"(p3) : CLOBBER_LIST);
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_more(int *arr, int n) {
    asm volatile ("" : : "r"(arr), "r"(n) : CLOBBER_LIST);
    for (int i = 0; i < n && i < 4; i++) {
        arr[i] += i;
    }
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to prevent optimization */
    global_seed = argc;
    
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    volatile int trigger = global_seed;
    
    /* Initialize variables with complex arithmetic to prevent optimization */
    v1 = trigger + 1;
    v2 = trigger * 2;
    v3 = trigger - v1;
    v4 = v1 * v2 + v3;
    v5 = v2 / (v1 ? v1 : 1) + trigger;
    v6 = v3 ^ v4;
    v7 = v4 | v5;
    v8 = v5 & v6;
    v9 = v6 + v7 - v8;
    v10 = v7 * v8 / (v9 ? v9 : 1);
    
    v11 = v8 << 2;
    v12 = v9 >> 1;
    v13 = v10 + v11 - v12;
    v14 = v11 * v12;
    v15 = v12 % (v13 ? v13 : 1);
    v16 = v13 ^ v14 ^ v15;
    v17 = v14 | v15 | v16;
    v18 = v15 & v16 & v17;
    v19 = v16 + v17 + v18;
    v20 = v17 - v18 + v19;
    
    v21 = v18 * 3;
    v22 = v19 / 2;
    v23 = v20 + v21 - v22;
    v24 = v21 * v22;
    v25 = v22 % (v23 ? v23 : 1);
    v26 = v23 ^ v24;
    v27 = v24 | v25;
    v28 = v25 & v26;
    v29 = v26 + v27 - v28;
    v30 = v27 * v28 / (v29 ? v29 : 1);
    
    /* Create conditional basic blocks where one path has high register pressure */
    int result = 0;
    
    /* First conditional: creates a basic block ending with a call */
    if (trigger & 1) {
        /* High register pressure path - many variables live across call */
        int t1 = v1 + v2 + v3;
        int t2 = v4 + v5 + v6;
        int t3 = v7 + v8 + v9;
        
        /* Call with many live registers */
        clobber_callee(&t1, &t2, &t3);
        
        /* Use results after call to keep variables live */
        v1 = t1 + v10;
        v2 = t2 + v11;
        v3 = t3 + v12;
        
        result += v1 + v2 + v3;
    } else {
        /* Low pressure path */
        result += v1 + v2;
    }
    
    /* Second conditional with different structure */
    if (trigger & 2) {
        /* Another high pressure scenario */
        int arr[6] = {v13, v14, v15, v16, v17, v18};
        
        /* Call that clobbers registers */
        clobber_more(arr, 6);
        
        /* Complex computation after call */
        for (int i = 0; i < 6; i++) {
            v13 += arr[i];
            v14 -= arr[i] * i;
        }
        
        result += v13 + v14 + v15;
    } else {
        result += v16 + v17;
    }
    
    /* Third conditional inside a loop - creates multiple call sites */
    for (int i = 0; i < 3; i++) {
        if ((trigger + i) & 4) {
            /* Variables v19-v26 are live here */
            int sum = v19 + v20 + v21 + v22;
            int prod = v23 * v24 * (v25 ? v25 : 1);
            
            clobber_callee(&sum, &prod, &v26);
            
            v19 = sum / 2;
            v20 = prod % 100;
            v21 = v26 + i;
            
            result += v19 + v20 + v21;
        } else {
            /* Different computation to create separate basic block */
            int diff = v27 - v28 - v29;
            result += diff + v30;
        }
    }
    
    /* Use all variables in final computation to prevent dead code elimination */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    
    /* Mix in some volatile operations */
    result ^= global_seed;
    result += trigger;
    
    printf("Result: %d\n", result);
    
    return result != 0;
}
