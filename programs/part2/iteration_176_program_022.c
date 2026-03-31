/* test-gengtype-coverage.c */
/* This file is designed to exercise the balanced character parsing
   in gengtype-parse.cc, specifically the switch cases for '(', '[', and '{' */

/* 1. Function-like macros with parentheses */
#define ADD(x, y) ((x) + (y))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SQUARE(x) ((x) * (x))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[3][4][5];
enum { SIZE = 10 };
int var_size[SIZE];
const int const_size = 20;
int dynamic_like[const_size + 5];
int attr_array[10] __attribute__((aligned(64)));

/* 4. GCC attributes with various brackets/parentheses */
int aligned_var __attribute__((aligned(32)));
int packed_struct __attribute__((packed));
int section_var __attribute__((section(".data")));

/* 5. Preprocessor conditionals */
#ifdef __GNUC__
#  define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
#  define GCC_SPECIFIC(x) (x)
#endif

/* 6. Struct/union definitions with nested initializers */
struct Outer {
    int a;
    union {
        int i;
        float f;
        char str[20];
    } u;
    struct {
        int x;
        int y;
        int z[3];
    } point;
};

/* Global instance with complex initializer */
struct Outer global_outer = {
    .a = 42,
    .u = { .f = 3.14 },
    .point = { 
        .x = 1, 
        .y = 2, 
        .z = { [0] = 10, [1] = 20, [2] = 30 }
    }
};

/* Another struct with designated initializers */
struct Nested {
    struct {
        int first;
        int second;
    } inner;
    int arr[2][3];
} nested_var = {
    .inner = { .first = 100, .second = 200 },
    .arr = { {1, 2, 3}, {4, 5, 6} }
};

/* 7. __typeof__ usage */
__typeof__(*complex_func_ptr) type_var;

/* 8. Compound literals */
int *compound_ptr = (int[]){1, 2, 3, 4, 5};
struct Outer *outer_ptr = &(struct Outer){
    .a = 99,
    .u = { .i = 123 },
    .point = { .x = 0, .y = 0, .z = {0} }
};

/* Main function containing various constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func)(int) = (int (*)(int))0;
    
    /* Array with computed size */
    int computed[ADD(5, 3) * 2];
    
    /* Nested initializer in local scope */
    struct Outer local = {
        .a = MAX(10, 20),
        .u = { .str = "test" },
        .point = { 
            .x = SQUARE(3),
            .y = 0,
            .z = { [1] = 100 }
        }
    };
    
    /* Compound literal in expression */
    int sum = ((int[]){1, 2, 3})[1] + ((int[]){4, 5, 6})[2];
    
    /* __builtin_choose_expr with parentheses */
    int chosen = __builtin_choose_expr(
        sizeof(int) == 4,
        ((int[]){1, 2, 3})[0],
        ((int[]){4, 5, 6})[0]
    );
    
    /* Nested attribute in declaration */
    int __attribute__((aligned(16))) aligned_local[4] = {0};
    
    /* Multi-dimensional array initializer */
    int matrix[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };
    
    /* Use typeof with compound literal */
    __typeof__(matrix[0]) row = {7, 8, 9};
    
    /* Prevent dead code elimination */
    if (global_outer.a > 0) {
        return local.a + sum + chosen + row[0];
    }
    
    return 0;
}

/* Additional constructs at file scope */
/* Lambda-like function pointer initialization */
void (*callback)(void) = (void (*)(void))0;

/* Array of function pointers */
int (*func_array[3])(int) = {
    (int (*)(int))0,
    (int (*)(int))0,
    (int (*)(int))0
};

/* Struct with flexible array member */
struct Flex {
    int count;
    int data[];
} flex_instance = { .count = 3 };

/* Union with nested struct */
union ComplexUnion {
    struct {
        int a;
        int b[2];
    } s;
    long long ll;
} complex_union = { .s = { .a = 1, .b = {2, 3} } };

/* Final check: all three characters in sequence */
int final_check[5] = { [0] = ({ int x = 5; x; }), 2, 3, 4, 5 };
