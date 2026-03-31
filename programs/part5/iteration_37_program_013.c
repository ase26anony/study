/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */
#include <stdint.h>
#include <stdlib.h>

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Bitfield struct for ZERO_EXTRACT */
struct bitfields {
    volatile unsigned int f1 : 3;
    volatile unsigned int f2 : 5;
    volatile unsigned int f3 : 8;
    volatile unsigned int f4 : 16;
    volatile unsigned int padding : 32;
} __attribute__((packed));

/* Union for STRICT_LOW_PART */
union split_int {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Complex memory structure */
struct level3 {
    volatile int data[4];
};

struct level2 {
    volatile struct level3 *l3;
    volatile int extra;
};

struct level1 {
    volatile struct level2 *l2;
    volatile int count;
};

/* Global variables to prevent optimization */
volatile struct bitfields g_bf = {0};
volatile union split_int g_split = {0};
volatile v4si g_vector = {0};
volatile struct level1 *g_top = NULL;

/* Inline assembly helper */
static inline void clobber_registers(void) {
    __asm__ volatile (
        "movl %%eax, %%ebx\n\t"
        "movl %%ecx, %%edx\n\t"
        :
        :
        : "eax", "ebx", "ecx", "edx", "memory"
    );
}

int main(int argc, char **argv) {
    /* Initialize with non-zero values */
    struct bitfields local_bf = {1, 2, 3, 4, 0};
    union split_int local_split = {.full = 0x12345678};
    register int reg_var asm("eax") = 0xDEADBEEF;
    v4si local_vec = {10, 20, 30, 40};
    
    /* Allocate complex memory structure */
    struct level1 *top = malloc(sizeof(struct level1));
    struct level2 *mid = malloc(sizeof(struct level2));
    struct level3 *bot = malloc(sizeof(struct level3));
    
    if (!top || !mid || !bot) return 1;
    
    top->l2 = mid;
    mid->l3 = bot;
    g_top = top;
    
    /* Initialize array */
    for (int i = 0; i < 4; i++) {
        bot->data[i] = i * 100;
    }
    
    /* Loop with multiple RTL patterns */
    volatile int loop_counter = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_counter < 0) loop_counter = 100;
    
    volatile int result = 0;
    
    while (loop_counter-- > 0) {
        /* 1. ZERO_EXTRACT: Bitfield assignments */
        local_bf.f2 = (loop_counter & 0x1F);      /* 5-bit field */
        local_bf.f3 = (loop_counter * 3) & 0xFF;  /* 8-bit field */
        g_bf.f4 = local_bf.f3;                    /* 16-bit field */
        
        /* Mix with global to prevent optimization */
        if (g_bf.f1 != 0) {
            local_bf.f1 = g_bf.f1;
        }
        
        /* 2. STRICT_LOW_PART: Partial register updates */
        local_split.parts.low = (loop_counter & 0xFFFF);
        /* Force high part computation */
        local_split.parts.high = (local_split.parts.low * 2) & 0xFFFF;
        
        /* Update global with partial assignment */
        g_split.parts.low = local_split.parts.high;
        
        /* 3. SUBREG: Register variable with smaller operations */
        reg_var = reg_var + 1;
        /* Truncation to smaller type */
        volatile short reg_low = (short)(reg_var & 0xFFFF);
        reg_var = (reg_var & 0xFFFF0000) | (reg_low + 1);
        
        /* Vector operations with element extraction (SUBREG) */
        local_vec[2] = reg_low;  /* Element assignment */
        volatile int vec_element = local_vec[1];  /* Element extraction */
        
        /* 4. Complex MEM addressing: Multi-level pointer dereferencing */
        int idx = (loop_counter & 3);
        /* Complex addressing: top->l2->l3->data[idx] */
        volatile int mem_val = top->l2->l3->data[idx];
        
        /* Even more complex: with computation in index */
        volatile int *ptr = &top->l2->l3->data[(idx + vec_element) & 3];
        *ptr = mem_val + local_bf.f3;
        
        /* 5. Inline assembly influencing RTL generation */
        __asm__ volatile (
            "movw %w[low], %%cx\n\t"   /* 'w' modifier for word register */
            "movl %[full], %%ebx\n\t"
            : 
            : [low] "r" (local_split.parts.low),
              [full] "r" (local_split.full)
            : "cx", "ebx", "memory"
        );
        
        /* Combine results */
        result += local_bf.f2 + local_split.parts.low + reg_low + mem_val;
        
        /* Conditional branch to create more complex CFG */
        if (result > 10000) {
            result >>= 1;
            clobber_registers();
        }
        
        /* Switch for additional control flow complexity */
        switch (loop_counter & 7) {
            case 0:
                local_bf.f1 = result & 7;
                break;
            case 1:
                local_split.parts.high = (result >> 8) & 0xFF;
                break;
            case 2:
                reg_var = (reg_var & 0xFFFF0000) | (result & 0xFFFF);
                break;
            default:
                /* Complex memory store with offset */
                top->l2->l3->data[(result >> 2) & 3] = result;
                break;
        }
    }
    
    /* Final computation using all variables */
    int final_result = 
        (int)local_bf.f1 + 
        (int)local_bf.f2 * 256 + 
        (int)local_split.full + 
        (reg_var & 0xFFFF) + 
        local_vec[0] + 
        top->l2->l3->data[0];
    
    /* Cleanup */
    free(bot);
    free(mid);
    free(top);
    
    return (final_result & 0xFF);  /* Return non-zero to indicate success */
}
