/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure with many members to force complex addressing */
struct BigStruct {
    int a, b, c, d, e, f, g, h;
    int arr[8];
    struct BigStruct *next;
    volatile int volatile_member;
};

/* Global arrays to create complex addressing modes */
static int global_array_1[256];
static int global_array_2[256];
static struct BigStruct global_structs[16];

/* Test 1: Complex array addressing with multiple index calculations */
static __attribute__((noinline)) 
int test_complex_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to consume registers */
    volatile int v1 = a, v2 = b, v3 = c, v4 = d, v5 = e, v6 = f;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int j1, j2, j3, j4, j5, j6, j7, j8, j9, j10;
    int k1, k2, k3, k4, k5, k6, k7, k8, k9, k10;
    
    /* Complex array indexing - forces RELOAD_FOR_INPUT_ADDRESS */
    i1 = global_array_1[v1 + v2];
    i2 = global_array_2[v3 * v4 + v5];
    
    /* Multi-level array indexing with pointer arithmetic */
    int *ptr1 = &global_array_1[v1];
    int *ptr2 = &global_array_2[v2];
    
    /* Nested addressing requiring address reloads */
    for (int x = 0; x < 4; x++) {
        /* Manual loop unrolling to increase pressure */
        i3 = ptr1[x * v3 + v4];
        i4 = ptr2[x * v5 + v6];
        
        /* More complex addressing */
        i5 = global_array_1[ptr1[x] + ptr2[x]];
        i6 = global_array_2[global_array_1[x] + global_array_2[x]];
    }
    
    /* Structure member access with offset computation */
    struct BigStruct *s = &global_structs[v1 % 16];
    i7 = s->arr[v2 % 8];
    i8 = s->arr[v3 % 8] + s->arr[v4 % 8];
    
    /* Pointer chasing with volatile - forces multiple reload types */
    s->volatile_member = v1;
    struct BigStruct *s2 = s->next;
    if (s2) {
        i9 = s2->arr[v5 % 8];
        s2->volatile_member = v2;
    }
    
    return i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9;
}

