/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((x) + (sizeof(struct { char c; }) * 2))
#define ARRAY_MACRO int arr_macro[] = NESTED_BRACES

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
/* Requirement 1: Balanced token sequences in type definitions */
struct Outer {
    /* Function pointer with nested parentheses and struct */
    int (*complex_fp)(int (*callback)(int[2][(3+1)]), 
                      struct { 
                          int a; 
                          union { 
                              char c; 
                              long l; 
                          } u; 
                      } s);
    
    /* Nested struct with function pointer returning array pointer */
    struct {
        int (*(*nested_fp)[(2+3)])(void);
    } inner;
};

/* Union with deeply nested type */
union DeepUnion {
    char c;
    struct {
        int i;
        struct {
            double d;
            int (*fp)(int, ...);
        } deep;
    } s;
    long l;
};
#endif /* TEST_COMPLEX */

/* Requirement 3: Attribute specifications with multiple parentheses */
struct __attribute__((aligned((16)), packed)) AttributedStruct {
    int data __attribute__((aligned((8))));
    char buffer[32] __attribute__((aligned((sizeof(double))))));
} __attribute__((packed));

/* C++11 style attribute (valid in C23/C++11) */
#if __cplusplus >= 201103L || __STDC_VERSION__ >= 202311L
[[deprecated("Use NewStruct instead")]]
#endif
typedef struct OldStruct {
    int value;
} OldStruct_t;

/* Requirement 4: Array declarations with complex dimensions */
int multi_dim_array[(2+3)*4][5];
int *pointer_array[(sizeof(struct {int x; char y[(4)];})/4)];

/* Array with nested type in size expression */
extern int extern_array[sizeof(union { 
    struct { 
        int a; 
        double b; 
    } s; 
    long long ll; 
})];

/* Requirement 5: Initializer lists with nested braces and parentheses */
int initialized = { { ( 1 + (2) ) } };

struct Point { 
    int x; 
    struct { 
        int y; 
        int z; 
    } coord; 
};

struct Point points[] = { 
    [0] = { 
        .x = (1 + (2 * 3)), 
        .coord = {
            .y = {2}, 
            .z = ( (4) ) 
        }
    }, 
    [1] = { 
        .x = 5, 
        .coord = { 
            .y = 6, 
            .z = 7 
        } 
    } 
};

/* Using macro with nested delimiters */
ARRAY_MACRO;

/* Function with complex return type and attributes */
#if defined(__GNUC__)
__attribute__((noinline, constructor))
#endif
static void* complex_function(
    int param1[(sizeof(int*) + 3)],
    struct { 
        int (*member)(int[][(2)]); 
    } param2
) {
    /* Local struct with nested braces */
    struct Local {
        int a;
        int b;
    } local = { .a = {1}, .b = 2 };
    
    /* Compound literal with nested delimiters */
    int *ptr = (int*)(&(struct { int x; int y; }){ .x = 1, .y = 2 });
    
    return (void*)&local;
}

/* Enum with complex initializers */
enum ComplexEnum {
    VALUE1 = (1 << (sizeof(char)*8 - 1)),
    VALUE2 = sizeof(struct { 
        char a; 
        int b[(2+2)]; 
    }),
    VALUE3 = {0}  /* Braced initializer in enum */
};

/* Requirement 1 & 4 combined: Typedef with nested everything */
typedef int (*(*ComplexTypedef)[(3)])(
    struct { 
        int a[2][2]; 
        union { 
            char c; 
            int i; 
        } u; 
    } param
);

/* Main function that references defined types to avoid dead code elimination */
int main(void) {
    static struct AttributedStruct as = { .data = 42 };
    struct Point *p = &points[0];
    
    /* Use macro expression */
    int x = COMPLEX_PARENS(10);
    
    /* Reference conditional types */
#ifdef TEST_COMPLEX
    struct Outer o;
    (void)o;
#endif
    
    /* Reference array */
    multi_dim_array[0][0] = 1;
    
    /* Reference typedef */
    ComplexTypedef ct = NULL;
    (void)ct;
    
    return 0;
}

/* Additional nested structures in global scope */
struct GlobalContainer {
    struct {
        int *array_ptr[(2 + (3 * 4))];
        struct {
            void (*func_ptr)(int, ...);
        } nested;
    } inner;
    
    union {
        char str[sizeof(struct { int a; double b; })];
        long values[4];
    } data;
};

/* Final conditional block with balanced tokens */
#if 0
/* This won't be compiled but will be parsed */
struct Unused {
    int triple_nested[(2)][(3)][(4)];
    void (*fp)(struct { 
        int a; 
        struct { 
            char c; 
        } s; 
    });
};
#endif
