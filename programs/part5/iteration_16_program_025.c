#include <stdio.h>
#include <stdlib.h>

/* Force no inlining to keep RTL complex */
static __attribute__((noinline)) 
unsigned long high_pressure_computation(int* data, int n) {
    /* Many distinct variables to create register pressure */
    register int a = data[0] ^ 0x55AA55AA;
    register int b = data[1] + 0x12345678;
    register int c = data[2] * 0x89ABCDEF;
    register int d = data[3] | 0xF0F0F0F0;
    register int e = data[4] & 0x0F0F0F0F;
    register int f = data[5] << 3;
    register int g = data[6] >> 2;
    register int h = data[7] + a;
    register int i = data[8] - b;
    register int j = data[9] * c;
    register int k = data[10] | d;
    register int l = data[11] & e;
    register int m = data[12] ^ f;
    register int nv = data[13] + g;  /* renamed to avoid conflict */
    register int o = data[14] - h;
    register int p = data[15] * i;
    
    /* Rematerialization candidates - pure functions of inputs */
    int cand1 = a + 0x11111111;  /* Will be kept live */
    int cand2 = b & 0xAAAAAAAA;
    int cand3 = c * 0x33333333;
    int cand4 = d | 0xCCCCCCCC;
    int cand5 = e ^ 0x55555555;
    
    unsigned long result = 0;
    
    /* Complex loop with many live values */
    for (int idx = 0; idx < n; idx++) {
        /* Use all rematerialization candidates inside loop */
        int temp1 = cand1 + (idx * 0x123);
        int temp2 = cand2 ^ (idx * 0x456);
        int temp3 = cand3 & (idx * 0x789);
        int temp4 = cand4 | (idx * 0xABC);
        int temp5 = cand5 - (idx * 0xDEF);
        
        /* Use many register variables in computations */
        int comp1 = a * temp1 + b;
        int comp2 = c / (temp2 + 1) + d;
        int comp3 = e << (temp3 & 0xF);
        int comp4 = f >> (temp4 & 0x7);
        int comp5 = g ^ temp5;
        int comp6 = h + (i * 2);
        int comp7 = j - (k / 3);
        int comp8 = l | (m << 2);
        int comp9 = nv & (o ^ 0xFF);
        int comp10 = p + (comp1 * comp2);
        
        /* Conditional to create merging points */
        if (idx % 3 == 0) {
            result += comp1 + comp3 + comp5 + comp7 + comp9;
        } else if (idx % 3 == 1) {
            result += comp2 + comp4 + comp6 + comp8 + comp10;
        } else {
            result += comp1 ^ comp2 ^ comp3 ^ comp4 ^ comp5 ^ 
                     comp6 ^ comp7 ^ comp8 ^ comp9 ^ comp10;
        }
        
        /* Modify some variables to prevent CSE */
        a += 0x01;
        b ^= 0x11;
        c -= 0x21;
        d |= 0x31;
        e &= 0x41;
        f <<= 1;
        g >>= 1;
        h += idx;
        i -= idx;
        j *= (idx + 1);
        k |= idx;
        l &= ~idx;
        m ^= idx;
        nv += (idx * 2);
        o -= (idx * 3);
        p *= (idx % 5 + 1);
    }
    
    /* Final use of rematerialization candidates */
    result += cand1 + cand2 + cand3 + cand4 + cand5;
    
    return result;
}

/* Another layer to increase complexity */
static __attribute__((noinline))
unsigned long nested_pressure(int* data, int iterations) {
    unsigned long total = 0;
    
    for (int outer = 0; outer < 3; outer++) {
        /* Create new set of variables each iteration */
        int base = data[outer % 16];
        
        /* More rematerialization candidates */
        int r1 = base + 0x1000;
        int r2 = base * 0x2000;
        int r3 = base | 0x3000;
        int r4 = base & 0x4000;
        int r5 = base ^ 0x5000;
        int r6 = base << 2;
        int r7 = base >> 2;
        int r8 = base + 0x6000;
        int r9 = base * 0x7000;
        int r10 = base | 0x8000;
        
        for (int inner = 0; inner < iterations; inner++) {
            /* Use all candidates in complex expressions */
            int mix1 = (r1 + inner) * (r2 - inner);
            int mix2 = (r3 | inner) & (r4 ^ inner);
            int mix3 = (r5 << (inner & 3)) + (r6 >> (inner & 3));
            int mix4 = (r7 * inner) | (r8 + inner);
            int mix5 = (r9 - inner) ^ (r10 & inner);
            
            /* Chain computations to keep values live */
            int chain1 = mix1 + mix2;
            int chain2 = mix3 * mix4;
            int chain3 = mix5 ^ chain1;
            int chain4 = chain2 - chain3;
            int chain5 = chain4 * (inner + 1);
            
            total += chain1 + chain2 + chain3 + chain4 + chain5;
            
            /* Modify candidates to prevent optimization */
            r1 += 0x11;
            r2 ^= 0x22;
            r3 |= 0x33;
            r4 &= 0x44;
            r5 -= 0x55;
            r6 <<= 1;
            r7 >>= 1;
            r8 += inner;
            r9 *= (inner % 7 + 1);
            r10 ^= inner;
        }
    }
    
    return total;
}

int main() {
    /* Initialize with non-zero values */
    int data[16];
    for (int i = 0; i < 16; i++) {
        data[i] = (i * 0x12345 + 0x6789) & 0x7FFFFFFF;
    }
    
    /* Call high-pressure functions */
    unsigned long result1 = high_pressure_computation(data, 100);
    unsigned long result2 = nested_pressure(data, 50);
    
    /* Combine results to ensure all computations are used */
    unsigned long final_result = result1 ^ result2;
    
    printf("Result: 0x%016lX\n", final_result);
    
    /* Verify with a simple check */
    if (final_result != 0) {
        printf("Computation successful (non-zero result)\n");
    }
    
    return 0;
}
