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

/* Volatile variables to prevent optimization */
volatile int global_flag = 1;
volatile int *volatile volatile_ptr;

/* Function with high register pressure and complex operations */
__attribute__((noinline))
unsigned long long trigger_reloads(int N, int seed) {
    /* Declare many variables to exceed available registers */
    int a = seed + 1, b = seed + 2, c = seed + 3, d = seed + 4;
    int e = seed + 5, f = seed + 6, g = seed + 7, h = seed + 8;
    int i = seed + 9, j = seed + 10, k = seed + 11, l = seed + 12;
    float fa = seed * 1.1f, fb = seed * 1.2f, fc = seed * 1.3f, fd = seed * 1.4f;
    double da = seed * 2.1, db = seed * 2.2, dc = seed * 2.3, dd = seed * 2.4;
    long la = seed * 3L, lb = seed * 4L, lc = seed * 5L, ld = seed * 6L;
    
    /* Multi-dimensional array for address reloads */
    int arr[200][200];
    for (int x = 0; x < 200; x++) {
        for (int y = 0; y < 200; y++) {
            arr[x][y] = x * y + seed;
        }
    }
    
    /* Packed struct for unaligned accesses */
    struct Packed packed = { .d = da, .i = a, .f = fa, .l = la, .c = 'X' };
    struct Packed *packed_ptr = &packed;
    
    /* Main computation loop with nested loops */
    for (int iter = 0; iter < N; iter++) {
        /* Complex array access pattern forcing address reloads */
        for (int x = 1; x < 100; x++) {
            for (int y = 1; y < 100; y++) {
                /* Swapping pattern with computed indices */
                int tmp = arr[x][y];
                arr[x][y] = arr[y][x] + arr[x-1][y-1];
                arr[y][x] = tmp - arr[x+1][y+1];
                
                /* Use computed indices for more complex addressing */
                int idx1 = (x * y) % 199;
                int idx2 = (x + y) % 199;
                arr[idx1][idx2] = arr[idx2][idx1] + iter;
            }
        }
        
        /* Chain of arithmetic operations keeping many variables live */
        a = b + c;
        b = c * d;
        c = d - e;
        d = e / (f + 1);
        e = f ^ g;
        f = g | h;
        g = h & i;
        h = i << 2;
        i = j >> 1;
        j = k + l;
        
        /* Floating point operations */
        fa = fb * fc;
        fb = fc + fd;
        fc = fd - fa;
        fd = fa / (fb + 1.0f);
        
        /* Double precision operations */
        da = db * dc;
        db = dc + dd;
        dc = dd - da;
        dd = da / (db + 1.0);
        
        /* Long operations */
        la = lb * lc;
        lb = lc + ld;
        lc = ld - la;
        ld = la / (lb + 1L);
        
        /* Inline assembly with conflicting constraints */
        /* Force input/output reloads with tied operands */
        asm volatile (
            "addl %1, %0\n\t"
            : "=r"(a) : "r"(b), "0"(a) : "cc"
        );
        
        asm volatile (
            "addsd %1, %0\n\t"
            : "=x"(da) : "x"(db), "0"(da) : 
        );
        
        /* Assembly with memory constraints */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "=m"(arr[10][10]) : "r"(c) : "%eax", "memory"
        );
        
        /* Conditional block for optional reloads */
        if (global_flag & 1) {
            /* Use different subset of variables */
            int t1 = a + b;
            int t2 = c + d;
            float t3 = fa + fb;
            double t4 = da + db;
            
            /* More assembly in conditional path */
            asm volatile (
                "imull %1, %0\n\t"
                : "+r"(t1) : "r"(t2) : "cc"
            );
            
            /* Update array based on condition */
            arr[t1 % 199][t2 % 199] += t3 + t4;
            
            /* Access packed struct through volatile pointer */
            volatile_ptr = &packed.i;
            *volatile_ptr = t1;
            
            /* Force memory access with volatile */
            asm volatile ("" : : "m"(*volatile_ptr));
        } else {
            /* Alternative path with different variables */
            int u1 = e + f;
            int u2 = g + h;
            float u3 = fc + fd;
            double u4 = dc + dd;
            
            /* Different assembly constraints */
            asm volatile (
                "orl %1, %0\n\t"
                : "+r"(u1) : "r"(u2) : "cc"
            );
            
            arr[u1 % 199][u2 % 199] += u3 + u4;
        }
        
        /* Access packed struct members - may require secondary reloads */
        packed.d = da + db;
        packed.i = a + b;
        packed.f = fa + fb;
        packed.l = la + lb;
        
        /* Force reloads by using packed struct in computation */
        double packed_d = packed.d;
        int packed_i = packed.i;
        
        /* More arithmetic mixing all types */
        a = a + packed_i;
        da = da + packed_d;
        fa = fa + packed.f;
        la = la + packed.l;
        
        /* Another assembly block with specific register constraints */
        register int r1 asm("ebx") = a;
        register int r2 asm("ecx") = b;
        asm volatile (
            "leal (%1, %0), %0\n\t"
            : "+r"(r1) : "r"(r2) : 
        );
        a = r1;
        
        /* Force spill/reload by calling external function */
        srand(iter + a);
        
        /* Complex expression using most variables */
        arr[iter % 199][(iter * 2) % 199] = 
            a + b + c + d + e + f + g + h + i + j + k + l +
            (int)fa + (int)fb + (int)fc + (int)fd +
            (int)da + (int)db + (int)dc + (int)dd +
            (int)la + (int)lb + (int)lc + (int)ld;
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    checksum += a + b + c + d + e + f + g + h + i + j + k + l;
    checksum += (unsigned long long)(fa * 1000);
    checksum += (unsigned long long)(fb * 1000);
    checksum += (unsigned long long)(fc * 1000);
    checksum += (unsigned long long)(fd * 1000);
    checksum += (unsigned long long)(da * 1000);
    checksum += (unsigned long long)(db * 1000);
    checksum += (unsigned long long)(dc * 1000);
    checksum += (unsigned long long)(dd * 1000);
    checksum += la + lb + lc + ld;
    
    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
            checksum += arr[x][y];
        }
    }
    
    checksum += (unsigned long long)packed.d;
    checksum += packed.i;
    checksum += (unsigned long long)packed.f;
    checksum += packed.l;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    printf("Starting reload test with N=%d, seed=%d\n", N, seed);
    
    unsigned long long result = trigger_reloads(N, seed);
    
    printf("Result checksum: %llu\n", result);
    
    return 0;
}
