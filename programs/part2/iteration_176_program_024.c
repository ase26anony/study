/* test_gengtype_balanced.c - Test file for gengtype balanced character parsing */

/* 1. Function-like macros with parentheses */
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define SQUARE(x) ((x) * (x))
#define COMPLEX_MACRO(a, b, c) (((a) + (b)) * (c))

/* 2. Complex declarators with parentheses */
int (*func_ptr)(int, double);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Multi-dimensional arrays and variable-length arrays */
int matrix[3][4];
int vla_matrix[][5] = {{1,2,3,4,5}, {6,7,8,9,10}};
enum { SIZE = 10 };
int sized_array[SIZE];
const int const_size = 20;
int var_size[const_size];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int section_var __attribute__((section(".data")));
int vector_type __attribute__((vector_size(16)));

/* 5. Struct with nested union and complex initializer */
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
        int array[4];
    } data;
    char *name;
};

/* Global struct with nested initializer */
struct Outer global_struct = {
    .type = 1,
    .data = {
        .point = {
            .x = 10,
            .y = {20}  /* Nested braces */
        }
    },
    .name = "test"
};

/* 6. Another struct with designated initializers */
struct Nested {
    int a;
    struct {
        int b;
        int c[3];
    } inner;
};

struct Nested nested_var = {
    .a = 1,
    .inner = {
        .b = 2,
        .c = {3, 4, 5}  /* Array initializer with braces */
    }
};

/* 7. Union with initializer */
union Data {
    int i;
    float f;
    char str[20];
};

union Data data_union = { .i = 42 };

/* 8. Preprocessor conditional with balanced characters */
#ifdef TEST_DEFINE
    #define CONDITIONAL_MACRO(x) ((x) + 1)
    int conditional_array[TEST_DEFINE ? 10 : 20];
#else
    #define CONDITIONAL_MACRO(x) ((x) - 1)
    int conditional_array[5];
#endif

/* 9. __typeof__ usage */
int typeof_var = 5;
__typeof__(typeof_var) typeof_copy;

/* 10. Compound literals */
int *compound_literal_ptr = (int[]){1, 2, 3, 4, 5};
struct Point {
    int x;
    int y;
} *point_ptr = &(struct Point){.x = 100, .y = 200};

/* 11. Function declarations with complex parameters */
void process_array(int arr[][5], int rows);
int (*get_callback(void))(int, int);

/* 12. Nested parentheses in expressions */
#define NESTED_EXPR(x) ((((x) + 1) * 2) - 3)

/* Main function containing all triggering constructs */
int main(void) {
    /* Use function-like macro */
    int max_val = MAX(10, 20);
    int square_val = SQUARE(5);
    
    /* Use complex declarator */
    int (*local_func_ptr)(int, double) = 0;
    
    /* Array access with brackets */
    int element = matrix[1][2];
    int vla_element = vla_matrix[0][3];
    
    /* Compound literal in function */
    int *local_ptr = (int[]){10, 20, 30, 40};
    
    /* GCC built-in with parentheses */
    int chosen = __builtin_choose_expr(1, 100, 200);
    
    /* __typeof__ in function */
    __typeof__(max_val) local_copy = max_val;
    
    /* Nested struct access */
    nested_var.inner.c[1] = 99;
    
    /* Array initialization with braces */
    int local_array[3] = {1, 2, 3};
    int multi_array[2][3] = {{1,2,3}, {4,5,6}};
    
    /* Use aligned variable */
    aligned_var = 42;
    
    /* Complex expression with parentheses */
    int result = (max_val * square_val) + (chosen / 10);
    
    /* Prevent dead code elimination */
    if (result > 0) {
        return 0;
    }
    
    return 1;
}

/* 13. Additional global with deeply nested braces */
struct DeeplyNested {
    struct {
        struct {
            int a;
            int b[2][2];
        } level2;
    } level1;
} deep_struct = {
    .level1 = {
        .level2 = {
            .a = 1,
            .b = {{1,2}, {3,4}}  /* Multi-dimensional array initializer */
        }
    }
};

/* 14. Function with array parameter (size in brackets) */
void process_matrix(int mat[][10], int size) {
    /* Function body with local variables */
    int local[10] = {0};
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 10; j++) {
            local[j] += mat[i][j];
        }
    }
}

/* 15. Enum with last comma (C99 feature) */
enum Colors {
    RED,
    GREEN,
    BLUE,
};

/* 16. Switch statement with cases (contains braces) */
int switch_example(int val) {
    switch (val) {
        case 1:
            return 10;
        case 2: {
            int temp = 20;
            return temp * 2;
        }
        default:
            return 0;
    }
}

/* 17. Do-while loop (contains parentheses and braces) */
int loop_example(void) {
    int i = 0;
    do {
        i++;
    } while (i < 10);
    return i;
}
