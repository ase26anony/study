/* early-remat-test.c
 * Test program to trigger early rematerialization privatization logic
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat early-remat-test.c -o early-remat-test
 * Also try: gcc -O3 -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) {
    volatile int y = x * 2;
    return y + 1;
}

__attribute__((noinline)) double external_func2(double x) {
    volatile double y = x * 3.14159;
    return y / 2.0;
}

__attribute__((noinline)) void* external_func3(void* p) {
    volatile intptr_t addr = (intptr_t)p;
    return (void*)(addr + 16);
}

/* Volatile function pointer to prevent inlining */
volatile void (*volatile_func_ptr)(void);

/* Stress function with complex control flow and high register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed + 1;
    int v3 = seed * 2;
    int v4 = seed / 3;
    float f1 = seed * 1.5f;
    float f2 = seed * 2.5f;
    double d1 = seed * 3.14159;
    double d2 = seed * 2.71828;
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    long long ll1 = (long long)seed * 1000000LL;
    long long ll2 = (long long)seed * 2000000LL;
    
    /* Pointers to create additional pressure */
    int* p1 = &v1;
    int* p2 = &v2;
    float* pf1 = &f1;
    double* pd1 = &d1;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    double fp_key_result = 0.0;
    
    /* Complex computation with many live variables */
    for (int i = 0; i < 10; i++) {
        v1 = v1 * 1103515245 + 12345;
        v2 = v2 ^ (v2 << 13);
        v3 = v3 ^ (v3 >> 17);
        v4 = v4 ^ (v4 << 5);
        
        f1 = f1 * 1.1f + (float)i;
        f2 = f2 * 0.9f - (float)i;
        d1 = d1 * 1.01 + (double)i;
        d2 = d2 * 0.99 - (double)i;
        
        /* Accumulate to key results */
        key_result += v1 + v2 + v3 + v4;
        fp_key_result += f1 + f2 + d1 + d2;
    }
    
    /* Create label addresses for goto-based control flow */
    void* labels[] = {
        &&label0, &&label1, &&label2, &&label3, 
        &&label4, &&label5, &&label6, &&label7
    };
    
    volatile int label_selector = key_result % 8;
    
    /* Use volatile variable to prevent optimization of conditional */
    volatile int cond_volatile = key_result;
    
    /* Deeply nested conditional structure */
    if (cond_volatile > 1000) {
        if (key_result % 3 == 0) {
            switch (key_result % 5) {
                case 0:
                    /* This is the critical block where key_result is used
                     * in inline assembly with many clobbered registers */
                    {
                        int temp = key_result;
                        /* Inline assembly that uses key_result and clobbers many registers */
                        __asm__ volatile (
                            "mov %0, %%eax\n\t"
                            "add $1, %%eax\n\t"
                            "mov %%eax, %0\n\t"
                            : "+r" (temp)
                            : 
                            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
                        );
                        key_result = temp;
                        
                        /* Additional computation to keep variables live */
                        f1 = f1 * 2.0f + (float)key_result;
                        d1 = d1 * 2.0 + (double)key_result;
                    }
                    break;
                    
                case 1:
                    /* Another conditional use with different mode (float) */
                    {
                        float ftemp = (float)key_result;
                        __asm__ volatile (
                            "flds %0\n\t"
                            "fadd %%st(0), %%st(0)\n\t"
                            "fstps %0\n\t"
                            : "+m" (ftemp)
                            :
                            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "memory"
                        );
                        f1 = ftemp;
                    }
                    break;
                    
                case 2:
                    /* Use with long long mode */
                    {
                        long long lltemp = (long long)key_result;
                        __asm__ volatile (
                            "mov %0, %%rax\n\t"
                            "imul $3, %%rax\n\t"
                            "mov %%rax, %0\n\t"
                            : "+r" (lltemp)
                            :
                            : "rax", "rbx", "rcx", "rdx", "memory"
                        );
                        ll1 = lltemp;
                    }
                    break;
                    
                default:
                    /* Call non-inlineable function with key_result */
                    key_result = external_func1(key_result);
                    break;
            }
            
            /* More nested conditionals */
            if (fp_key_result > 10000.0) {
                volatile double dtemp = fp_key_result;
                dtemp = external_func2(dtemp);
                fp_key_result = dtemp;
                
                /* Use goto with computed label address */
                goto *labels[label_selector];
            }
        } else if (key_result % 7 == 0) {
            /* Alternative path with different register usage */
            int* volatile ptemp = (int*)external_func3(&key_result);
            v1 = *ptemp;
        }
    } else {
        /* Different control flow path */
        switch (key_result % 4) {
            case 0:
                key_result = key_result << 2;
                break;
            case 1:
                key_result = key_result >> 2;
                break;
            case 2:
                key_result = key_result * 3;
                break;
            case 3:
                key_result = key_result / 3;
                break;
        }
    }
    
    /* Label targets for goto */
    label0:
        v1 += 100;
        goto label_end;
    label1:
        v2 += 200;
        goto label_end;
    label2:
        v3 += 300;
        goto label_end;
    label3:
        v4 += 400;
        goto label_end;
    label4:
        f1 += 500.0f;
        goto label_end;
    label5:
        f2 += 600.0f;
        goto label_end;
    label6:
        d1 += 700.0;
        goto label_end;
    label7:
        d2 += 800.0;
        goto label_end;
    label_end:
    
    /* Use key_result again after conditional blocks */
    int final_result = key_result;
    
    /* Complex final computation using all variables to keep them live */
    final_result += v1 + v2 + v3 + v4;
    final_result += (int)f1 + (int)f2;
    final_result += (int)d1 + (int)d2;
    final_result += c1 + s1;
    final_result += (int)(ll1 & 0xFFFFFFFF) + (int)(ll2 >> 32);
    final_result += *p1 + *p2;
    final_result += (int)(*pf1) + (int)(*pd1);
    
    /* One more conditional use with inline assembly */
    if (final_result % 11 == 0) {
        __asm__ volatile (
            "mov %0, %%eax\n\t"
            "xor %%ebx, %%ebx\n\t"
            "mov $1, %%ecx\n\t"
            "cmp %%eax, %%ecx\n\t"
            "cmovg %%ecx, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "+r" (final_result)
            :
            : "eax", "ebx", "ecx", "edx", "memory"
        );
    }
    
    return final_result;
}

