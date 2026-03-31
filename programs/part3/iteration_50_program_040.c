/* Compile with: g++ -O2 -fno-inline -fno-strict-aliasing -fdump-rtl-auto_inc_dec -o test_auto_inc test_auto_inc.cc */
/* Also try: g++ -O3 -funroll-loops -fno-omit-frame-pointer -fno-schedule-insns -o test_auto_inc test_auto_inc.cc */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#pragma GCC optimize("O3")

/* Heterogeneous structure with mixed data types */
struct MixedData {
    char c;
    short s;
    int i;
    float f;
    double d;
    long l;
};

/* Another structure for nested access */
struct NestedStruct {
    int id;
    float values[4];
    struct MixedData* next;
};

/* __attribute__((noinline)) helper to create another context */
__attribute__((noinline)) 
static long process_subset(int* arr, int size) {
    register int* p asm("r12") = arr;  /* register hint */
    long sum = 0;
    int i;
    
    /* Pattern 1: Simple post-increment with zero offset */
    for (i = 0; i < size; i++) {
        sum += *p;  /* This should become [reg + 0] */
        p++;        /* Post-increment */
    }
    
    /* Insert function call to force re-evaluation */
    sum += (long)sin(sum * 0.01);
    
    /* Pattern 2: Post-decrement */
    p = &arr[size - 1];
    for (i = 0; i < size; i++) {
        sum += *p;  /* [reg + 0] */
        p--;        /* Post-decrement */
    }
    
    return sum;
}

/* Function with aggressive optimization attribute */
__attribute__((optimize("O3")))
static double process_floats(float* arr, int n) {
    register float* q = arr;
    double sum = 0.0;
    int count = n;
    
    /* Pattern 3: while loop with post-increment */
    while (count-- > 0) {
        sum += *q;  /* [reg + 0] */
        q++;        /* Post-increment */
        
        /* Function call to obscure aliasing */
        if (count % 10 == 0) {
            sum += cos(sum);
        }
    }
    
    return sum;
}

