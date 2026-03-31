/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force specific architecture for register scarcity */
#ifdef __x86_64__
#error "Compile with -m32 for 32-bit target to increase register pressure"
#endif

/* Volatile variables to prevent optimization */
static volatile int vol_global = 42;
static volatile int vol_array[256];

/* Complex structure for address computations */
struct Nested {
    int data[8];
    struct Nested *next;
    short offsets[4];
};

struct Container {
    struct Nested levels[3][2];
    int matrix[4][4];
    volatile long counters[8];
};

/* ========== TEST FUNCTION 1: Complex Array Addressing ========== */
/* Targets: RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
static int __attribute__((noinline))
test_complex_addressing(struct Container *cont, int idx1, int idx2, int idx3) {
    /* Many local variables to consume registers */
    register int r0 asm("eax") = idx1;
    register int r1 asm("ebx") = idx2;
    register int r2 asm("ecx") = idx3;
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int *ptr1, *ptr2, *ptr3;
    short s1, s2, s3, s4;
    long l1, l2, l3;
    
    /* Force values into variables */
    v1 = r0 * 2;
    v2 = r1 + 3;
    v3 = r2 - 1;
    v4 = vol_global;
    v5 = v1 + v2;
    v6 = v3 * v4;
    v7 = v5 - v6;
    v8 = v7 / 2;
    v9 = v8 + 100;
    v10 = v9 * 3;
    
    /* Complex array indexing - will need address reloads */
    t1 = cont->levels[0][1].data[v1 & 7];
    t2 = cont->levels[1][0].data[v2 & 7];
    t3 = cont->matrix[v3 & 3][v4 & 3];
    
    /* Multi-level address computation */
    t4 = cont->levels[(v1+v2) & 1][(v3+v4) & 1].data[(v5+v6) & 7];
    t5 = cont->levels[(v7+v8) & 1][(v9+v10) & 1].offsets[(v1+v2) & 3];
    
    /* Pointer arithmetic with multiple bases */
    ptr1 = &cont->levels[0][0].data[0];
    ptr2 = &cont->levels[1][1].data[0];
    ptr3 = &cont->matrix[0][0];
    
    /* Address of address computation - triggers RELOAD_FOR_INPADDR_ADDRESS */
    t6 = *(ptr1 + (v1 * v2) + (v3 << 2));
    t7 = *(ptr2 + (v4 * v5) + (v6 >> 1));
    t8 = *(ptr3 + (v7 * 4) + (v8 * 4) + v9);
    
    /* More computations to use all temporaries */
    t9 = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
    t10 = t9 * v10;
    
    /* Volatile memory access forces spills */
    vol_array[(t10 ^ v1) & 255] = t9;
    
    return t10 + vol_array[(t9 ^ v2) & 255];
}

/* ========== TEST FUNCTION 2: Inline Assembly with Multiple Outputs ========== */
/* Targets: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_FOR_OPERAND_ADDRESS */
static int __attribute__((noinline))
test_asm_reloads(int a, int b, int c, int d, int e, int f, int g, int h) {
    int out1, out2, out3, out4;
    volatile int mem1, mem2, mem3, mem4;
    int *ptr1 = &mem1;
    int *ptr2 = &mem2;
    int *ptr3 = &mem3;
    int *ptr4 = &mem4;
    
    /* Complex address expressions for outputs */
    int idx1 = a + b;
    int idx2 = c + d;
    int idx3 = e + f;
    int idx4 = g + h;
    
    /* Inline asm with memory outputs at complex addresses */
    /* This should trigger RELOAD_FOR_OUTPUT_ADDRESS */
    asm volatile (
        "movl %[val1], (%[addr1], %[idx1], 4)\n\t"
        "movl %[val2], (%[addr2], %[idx2], 2)\n\t"
        : 
        : [val1] "r" (a), [addr1] "r" (ptr1), [idx1] "r" (idx1),
          [val2] "r" (b), [addr2] "r" (ptr2), [idx2] "r" (idx2)
        : "memory"
    );
    
    /* More complex: output to address that itself needs computation */
    /* Should trigger RELOAD_FOR_OUTADDR_ADDRESS */
    asm volatile (
        "leal (%[base], %[offset], 4), %%edi\n\t"
        "movl %[value], (%%edi)\n\t"
        : 
        : [base] "r" (ptr3), [offset] "r" (idx3), [value] "r" (c)
        : "edi", "memory"
    );
    
    /* Operand that is itself an address needing reload (RELOAD_FOR_OPERAND_ADDRESS) */
    int * volatile volatile_ptr = ptr4;
    asm volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%[ptr])\n\t"
        : 
        : [ptr] "r" (volatile_ptr)
        : "eax", "memory"
    );
    
    /* Use results */
    out1 = mem1 + mem2;
    out2 = mem3 + mem4;
    out3 = out1 * out2;
    out4 = out3 + a + b + c + d + e + f + g + h;
    
    return out4;
}

