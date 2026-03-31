#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")
#pragma GCC push_options

/* Heterogeneous structure with mixed alignment */
struct S {
    char c;
    int a;
    float b;
    double d;
    short s;
    long l;
} __attribute__((packed));

/* Helper function marked noinline to create separate optimization context */
__attribute__((noinline)) 
static void process_subset(int* restrict arr, int n, int* out) {
    register int* p = arr;
    register int* end = arr + n;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment with zero offset */
    while (p != end) {
        sum += *p++;  /* Should trigger [reg + 0] addressing */
    }
    
    /* Pattern 2: Post-decrement in reverse */
    p = end - 1;
    while (p >= arr) {
        sum += *p--;  /* Should trigger [reg + 0] addressing */
    }
    
    *out = sum;
}

/* Another noinline helper with different type */
__attribute__((noinline))
static double process_floats(float* restrict farr, int n) {
    register float* q = farr;
    double sum = 0.0;
    
    /* Mix pointer and index in same loop */
    for (int i = 0; i < n; i++) {
        sum += *(q + i);  /* Base + index, should decompose */
        /* Function call as compiler barrier */
        if (i % 16 == 0) {
            sum += sin(sum);
        }
    }
    
    /* Pure pointer arithmetic loop */
    float* end = farr + n;
    while (q < end) {
        sum += *q++;
        /* Array indexing with post-increment */
        sum += farr[(q - farr) - 1];
    }
    
    return sum;
}

/* Function with attribute to force specific optimization */
__attribute__((optimize("O3")))
static long process_structs(struct S* sarr, int n) {
    long total = 0;
    struct S* ptr = sarr;
    
    /* Loop through struct array accessing different fields */
    for (int i = 0; i < n; i++, ptr++) {
        /* Different constant offsets within struct */
        total += ptr->a;      /* offset 4 (after char) */
        total += (long)ptr->b; /* offset 8 */
        total += (long)ptr->d; /* offset 12 */
        
        /* Compound expression with post-increment */
        total += sarr[i].s + sarr[i++].l;
    }
    
    /* Nested loop with pointer reset */
    for (int j = 0; j < 3; j++) {
        ptr = sarr;
        for (int i = 0; i < n; i++) {
            /* Multiple zero-offset accesses via different "registers" */
            total += ptr->a;
            ptr++;
        }
    }
    
    return total;
}

int main() {
    /* Declare arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[100];
    short arr_s[400];
    struct S arr_struct[50];
    
    /* Initialize with random values */
    srand(42);
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        if (i < 100) arr_d[i] = (double)(rand() % 1000) / 5.0;
        if (i < 400) arr_s[i] = (short)(rand() % 1000);
    }
    
    for (int i = 0; i < 50; i++) {
        arr_struct[i].c = (char)(rand() % 256);
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (float)(rand() % 1000) / 10.0f;
        arr_struct[i].d = (double)(rand() % 1000) / 5.0;
        arr_struct[i].s = (short)(rand() % 1000);
        arr_struct[i].l = rand() % 1000;
    }
    
    int checksum = 0;
    double fp_checksum = 0.0;
    
    /* PATTERN 1: Simple post-increment with int pointer */
    {
        int* restrict p = arr_i;
        int* end = arr_i + 100;
        
        #pragma GCC unroll 4
        for (; p < end; p++) {
            checksum += *p;  /* Should become [reg + 0] */
        }
        
        /* Compiler barrier */
        fp_checksum += cos(checksum);
    }
    
    /* PATTERN 2: While loop with post-decrement */
    {
        float* q = arr_f + 99;
        int counter = 100;
        
        while (counter-- > 0) {
            fp_checksum += *q--;  /* Post-decrement */
            
            /* Mix with array indexing */
            if (counter % 10 == 0) {
                fp_checksum += arr_f[counter];
            }
        }
        
        /* Reset pointer and use different alias */
        float* r = arr_f;
        for (int i = 0; i < 100; i++) {
            fp_checksum += r[i];  /* Base + index */
            r = &arr_f[0];  /* Force base register reload */
        }
    }
    
    /* PATTERN 3: Nested loops with induction variables */
    {
        int* base_ptr = arr_i;
        
        for (int outer = 0; outer < 5; outer++) {
            register int* p = base_ptr + outer * 20;
            
            /* Inner loop with pointer stepping */
            for (int inner = 0; inner < 20; inner++) {
                checksum += *p++;
                
                /* Inline assembly to clobber memory/registers */
                asm volatile("" ::: "memory");
            }
            
            /* Function call between pointer operations */
            fp_checksum += sin(fp_checksum);
        }
    }
    
    /* PATTERN 4: Mixed pointer and index arithmetic */
    {
        int idx = 0;
        int* ptr = &arr_i[0];
        
        for (idx = 0; idx < 100; idx++) {
            /* Both forms in same loop */
            checksum += *(ptr + idx);  /* Base + index */
            checksum += ptr[idx];      /* Array indexing */
            
            /* Compound with post-increment */
            checksum += arr_i[idx++] * 2;
        }
        
        /* Pointer arithmetic with different types */
        char* cptr = (char*)arr_i;
        for (int i = 0; i < 100 * sizeof(int); i += sizeof(int)) {
            checksum += *(int*)(cptr + i);  /* Byte offset calculation */
        }
    }
    
    /* PATTERN 5: Process structs with field accesses */
    long struct_total = process_structs(arr_struct, 50);
    
    /* PATTERN 6: Call helper functions */
    int subset_sum = 0;
    process_subset(arr_i + 50, 50, &subset_sum);
    checksum += subset_sum;
    
    fp_checksum += process_floats(arr_f + 100, 50);
    
    /* PATTERN 7: Complex expression with multiple increments */
    {
        int* p1 = arr_i;
        int* p2 = arr_i + 50;
        
        for (int i = 0; i < 50; i++) {
            /* Multiple memory accesses with zero offset */
            checksum += *p1 + *p2;
            p1++;
            p2++;
            
            /* Access via different base registers */
            checksum += arr_i[i] + arr_i[i + 50];
        }
    }
    
    /* Final computation to prevent elimination */
    double final_result = checksum + fp_checksum + struct_total;
    printf("Result: %f\n", final_result);
    
    return 0;
}
