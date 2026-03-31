#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    float f;
    long l;
    char c;
};

/* Volatile flag for conditional execution */
volatile int reload_flag = 1;

/* Target function that will induce many reloads */
__attribute__((noinline))
unsigned long long induce_reloads(int N, int init_val) {
    /* Force many live scalar variables - more than available registers */
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
    struct Packed packed_arr[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 131 + j * 17) % 256;
        }
    }
    
    for (int i = 0; i < 64; i++) {
        packed_arr[i].d = i * 1.234;
        packed_arr[i].i = i * 567;
        packed_arr[i].f = i * 8.91f;
        packed_arr[i].l = i * 1234L;
        packed_arr[i].c = i % 256;
    }
    
    /* Volatile pointer to packed struct to prevent optimization */
    volatile struct Packed *volatile_packed = packed_arr;
    
    /* Main computation loop - creates register pressure and dependencies */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1; j++) {
            /* Complex array access pattern requiring address reloads */
            int temp1 = arr[i][j];
            int temp2 = arr[j][i];
            
            /* Inline assembly with conflicting constraints to force reloads */
            /* Tied operand constraint (0) forces input/output to same register */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(temp1)        /* read-write operand, tied */
                : "r"(temp2)         /* input operand */
                : "cc"
            );
            
            /* Another asm with memory constraint */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+m"(arr[i][j])    /* memory output */
                : "r"(temp1)         /* register input */
                : "%eax", "cc"
            );
            
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
            
            /* Access packed struct through volatile pointer - may need secondary reload */
            int packed_val = volatile_packed[(i + j) % 64].i;
            
            /* Conditional block for optional reloads */
            if (reload_flag) {
                /* Use different subset of variables inside conditional */
                asm volatile (
                    "imull %1, %0\n\t"
                    : "+r"(packed_val)
                    : "r"(a)
                    : "cc"
                );
                
                /* More variable chaining only in conditional path */
                int cond_var = packed_val + b + c;
                float fcond = fa * 1.5f + fb;
                double dcond = da * 1.7 + db;
                
                /* Use results to prevent elimination */
                arr[i][j] += cond_var;
                fa += fcond;
                da += dcond;
            } else {
                /* Alternative path with different variable usage */
                arr[i][j] -= packed_val;
            }
            
            /* Another asm with specific register constraints */
            asm volatile (
                "movl %1, %%edx\n\t"
                "subl %%edx, %0\n\t"
                : "+r"(h)
                : "r"(g)
                : "%edx", "cc"
            );
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    checksum += a + b + c + d + e + f + g + h;
    checksum += (unsigned long long)(fa + fb + fc + fd);
    checksum += (unsigned long long)(da + db + dc + dd);
    checksum += la + lb + lc + ld;
    
    for (int i = 0; i < 64; i++) {
        checksum += packed_arr[i].i;
        checksum += (unsigned long long)packed_arr[i].d;
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            checksum += arr[i][j];
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    
    if (N > 120) N = 120;  /* Prevent stack overflow */
    if (N < 10) N = 10;
    
    srand(time(NULL));
    int init_val = rand() % 1000 + 1;
    
    /* Call reload-inducing function multiple times */
    unsigned long long total_checksum = 0;
    for (int iter = 0; iter < 3; iter++) {
        total_checksum += induce_reloads(N + iter, init_val + iter);
    }
    
    printf("Checksum: %llu\n", total_checksum);
    return 0;
}