/* ========== TEST FUNCTION 3: Structure Chains and Pointer Chasing ========== */
/* Targets: RELOAD_FOR_OPADDR_ADDR, RELOAD_FOR_OTHER_ADDRESS, RELOAD_OTHER */
static int __attribute__((noinline))
test_structure_chains(struct Nested *start, int iterations) {
    struct Nested *current = start;
    int sum = 0;
    volatile int temp;
    int local1, local2, local3, local4, local5, local6, local7, local8;
    
    /* Unroll loop to increase register pressure */
    #pragma GCC unroll 4
    for (int i = 0; i < iterations && current != NULL; i++) {
        /* Many local computations */
        local1 = current->data[0];
        local2 = current->data[1];
        local3 = current->data[2];
        local4 = current->data[3];
        local5 = current->offsets[0];
        local6 = current->offsets[1];
        local7 = current->offsets[2];
        local8 = current->offsets[3];
        
        /* Complex expression using all locals */
        temp = local1 * local2 + local3 * local4 - local5 * local6 + local7 * local8;
        
        /* Pointer chasing with address computation */
        /* Address of structure member's address */
        int **ptr_to_ptr = &current->next;
        
        /* This pattern may trigger RELOAD_FOR_OPADDR_ADDR */
        asm volatile (
            "movl %[ptr], %%esi\n\t"
            "movl (%%esi), %%eax\n\t"
            "testl %%eax, %%eax\n\t"
            "jz 1f\n\t"
            "movl %%eax, %[current]\n\t"
            "1:\n\t"
            : [current] "=r" (current)
            : [ptr] "r" (ptr_to_ptr)
            : "eax", "esi", "cc"
        );
        
        sum += temp;
    }
    
    /* Force use of many variables in final computation */
    int result = sum + local1 + local2 + local3 + local4 + local5 + local6 + local7 + local8;
    
    /* RELOAD_OTHER might be triggered by obscure constraints */
    /* Using alternative constraints */
    int alt_result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r,m" (alt_result)  /* Alternative constraints */
        : "r,m" (result)
        : "eax"
    );
    
    return alt_result;
}

/* ========== TEST FUNCTION 4: Mixed Addressing Modes ========== */
/* Comprehensive test hitting all reload types */
static int __attribute__((noinline))
test_mixed_reloads(struct Container *c1, struct Container *c2, int seed) {
    volatile long vl1, vl2, vl3;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int *p1, *p2, *p3, *p4;
    
    /* Initialize many variables */
    i1 = seed * 1;
    i2 = seed * 2;
    i3 = seed * 3;
    i4 = seed * 4;
    i5 = seed * 5;
    i6 = seed * 6;
    i7 = seed * 7;
    i8 = seed * 8;
    i9 = seed * 9;
    i10 = seed * 10;
    
    /* Various address computations */
    p1 = &c1->levels[0][0].data[i1 & 7];
    p2 = &c1->matrix[i2 & 3][i3 & 3];
    p3 = &c2->levels[1][1].data[i4 & 7];
    p4 = &c2->counters[i5 & 7];
    
    /* Input reloads */
    vl1 = *p1 + *p2 + *p3 + *p4;
    
    /* Input address reloads */
    int idx_a = i6 + i7;
    int idx_b = i8 + i9;
    vl2 = c1->levels[(idx_a >> 1) & 1][(idx_b >> 2) & 1].data[(i10) & 7];
    
    /* Output address reloads via inline asm */
    int out_addr_idx = i1 + i2 + i3;
    asm volatile (
        "movl %[val], (%[base], %[idx], 4)\n\t"
        :
        : [val] "r" (i4), [base] "r" (p1), [idx] "r" (out_addr_idx)
        : "memory"
    );
    
    /* Operand address reloads */
    int **addr_of_p2 = &p2;
    asm volatile (
        "movl %[addr], %%edi\n\t"
        "movl (%%edi), %%eax\n\t"
        "incl %%eax\n\t"
        "movl %%eax, (%%edi)\n\t"
        :
        : [addr] "r" (addr_of_p2)
        : "eax", "edi", "memory"
    );
    
    /* Other address reloads */
    volatile int * volatile vptr = p3;
    *vptr = i5 + i6;
    
    /* Use all results */
    vl3 = vl1 + vl2 + *p1 + *p2 + *p3 + *p4 + i7 + i8 + i9 + i10;
    
    return (int)vl3;
}

/* ========== MAIN DRIVER ========== */
int main(void) {
    struct Container cont1, cont2;
    struct Nested chain[10];
    int total = 0;
    
    /* Initialize data structures */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 8; k++) {
                cont1.levels[i][j].data[k] = i * 100 + j * 10 + k;
                cont2.levels[i][j].data[k] = i * 200 + j * 20 + k;
            }
            for (int k = 0; k < 4; k++) {
                cont1.levels[i][j].offsets[k] = i * 4 + j * 2 + k;
                cont2.levels[i][j].offsets[k] = i * 8 + j * 4 + k;
            }
            cont1.levels[i][j].next = NULL;
            cont2.levels[i][j].next = NULL;
        }
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cont1.matrix[i][j] = i * 4 + j;
            cont2.matrix[i][j] = i * 8 + j;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        cont1.counters[i] = i * 10;
        cont2.counters[i] = i * 20;
    }
    
    /* Initialize chain */
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 8; j++) {
            chain[i].data[j] = i * 8 + j;
        }
        for (int j = 0; j < 4; j++) {
            chain[i].offsets[j] = i * 4 + j;
        }
        chain[i].next = &chain[i + 1];
    }
    chain[9].next = NULL;
    
    /* Call all test functions with different arguments */
    total += test_complex_addressing(&cont1, 1, 2, 3);
    total += test_complex_addressing(&cont2, 4, 5, 6);
    
    total += test_asm_reloads(10, 20, 30, 40, 50, 60, 70, 80);
    total += test_asm_reloads(11, 21, 31, 41, 51, 61, 71, 81);
    
    total += test_structure_chains(&chain[0], 5);
    total += test_structure_chains(&chain[3], 4);
    
    total += test_mixed_reloads(&cont1, &cont2, 100);
    total += test_mixed_reloads(&cont2, &cont1, 200);
    
    /* Use result to prevent dead code elimination */
    vol_global = total;
    
    return total & 255; /* Return non-zero to indicate execution */
}
