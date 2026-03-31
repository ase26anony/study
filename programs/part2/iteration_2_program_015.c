/* early-remat-test.c
 * Designed to trigger early_remat::privatize_cond_register_use
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early-remat-test.c -o early-remat-test
 * Also try: gcc -O3 -m32 -march=i686 -fno-dse -fearly-remat early-remat-test.c -o early-remat-test-32
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

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 3.14159;
volatile void* g_volatile_ptr = NULL;

/* Function pointer with volatile target */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t volatile_func_ptr = external_func1;

/* Stress function with complex control flow and high register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    int var1 = seed + 1;
    int var2 = seed * 2;
    int var3 = seed / 3;
    int var4 = seed - 100;
    int var5 = seed | 0xFF;
    int var6 = seed & 0x0F;
    int var7 = seed ^ 0x55;
    int var8 = seed << 2;
    int var9 = seed >> 1;
    int var10 = ~seed;
    
    float fvar1 = seed * 1.5f;
    float fvar2 = seed / 2.0f;
    float fvar3 = seed + 3.14f;
    float fvar4 = seed - 6.28f;
    
    double dvar1 = seed * 2.71828;
    double dvar2 = seed / 1.41421;
    double dvar3 = seed + 1.61803;
    double dvar4 = seed - 0.57721;
    
    char cvar1 = seed & 0xFF;
    short svar1 = seed & 0xFFFF;
    long long llvar1 = (long long)seed * 1000000LL;
    long long llvar2 = (long long)seed * 2000000LL;
    
    /* Pointers to create additional pressure */
    int* ptr1 = &var1;
    int* ptr2 = &var2;
    float* fptr1 = &fvar1;
    double* dptr1 = &dvar1;
    
    /* Key intermediate result - this is the register we want to stress */
    int key_result = var1 + var2 + var3;
    
    /* Volatile control variables */
    volatile int vctrl1 = g_volatile_counter;
    volatile int vctrl2 = g_volatile_counter + 1;
    volatile double vctrl3 = g_volatile_double;
    
    /* Label addresses for complex control flow */
    void* labels[] = {
        &&label_0, &&label_1, &&label_2, &&label_3,
        &&label_4, &&label_5, &&label_6, &&label_7
    };
    
    volatile void* volatile_label_ptr = labels[seed & 0x7];
    
    /* Complex nested conditional structure */
    if (vctrl1 > 0) {
        /* First level: compute using key_result */
        key_result = external_func1(key_result);
        
        if (vctrl2 % 2 == 0) {
            /* Second level: more computation */
            key_result *= 2;
            
            switch (vctrl1 & 0x3) {
                case 0:
                    /* Deeply nested case 0 */
                    {
                        int temp = key_result;
                        
                        /* Use inline assembly with many clobbered registers */
                        /* This creates high register pressure */
                        asm volatile (
                            "/* Begin clobbering registers */\n\t"
                            "mov %0, %0\n\t"
                            : "+r" (temp)
                            : 
                            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                              "memory", "cc"
                        );
                        
                        key_result = temp + 1;
                        
                        /* Conditional goto using volatile pointer */
                        if (vctrl3 > 2.0) {
                            goto *volatile_label_ptr;
                        }
                    }
                    break;
                    
                case 1:
                    /* Deeply nested case 1 - different mode usage */
                    {
                        double dtemp = (double)key_result;
                        dtemp = external_func2(dtemp);
                        
                        /* Mixed mode computation */
                        key_result = (int)(dtemp * 100.0);
                        
                        /* More register pressure */
                        asm volatile (
                            "/* More register clobbering */\n\t"
                            : 
                            : 
                            : "eax", "ebx", "ecx", "edx", "esi", "edi",
                              "xmm0", "xmm1", "xmm2", "xmm3",
                              "xmm4", "xmm5", "xmm6", "xmm7",
                              "memory"
                        );
                    }
                    break;
                    
                case 2:
                    /* Deeply nested case 2 - pointer modes */
                    {
                        void* ptemp = external_func3(&key_result);
                        int* iptr = (int*)ptemp;
                        
                        /* Access through pointer */
                        key_result = *iptr + key_result;
                        
                        /* Unpredictable control flow */
                        if (volatile_func_ptr) {
                            key_result = volatile_func_ptr(key_result);
                        }
                    }
                    break;
                    
                case 3:
                    /* Deeply nested case 3 - mixed integer sizes */
                    {
                        char ctemp = (char)key_result;
                        short stemp = (short)key_result;
                        long long lltemp = (long long)key_result;
                        
                        /* Operations forcing different modes */
                        key_result = (int)((ctemp * stemp) + (lltemp >> 32));
                        
                        /* Another inline asm barrier */
                        asm volatile (
                            "/* Barrier */\n\t"
                            : 
                            : 
                            : "memory"
                        );
                    }
                    break;
            }
            
            /* After switch, use key_result again */
            key_result += var4 + var5;
            
        } else {
            /* Alternative path */
            key_result -= var6 + var7;
            
            /* Jump to label based on volatile condition */
            if (vctrl1 & 0x1) {
                goto *labels[3];
            }
        }
        
        /* Common code after inner if */
        key_result |= var8;
        
    } else {
        /* Alternative outer path */
        key_result = var9 ^ var10;
        
        /* Loop to create more register pressure */
        for (int i = 0; i < 10; i++) {
            key_result += i;
            
            /* Function call in loop creates live range splits */
            if (i & 0x1) {
                key_result = external_func1(key_result);
            }
        }
    }
    
    /* Define labels for goto targets */
    label_0:
        key_result += 1000;
        goto label_end;
    
    label_1:
        key_result += 2000;
        goto label_end;
    
    label_2:
        key_result += 3000;
        goto label_end;
    
    label_3:
        key_result += 4000;
        goto label_end;
    
    label_4:
        key_result += 5000;
        goto label_end;
    
    label_5:
        key_result += 6000;
        goto label_end;
    
    label_6:
        key_result += 7000;
        goto label_end;
    
    label_7:
        key_result += 8000;
        /* fall through */
    
    label_end:
    
    /* Final computation using all variables to keep them live */
    int final_result = key_result;
    final_result += var1 * var2;
    final_result += (int)(fvar1 * fvar2);
    final_result += (int)(dvar1 / dvar2);
    final_result += cvar1 + svar1;
    final_result += (int)(llvar1 >> 32);
    final_result += *ptr1 + *ptr2;
    final_result += (int)(*fptr1);
    final_result += (int)(*dptr1);
    
    /* One more volatile barrier */
    asm volatile ("" : : : "memory");
    
    return final_result;
}

