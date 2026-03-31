/* reload_trigger.c - Program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and potential secondary reloads */
struct __attribute__((packed)) PackedStruct {
    double d;
    int i;
    char c;
    long l;
    float f;
};

/* Volatile variables to prevent optimization */
volatile int global_flag = 1;
volatile int* volatile_pointer;

/* Target function with high register pressure */
__attribute__((noinline))
static long process_data(int N, int seed) {
    /* Declare many variables to exceed available registers */
    int a = seed + 1, b = seed + 2, c = seed + 3, d = seed + 4;
    int e = seed + 5, f = seed + 6, g = seed + 7, h = seed + 8;
    int i = seed + 9, j = seed + 10, k = seed + 11, l = seed + 12;
    int m = seed + 13, n = seed + 14, o = seed + 15, p = seed + 16;
    
    /* Mixed types to use different register classes */
    float fa = seed * 1.1f, fb = seed * 1.2f, fc = seed * 1.3f;
    double da = seed * 2.1, db = seed * 2.2, dc = seed * 2.3;
    long la = seed * 3L, lb = seed * 4L, lc = seed * 5L;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for unaligned accesses */
    struct PackedStruct ps;
    ps.d = seed * 1.5;
    ps.i = seed;
    ps.c = seed & 0xFF;
    ps.l = seed * 10L;
    ps.f = seed * 0.7f;
    
    /* Initialize array with non-constant pattern */
    for (int x = 0; x < 128; x++) {
        for (int y = 0; y < 128; y++) {
            arr[x][y] = (x * 7919 + y * 104729 + seed) & 0x7FF;
        }
    }
    
    /* Main computation loop - creates complex data dependencies */
    for (int iter = 0; iter < N; iter++) {
        /* Chain computations to keep variables live */
        a = b + c;
        b = c + d;
        c = d + e;
        d = e + f;
        e = f + g;
        f = g + h;
        g = h + i;
        h = i + j;
        i = j + k;
        j = k + l;
        k = l + m;
        l = m + n;
        m = n + o;
        n = o + p;
        o = p + a;
        p = a + b;
        
        /* Float computations */
        fa = fb + fc;
        fb = fc + fa;
        fc = fa * 1.1f - fb;
        
        /* Double computations */
        da = db * 1.01;
        db = dc / 1.02;
        dc = da + db;
        
        /* Long computations */
        la = lb + lc;
        lb = lc - la;
        lc = la * 2 - lb;
        
        /* Complex array access with computed indices - forces address reloads */
        int idx1 = (a + iter) & 127;
        int idx2 = (b + iter * 3) & 127;
        int idx3 = (c + iter * 5) & 127;
        int idx4 = (d + iter * 7) & 127;
        
        /* Array operations that require multiple values live */
        int temp = arr[idx1][idx2];
        arr[idx1][idx2] = arr[idx3][idx4];
        arr[idx3][idx4] = temp;
        
        /* Inline assembly with conflicting constraints */
        /* Force input reloads with "r" constraints */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r" (a)          /* read-write operand */
            : "r" (b)           /* input operand */
            : "cc"
        );
        
        /* Another asm with different constraints */
        asm volatile (
            "imull %1, %0\n\t"
            : "+r" (c)
            : "r" (d)
            : "cc"
        );
        
        /* Asm with tied operands (output = input 0) */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "=r" (e)
            : "0" (f), "r" (g)  /* "0" means same as output 0 */
            : "%eax", "cc"
        );
        
        /* Conditional block for optional reloads */
        if (global_flag & 1) {
            /* Use different variables in conditional path */
            volatile int cond_temp = arr[idx2][idx1];
            
            /* More asm in conditional path */
            asm volatile (
                "subl %1, %0\n\t"
                : "+r" (h)
                : "r" (i)
                : "cc"
            );
            
            /* Access packed struct through volatile pointer */
            struct PackedStruct* volatile ps_ptr = &ps;
            ps_ptr->i = cond_temp;
            
            /* Force memory reload */
            asm volatile (
                "movl %1, %%ebx\n\t"
                "movl %%ebx, %0\n\t"
                : "=m" (ps_ptr->i)
                : "r" (cond_temp)
                : "%ebx", "memory"
            );
        } else {
            /* Alternative path with different variable usage */
            asm volatile (
                "xorl %1, %0\n\t"
                : "+r" (j)
                : "r" (k)
                : "cc"
            );
        }
        
        /* More complex addressing with multiple indices */
        int idx5 = (e + idx1) & 127;
        int idx6 = (f + idx2) & 127;
        
        /* This creates address computation pressure */
        arr[idx5][idx6] = arr[idx6][idx5] + arr[idx1][idx2];
        
        /* Use packed struct members in computation */
        ps.i = a + b;
        ps.f = fa + fb;
        
        /* Force spill/reload by using all variables */
        la = la + a + b + c + d;
        lb = lb + e + f + g + h;
        lc = lc + i + j + k + l;
        
        da = da + fa + fb + fc;
        db = db + ps.d;
        
        /* Prevent loop invariant removal */
        if (iter % 100 == 0) {
            global_flag ^= 1;
        }
    }
    
    /* Compute checksum using all variables */
    long checksum = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    checksum += (long)(fa + fb + fc);
    checksum += (long)(da + db + dc);
    checksum += la + lb + lc;
    checksum += ps.i + ps.l + (long)ps.f;
    
    /* Add array elements to checksum */
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            checksum += arr[x][y];
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 500;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    printf("Running with N=%d, seed=%d\n", N, seed);
    srand(seed);
    
    /* Call the function multiple times to increase pressure */
    long total = 0;
    for (int i = 0; i < 3; i++) {
        total += process_data(N / (i + 1) + 1, seed + i * 1000);
    }
    
    printf("Checksum: %ld\n", total);
    
    /* Use volatile pointer access */
    volatile_pointer = &global_flag;
    *volatile_pointer = total & 1;
    
    return (int)(total & 0x7FFFFFFF);
}
