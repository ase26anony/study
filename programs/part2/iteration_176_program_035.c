/* test-gengtype-coverage.c */
/* This file is designed to exercise the balanced character parsing
   logic in gengtype-parse.cc */

/* 1. Function-like macros with parentheses */
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define SQUARE(x) ((x) * (x))
#define NESTED_MACRO(x) (SQUARE((x) + 1))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets - multi-dimensional */
int multi_dim_array[3][4][5];
extern int incomplete_array[];

/* 4. Array with non-constant size using enum */
enum { ARRAY_SIZE = 10 };
int sized_array[ARRAY_SIZE];

/* 5. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int section_var __attribute__((section(".data")));

/* 6. C++ style alignas (C11/C++11) */
_Alignas(16) int aligned_c11;
#ifdef __cplusplus
alignas(32) double aligned_cpp;
#endif

/* 7. Struct with nested union and complex initializer */
struct Outer {
    int type;
    union {
        int ival;
        double dval;
        char *sval;
        int arr_val[5];
    } data;
    struct Inner {
        int x;
        int y;
    } point;
};

/* Global instance with nested brace initializer */
struct Outer global_var = { 
    .type = 1,
    .data = { .ival = 42 },
    .point = { .x = 10, .y = {20} }
};

/* 8. Another struct with designated initializers */
struct ComplexInit {
    int a;
    int b[3];
    struct {
        float f;
        char c;
    } nested;
} complex_global = { 
    .a = 1, 
    .b = { [0] = 2, [2] = 3 },
    .nested = { .f = 3.14, .c = 'A' }
};

/* 9. Union with array */
union DataUnion {
    int i;
    float f;
    char str[20];
    struct {
        int x;
        int y;
    } coord;
};

/* 10. Preprocessor conditionals containing triggering constructs */
#ifdef TEST_FEATURE
    #define FEATURE_MACRO(x) (((x) + 1) * 2)
    int feature_array[FEATURE_MACRO(5)];
    struct FeatureStruct {
        int val;
    } feature_var = { .val = 100 };
#else
    #define DEFAULT_MACRO(x) ((x) * 3)
    int default_array[DEFAULT_MACRO(3)];
#endif

/* 11. __typeof__ usage */
int typeof_var;
__typeof__(typeof_var) typeof_copy;
__typeof__(*complex_func_ptr) typeof_func_result;

/* 12. Variable Length Array (C99) - if supported */
void vla_function(int n) {
    int vla[n];
    int vla_2d[n][n+1];
}

/* 13. Compound literals */
int *compound_literal_ptr = (int[]){1, 2, 3, 4};
struct Point { int x; int y; };
struct Point *point_ptr = &(struct Point){.x = 5, .y = 10};

/* 14. GCC builtins with parentheses */
int builtin_result = __builtin_choose_expr(1, 42, 0);
int constant_p = __builtin_constant_p(1+2);
long long abs_result = __builtin_llabs(-100LL);

/* 15. Nested parentheses in expressions */
#define NESTED_EXPR(x) (((((x) + 1) * 2) - 3) / 4)

/* Main function containing multiple triggering constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func_ptr)(int) = (int (*)(int))0;
    
    /* Array with computed size */
    int computed_size = MAX(5, 10);
    int dynamic_array[computed_size > 8 ? 10 : 5];
    
    /* Nested initializers */
    struct Outer local_var = {
        .type = 2,
        .data = { .dval = 3.14159 },
        .point = { .x = 0, .y = 0 }
    };
    
    /* Compound literal in expression */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += ((int[]){10, 20, 30})[i];
    }
    
    /* __typeof__ in function */
    __typeof__(sum) sum_copy = sum;
    
    /* GCC attribute on local variable */
    int local_aligned __attribute__((aligned(8))) = 42;
    
    /* Nested parentheses in actual code */
    int nested_result = ((((1 + 2) * 3) - 4) / 5);
    
    /* Array with multiple dimensions */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    /* Use all variables to avoid dead code elimination */
    return global_var.type + complex_global.a + sum + nested_result + 
           matrix[0][0] + local_aligned + builtin_result;
}

/* 16. Additional global with deeply nested braces */
struct DeeplyNested {
    struct Level1 {
        struct Level2 {
            struct Level3 {
                int values[4];
            } l3;
        } l2[2];
    } l1;
} deep_nested = {
    .l1 = {
        .l2 = {
            [0] = { .l3 = { .values = {1, 2, {3}, 4} } },
            [1] = { .l3 = { .values = {5, 6, 7, 8} } }
        }
    }
};

/* 17. Function with parameter attributes */
void attributed_func(int param1 __attribute__((unused)), 
                     char *param2 __attribute__((nonnull))) {
    /* Empty */
}

/* 18. Asm statement with braces (GCC extension) */
void asm_example(void) {
    int src = 1, dst;
    asm volatile ("mov %1, %0"
                  : "=r"(dst)
                  : "r"(src));
}

/* 19. Array in struct with bitfield */
struct WithBitfield {
    unsigned int flags : 4;
    int small_array[2];
    unsigned int more_flags : 8;
} bitfield_struct = { .flags = 3, .small_array = {1, 2}, .more_flags = 0xFF };

/* 20. Multiple consecutive brackets (edge case) */
int consecutive_brackets[2][3][4][5];