/* Another function with different data type patterns */
__attribute__((noinline, optimize("O0")))
double mixed_mode_function(int base) {
    volatile char c = base & 0xFF;
    volatile short s = base & 0xFFFF;
    volatile int i = base;
    volatile long long ll = (long long)base * 1000LL;
    volatile float f = (float)base * 1.234f;
    volatile double d = (double)base * 5.678;
    
    /* Complex conditional with mixed type computations */
    double result = 0.0;
    
    for (int iter = 0; iter < 20; iter++) {
        /* Force different machine modes through type mixing */
        switch (iter % 6) {
            case 0:  /* char mode */
                result += (double)(c * iter);
                c = (c + 1) & 0xFF;
                break;
            case 1:  /* short mode */
                result += (double)(s * iter);
                s = (s + 1) & 0xFFFF;
                break;
            case 2:  /* int mode */
                result += (double)(i * iter);
                i = i + 1;
                break;
            case 3:  /* long long mode */
                result += (double)(ll * iter);
                ll = ll + 1000LL;
                break;
            case 4:  /* float mode */
                result += (double)(f * (float)iter);
                f = f * 1.1f;
                break;
            case 5:  /* double mode */
                result += d * (double)iter;
                d = d * 1.01;
                break;
        }
        
        /* Conditional inline assembly with different clobbers */
        if (iter % 7 == 0) {
            double dtemp = result;
            __asm__ volatile (
                "fldl %0\n\t"
                "fsqrt\n\t"
                "fstpl %0\n\t"
                : "+m" (dtemp)
                :
                : "st", "st(1)", "st(2)", "st(3)", "memory"
            );
            result = dtemp;
        }
    }
    
    return result;
}

int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Testing early rematerialization privatization...\n");
    
    /* Call stress function multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        int result = stress_function(seed + i);
        total += result;
        
        double mixed = mixed_mode_function(seed + i);
        total += (int)mixed;
        
        /* Create artificial register pressure between calls */
        volatile int pressure[16];
        for (int j = 0; j < 16; j++) {
            pressure[j] = result + j + (int)mixed;
        }
        
        /* Use pressure array to prevent optimization */
        for (int j = 0; j < 16; j++) {
            total += pressure[j];
        }
    }
    
    printf("Final result: %d\n", total);
    printf("Test completed.\n");
    
    return total > 0 ? 0 : 1;
}
