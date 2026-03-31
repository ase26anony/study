/* reload_coverage.c - Program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) PackedStruct {
    double d;
    int i;
    float f;
    long l;
    char c;
    short s;
};

/* Volatile flag for conditional execution */
volatile int volatile_flag = 0;

/* Target function with high register pressure */
__attribute__((noinline))
unsigned long long trigger_reloads(int N, int init_val) {
    /* Declare many scalar variables to exceed available registers */
    int a = init_val + 1;
    int b = init_val + 2;
    int c = init_val + 3;
    int d = init_val + 4;
    int e = init_val + 5;
    int f = init_val + 6;
    int g = init_val + 7;
    int h = init_val + 8;
    
    float fa = init_val * 1.1f;
    float fb = init_val * 1.2f;
    float fc = init_val * 1.3f;
    float fd = init_val * 1.4f;
    float fe = init_val * 1.5f;
    
    double da = init_val * 2.1;
    double db = init_val * 2.2;
    double dc = init_val * 2.3;
    double dd = init_val * 2.4;
    double de = init_val * 2.5;
    
    long la = init_val * 3L;
    long lb = init_val * 4L;
    long lc = init_val * 5L;
    long ld = init_val * 6L;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for unaligned accesses */
    struct PackedStruct ps;
    ps.d = init_val * 1.5;
    ps.i = init_val;
    ps.f = init_val * 0.5f;
    ps.l = init_val * 10L;
    ps.c = init_val & 0xFF;
    ps.s = init_val & 0xFFFF;
    
    /* Volatile pointer to packed struct */
    volatile struct PackedStruct *volatile_ps = &ps;
    
    /* Initialize array with non-constant pattern */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 7919 + j * 65537) & 0xFF;
        }
    }
    
    /* Main computation loop with high register pressure */
    unsigned long long checksum = 0;
    
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1; j++) {
            /* Complex addressing modes - force address register reloads */
            int idx1 = (i * 17) % 127;
            int idx2 = (j * 23) % 127;
            
            /* Chain of interdependent computations */
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
            fc = fd + fe;
            fd = fe + fa;
            fe = fa + fb;
            
            /* Double computations */
            da = db + dc;
            db = dc + dd;
            dc = dd + de;
            dd = de + da;
            de = da + db;
            
            /* Long computations */
            la = lb + lc;
            lb = lc + ld;
            lc = ld + la;
            ld = la + lb;
            
            /* Inline assembly with conflicting constraints */
            /* Force input/output reloads with tied operands */
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r"(a)
                : "r"(b), "0"(c)
                : "cc"
            );
            
            /* Another asm with different constraints */
            asm volatile (
                "imul %0, %1\n\t"
                : "+r"(d)
                : "r"(e)
                : "cc"
            );
            
            /* Memory constraint to force spills */
            asm volatile (
                "addl %1, %0\n\t"
                : "+m"(arr[idx1][idx2])
                : "r"(f)
                : "cc"
            );
            
            /* Array access with complex addressing - forces address reloads */
            arr[idx2][idx1] = arr[idx1][idx2] + arr[j][i];
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different variables inside conditional */
                int t1 = a + b;
                int t2 = c + d;
                float ft = fa + fb;
                double dt = da + db;
                
                /* More asm in conditional path */
                asm volatile (
                    "sub %0, %1, %2\n\t"
                    : "=r"(t1)
                    : "r"(t2), "0"(g)
                );
                
                arr[i][j] = t1 + t2;
                checksum += t1 + t2 + (int)ft + (int)dt;
            } else {
                /* Different computation in else path */
                int t3 = e + f;
                int t4 = g + h;
                float ft2 = fc + fd;
                double dt2 = dc + dd;
                
                asm volatile (
                    "xor %0, %1, %2\n\t"
                    : "=r"(t3)
                    : "r"(t4), "0"(h)
                );
                
                arr[j][i] = t3 + t4;
                checksum += t3 + t4 + (int)ft2 + (int)dt2;
            }
            
            /* Access packed struct through volatile pointer - may need secondary reloads */
            volatile_ps->i = arr[i][j];
            volatile_ps->f = fa;
            volatile_ps->d = da;
            
            /* More computations using packed struct members */
            a += volatile_ps->i;
            fa += volatile_ps->f;
            da += volatile_ps->d;
            
            /* Update checksum with many variables to keep them live */
            checksum += a + b + c + d + e + f + g + h;
            checksum += (int)fa + (int)fb + (int)fc + (int)fd + (int)fe;
            checksum += (int)da + (int)db + (int)dc + (int)dd + (int)de;
            checksum += la + lb + lc + ld;
            checksum += arr[i][j] + arr[j][i] + arr[idx1][idx2] + arr[idx2][idx1];
        }
        
        /* Occasionally toggle volatile flag */
        if (i % 37 == 0) {
            volatile_flag = !volatile_flag;
        }
    }
    
    /* Final computation using all variables */
    unsigned long long final_result = checksum;
    final_result += a + b + c + d + e + f + g + h;
    final_result += (unsigned long long)fa + (unsigned long long)fb;
    final_result += (unsigned long long)da + (unsigned long long)db;
    final_result += la + lb + lc + ld;
    
    /* Sum array elements */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            final_result += arr[i][j];
        }
    }
    
    return final_result;
}

/* Helper function to create more register pressure */
__attribute__((noinline))
unsigned long long helper_function(int x, int y) {
    int v1 = x * 3;
    int v2 = y * 5;
    int v3 = x + y;
    int v4 = x - y;
    int v5 = x * y;
    int v6 = x ^ y;
    int v7 = x | y;
    int v8 = x & y;
    
    float f1 = x * 1.1f;
    float f2 = y * 1.2f;
    float f3 = f1 + f2;
    float f4 = f1 - f2;
    
    double d1 = x * 2.1;
    double d2 = y * 2.2;
    double d3 = d1 + d2;
    double d4 = d1 - d2;
    
    /* Inline asm with memory constraints */
    asm volatile (
        "mov %1, %%eax\n\t"
        "add %2, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=m"(v1)
        : "r"(v2), "r"(v3)
        : "%eax", "cc"
    );
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
           (unsigned long long)f1 + (unsigned long long)f2 +
           (unsigned long long)d1 + (unsigned long long)d2;
}

int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    if (N > 120) N = 120; /* Prevent stack overflow */
    if (N < 10) N = 10;
    
    /* Initialize random seed */
    srand(time(NULL));
    int init_val = rand() % 1000;
    
    printf("Starting reload coverage test with N=%d, init=%d\n", N, init_val);
    
    /* Call target function multiple times with different parameters */
    unsigned long long total = 0;
    
    for (int iter = 0; iter < 3; iter++) {
        unsigned long long result = trigger_reloads(N + iter, init_val + iter);
        total += result;
        
        /* Call helper to create additional reload contexts */
        total += helper_function(iter * 100, N);
        
        printf("Iteration %d: result = %llu\n", iter, result);
    }
    
    /* Use volatile to prevent optimization */
    volatile unsigned long long final_total = total;
    printf("Final total: %llu\n", final_total);
    
    return (final_total > 0) ? 0 : 1;
}
