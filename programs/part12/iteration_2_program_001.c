/* reload_test.c - Program to force GCC reload pass to initialize all reload struct fields */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) Packed {
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
static long long force_reloads(int N, int seed) {
    /* Declare many scalar variables of mixed types */
    int a = seed + 1, b = seed + 2, c = seed + 3, d = seed + 4;
    int e = seed + 5, f = seed + 6, g = seed + 7, h = seed + 8;
    float fa = seed * 1.1f, fb = seed * 1.2f, fc = seed * 1.3f, fd = seed * 1.4f;
    double da = seed * 2.1, db = seed * 2.2, dc = seed * 2.3, dd = seed * 2.4;
    long la = seed * 3L, lb = seed * 4L, lc = seed * 5L, ld = seed * 6L;
    int extra1 = seed * 7, extra2 = seed * 8, extra3 = seed * 9;
    float extra4 = seed * 3.14f;
    double extra5 = seed * 6.28;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for unaligned accesses */
    struct Packed packed_arr[64];
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 7919 + j * 65537 + seed) & 0xFF;
        }
    }
    
    for (int i = 0; i < 64; i++) {
        packed_arr[i].d = (i * 2.71828 + seed);
        packed_arr[i].i = (i * 314159 + seed);
        packed_arr[i].c = (i + seed) & 0xFF;
        packed_arr[i].l = (i * 2718281L + seed);
        packed_arr[i].f = (i * 1.41421f + seed);
    }
    
    /* Complex nested loops with many live variables */
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N; j++) {
            /* Create data dependencies between variables */
            a = b + c;
            b = c + d;
            c = d + e;
            d = e + f;
            e = f + g;
            f = g + h;
            g = h + a;
            
            /* Floating point chains */
            fa = fb + fc;
            fb = fc + fd;
            fc = fd + fa;
            fd = fa * 1.1f;
            
            /* Double precision chains */
            da = db + dc;
            db = dc + dd;
            dc = dd * 1.01;
            dd = da * 0.99;
            
            /* Long integer chains */
            la = lb + lc;
            lb = lc + ld;
            lc = ld * 3;
            ld = la / 2;
            
            /* Array access with complex addressing - forces address reloads */
            int idx1 = (i * 17 + j * 23) & 127;
            int idx2 = (i * 29 + j * 31) & 127;
            
            /* Swap array elements using many temporary variables */
            int temp1 = arr[idx1][idx2];
            int temp2 = arr[idx2][idx1];
            arr[idx1][idx2] = temp2 + a;
            arr[idx2][idx1] = temp1 + b;
            
            /* Access packed struct through volatile pointer - forces secondary reloads */
            volatile struct Packed *volatile_ptr = &packed_arr[(i + j) & 63];
            extra1 = volatile_ptr->i;
            extra4 = volatile_ptr->f;
            
            /* Inline assembly with conflicting constraints */
            /* Force input reloads with tied operands */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(extra1), "+r"(extra2)
                : "r"(extra3)
                : "cc"
            );
            
            /* Another asm with memory constraint */
            asm volatile (
                "mov %1, %%eax\n\t"
                "add %%eax, %0\n\t"
                : "+m"(arr[i & 127][j & 127])
                : "r"(extra1)
                : "%eax", "cc"
            );
            
            /* Floating point asm with constraints */
            asm volatile (
                "addss %1, %0\n\t"
                : "+x"(extra4)
                : "x"(fa)
                : 
            );
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different subset of variables */
                extra2 = arr[(i * 11) & 127][(j * 13) & 127] + extra1;
                extra3 = packed_arr[(i * 7) & 63].i;
                
                /* More asm with specific register constraints */
                register int r1 asm("ebx") = extra2;
                register int r2 asm("ecx") = extra3;
                asm volatile (
                    "imull %1, %0\n\t"
                    : "+r"(r1)
                    : "r"(r2)
                    : "cc"
                );
                extra2 = r1;
            } else {
                /* Alternative path with different variables */
                extra5 = packed_arr[(j * 5) & 63].d;
                da = extra5 * db;
            }
            
            /* Chain all variables together to keep them live */
            h = a + b + c + d + e + f + g + extra1 + extra2 + extra3;
            fa = fa + fb + fc + fd + extra4;
            da = da + db + dc + dd + extra5;
            la = la + lb + lc + ld + h;
        }
        
        /* Update volatile index to prevent loop optimizations */
        volatile_index = i;
    }
    
    /* Compute checksum using all variables to prevent dead code elimination */
    long long checksum = 0;
    checksum += a + b + c + d + e + f + g + h;
    checksum += (long long)(fa * 100) + (long long)(fb * 100) + 
                (long long)(fc * 100) + (long long)(fd * 100);
    checksum += (long long)(da * 100) + (long long)(db * 100) + 
                (long long)(dc * 100) + (long long)(dd * 100);
    checksum += la + lb + lc + ld;
    checksum += extra1 + extra2 + extra3 + (long long)(extra4 * 100) + 
                (long long)(extra5 * 100);
    
    /* Add array elements to checksum */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += arr[i][j];
        }
    }
    
    /* Add packed struct elements */
    for (int i = 0; i < 8; i++) {
        checksum += packed_arr[i].i + (long long)packed_arr[i].d + 
                   packed_arr[i].l + (long long)(packed_arr[i].f * 100);
    }
    
    return checksum;
}

/* Wrapper function to increase register pressure further */
__attribute__((noinline))
static long long wrapper_function(int N, int iterations) {
    long long total = 0;
    for (int iter = 0; iter < iterations; iter++) {
        total += force_reloads(N, iter * 12345);
        
        /* Add more register pressure between calls */
        int x1 = iter * 3, x2 = iter * 5, x3 = iter * 7;
        double y1 = iter * 1.234, y2 = iter * 5.678;
        float z1 = iter * 2.34f, z2 = iter * 6.78f;
        
        /* Force these to be used */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r"(x1)
            : "r"(x2)
            : "cc"
        );
        
        total += x1 + x2 + x3 + (long long)(y1 * 100) + 
                (long long)(y2 * 100) + (long long)(z1 * 100) + 
                (long long)(z2 * 100);
    }
    return total;
}

int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int iterations = (argc > 2) ? atoi(argv[2]) : 5;
    
    if (N > 100) N = 100;  /* Prevent excessive runtime */
    if (iterations > 10) iterations = 10;
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    int seed = rand();
    
    printf("Starting reload test with N=%d, iterations=%d, seed=%d\n", 
           N, iterations, seed);
    
    /* Call the reload-intensive function */
    long long result = wrapper_function(N, iterations);
    
    printf("Result checksum: %lld\n", result);
    printf("Volatile index: %d\n", volatile_index);
    
    return 0;
}
