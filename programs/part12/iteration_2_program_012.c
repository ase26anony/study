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
unsigned long long force_reloads(int N, int init_val) {
    /* Declare many variables to exceed available registers */
    int a = init_val + 1;
    int b = init_val * 2;
    int c = init_val / 3;
    int d = init_val - 4;
    int e = init_val + 5;
    int f = init_val * 6;
    int g = init_val / 7;
    int h = init_val - 8;
    long la = init_val * 100L;
    long lb = init_val * 200L;
    long lc = init_val * 300L;
    float fa = init_val * 1.1f;
    float fb = init_val * 2.2f;
    float fc = init_val * 3.3f;
    float fd = init_val * 4.4f;
    double da = init_val * 1.111;
    double db = init_val * 2.222;
    double dc = init_val * 3.333;
    double dd = init_val * 4.444;
    double de = init_val * 5.555;
    
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
        packed_arr[i].d = i * 1.2345;
        packed_arr[i].i = i * 5678;
        packed_arr[i].c = i & 0xFF;
        packed_arr[i].l = i * 123456789L;
        packed_arr[i].f = i * 9.876f;
    }
    
    /* Complex loop with many live variables and dependencies */
    unsigned long long checksum = 0;
    
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N; j++) {
            /* Force address reloads with complex array indexing */
            int idx1 = (i * 17 + j * 23) % 127;
            int idx2 = (i * 29 + j * 31) % 127;
            
            /* Array operations that require address calculations */
            arr[idx1][idx2] = arr[idx2][idx1] + a;
            
            /* Chain of arithmetic operations to keep variables live */
            a = b + c;
            b = c * d - e;
            c = d / (f + 1);
            d = e + g * h;
            e = f - g + h;
            f = g * 2 + h / 3;
            g = h + a - b;
            h = a * b / (c + 1);
            
            /* Long variable chain */
            la = lb + lc * 2;
            lb = lc - la / 3;
            lc = la * lb + 12345;
            
            /* Float operations */
            fa = fb + fc * 1.5f;
            fb = fc - fd / 2.0f;
            fc = fd * fa + 3.14f;
            fd = fa / fb - 2.71f;
            
            /* Double operations with more variables than FP registers */
            da = db + dc;
            db = dc - dd;
            dc = dd * de;
            dd = de / (da + 1.0);
            de = da * db - dc;
            
            /* Inline assembly with conflicting constraints */
            /* Force input reloads with "r" constraints */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r" (a)
                : "r" (b)
                : "cc"
            );
            
            /* Force output reload with specific register constraint on x86 */
            #ifdef __x86_64__
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+r" (c)
                : "r" (d)
                : "%eax", "cc"
            );
            #endif
            
            /* Assembly with tied operand (forces reload) */
            int tmp = e;
            asm volatile (
                "add %1, %0\n\t"
                : "+r" (tmp)
                : "0" (f)
                : "cc"
            );
            e = tmp;
            
            /* Memory constraint to force load/store */
            asm volatile (
                "addl %1, %0\n\t"
                : "+m" (g)
                : "r" (h)
                : "cc"
            );
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different variables inside conditional */
                double temp = packed_arr[i % 64].d;
                temp += packed_arr[j % 64].f;
                
                /* Force reload with volatile pointer access */
                volatile double *vol_ptr = &packed_arr[(i + j) % 64].d;
                *vol_ptr = temp * 2.0;
                
                /* More arithmetic to keep variables live */
                la += (long)(temp * 1000.0);
                lb -= (long)(temp * 500.0);
            } else {
                /* Alternative path with different variable usage */
                arr[i % 127][j % 127] += volatile_index;
            }
            
            /* Access packed struct through volatile pointer */
            volatile struct Packed *p = &packed_arr[(i * j) % 64];
            p->i = a + b + c;
            p->l = la + lb;
            
            /* Update checksum with many variables */
            checksum += a + b + c + d + e + f + g + h;
            checksum += (unsigned long long)la + lb + lc;
            checksum += (unsigned long long)(fa * 1000) + (unsigned long long)(fb * 1000);
            checksum += (unsigned long long)(da * 1000) + (unsigned long long)(db * 1000);
            checksum += arr[i % 127][j % 127];
        }
        
        /* Occasionally update volatile flag */
        if (i % 1000 == 0) {
            volatile_flag = !volatile_flag;
            volatile_index = i;
        }
    }
    
    /* Final computation using all variables */
    double final_result = da + db + dc + dd + de;
    final_result += fa + fb + fc + fd;
    final_result += a + b + c + d + e + f + g + h;
    final_result += la + lb + lc;
    
    checksum += (unsigned long long)final_result;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int init_val = (argc > 2) ? atoi(argv[2]) : 12345;
    
    srand(time(NULL));
    
    printf("Starting reload test with N=%d, init=%d\n", N, init_val);
    
    unsigned long long result = force_reloads(N, init_val);
    
    printf("Result checksum: %llu\n", result);
    
    /* Use result to prevent optimization */
    if (result == 0x123456789ABCDEF0ULL) {
        printf("Impossible condition\n");
    }
    
    return 0;
}
