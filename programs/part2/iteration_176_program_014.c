/* test-gengtype-coverage.c - Test file for gengtype balanced character parsing */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(x, y) ((x) * (y))
#define COMPLEX_MACRO(a, b, c) ({ \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    (_a + _b) * (c); \
})

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[10][20];
int var_size[FOO(5)][BAR(2, 3)];
enum { SIZE = 15 };
int enum_array[SIZE];
const int const_size = 8;
int const_array[const_size];

/* 4. GCC attributes with parentheses and brackets */
int attr1 __attribute__((aligned(16)));
int attr2 __attribute__((vector_size(32)));
int attr3 __attribute__((aligned(16), packed));

/* 5. Structure with nested union and complex initializer */
struct Outer {
    int a;
    union {
        int i;
        double d;
        struct {
            char c;
            short s;
        } inner;
    } u;
    int *ptr_array[5];
};

/* Global instance with nested brace initializer */
struct Outer global_var = {
    .a = FOO(10),
    .u = { 
        .inner = { 
            .c = 'x', 
            .s = 42 
        } 
    },
    .ptr_array = { NULL, NULL, NULL, NULL, NULL }
};

/* 6. Another structure with designated initializers */
struct Point {
    int x;
    int y;
    int z;
};

struct Line {
    struct Point start;
    struct Point end;
};

/* 7. Compound literal usage */
struct Line line1 = {
    .start = { .x = 0, .y = 0, .z = 0 },
    .end = { .x = 10, .y = 20, .z = 30 }
};

/* 8. Preprocessor conditionals */
#ifdef __GNUC__
#define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
#define GCC_SPECIFIC(x) (x)
#endif

/* 9. __typeof__ usage */
#define MAX(a, b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b; \
})

/* 10. C++ style alignas (C11/C++11) */
#ifdef __cplusplus
alignas(16) int aligned_int;
#else
_Alignas(16) int aligned_int;
#endif

/* 11. Function with complex parameter list */
int process_data(int (*callback)(int, void*), 
                 int data[], 
                 size_t size,
                 struct Point points[]) {
    /* Nested blocks with braces */
    {
        int local = 0;
        for (int i = 0; i < size; i++) {
            local += data[i];
        }
    }
    
    /* Switch with braces */
    switch (size) {
        case 0: return 0;
        case 1: return data[0];
        default: {
            int sum = 0;
            for (size_t i = 0; i < size; i++) {
                sum += data[i];
            }
            return sum;
        }
    }
}

/* 12. Main function containing all triggering constructs */
int main(void) {
    /* Use function-like macro */
    int x = FOO(5);
    int y = BAR(x, 2);
    
    /* Complex function pointer usage */
    int (*func_array[3])(int) = { NULL, NULL, NULL };
    
    /* Multi-dimensional array access with brackets */
    multi_dim[0][0] = 1;
    multi_dim[1][1] = 2;
    
    /* Compound literal */
    int *dynamic_array = (int[]){1, 2, 3, 4, 5};
    
    /* Nested structure initialization with braces */
    struct Outer local_var = {
        .a = 100,
        .u = { .i = 42 },
        .ptr_array = { &x, &y, NULL }
    };
    
    /* __typeof__ with parentheses */
    __typeof__(*dynamic_array) first_element = dynamic_array[0];
    
    /* GCC built-in with parentheses */
    int chosen = __builtin_choose_expr(sizeof(int) == 4, 42, 24);
    
    /* Array with computed size using ternary */
    int computed_array[__builtin_constant_p(1) ? 10 : 20];
    
    /* Nested initializer list */
    int matrix[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };
    
    /* Lambda-like expression (GCC extension) */
    int result = ({
        int sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += dynamic_array[i];
        }
        sum;
    });
    
    /* Attribute in local variable */
    int local_attr __attribute__((unused)) = result;
    
    /* Use MAX macro with __typeof__ */
    int max_val = MAX(x, y);
    
    /* Return statement with parentheses */
    return (result + max_val + chosen + first_element);
}

/* 13. Additional constructs at file scope */
#if 0
/* This block won't be compiled but will be parsed */
int disabled_code(int param) {
    return ({ 
        struct { 
            int a; 
            int b[3]; 
        } s = { 
            .a = param, 
            .b = {1, 2, 3} 
        }; 
        s.a + s.b[0]; 
    });
}
#endif

/* 14. Union with anonymous struct (C11) */
union Anonymous {
    struct {
        int x, y;
    };
    int coords[2];
};

/* 15. Final global with all character types */
struct FinalStruct {
    int (*func)(int[][5], struct Point);
    union {
        int i;
        double d;
    } values[3];
} final_instance = {
    .func = NULL,
    .values = {
        [0] = { .i = 1 },
        [1] = { .d = 2.0 },
        [2] = { .i = 3 }
    }
};
