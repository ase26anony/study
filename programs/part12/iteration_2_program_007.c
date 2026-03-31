/* reload_coverage.c - Program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) PackedStruct {
    double d;
    int i;
    float f;
    long l;
    char c;
    short s;
};

/* Volatile variables to prevent optimization */
volatile int volatile_flag = 1;
volatile int* volatile_ptr;

/* Target function that will create register pressure and reloads */
__attribute__((noinline))
static int create_reloads(int N, int init_val) {
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
    
    double da = init_val * 1.11;
    double db = init_val * 2.22;
    double dc = init_val * 3.33;
    double dd = init_val * 4.44;
    
    long la = init_val * 10L;
    long lb = init_val * 20L;
    long lc = init_val * 30L;
    long ld = init_val * 40L;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for unaligned accesses */
    struct PackedStruct ps;
    ps.d = da;
    ps.i = a;
    ps.f = fa;
    ps.l = la;
    ps.c = (char)init_val;
    ps.s = (short)init_val;
    
    /* Volatile pointer to packed struct */
    volatile struct PackedStruct* vps = &ps;
    
    /* Initialize array with non-constant pattern */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 7919 + j * 65537) & 0xFF;
        }
    }
    
    /* Main computation loop - creates register pressure */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1 && j < 127; j++) {
            /* Complex array access pattern requiring address reloads */
            int temp1 = arr[i][j];
            int temp2 = arr[j][i];
            int temp3 = arr[i-1][j+1];
            int temp4 = arr[i+1][j-1];
            
            /* Chain computations to keep variables live */
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
            
            da = db + dc;
            db = dc + dd;
            dc = dd + da;
            dd = da + db;
            
            la = lb + lc;
            lb = lc + ld;
            lc = ld + la;
            ld = la + lb;
            
            /* Inline assembly with conflicting constraints */
            /* Force input reloads with "r" constraints */
            asm volatile (
                "addl %1, %0\n\t"
                "addl %2, %0"
                : "+r" (a)      /* Read-write operand, tied */
                : "r" (b), "r" (c)
                : "cc"
            );
            
            /* Another asm with memory constraint */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0"
                : "+m" (arr[i][j])  /* Memory output */
                : "r" (temp1)       /* Register input */
                : "%eax", "cc"
            );
            
            /* Asm with tied operand (forces reload) */
            asm volatile (
                "imull %1, %0"
                : "+0" (b)      /* Tied to input 0 */
                : "r" (c)
                : "cc"
            );
            
            /* Floating point asm */
            asm volatile (
                "addss %1, %0"
                : "+x" (fa)     /* xmm register constraint */
                : "x" (fb)
            );
            
            /* Double precision asm */
            asm volatile (
                "addsd %1, %0"
                : "+x" (da)
                : "x" (db)
            );
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different variables inside conditional */
                int t1 = a + b;
                int t2 = c + d;
                float ft1 = fa + fb;
                double dt1 = da + db;
                
                /* Access packed struct through volatile pointer */
                vps->i = t1;
                vps->f = ft1;
                
                /* More asm in conditional path */
                asm volatile (
                    "subl %1, %0"
                    : "+r" (t1)
                    : "r" (t2)
                    : "cc"
                );
                
                a = t1 + t2;
                fa = ft1 + vps->f;
            } else {
                /* Alternative path with different variable usage */
                int t3 = e + f;
                int t4 = g + h;
                
                asm volatile (
                    "xorl %1, %0"
                    : "+r" (t3)
                    : "r" (t4)
                    : "cc"
                );
                
                b = t3 ^ t4;
            }
            
            /* Complex array update with address computation */
            arr[j][i] = arr[i][j] + temp2 + temp3 - temp4;
            
            /* Access packed struct member (unaligned, may need secondary reload) */
            ps.i = a + b;
            ps.f = fa + fb;
            ps.d = da + db;
            ps.l = la + lb;
            
            /* Use packed struct values */
            c = ps.i + (int)ps.l;
            fc = ps.f * 2.0f;
            dc = ps.d / 2.0;
        }
        
        /* Occasionally update volatile flag */
        if ((i & 0xFF) == 0) {
            volatile_flag = i & 1;
        }
    }
    
    /* Final computation using all variables to prevent elimination */
    int sum = a + b + c + d + e + f + g + h;
    sum += (int)fa + (int)fb + (int)fc + (int)fd;
    sum += (int)da + (int)db + (int)dc + (int)dd;
    sum += (int)la + (int)lb + (int)lc + (int)ld;
    
    /* Add array checksum */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            sum += arr[i][j];
        }
    }
    
    /* Add packed struct checksum */
    sum += ps.i + (int)ps.f + (int)ps.d + (int)ps.l + ps.c + ps.s;
    
    return sum;
}

