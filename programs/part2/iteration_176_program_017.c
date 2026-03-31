/* test-gengtype-coverage.c */
/* This file is designed to exercise the balanced character parsing
   in gengtype-parse.cc, specifically the switch cases for '(', '[', and '{' */

/* 1. Function-like macros with parentheses */
#define ADD(x, y) ((x) + (y))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define COMPLEX_MACRO(x) do { (x)++; } while(0)

/* 2. Complex declarators with parentheses */
int (*global_func_ptr)(double, int);
int (*(*nested_func_ptr)(void))[10];
void (*signal(int sig, void (*handler)(int)))(int);

/* 3. Array declarations with brackets */
int multi_dim[10][20];
extern int incomplete[];
const int const_array[] = {1, 2, 3};
enum { SIZE = 100 };
int var_len[SIZE + 1];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int vector_var __attribute__((vector_size(16)));

/* 5. Preprocessor conditionals */
#ifdef __GNUC__
# define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
# define GCC_SPECIFIC(x) (x)
#endif

/* 6. Struct with nested union and complex initializer */
struct Outer {
    int type;
    union {
        struct {
            int x;
            int y;
        } point;
        struct {
            int width;
            int height;
        } rect;
    } data;
    int (*callback)(struct Outer *);
};

/* Global struct with designated initializer containing nested braces */
struct Outer global_outer = {
    .type = 1,
    .data = {
        .point = {
            .x = 10,
            .y = {20}  /* Nested braces in designated initializer */
        }
    },
    .callback = 0
};

/* 7. Another struct with array member */
struct WithArray {
    int ids[5];
    struct Outer nested;
};

/* 8. Compound literal in global scope */
int *global_ptr = (int[]){1, 2, 3, 4};

/* 9. __typeof__ usage */
__typeof__(*global_ptr) type_var;

/* 10. Main function containing various constructs */
int main(void) {
    /* Function pointer declaration with parentheses */
    int (*local_func)(int) = 0;
    
    /* Multi-dimensional array with initialization */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    /* Array with computed size using ternary in brackets */
    int dyn_size[__builtin_constant_p(1) ? 10 : 20];
    
    /* Compound literal */
    int *local_ptr = (int[]){10, 20, 30};
    
    /* Nested struct initialization with braces */
    struct WithArray local_struct = {
        .ids = {0, 1, 2, 3, 4},
        .nested = {
            .type = 2,
            .data = {
                .rect = {
                    .width = 100,
                    .height = 200
                }
            },
            .callback = 0
        }
    };
    
    /* __builtin_choose_expr with parentheses */
    int chosen = __builtin_choose_expr(1, 42, 0);
    
    /* Lambda-like expression using statement expression (GCC extension) */
    int result = ({
        int sum = 0;
        for (int i = 0; i < 3; i++) {
            sum += local_ptr[i];
        }
        sum;
    });
    
    /* Use of function-like macro */
    result = ADD(result, chosen);
    result = MAX(result, 100);
    
    /* Attribute on local variable */
    int local_aligned __attribute__((aligned(8))) = result;
    
    /* __typeof__ in local scope */
    __typeof__(local_aligned) copy = local_aligned;
    
    /* Prevent dead code elimination */
    if (global_ptr[0] + matrix[0][0] + local_struct.ids[0] + copy > 0) {
        return 0;
    }
    
    return 1;
}

/* 11. Additional complex type at file scope */
union ComplexUnion {
    int (*func_array[5])(void);
    struct {
        int (*nested_func)(int[][10]);
    } inner;
};

/* 12. Pointer to array */
int (*ptr_to_array)[10];

/* 13. Function returning pointer to array */
int (*returns_array_ptr(void))[5] {
    static int arr[5] = {1, 2, 3, 4, 5};
    return &arr;
}

/* 14. Nested parentheses in declarator */
void (*(*complex_nested)(int))(void) = 0;

/* 15. Array of function pointers */
int (*func_ptr_array[3])(int, int);

/* 16. Struct containing flexible array member */
struct FlexStruct {
    int count;
    int data[];  /* Flexible array member */
};

/* 17. Designated initializer with array indices */
int designated_array[10] = {[0] = 1, [5] = 2, [9] = 3};

/* 18. Nested switch-like macro to generate more parentheses */
#define NESTED_CALL(f, x) (f((x)))
int double_it(int x) { return x * 2; }
int test_nested = NESTED_CALL(double_it, 21);

/* 19. alignas specifier (C11/C++11) */
_Alignas(32) char aligned_buffer[64];

/* 20. Final struct with bitfield and anonymous struct */
struct Final {
    unsigned int flag : 1;
    struct {
        int a;
        int b;
    };
    int arr[3];
} final_instance = {
    .flag = 1,
    .a = 100,
    .b = 200,
    .arr = {300, 400, 500}
};
