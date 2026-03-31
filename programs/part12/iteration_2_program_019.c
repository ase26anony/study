#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and potential secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    float f;
    long l;
    char c;
};

/* Volatile variables to prevent optimization */
volatile int global_flag = 1;
volatile int *volatile volatile_ptr;

/* Target function that will induce many reloads */
__attribute__((noinline))
static long reload_inducing_function(int N, int init_val) {
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
    
    long la = init_val * 100L;
    long lb = init_val * 200L;
    long lc = init_val * 300L;
    long ld = init_val * 400L;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for unaligned accesses */
    struct Packed packed_arr[64];
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 7919 + j * 65537) & 0xFF;
        }
    }
    
    for (int i = 0; i < 64; i++) {
        packed_arr[i].d = i * 3.14159;
        packed_arr[i].i = i * 17;
        packed_arr[i].f = i * 2.71828f;
        packed_arr[i].l = i * 1000L;
        packed_arr[i].c = i & 0xFF;
    }
    
    /* Complex nested loops with many live variables */
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N; j++) {
            /* Create complex addressing with multiple live variables */
            int idx1 = (i * a + j * b) & 127;
            int idx2 = (i * c + j * d) & 127;
            
            /* Force address reloads with array accesses */
            int temp1 = arr[idx1][idx2];
            int temp2 = arr[idx2][idx1];
            
            /* Chain arithmetic operations to keep variables live */
            a = b + c * temp1;
            b = c + d * temp2;
            c = d + e * a;
            d = e + f * b;
            e = f + g * c;
            f = g + h * d;
            g = h + a * e;
            h = a + b * f;
            
            /* Float operations */
            fa = fb + fc * temp1;
            fb = fc + fd * temp2;
            fc = fd + fa * a;
            fd = fa + fb * b;
            
            /* Double operations */
            da = db + dc * temp1;
            db = dc + dd * temp2;
            dc = dd + da * a;
            dd = da + db * b;
            
            /* Long operations */
            la = lb + lc * temp1;
            lb = lc + ld * temp2;
            lc = ld + la * a;
            ld = la + lb * b;
            
            /* Inline assembly with conflicting constraints */
            /* Force input/output reloads with tied operands */
            asm volatile (
                "addl %1, %0\n\t"
                "addl %2, %0"
                : "+r" (a), "+r" (b)
                : "r" (c)
                : "cc"
            );
            
            /* Another asm with memory constraint */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0"
                : "+m" (arr[i & 127][j & 127])
                : "r" (d)
                : "%eax", "cc"
            );
            
            /* Float asm with constraints */
            asm volatile (
                "addss %1, %0\n\t"
                "mulss %2, %0"
                : "+x" (fa)
                : "x" (fb), "x" (fc)
            );
            
            /* Conditional block for optional reloads */
            if (global_flag & 1) {
                /* Use different variables inside conditional */
                int t1 = a + b;
                int t2 = c + d;
                
                /* More asm with constraints */
                asm volatile (
                    "imull %1, %0"
                    : "+r" (t1)
                    : "r" (t2)
                    : "cc"
                );
                
                /* Access packed struct through volatile pointer */
                volatile_ptr = &packed_arr[(i * j) & 63].i;
                *volatile_ptr = t1;
                
                /* Chain more operations */
                a = t1 + t2;
                b = t1 - t2;
            } else {
                /* Alternative path with different variables */
                float ft1 = fa + fb;
                float ft2 = fc + fd;
                
                asm volatile (
                    "mulss %1, %0"
                    : "+x" (ft1)
                    : "x" (ft2)
                );
                
                volatile_ptr = &packed_arr[(i * j) & 63].i;
                *volatile_ptr = (int)ft1;
                
                fa = ft1 + ft2;
                fb = ft1 - ft2;
            }
            
            /* Access packed struct member - may need secondary reload */
            packed_arr[(i + j) & 63].d = da + db;
            packed_arr[(i - j) & 63].i = a + b;
            
            /* More complex array access pattern */
            arr[(i * 3) & 127][(j * 5) & 127] = 
                arr[(i * 7) & 127][(j * 11) & 127] + 
                arr[(i * 13) & 127][(j * 17) & 127];
        }
    }
    
    /* Compute checksum using all variables */
    long checksum = 0;
    checksum += a + b + c + d + e + f + g + h;
    checksum += (long)(fa + fb + fc + fd);
    checksum += (long)(da + db + dc + dd);
    checksum += la + lb + lc + ld;
    
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            checksum += arr[i][j];
        }
    }
    
    for (int i = 0; i < 64; i++) {
        checksum += (long)packed_arr[i].d;
        checksum += packed_arr[i].i;
        checksum += (long)packed_arr[i].f;
        checksum += packed_arr[i].l;
        checksum += packed_arr[i].c;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    int init_val = rand() % 1000 + 1;
    
    /* Call reload-inducing function multiple times */
    long total_checksum = 0;
    for (int iter = 0; iter < 3; iter++) {
        total_checksum += reload_inducing_function(N + iter, init_val + iter);
    }
    
    printf("Checksum: %ld\n", total_checksum);
    return 0;
}
