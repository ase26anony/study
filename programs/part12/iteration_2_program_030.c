/* reload_test.c - Program to force GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) PackedStruct {
    double d;
    int i;
    char c;
    long l;
    float f;
};

/* Volatile variables to prevent optimization */
volatile int volatile_flag = 1;
volatile int volatile_index = 0;

/* Target function with high register pressure */
__attribute__((noinline))
unsigned long long force_reloads(int N, int init_val) {
    /* Declare many scalar variables to exceed available registers */
    int a = init_val + 1;
    int b = init_val * 2;
    int c = init_val / 3;
    int d = init_val - 4;
    int e = init_val + 5;
    int f = init_val * 6;
    int g = init_val / 7;
    int h = init_val - 8;
    
    float fa = init_val * 1.1f;
    float fb = init_val * 2.2f;
    float fc = init_val * 3.3f;
    float fd = init_val * 4.4f;
    
    double da = init_val * 1.111;
    double db = init_val * 2.222;
    double dc = init_val * 3.333;
    double dd = init_val * 4.444;
    
    long la = init_val * 100L;
    long lb = init_val * 200L;
    long lc = init_val * 300L;
    long ld = init_val * 400L;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for unaligned accesses */
    struct PackedStruct ps;
    ps.d = init_val * 1.5;
    ps.i = init_val;
    ps.c = init_val & 0xFF;
    ps.l = init_val * 1000L;
    ps.f = init_val * 2.5f;
    
    /* Volatile pointer to packed struct */
    volatile struct PackedStruct *volatile_ps = &ps;
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 131 + j * 17) & 0xFFF;
        }
    }
    
    /* Main computation loop - creates complex data dependencies */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1 && j < 127; j++) {
            /* Complex array access pattern forcing address reloads */
            int idx1 = (i * j) % 127;
            int idx2 = (i + j) % 127;
            
            /* Inline assembly with conflicting constraints */
            /* Force input reloads with "r" constraints */
            asm volatile (
                "addl %1, %0\n\t"
                "addl %2, %0"
                : "+r"(a)          /* tied output/input */
                : "r"(arr[idx1][idx2]), "r"(b)
                : "cc"
            );
            
            /* Another asm with different constraints */
            asm volatile (
                "imull %1, %0"
                : "+r"(c)
                : "r"(d)
                : "cc"
            );
            
            /* Float/double operations to use FP registers */
            fa = fb * fc + fd;
            da = db - dc * dd;
            
            /* Long operations */
            la = lb + lc - ld;
            
            /* Chain computations to keep variables live */
            e = f + g * h;
            b = c - d + e;
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different variables inside conditional */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "addl %2, %%eax\n\t"
                    "movl %%eax, %0"
                    : "=r"(f)
                    : "r"(g), "r"(h)
                    : "%eax", "cc"
                );
                
                /* Access packed struct through volatile pointer */
                int tmp = volatile_ps->i;
                float ftmp = volatile_ps->f;
                
                /* More computations with packed struct data */
                fa = fa * ftmp + tmp;
                a = a + tmp;
            } else {
                /* Alternative path with different variable usage */
                asm volatile (
                    "subl %1, %0"
                    : "+r"(g)
                    : "r"(h)
                    : "cc"
                );
                
                double dtmp = volatile_ps->d;
                da = da * dtmp;
            }
            
            /* Array manipulation with computed indices */
            /* This forces address register reloads */
            arr[j][i] = arr[i][j] + a;
            arr[idx1][idx2] = arr[idx2][idx1] * b;
            
            /* Update array element using packed struct member */
            arr[i % 127][j % 127] += volatile_ps->i;
            
            /* More arithmetic to create data dependencies */
            h = a * b + c - d;
            g = e * f / (h + 1);
            
            /* Float chain */
            fb = fc * 1.1f + fd;
            fc = fd * 0.9f - fa;
            
            /* Double chain */
            db = dc * 1.01 + dd;
            dc = dd * 0.99 - da;
            
            /* Long chain */
            lb = lc << 2;
            lc = ld >> 1;
            ld = la + lb - lc;
            
            /* Force spill/reload by using all variables */
            volatile_index = i + j;
            if (volatile_index & 1) {
                la = la + volatile_ps->l;
            }
        }
        
        /* Cross-iteration dependencies */
        a = b + c * (i % 10);
        b = c + d * ((i + 1) % 10);
        c = d + e * ((i + 2) % 10);
        
        /* Update packed struct occasionally */
        if (i % 13 == 0) {
            volatile_ps->i = a;
            volatile_ps->f = fa;
        }
    }
    
    /* Final computation using all variables to prevent elimination */
    unsigned long long checksum = 0;
    
    checksum += a + b + c + d + e + f + g + h;
    checksum += (unsigned long long)(fa * 1000);
    checksum += (unsigned long long)(fb * 1000);
    checksum += (unsigned long long)(fc * 1000);
    checksum += (unsigned long long)(fd * 1000);
    checksum += (unsigned long long)(da * 1000);
    checksum += (unsigned long long)(db * 1000);
    checksum += (unsigned long long)(dc * 1000);
    checksum += (unsigned long long)(dd * 1000);
    checksum += la + lb + lc + ld;
    
    /* Include array in checksum */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += arr[i][j];
        }
    }
    
    /* Include packed struct */
    checksum += (unsigned long long)(volatile_ps->d * 1000);
    checksum += volatile_ps->i;
    checksum += volatile_ps->c;
    checksum += volatile_ps->l;
    checksum += (unsigned long long)(volatile_ps->f * 1000);
    
    return checksum;
}

