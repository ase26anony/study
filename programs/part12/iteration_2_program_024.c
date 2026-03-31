/* reload_coverage.c - Program to exercise GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Packed struct to force unaligned accesses and potential secondary reloads */
struct __attribute__((packed)) PackedStruct {
    double d;
    int i;
    float f;
    long l;
    char c;
};

/* Volatile flag for conditional execution */
volatile int reload_flag = 1;

/* Target function with high register pressure */
__attribute__((noinline))
static int force_reloads(int N, int init_val) {
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
    ps.d = da;
    ps.i = a;
    ps.f = fa;
    ps.l = la;
    ps.c = 'R';
    
    /* Volatile pointer to packed struct */
    volatile struct PackedStruct *volatile_ps = &ps;
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 131 + j * 17) % 256;
        }
    }
    
    /* Main computation loop - creates complex data dependencies */
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N && j < 127; j++) {
            /* Complex array access pattern requiring address reloads */
            int temp = arr[i][j] + arr[j][i];
            
            /* Inline assembly with conflicting constraints */
            /* Force input reload with "r" constraint */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(temp)          /* read-write operand */
                : "r"(arr[i-1][j])    /* input in register */
                : "cc"
            );
            
            /* Another asm with memory constraint */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(temp)
                : "m"(arr[j][i-1])    /* memory operand */
                : "cc"
            );
            
            arr[i][j] = temp;
            
            /* Chain of arithmetic operations keeping many vars live */
            a = b + c;
            b = c + d;
            c = d + e;
            d = e + f;
            e = f + g;
            f = g + h;
            g = h + a;
            h = a + b;
            
            /* Floating point computations */
            fa = fb + fc;
            fb = fc + fd;
            fc = fd + fa;
            fd = fa + fb;
            
            /* Double computations */
            da = db + dc;
            db = dc + dd;
            dc = dd + da;
            dd = da + db;
            
            /* Long computations */
            la = lb + lc;
            lb = lc + ld;
            lc = ld + la;
            ld = la + lb;
            
            /* Conditional block for optional reloads */
            if (reload_flag) {
                /* Use different subset of variables */
                asm volatile (
                    "imull %1, %0\n\t"
                    : "+r"(a)
                    : "r"(b)
                    : "cc"
                );
                
                /* Access packed struct through volatile pointer */
                volatile_ps->i = a;
                volatile_ps->d = da;
                
                /* Force memory reload */
                asm volatile (
                    "movl %1, %0\n\t"
                    : "=r"(c)
                    : "m"(volatile_ps->i)
                    : 
                );
            } else {
                /* Alternative path with different variable usage */
                asm volatile (
                    "addq %1, %0\n\t"
                    : "+r"(la)
                    : "r"(lb)
                    : "cc"
                );
            }
            
            /* More inline asm with tied operands */
            asm volatile (
                "addl %2, %1\n\t"
                "movl %1, %0\n\t"
                : "=r"(d), "+r"(e)
                : "r"(f)
                : "cc"
            );
            
            /* Force spill/reload by using all variables in computation */
            int complex_expr = a + b + c + d + e + f + g + h;
            float fcomplex = fa + fb + fc + fd;
            double dcomplex = da + db + dc + dd;
            long lcomplex = la + lb + lc + ld;
            
            /* Use results to prevent elimination */
            arr[i][j] += complex_expr;
            volatile_ps->f = fcomplex;
            if (i % 32 == 0) {
                volatile_ps->d = dcomplex;
                volatile_ps->l = lcomplex;
            }
        }
        
        /* Cross-iteration dependencies */
        if (i > 1) {
            a = arr[i-1][i-2] + arr[i-2][i-1];
        }
    }
    
    /* Compute checksum using all variables */
    int checksum = a + b + c + d + e + f + g + h;
    checksum += (int)fa + (int)fb + (int)fc + (int)fd;
    checksum += (int)da + (int)db + (int)dc + (int)dd;
    checksum += (int)la + (int)lb + (int)lc + (int)ld;
    
    /* Include array in checksum */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += arr[i][j];
        }
    }
    
    return checksum;
}

/* Another function to create different reload patterns */
__attribute__((noinline))
static int secondary_reload_test(int N) {
    /* Variables with different types to force different register classes */
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0, d5 = 5.0;
    float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f, f4 = 4.5f;
    long long ll1 = 100, ll2 = 200, ll3 = 300, ll4 = 400;
    
    /* Array with complex indexing */
    double darr[100];
    for (int i = 0; i < 100; i++) {
        darr[i] = i * 1.234;
    }
    
    /* Loop with mixed computations */
    for (int i = 0; i < N; i++) {
        /* Force reloads with inline asm using specific constraints */
        asm volatile (
            "addsd %1, %0\n\t"
            : "+x"(d1)        /* xmm register constraint */
            : "x"(d2)
        );
        
        asm volatile (
            "addss %1, %0\n\t"
            : "+x"(f1)
            : "x"(f2)
        );
        
        /* Memory access with computed index */
        int idx = (i * 37) % 100;
        d3 = darr[idx] + darr[(idx + 50) % 100];
        
        /* Chain computations */
        d4 = d1 * d2 + d3;
        d5 = d4 / (d1 + 1.0);
        
        f3 = f1 + f2;
        f4 = f3 * 2.0f - f1;
        
        ll3 = ll1 + ll2;
        ll4 = ll3 * 2 - ll1;
        
        /* Update array */
        darr[i % 100] = d4 + d5 + f3 + f4 + ll3 + ll4;
    }
    
    return (int)(d1 + d2 + d3 + d4 + d5 + f1 + f2 + f3 + f4 + ll1 + ll2 + ll3 + ll4);
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    if (N < 10) N = 10;
    if (N > 1000) N = 1000;
    
    int seed = (argc > 2) ? atoi(argv[2]) : 12345;
    srand(seed);
    
    int init_val = rand() % 1000;
    
    printf("Starting reload coverage test with N=%d, init=%d\n", N, init_val);
    
    /* Call functions that create different reload patterns */
    int result1 = force_reloads(N, init_val);
    printf("Result1: %d\n", result1);
    
    int result2 = secondary_reload_test(N);
    printf("Result2: %d\n", result2);
    
    /* Final checksum */
    int final = result1 + result2;
    printf("Final checksum: %d\n", final);
    
    return final != 0 ? 0 : 1;
}