/* Test 2: Inline assembly with multiple outputs and constraints */
static __attribute__((noinline))
int test_asm_reloads(int a, int b, int c, int d) {
    int out1, out2, out3, out4;
    int addr1, addr2;
    volatile int mem1, mem2;
    
    /* Complex address computation for output */
    int *addr_ptr = &global_array_1[a + b];
    
    /* Inline asm with memory output - forces RELOAD_FOR_OUTPUT_ADDRESS */
    __asm__ volatile (
        "movl %[input1], %[output1]\n\t"
        "addl %[input2], %[output1]\n\t"
        "movl %[output1], (%[addr])\n\t"
        : [output1] "=r" (out1), "=m" (*addr_ptr)
        : [input1] "r" (a), [input2] "r" (b), [addr] "r" (addr_ptr)
        : "memory"
    );
    
    /* Another asm with multiple alternative constraints */
    __asm__ volatile (
        "imull %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        : [out1] "=r,r,m" (out2)
        : [in1] "r,r,i" (c), [in2] "r,i,r" (d)
        : "cc"
    );
    
    /* Asm with clobbered registers to increase pressure */
    __asm__ volatile (
        "movl $0, %%eax\n\t"
        "movl $0, %%ebx\n\t"
        "movl $0, %%ecx\n\t"
        "movl $0, %%edx\n\t"
        "movl $0, %%esi\n\t"
        "movl $0, %%edi\n\t"
        : 
        : 
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Use computed addresses */
    addr1 = (int)(&global_array_2[a * b + c]);
    addr2 = (int)(&global_structs[d % 16].arr[c % 8]);
    
    /* Force address reloads through volatile */
    mem1 = *(volatile int *)addr1;
    mem2 = *(volatile int *)addr2;
    
    return out1 + out2 + mem1 + mem2;
}

/* Test 3: Pointer chains and operand address reloads */
static __attribute__((noinline))
int test_pointer_chains(int seed) {
    /* Create many pointer variables */
    int *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10;
    int **pp1, **pp2, **pp3;
    volatile int *vp1, *vp2;
    
    /* Initialize pointers with complex expressions */
    p1 = &global_array_1[seed];
    p2 = &global_array_2[seed * 2];
    p3 = &global_array_1[seed * 3 % 256];
    p4 = &global_array_2[seed * 4 % 256];
    p5 = &global_array_1[seed * 5 % 256];
    
    /* Pointer to pointer - forces RELOAD_FOR_OPERAND_ADDRESS */
    pp1 = &p1;
    pp2 = &p2;
    pp3 = &p3;
    
    /* Complex pointer arithmetic */
    vp1 = (volatile int *)(p1 + seed);
    vp2 = (volatile int *)(p2 + seed * 2);
    
    /* Multiple dereferences with address computations */
    int sum = 0;
    
    /* Unrolled loop with pointer chasing */
    sum += *p1 + *p2;
    sum += *(p1 + seed) + *(p2 + seed * 2);
    sum += **(pp1 + 0) + **(pp2 + 0);
    
    /* More complex chain */
    p6 = *pp1 + **pp2;
    p7 = *pp2 + **pp3;
    
    /* Volatile accesses force reloads */
    sum += *vp1;
    sum += *vp2;
    
    /* Structure pointer chain */
    struct BigStruct *sp = &global_structs[seed % 16];
    for (int i = 0; i < 4; i++) {
        sum += sp->arr[i];
        if (sp->next) {
            sp = sp->next;
            sum += sp->a;
        }
    }
    
    return sum;
}

/* Test 4: Mixed addressing modes and builtins */
static __attribute__((noinline))
int test_mixed_addressing(int a, int b, int c, int d, int e, int f) {
    /* Use __builtin_expect to create data dependencies */
    int x = __builtin_expect(a > 0, 1) ? b : c;
    int y = __builtin_expect(b > 0, 0) ? c : d;
    int z = __builtin_expect(c > 0, 1) ? d : e;
    
    /* Complex expression with many temporaries */
    int t1 = a + b * c - d / (e + 1);
    int t2 = b + c * d - e / (f + 1);
    int t3 = c + d * e - f / (a + 1);
    int t4 = d + e * f - a / (b + 1);
    int t5 = e + f * a - b / (c + 1);
    int t6 = f + a * b - c / (d + 1);
    
    /* Array accesses with complex indices */
    int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = i * i;
    }
    
    /* Multi-dimensional style addressing */
    int idx1 = (a * b + c) % 16;
    int idx2 = (d * e + f) % 16;
    int idx3 = (t1 * t2 + t3) % 16;
    int idx4 = (t4 * t5 + t6) % 16;
    
    /* Force many different addressing modes */
    int val1 = arr[idx1] + arr[idx2];
    int val2 = arr[arr[idx3] % 16] + arr[arr[idx4] % 16];
    
    /* Pointer to array element */
    int *ptr_arr[8];
    for (int i = 0; i < 8; i++) {
        ptr_arr[i] = &arr[(i * a + b) % 16];
    }
    
    /* Chain of dereferences */
    int sum = 0;
    sum += *ptr_arr[0] + *ptr_arr[1];
    sum += *ptr_arr[2] + *ptr_arr[3];
    sum += *ptr_arr[4] + *ptr_arr[5];
    sum += *ptr_arr[6] + *ptr_arr[7];
    
    /* Use all temporaries */
    sum += t1 + t2 + t3 + t4 + t5 + t6;
    
    return sum + val1 + val2 + x + y + z;
}

/* Main driver that calls all tests */
int main(int argc, char **argv) {
    /* Initialize globals */
    for (int i = 0; i < 256; i++) {
        global_array_1[i] = i;
        global_array_2[i] = 255 - i;
    }
    
    for (int i = 0; i < 16; i++) {
        global_structs[i].a = i * 10;
        global_structs[i].b = i * 20;
        global_structs[i].c = i * 30;
        global_structs[i].d = i * 40;
        global_structs[i].e = i * 50;
        global_structs[i].f = i * 60;
        global_structs[i].g = i * 70;
        global_structs[i].h = i * 80;
        for (int j = 0; j < 8; j++) {
            global_structs[i].arr[j] = i * 100 + j;
        }
        global_structs[i].next = (i < 15) ? &global_structs[i + 1] : NULL;
        global_structs[i].volatile_member = 0;
    }
    
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int result = 0;
    
    /* Call all test functions with different arguments */
    result += test_complex_addressing(seed, seed+1, seed+2, seed+3, seed+4, seed+5);
    result += test_asm_reloads(seed, seed+10, seed+20, seed+30);
    result += test_pointer_chains(seed);
    result += test_mixed_addressing(seed, seed+1, seed+2, seed+3, seed+4, seed+5);
    
    /* Use result to prevent dead code elimination */
    volatile int dummy = result;
    
    return result & 0xFF;
}

#pragma GCC pop_options