/* Another function to create different reload patterns */
__attribute__((noinline))
double secondary_reload_test(int N) {
    /* Variables that will need secondary reloads */
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0;
    long double ld1 = 10.0L, ld2 = 20.0L;
    int i1 = 100, i2 = 200, i3 = 300, i4 = 400;
    
    /* Packed struct with mixed types */
    struct __attribute__((packed)) MixedPacked {
        long double ld;
        double d;
        int i;
        char c;
    } mp;
    
    mp.ld = 123.456L;
    mp.d = 789.012;
    mp.i = 13579;
    mp.c = 'X';
    
    volatile struct MixedPacked *volatile_mp = &mp;
    
    for (int i = 0; i < N; i++) {
        /* Operations that might require secondary reloads */
        d1 = d2 * d3 + d4;
        d2 = d3 / d4 - d1;
        
        /* Access packed long double - often needs special handling */
        if (i % 7 == 0) {
            ld1 = volatile_mp->ld * 1.1L;
            volatile_mp->d = d1;
        }
        
        /* Inline asm with memory constraints */
        asm volatile (
            "movq %1, %%rax\n\t"
            "addq %2, %%rax\n\t"
            "movq %%rax, %0"
            : "=m"(i1)    /* memory output constraint */
            : "r"(i2), "r"(i3)
            : "%rax", "cc"
        );
        
        /* Another asm with tied register */
        asm volatile (
            "addl %1, %0\n\t"
            "imull %2, %0"
            : "+r"(i2)
            : "r"(i3), "r"(i4)
            : "cc"
        );
        
        /* Update packed struct */
        volatile_mp->i = i1 + i2;
        
        /* Chain computations */
        i3 = i4 * (i % 31);
        i4 = i1 + i2 - i3;
        
        d3 = d4 * (i % 17);
        d4 = d1 + d2 - d3;
    }
    
    return d1 + d2 + d3 + d4 + ld1 + ld2 + mp.d + i1 + i2 + i3 + i4;
}

int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    
    /* Seed RNG for variable initialization */
    srand((unsigned int)time(NULL));
    int init_val = rand() % 1000 + 1;
    
    printf("Starting reload test with N=%d, init_val=%d\n", N, init_val);
    
    /* Call functions that force reloads */
    unsigned long long checksum1 = force_reloads(N, init_val);
    double checksum2 = secondary_reload_test(N / 2);
    
    /* Additional computation to use results */
    double final_result = (double)checksum1 + checksum2;
    
    printf("Checksum1: %llu\n", checksum1);
    printf("Checksum2: %f\n", checksum2);
    printf("Final result: %f\n", final_result);
    
    /* Prevent dead code elimination */
    volatile int dummy = (int)final_result;
    return dummy & 0xFF;
}