/* Main function to drive the test */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Call stress function multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        g_volatile_counter = i;
        g_volatile_double = 3.14159 * i;
        g_volatile_ptr = (void*)(uintptr_t)i;
        
        /* Alternate function pointer target */
        if (i % 2 == 0) {
            volatile_func_ptr = external_func1;
        }
        
        int r = stress_function(i);
        result += r;
        
        /* Prevent optimization of loop */
        asm volatile ("" : : "r"(result) : "memory");
    }
    
    printf("Final result: %d\n", result);
    
    /* Additional test with different optimization barriers */
    {
        /* Test with mixed data types in conditional expression */
        volatile int cond = argc > 1 ? 1 : 0;
        
        if (cond) {
            /* Create a scenario with different register modes */
            char small = 42;
            short medium = 1000;
            int normal = 100000;
            long long large = 10000000000LL;
            float f = 3.14f;
            double d = 2.71828;
            
            /* Complex expression forcing different modes */
            int complex_result = (int)((small * medium) + 
                                      (normal / 3) + 
                                      (int)(large >> 16) + 
                                      (int)(f * 100.0f) + 
                                      (int)(d * 10.0));
            
            /* Use in inline asm with clobbers */
            asm volatile (
                "/* Mixed mode clobber test */\n\t"
                : "+r" (complex_result)
                : 
                : "r0", "r1", "r2", "r3", "r4", "r5",
                  "s0", "s1", "s2", "s3", "s4", "s5",
                  "memory", "cc"
            );
            
            result += complex_result;
        }
    }
    
    return result & 0xFF;
}
