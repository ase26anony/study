/* test_gengtype_coverage.h - Complex type definitions to cover consume_balanced parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex nested type definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[3])(void (*)(int), char);
    
    /* Union with anonymous struct containing bit-fields */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:3;
            unsigned int flag3:4;
        } bits;
        long long value;
    } data_union;
    
    /* Multi-dimensional array */
    double matrix[4][5][2];
    
    /* Nested struct with function pointer returning pointer to array */
    struct InnerStruct {
        char *(*get_name)(int id);
        int (*process)(int (*callback)(int, char **), void *context);
        float values[10];
    } inner;
    
    /* Pointer to function returning pointer to function */
    void (*(*complex_func)(int (*)(double)))(char *);
};

/* 2. Function pointer declarations with varied signatures */
typedef int (*SimpleFunc)(void);
typedef char *(*StringProcessor)(const char *input, int length);
typedef void (*(*MetaFunc)(int))(double);

/* Function pointer with nested parameter */
int (*signal_handler)(int sig, void (*handler)(int));
void (*(*get_handler_factory(void))(int))(void);

/* 3. Multi-dimensional arrays and flexible array members */
struct ArrayContainer {
    int fixed[5][10];
    int *ptr_array[3];
    int flexible[];
};

struct NestedArrays {
    char *strings[20][30];
    struct ArrayContainer *containers[5];
    float (*func_ptrs[2][3])(int, int);
};

/* 4. Nested anonymous structs/unions with bit-fields */
struct BitFieldStruct {
    union {
        struct {
            unsigned int a:2;
            unsigned int b:4;
            unsigned int c:6;
        } small_fields;
        struct {
            unsigned long x:16;
            unsigned long y:16;
        } wide_fields;
    } u1;
    
    struct {
        int regular_field;
        union {
            short s;
            char c[2];
        } anonymous_union;
    } nested_anon;
    
    unsigned int trailing:10;
};

/* 5. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))()

/* Usage of macros */
PTR_FUNC(int) *int_func_ptr;
ARRAY_TYPE(char *, 10) string_array;
NESTED_PTR(double) complex_double_func;

struct MacroStruct {
    PTR_FUNC(void) void_func;
    ARRAY_TYPE(struct BitFieldStruct, 5) bitfield_array;
};

/* 6. Attribute syntax with parentheses */
struct __attribute__((aligned(16), packed)) AlignedStruct {
    int data[4];
    char padding;
} __attribute__((deprecated));

typedef int __attribute__((vector_size(16))) v4si;

int __attribute__((noinline, noclone)) 
special_function(int x, int y) __attribute__((warn_unused_result));

struct __attribute__((designated_init)) DesignatedInit {
    int field1;
    char field2;
    float field3;
};

/* 7. All bracket types in single declaration - The ultimate test */
struct UltimateType {
    /* Function returning pointer to function taking array pointer */
    void (*(*level1)(int (*)(char[10]), void *))(float);
    
    /* Array of function pointers with complex signatures */
    int (*(*func_array[2][3])(int (*)(char **), double))(void);
    
    /* Nested anonymous union with bit-fields and function pointer */
    union {
        struct {
            unsigned int a:5;
            unsigned int b:11;
            void (*callback)(int, int (*)(int));
        } s1;
        struct {
            long long data;
            char (*strings[5])[20];
        } s2;
    } u;
    
    /* Multi-dimensional flexible array member (GCC extension) */
    int flex_member[][3][2];
    
    /* Function pointer with attributes */
    int (__attribute__((stdcall)) *attr_func)(int, ...);
} __attribute__((aligned(32)));

/* Additional complex combinations */
typedef union {
    struct {
        int (*(*get_processor(void))[5])(int);
        char buffer[100];
    } processor_section;
    
    struct {
        void (*handlers[10])(union UltimateType *);
        float matrix[][4];
    } handler_section;
} UnionWithEverything;

/* Pointer to array of function pointers */
int (*(*pointer_to_funcptr_array)[10])(int, char);

/* Nested typedef with all bracket types */
typedef struct {
    int (*compare)(const void *, const void *);
    void (*free)(void *);
    unsigned int flags:8;
    char name[];
} GenericCallback __attribute__((transparent_union));

/* Function with parameter containing nested brackets */
void register_callback(
    int (*callback)(int, char (*)[10], void (*)(void)),
    void *context __attribute__((nonnull))
);

/* Variable with complex declarator */
int (*(*global_complex_var)(int (*)(int[5])))[10];

/* Final test: Everything combined */
struct __attribute__((may_alias)) FinalTest {
    /* [ ] brackets */
    int array_decl[5][sizeof(int*)];
    
    /* ( ) brackets */
    void (*func_ptr)(struct FinalTest *self, int param);
    
    /* { } brackets */
    union {
        struct {
            int x:4;
            int y:12;
            int z:16;
        } packed_bits;
        unsigned int raw;
    } bitfield_container;
    
    /* All together */
    int (*(*all_together[2])(int (*)(char[10])))[3];
} __attribute__((packed));

#endif /* TEST_GENGTYPE_COVERAGE_H */