/* Another function with different register pressure pattern */
__attribute__((noinline))
static int more_reloads(int N, int seed) {
    /* Different set of variables */
    int x1 = seed, x2 = seed * 2, x3 = seed * 3, x4 = seed * 4;
    int x5 = seed * 5, x6 = seed * 6, x7 = seed * 7, x8 = seed * 8;
    int x9 = seed * 9, x10 = seed * 10, x11 = seed * 11, x12 = seed * 12;
    
    double y1 = seed * 1.5, y2 = seed * 2.5, y3 = seed * 3.5, y4 = seed * 4.5;
    float z1 = seed * 0.5f, z2 = seed * 1.5f, z3 = seed * 2.5f, z4 = seed * 3.5f;
    
    int small_arr[8][8];
    
    /* Initialize */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            small_arr[i][j] = (i * 13 + j * 17) & 0xF;
        }
    }
    
    for (int i = 0; i < N && i < 100; i++) {
        /* Many interdependent computations */
        x1 = x2 + x3;
        x2 = x3 + x4;
        x3 = x4 + x5;
        x4 = x5 + x6;
        x5 = x6 + x7;
        x6 = x7 + x8;
        x7 = x8 + x9;
        x8 = x9 + x10;
        x9 = x10 + x11;
        x10 = x11 + x12;
        x11 = x12 + x1;
        x12 = x1 + x2;
        
        y1 = y2 + y3;
        y2 = y3 + y4;
        y3 = y4 + y1;
        y4 = y1 + y2;
        
        z1 = z2 + z3;
        z2 = z3 + z4;
        z3 = z4 + z1;
        z4 = z1 + z2;
        
        /* Asm with multiple constraints */
        asm volatile (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0"
            : "=r" (x1)
            : "r" (x2), "r" (x3)
            : "%eax", "cc"
        );
        
        /* Memory-to-memory asm (forces reloads) */
        int idx = i & 7;
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0"
            : "+m" (small_arr[idx][idx])
            : "r" (x4)
            : "%eax", "cc"
        );
        
        /* Use all variables in condition */
        if (x1 > x2 || y1 > y2 || z1 > z2) {
            x12 = x1 - x2;
            y4 = y1 - y2;
            z4 = z1 - z2;
        }
    }
    
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 + x11 + x12 +
           (int)y1 + (int)y2 + (int)y3 + (int)y4 +
           (int)z1 + (int)z2 + (int)z3 + (int)z4;
}

int main(int argc, char** argv) {
    /* Use command line argument for loop bound */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int seed = (argc > 2) ? atoi(argv[2]) : 12345;
    
    if (N < 10) N = 10;
    if (N > 1000) N = 1000;
    
    printf("Starting reload coverage test with N=%d, seed=%d\n", N, seed);
    
    /* Call functions to create reload scenarios */
    int result1 = create_reloads(N, seed);
    printf("Result 1: %d\n", result1);
    
    int result2 = more_reloads(N, seed + 1);
    printf("Result 2: %d\n", result2);
    
    /* Final checksum */
    int final_result = result1 + result2;
    printf("Final result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
