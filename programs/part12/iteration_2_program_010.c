/* reload_trigger.c - Program to force GCC's reload pass to initialize all reload struct fields */
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
volatile int volatile_index = 0;

/* Target function that creates maximum register pressure */
__attribute__((noinline))
static int64_t force_reloads(int N, int init_val) {
    /* Declare many scalar variables of mixed types to exceed register file */
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
    ps.d = init_val * 1.5;
    ps.i = init_val;
    ps.f = init_val * 2.5f;
    ps.l = init_val * 100L;
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
    
    /* Main computation loop - creates complex data dependencies */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1 && j < 127; j++) {
            /* Complex array access pattern forcing address reloads */
            int idx1 = (i * j) % 127;
            int idx2 = (i + j) % 127;
            
            /* Chain of arithmetic operations keeping many variables live */
            a = b + c;
            b = c * d;
            c = d - e;
            d = e + f;
            e = f / (g ? g : 1);
            f = g * h;
            g = h + a;
            h = a * b;
            
            /* Floating point chain */
            fa = fb + fc;
            fb = fc * fd;
            fc = fd - fa;
            fd = fa * 1.5f;
            
            /* Double precision chain */
            da = db + dc;
            db = dc * dd;
            dc = dd - da;
            dd = da * 1.5;
            
            /* Long integer chain */
            la = lb + lc;
            lb = lc * ld;
            lc = ld - la;
            ld = la * 2L;
            
            /* Inline assembly with conflicting constraints to force reloads */
            /* Tied operand constraint (output = input 0) */
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r"(a)
                : "r"(b), "0"(c)
                : "cc"
            );
            
            /* Another asm with memory constraint */
            asm volatile (
                "imul %0, %1\n\t"
                : "+r"(d)
                : "rm"(e)
                : "cc"
            );
            
            /* Floating point asm with specific constraints */
            asm volatile (
                "addss %0, %1\n\t"
                : "=x"(fa)
                : "x"(fb), "0"(fc)
            );
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different subset of variables inside conditional */
                int t1 = a + b;
                int t2 = c + d;
                float t3 = fa + fb;
                double t4 = da + db;
                
                /* More inline asm in conditional path */
                asm volatile (
                    "sub %0, %1, %2\n\t"
                    : "=r"(t1)
                    : "r"(t2), "0"(t1)
                    : "cc"
                );
                
                /* Update array using conditional variables */
                arr[idx1][idx2] += t1 + t2;
                
                /* Access packed struct through volatile pointer */
                volatile_ps->i = t1;
                volatile_ps->f = t3;
            } else {
                /* Alternative path with different variable usage */
                int u1 = e + f;
                int u2 = g + h;
                float u3 = fc + fd;
                double u4 = dc + dd;
                
                asm volatile (
                    "xor %0, %1, %2\n\t"
                    : "=r"(u1)
                    : "r"(u2), "0"(u1)
                    : "cc"
                );
                
                arr[idx2][idx1] += u1 + u2;
                volatile_ps->l = u1;
                volatile_ps->d = u4;
            }
            
            /* Complex array manipulation - forces address register reloads */
            int temp = arr[i][j];
            arr[i][j] = arr[j][i] + arr[i-1][j] + arr[i][j-1];
            arr[j][i] = temp + arr[i+1][j] + arr[i][j+1];
            
            /* More arithmetic mixing all types */
            a = a + (int)fa + (int)da + (int)(la & 0xFF);
            fa = fa + (float)a + (float)da;
            da = da + (double)a + (double)fa;
            la = la + (long)a + (long)(da * 100);
            
            /* Another asm with immediate and register constraints */
            asm volatile (
                "lea (%1, %2, 2), %0\n\t"
                : "=r"(b)
                : "r"(c), "r"(d)
            );
        }
        
        /* Update volatile index to prevent loop optimizations */
        volatile_index = i;
    }
    
    /* Compute checksum using all variables to prevent dead code elimination */
    int64_t checksum = 0;
    checksum += a + b + c + d + e + f + g + h;
    checksum += (int64_t)(fa + fb + fc + fd);
    checksum += (int64_t)(da + db + dc + dd);
    checksum += la + lb + lc + ld;
    
    /* Add array elements to checksum */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += arr[i][j];
        }
    }
    
    /* Add packed struct members */
    checksum += (int64_t)ps.d;
    checksum += ps.i;
    checksum += (int64_t)ps.f;
    checksum += ps.l;
    checksum += ps.c;
    checksum += ps.s;
    
    return checksum;
}

/* Another function to create cross-function register pressure */
__attribute__((noinline))
static void use_more_registers(int *ptr, float *fptr, double *dptr, long *lptr) {
    int local1 = *ptr + 1;
    int local2 = local1 * 2;
    float local3 = *fptr * 1.5f;
    double local4 = *dptr * 2.5;
    long local5 = *lptr * 3L;
    
    /* Inline asm with multiple constraints */
    asm volatile (
        "mov %1, %0\n\t"
        "add %0, %2\n\t"
        : "=r"(local1), "+r"(local2)
        : "rm"(local5)
        : "cc"
    );
    
    *ptr = local1 + local2;
    *fptr = local3 + (float)local1;
    *dptr = local4 + (double)local2;
    *lptr = local5 + local1;
}

int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound */
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    if (N < 10) N = 100;
    if (N > 1000) N = 1000;
    
    /* Initialize with non-constant value */
    int init_val = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Variables to pass to secondary function */
    int int_var = init_val;
    float float_var = init_val * 1.5f;
    double double_var = init_val * 2.5;
    long long_var = init_val * 100L;
    
    /* Call the reload-intensive function multiple times */
    int64_t total_checksum = 0;
    for (int iter = 0; iter < 3; iter++) {
        total_checksum += force_reloads(N, init_val + iter);
        
        /* Call secondary function to create inter-procedural register pressure */
        use_more_registers(&int_var, &float_var, &double_var, &long_var);
        
        /* Toggle volatile flag to exercise both conditional paths */
        volatile_flag = !volatile_flag;
    }
    
    /* Final computation using all modified variables */
    total_checksum += int_var;
    total_checksum += (int64_t)float_var;
    total_checksum += (int64_t)double_var;
    total_checksum += long_var;
    
    printf("Checksum: %ld\n", (long)total_checksum);
    return 0;
}