int main() {
    srand(time(NULL));
    
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    char arr_c[200];
    short arr_s[200];
    struct MixedData arr_md[50];
    struct NestedStruct arr_ns[25];
    
    /* Initialize arrays with random data */
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)rand() / RAND_MAX;
        arr_d[i] = (double)rand() / RAND_MAX;
        arr_c[i] = rand() % 256;
        arr_s[i] = rand() % 10000;
    }
    
    for (int i = 0; i < 50; i++) {
        arr_md[i].c = rand() % 256;
        arr_md[i].s = rand() % 10000;
        arr_md[i].i = rand() % 1000000;
        arr_md[i].f = (float)rand() / RAND_MAX;
        arr_md[i].d = (double)rand() / RAND_MAX;
        arr_md[i].l = rand() * 1000L;
    }
    
    for (int i = 0; i < 25; i++) {
        arr_ns[i].id = i;
        for (int j = 0; j < 4; j++) {
            arr_ns[i].values[j] = (float)rand() / RAND_MAX;
        }
        arr_ns[i].next = &arr_md[i % 50];
    }
    
    long total_sum = 0;
    double float_sum = 0.0;
    
    /* ===== PATTERN 1: Multiple pointer aliases with zero offset ===== */
    {
        int* p1 = arr_i;
        int* p2 = &arr_i[50];
        int* p3 = &arr_i[100];
        
        #pragma GCC unroll 4
        for (int i = 0; i < 50; i++) {
            /* Multiple base registers with [reg + 0] addressing */
            total_sum += *p1;  /* Should trigger reg1_is_const = true, reg1_val = 0 */
            total_sum += *p2;
            total_sum += *p3;
            
            /* Post-increment operations */
            p1++;
            p2++;
            p3++;
            
            /* Mix with array indexing */
            total_sum += arr_i[i * 2];  /* Different addressing mode */
        }
    }
    
    /* Function call barrier */
    total_sum += (long)sin(total_sum * 0.001);
    
    /* ===== PATTERN 2: Nested loops with pointer reset ===== */
    {
        float* __restrict q = arr_f;  /* restrict helps alias analysis */
        
        for (int outer = 0; outer < 3; outer++) {
            /* Inner loop with pointer traversal */
            for (int inner = 0; inner < 50; inner++) {
                float_sum += *q;  /* [reg + 0] */
                q++;              /* Post-increment */
                
                /* Compound expression with index modification */
                float_sum += arr_f[inner * 2 + outer];
            }
            
            /* Reset pointer for next outer iteration */
            q = &arr_f[outer * 20];
            
            /* Function call between pointer operations */
            float_sum += cos(float_sum);
        }
    }
    
    /* ===== PATTERN 3: Mixed integer index and pointer arithmetic ===== */
    {
        int* base = arr_i;
        int idx = 0;
        
        /* Loop with mixed pointer/index arithmetic */
        for (int i = 0; i < 100; i++) {
            int* ptr = base + idx;  /* Base + index calculation */
            total_sum += *ptr;      /* Should become [reg + 0] after optimization */
            
            /* Multiple ways to update */
            if (i % 2 == 0) {
                idx++;              /* Integer index increment */
            } else {
                base = &arr_i[i];   /* Pointer reassignment */
                idx = 0;
            }
            
            /* Array access with pre-decrement */
            if (i > 0) {
                total_sum += arr_i[--idx];  /* arr[--index] pattern */
                idx++;
            }
        }
    }
    
    /* ===== PATTERN 4: Structure field accesses (base + constant offset) ===== */
    {
        struct MixedData* md_ptr = arr_md;
        
        for (int i = 0; i < 50; i++) {
            /* Structure field accesses - first field has offset 0 */
            total_sum += md_ptr->c;  /* First field: offset 0 */
            total_sum += md_ptr->s;
            total_sum += md_ptr->i;
            float_sum += md_ptr->f;
            float_sum += md_ptr->d;
            
            md_ptr++;  /* Post-increment through struct array */
            
            /* Access with different pointer alias */
            struct MixedData* alias = &arr_md[i];
            total_sum += alias->l;  /* Different field, different offset */
        }
    }
    
    /* ===== PATTERN 5: Complex nested structure traversal ===== */
    {
        struct NestedStruct* ns_ptr = arr_ns;
        
        for (int i = 0; i < 25; i++) {
            /* Access first field (offset 0) */
            total_sum += ns_ptr->id;
            
            /* Pointer to float array within struct */
            float* fptr = ns_ptr->values;
            for (int j = 0; j < 4; j++) {
                float_sum += *fptr;  /* [reg + 0] */
                fptr++;              /* Post-increment within struct */
            }
            
            /* Follow pointer to another struct */
            if (ns_ptr->next) {
                total_sum += ns_ptr->next->i;  /* Indirect access */
            }
            
            ns_ptr++;  /* Post-increment main pointer */
        }
    }
    
    /* ===== PATTERN 6: Different data types with alignment considerations ===== */
    {
        char* cp = arr_c;
        short* sp = arr_s;
        double* dp = arr_d;
        
        /* Process different types in interleaved manner */
        for (int i = 0; i < 100; i++) {
            /* Different alignments: char (1), short (2), double (8) */
            total_sum += *cp;  /* [reg + 0] */
            total_sum += *sp;  /* [reg + 0] */
            float_sum += *dp;  /* [reg + 0] */
            
            /* Post-increment each pointer */
            cp++;
            sp++;
            dp++;
            
            /* Array indexing with post-increment in expression */
            if (i < 99) {
                total_sum += arr_c[i++];  /* Compound expression */
            }
        }
    }
    
    /* Call helper functions for additional contexts */
    total_sum += process_subset(arr_i, 100);
    float_sum += process_floats(arr_f, 100);
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %ld (int), %f (float)\n", total_sum, float_sum);
    
    /* Use inline assembly as compiler barrier */
    asm volatile("" : : : "memory");
    
    return (total_sum > 0) ? 0 : 1;
}
