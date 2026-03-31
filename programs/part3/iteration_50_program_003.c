/* Compile with: g++ -O2 -fno-inline -fno-strict-aliasing -fdump-rtl-auto_inc_dec -o test_auto_inc test_auto_inc.cc */
/* Also try: g++ -O3 -funroll-loops -fno-omit-frame-pointer -fno-schedule-insns -o test_auto_inc test_auto_inc.cc */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

#pragma GCC optimize("O3")

/* Heterogeneous structure with different alignments */
struct __attribute__((packed)) MixedData {
    char c;
    short s;
    int i;
    float f;
    long long ll;
    double d;
};

/* Helper function marked noinline to create separate compilation context */
__attribute__((noinline)) 
static void process_subset(int* arr, float* farr, MixedData* mdata, int n, double* result) {
    register int* p = arr;
    register float* q = farr;
    register MixedData* r = mdata;
    
    double sum = 0.0;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (int i = 0; i < n; ++i) {
        /* These decompose to [reg + 0] addressing */
        sum += *p;      /* Base reg + 0 offset */
        sum += *q;      /* Different base reg + 0 offset */
        sum += r->i;    /* Base reg + 0 offset (first field after packed char/short) */
        
        /* Post-increment operations */
        p++;
        q++;
        r++;
    }
    
    *result = sum;
}

/* Another noinline helper with different pattern */
__attribute__((noinline))
static void process_with_index(MixedData* data, int count, double* output) {
    double sum = 0.0;
    
    /* Mix pointer and index arithmetic */
    MixedData* base = data;
    for (int idx = 0; idx < count; idx++) {
        /* ptr = &base + idx pattern */
        MixedData* ptr = base + idx;
        
        /* Access with zero offset through pointer */
        sum += ptr->f;
        sum += ptr->d;
        
        /* Compound expression with increment */
        if (idx % 2 == 0) {
            sum += (base + idx)->i;  /* Another [reg + 0] pattern */
        }
    }
    
    *output = sum;
}

int main() {
    srand(time(NULL));
    
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    short arr_s[200];
    MixedData arr_m[100];
    
    /* Initialize with random data */
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
        arr_s[i] = (short)(rand() % 1000);
    }
    
    for (int i = 0; i < 100; i++) {
        arr_m[i].c = (char)(rand() % 256);
        arr_m[i].s = (short)(rand() % 1000);
        arr_m[i].i = rand() % 1000;
        arr_m[i].f = (float)(rand() % 1000) / 10.0f;
        arr_m[i].ll = (long long)rand() * rand();
        arr_m[i].d = (double)(rand() % 1000) / 10.0;
    }
    
    double total_sum = 0.0;
    
    /* PATTERN 1: Simple post-increment in for loop */
    {
        int sum = 0;
        /* Use restrict to help alias analysis */
        int* __restrict p = arr_i;
        
        #pragma GCC unroll 4
        for (int i = 0; i < 200; i++) {
            /* Post-increment access - should trigger find_inc */
            sum += *p++;
        }
        total_sum += sum;
        
        /* Compiler barrier via math function */
        total_sum += sin(total_sum);
    }
    
    /* PATTERN 2: Post-decrement in while loop with reset */
    {
        float sum_f = 0.0f;
        float* q = &arr_f[199];  /* Start from end */
        
        /* While loop with post-decrement */
        while (q >= arr_f) {
            sum_f += *q--;
        }
        total_sum += sum_f;
        
        /* Reset pointer and use different pattern */
        q = arr_f;
        for (int i = 0; i < 200; i += 2) {
            /* Access with zero offset through different alias */
            float* __restrict alias_q = q + i;
            sum_f += *alias_q;  /* [reg + 0] addressing */
        }
        total_sum += sum_f;
        
        /* Another compiler barrier */
        total_sum += cos(total_sum);
    }
    
    /* PATTERN 3: Nested loops with struct access */
    {
        double struct_sum = 0.0;
        
        /* Outer loop resets pointer */
        for (int outer = 0; outer < 3; outer++) {
            MixedData* ptr = arr_m;
            
            /* Inner loop uses pointer increment */
            for (int inner = 0; inner < 50; inner++) {
                /* Multiple zero-offset accesses to different fields */
                struct_sum += ptr->i;    /* [reg + offset], offset may be 0 after alignment */
                struct_sum += ptr->f;
                struct_sum += ptr->d;
                
                /* Post-increment */
                ptr++;
            }
            
            /* Inline assembly that clobbers memory - forces re-evaluation */
            asm volatile("" ::: "memory");
        }
        total_sum += struct_sum;
    }
    
    /* PATTERN 4: Mixed integer index and pointer arithmetic */
    {
        long long index_sum = 0;
        
        /* Base pointer */
        int* base_ptr = arr_i;
        
        /* Loop with mixed pointer/index arithmetic */
        for (int idx = 0; idx < 200; idx++) {
            /* ptr = base + idx pattern */
            int* current = base_ptr + idx;
            
            /* Access with what may become [reg + 0] */
            index_sum += *current;
            
            /* Compound expression in same statement */
            if (idx % 3 == 0) {
                index_sum += arr_i[idx++];  /* arr[index++] pattern */
            }
        }
        total_sum += index_sum;
        
        /* Compiler barrier */
        total_sum = sqrt(fabs(total_sum));
    }
    
    /* PATTERN 5: Array indexing with pre/post increments */
    {
        double complex_sum = 0.0;
        int index = 0;
        
        /* Compound expressions in loop condition */
        while (index < 200) {
            /* Multiple increment patterns */
            complex_sum += arr_d[index++];
            complex_sum += arr_d[--index];  /* Cancel out the increment */
            
            if (index % 10 == 0) {
                /* Different base register for same array */
                double* __restrict dptr = arr_d;
                complex_sum += *(dptr + index);  /* [reg + 0] if dptr is &arr_d[index] */
            }
            
            index += 2;
        }
        total_sum += complex_sum;
    }
    
    /* PATTERN 6: Call helper functions that have their own patterns */
    {
        double helper_result1, helper_result2;
        
        /* First helper - uses multiple base registers */
        process_subset(arr_i, arr_f, arr_m, 50, &helper_result1);
        total_sum += helper_result1;
        
        /* Compiler barrier between helpers */
        asm volatile("" ::: "memory");
        
        /* Second helper - mixes pointer and index */
        process_with_index(arr_m, 50, &helper_result2);
        total_sum += helper_result2;
    }
    
    /* PATTERN 7: Different data types in sequence */
    {
        /* Traverse different arrays with same pointer-style loop */
        char* cptr = (char*)arr_i;  /* Treat int array as bytes */
        short* sptr = arr_s;
        
        for (int i = 0; i < 400; i++) {
            /* Different sized accesses from different base registers */
            total_sum += cptr[i];  /* Byte access */
            
            if (i % 4 == 0) {
                total_sum += *sptr++;  /* Post-increment on short pointer */
            }
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %lf\n", total_sum);
    
    return 0;
}
