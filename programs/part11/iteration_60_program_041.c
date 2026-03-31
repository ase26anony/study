/* Test case for GCC early rematerialization pass - targeting lines 930-937 in early-remat.cc */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile short v3 = 1000;

/* Global array to create address calculations */
int global_arr[1000];

/* Complex function with high register pressure */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many distinct intermediate values to create register pressure */
    int a = x1 + x2;
    int b = a * x3;
    int c = b - x4;
    int d = c ^ x5;
    int e = d | x6;
    int f = e & x7;
    int g = f + x8;
    int h = g - x9;
    int i = h * x10;
    int j = i ^ a;
    int k = j + b;
    int l = k - c;
    int m = l * d;
    int n = m | e;
    int o = n & f;
    int p = o ^ g;
    int q = p + h;
    int r = q - i;
    int s = r * j;
    int t = s | k;
    int u = t & l;
    int v = u ^ m;
    int w = v + n;
    int x = w - o;
    int y = x * p;
    int z = y | q;
    
    /* Mixed-type operations to create mode conversions */
    short s1 = (short)r;
    short s2 = (short)s;
    int i1 = (int)s1 * (int)s2;
    char c1 = (char)t;
    char c2 = (char)u;
    int i2 = (int)c1 + (int)c2;
    
    /* Use volatile to prevent optimization and create dataflow edges */
    if (__builtin_expect(v1 > 10000, 0)) {
        a += v1;
        b += v2;
    }
    
    /* Complex switch to create control flow with different live sets */
    int selector = (z & 0xF);
    int result = 0;
    
    switch (selector) {
        case 0:
            result = a + b + (int)s1;
            break;
        case 1:
            result = c + d + (int)c1;
            break;
        case 2:
            result = e + f + i1;
            break;
        case 3:
            result = g + h + i2;
            break;
        case 4:
            result = i + j + (short)z;
            break;
        case 5:
            result = k + l + (char)w;
            break;
        case 6:
            result = m + n + s2;
            break;
        case 7:
            result = o + p + c2;
            break;
        case 8:
            result = q + r + (int)s1 * 2;
            break;
        case 9:
            result = s + t + (int)c1 * 3;
            break;
        case 10:
            result = u + v + i1 / 2;
            break;
        case 11:
            result = w + x + i2 / 3;
            break;
        case 12:
            result = y + z + (short)a;
            break;
        case 13:
            result = a + c + e + g;
            break;
        case 14:
            result = b + d + f + h;
            break;
        default:
            result = i + k + m + o;
            break;
    }
    
    /* Address calculations that might be rematerialized */
    int *ptr1 = &global_arr[a % 1000];
    int *ptr2 = &global_arr[b % 1000];
    int *ptr3 = &global_arr[c % 1000];
    
    /* Multiple uses of address calculations */
    *ptr1 += result;
    *ptr2 += result + 1;
    *ptr3 += result + 2;
    
    /* More mixed-type operations */
    long long ll1 = (long long)result * (long long)a;
    long long ll2 = (long long)b * (long long)c;
    long long ll3 = ll1 + ll2;
    
    /* Use inline assembly to create complex dataflow */
    asm volatile ("# Dummy assembly %0 %1 %2" 
                  : "+r" (result), "+r" (ll3)
                  : "r" (v3)
                  : "cc", "memory");
    
    return result + (int)ll3;
}

/* Another function with vector operations */
typedef int v4si __attribute__ ((vector_size (16)));
int __attribute__((noinline))
vector_ops(v4si v1, v4si v2, v4si v3) {
    v4si r1 = v1 + v2;
    v4si r2 = v1 * v3;
    v4si r3 = r1 & r2;
    v4si r4 = r1 | r2;
    v4si r5 = r3 ^ r4;
    
    /* Extract elements to scalar operations */
    int sum = r5[0] + r5[1] + r5[2] + r5[3];
    
    /* Mode mixing */
    short s0 = (short)r5[0];
    short s1 = (short)r5[1];
    int mixed = (int)s0 * (int)s1;
    
    return sum + mixed;
}

/* Packed structure for sub-register accesses */
struct __attribute__((packed)) packed_data {
    unsigned short a;
    unsigned char b;
    unsigned int c;
    unsigned short d;
};

int __attribute__((noinline))
packed_ops(struct packed_data *p) {
    /* These accesses involve mode changes */
    unsigned int val1 = p->a;  /* zero-extend from short to int */
    unsigned int val2 = p->b;  /* zero-extend from char to int */
    unsigned int val3 = p->c;
    unsigned int val4 = p->d;
    
    return val1 * val2 + val3 - val4;
}

int main() {
    int i, j;
    int total = 0;
    
    /* Initialize global array */
    for (i = 0; i < 1000; i++) {
        global_arr[i] = i;
    }
    
    /* Create packed structure */
    struct packed_data pd;
    pd.a = 100;
    pd.b = 50;
    pd.c = 1000;
    pd.d = 200;
    
    /* Create vectors */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    
    /* Main computation loop - creates sustained register pressure */
    for (j = 0; j < 10000; j++) {
        /* Vary inputs to prevent constant propagation */
        int base = j + v1;
        
        /* Call function with many arguments to increase register pressure */
        int res1 = compute_heavy(
            base + 1, base + 2, base + 3, base + 4, base + 5,
            base + 6, base + 7, base + 8, base + 9, base + 10);
        
        /* More operations to keep values live */
        int res2 = vector_ops(vec1, vec2, vec3);
        int res3 = packed_ops(&pd);
        
        /* Complex expression with many temporaries */
        int t1 = res1 * res2;
        int t2 = res2 + res3;
        int t3 = res3 ^ res1;
        int t4 = t1 & t2;
        int t5 = t2 | t3;
        int t6 = t3 - t1;
        int t7 = t4 * t5;
        int t8 = t5 + t6;
        int t9 = t6 ^ t4;
        int t10 = t7 & t8;
        int t11 = t8 | t9;
        int t12 = t9 - t7;
        int t13 = t10 * t11;
        int t14 = t11 + t12;
        int t15 = t12 ^ t10;
        
        /* Use volatile condition */
        if (__builtin_expect(v2 > 50000, 0)) {
            t13 += v3;
            t14 += v1;
        }
        
        /* Mixed-type final computation */
        short final_s = (short)t13 + (short)t14 + (short)t15;
        total += (int)final_s + t13 + t14 + t15;
        
        /* Modify vectors slightly */
        vec1[0] += 1;
        vec2[1] += 1;
        vec3[2] += 1;
        
        /* Modify packed data */
        pd.a = (pd.a + 1) % 65535;
        pd.b = (pd.b + 1) % 256;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
