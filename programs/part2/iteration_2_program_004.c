/* early_remat_test.c
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early_remat_test.c -o test
 * For 32-bit: gcc -O2 -m32 -march=i686 -fno-dse -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat early_remat_test.c -o test32
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) {
    volatile int dummy = x;
    return dummy + 1;
}

__attribute__((noinline)) float external_func2(float x) {
    volatile float dummy = x;
    return dummy * 2.0f;
}

__attribute__((noinline)) double external_func3(double x) {
    volatile double dummy = x;
    return dummy / 3.0;
}

/* Volatile function pointer to prevent inlining */
volatile void (*volatile_fptr)(void);

/* Stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    float f1 = seed * 3.14f;
    double d1 = seed * 2.71828;
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    long long ll1 = seed * 1000LL;
    int v3, v4, v5, v6, v7, v8, v9, v10;
    float f2, f3, f4;
    double d2, d3;
    
    /* Intermediate result that will be used conditionally */
    int intermediate = 0;
    
    /* Complex computation creating register pressure */
    v3 = v1 + v2;
    v4 = v2 * v3;
    v5 = v3 - v4;
    v6 = v4 / (v5 ? v5 : 1);
    v7 = v5 ^ v6;
    v8 = v6 | v7;
    v9 = v7 & v8;
    v10 = v8 << 3;
    
    f2 = f1 * 2.0f;
    f3 = f2 + 1.5f;
    f4 = f3 / 0.75f;
    
    d2 = d1 * 1.414;
    d3 = d2 / 0.707;
    
    /* Key intermediate computation - this will be targeted for rematerialization */
    intermediate = v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    intermediate += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    intermediate += (int)d1 + (int)d2 + (int)d3;
    intermediate += c1 + s1 + (int)(ll1 & 0xFFFFFFFF);
    
    /* Create label addresses for goto */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label = labels[seed % 5];
    
    /* Complex nested conditional structure */
    if (v1 > 100) {
        if (v2 < 200) {
            switch (v3 % 4) {
                case 0:
                    /* Use intermediate in inline asm with many clobbers */
                    asm volatile (
                        "add %0, %0, #1\n\t"
                        : "+r" (intermediate)
                        : 
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                          "memory"
                    );
                    break;
                case 1:
                    /* Call non-inlineable function using intermediate */
                    intermediate = external_func1(intermediate);
                    break;
                case 2:
                    /* Use intermediate with floating point conversion */
                    f1 = (float)intermediate;
                    intermediate = (int)external_func2(f1);
                    break;
                case 3:
                    /* Jump using volatile function pointer */
                    if (volatile_fptr) {
                        volatile_fptr();
                        intermediate += 1000;
                    }
                    break;
            }
            
            /* Deeply nested condition */
            if (f4 > 10.0f) {
                for (int i = 0; i < 3; i++) {
                    intermediate += i;
                    if (i == 1) {
                        /* Another inline asm with clobbers */
                        asm volatile (
                            "sub %0, %0, #5\n\t"
                            : "+r" (intermediate)
                            :
                            : "r0", "r1", "r2", "r3", "memory"
                        );
                    }
                }
            }
        } else {
            /* Alternative path using intermediate */
            intermediate *= 2;
        }
        
        /* Conditional goto */
        if (v4 & 0x1) {
            goto *volatile_label;
        }
    } else {
        /* Different computation path */
        intermediate -= 500;
    }
    
label1:
    intermediate += 100;
    goto end;
    
label2:
    intermediate += 200;
    goto end;
    
label3:
    intermediate += 300;
    goto end;
    
label4:
    intermediate += 400;
    goto end;
    
label5:
    intermediate += 500;
    /* Fall through */
    
end:
    /* Use intermediate again after conditional blocks */
    int result = intermediate;
    
    /* More computations to keep variables live */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += (int)(f1 + f2 + f3 + f4);
    result += (int)(d1 + d2 + d3);
    result += c1 + s1 + (int)(ll1 >> 32);
    
    /* Final barrier */
    asm volatile ("" : : "r"(result) : "memory");
    
    return result;
}

/* Another function with mixed data types */
__attribute__((noinline))
long long mixed_type_test(int base) {
    volatile char c = base & 0xFF;
    volatile short s = base & 0x7FFF;
    volatile int i = base * 100;
    volatile long long ll = (long long)base * 1000000LL;
    volatile float f = base * 3.14159f;
    volatile double d = base * 2.718281828459045;
    
    /* Complex conditional use of different types */
    long long accumulator = 0;
    
    if (c > 50) {
        accumulator += c;
        if (s > 1000) {
            accumulator += s;
            switch (i % 5) {
                case 0: accumulator += i; break;
                case 1: accumulator += (long long)f; break;
                case 2: accumulator += (long long)d; break;
                case 3: accumulator += ll; break;
                case 4: accumulator += i * 2; break;
            }
            
            /* Inline asm with register clobbering */
            asm volatile (
                "mov r0, %0\n\t"
                "add r0, r0, #100\n\t"
                "mov %0, r0\n\t"
                : "+r" (accumulator)
                :
                : "r0", "r1", "r2", "r3", "r4", "memory"
            );
        }
    }
    
    /* Use accumulator outside conditional */
    accumulator += i + ll;
    
    return accumulator;
}

int main(int argc, char** argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* Set volatile function pointer */
    volatile_fptr = (void (*)(void))&external_func1;
    
    /* Call stress function multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += stress_function(seed + i);
        total += mixed_type_test(seed + i * 100);
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
