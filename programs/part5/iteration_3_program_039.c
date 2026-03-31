/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to exercise
 * the type enumeration switch in gengtype.cc (lines 182-213).
 * When processed by gengtype during a GCC build, it should trigger
 * multiple type kind cases.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC build this would be
 * the actual garbage collector annotation macro */
#define GTY(x) 

/* Forward declarations to create circular references */
struct CircularList;
struct Tree;
union PolyValue;

/* ========== TYPE_SCALAR triggers ========== */
GTY(())
struct Scalars {
    int int_field;          /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    double double_field;    /* TYPE_SCALAR */
    _Bool bool_field;       /* TYPE_SCALAR */
    long long_field;        /* TYPE_SCALAR */
    short short_field;      /* TYPE_SCALAR */
    signed char schar_field;/* TYPE_SCALAR */
};

/* ========== TYPE_STRING triggers ========== */
GTY(())
struct WithStrings {
    const char* static_string;  /* TYPE_STRING */
    char* dynamic_string;       /* TYPE_POINTER (but content may be TYPE_STRING) */
    char fixed_string[64];      /* TYPE_ARRAY of TYPE_SCALAR */
};

/* ========== TYPE_STRUCT triggers ========== */
GTY(())
struct NestedStruct {
    struct Scalars scalars;     /* TYPE_STRUCT */
    struct WithStrings strings; /* TYPE_STRUCT */
    int extra_field;
};

/* ========== TYPE_UNION triggers ========== */
GTY(())
union DataUnion {
    int as_int;                 /* TYPE_SCALAR */
    float as_float;             /* TYPE_SCALAR */
    double as_double;           /* TYPE_SCALAR */
    void* as_pointer;           /* TYPE_POINTER */
    char as_string[32];         /* TYPE_ARRAY */
};

/* ========== TYPE_POINTER triggers ========== */
GTY(())
struct WithPointers {
    int* int_ptr;               /* TYPE_POINTER to TYPE_SCALAR */
    struct Scalars* struct_ptr; /* TYPE_POINTER to TYPE_STRUCT */
    union DataUnion* union_ptr; /* TYPE_POINTER to TYPE_UNION */
    void (*func_ptr)(void);     /* TYPE_POINTER (function pointer) */
    struct WithPointers* self;  /* TYPE_POINTER (self-reference) */
};

/* ========== TYPE_ARRAY triggers ========== */
GTY(())
struct WithArrays {
    int int_array[10];          /* TYPE_ARRAY of TYPE_SCALAR */
    struct Scalars struct_array[5]; /* TYPE_ARRAY of TYPE_STRUCT */
    union DataUnion union_array[3]; /* TYPE_ARRAY of TYPE_UNION */
    char* pointer_array[8];     /* TYPE_ARRAY of TYPE_POINTER */
    int multi_dim[3][4][5];     /* TYPE_ARRAY of TYPE_ARRAY of TYPE_ARRAY */
};

/* ========== TYPE_CALLBACK triggers ========== */
/* Function pointer types that might be treated as TYPE_CALLBACK */
typedef int (*Comparator)(const void*, const void*);
typedef void (*CallbackFunc)(int, void*);

GTY(())
struct WithCallbacks {
    Comparator compare;         /* Possibly TYPE_CALLBACK */
    CallbackFunc callback;      /* Possibly TYPE_CALLBACK */
    void (*handlers[5])(void);  /* TYPE_ARRAY of TYPE_POINTER/TYPE_CALLBACK */
};

/* ========== Complex nested structure ========== */
GTY(())
struct ComplexNode {
    int id;                     /* TYPE_SCALAR */
    char* name;                 /* TYPE_POINTER/TYPE_STRING */
    
    /* Nested anonymous union */
    union {
        struct {
            int x, y;           /* TYPE_SCALAR, TYPE_SCALAR */
        } point;
        struct {
            float width, height;/* TYPE_SCALAR, TYPE_SCALAR */
        } rect;
    } shape;                    /* TYPE_UNION */
    
    struct ComplexNode* left;   /* TYPE_POINTER */
    struct ComplexNode* right;  /* TYPE_POINTER */
    struct ComplexNode* children[4]; /* TYPE_ARRAY of TYPE_POINTER */
    
    /* Flexible array member */
    int data[];                 /* TYPE_ARRAY (flexible) */
};

/* ========== Circular reference structure ========== */
GTY(())
struct CircularList {
    int value;                  /* TYPE_SCALAR */
    struct CircularList* next;  /* TYPE_POINTER (circular) */
    struct CircularList* prev;  /* TYPE_POINTER (circular) */
};

/* ========== Tree-like structure ========== */
GTY(())
struct Tree {
    enum { NODE_INT, NODE_FLOAT, NODE_STRING } type; /* TYPE_SCALAR (enum) */
    
    union {
        int int_val;            /* TYPE_SCALAR */
        float float_val;        /* TYPE_SCALAR */
        char* string_val;       /* TYPE_POINTER/TYPE_STRING */
    } value;                    /* TYPE_UNION */
    
    struct Tree* left;          /* TYPE_POINTER */
    struct Tree* right;         /* TYPE_POINTER */
};

/* ========== Bitfield structure ========== */
GTY(())
struct WithBitfields {
    unsigned int flag1 : 1;     /* TYPE_SCALAR (bitfield) */
    unsigned int flag2 : 2;     /* TYPE_SCALAR (bitfield) */
    unsigned int flag3 : 3;     /* TYPE_SCALAR (bitfield) */
    int normal_field;           /* TYPE_SCALAR */
};

/* ========== External reference ========== */
/* Declare but don't define - forces gengtype to consider the type */
struct ExternalStruct;
extern struct ExternalStruct* external_ref;

/* ========== Function to prevent optimization ========== */
/* Use volatile and noinline to ensure types are referenced */
static volatile size_t global_size_sum = 0;

__attribute__((noinline)) 
static void accumulate_size(size_t size) {
    global_size_sum += size;
}

__attribute__((noinline))
static void take_address(const void* ptr) {
    /* Do nothing, just force address to be taken */
    (void)ptr;
}

/* ========== Main function ========== */
int main(void) {
    /* Declare instances of all complex types */
    struct Scalars scalars;
    struct WithStrings strings;
    struct NestedStruct nested;
    union DataUnion data_union;
    struct WithPointers pointers;
    struct WithArrays arrays;
    struct WithCallbacks callbacks;
    struct ComplexNode* complex_node = NULL;
    struct CircularList circular_list[2];
    struct Tree tree;
    struct WithBitfields bitfields;
    
    /* Initialize circular references */
    circular_list[0].next = &circular_list[1];
    circular_list[0].prev = &circular_list[1];
    circular_list[1].next = &circular_list[0];
    circular_list[1].prev = &circular_list[0];
    
    /* Force computation of sizes for all types */
    accumulate_size(sizeof(struct Scalars));
    accumulate_size(sizeof(struct WithStrings));
    accumulate_size(sizeof(struct NestedStruct));
    accumulate_size(sizeof(union DataUnion));
    accumulate_size(sizeof(struct WithPointers));
    accumulate_size(sizeof(struct WithArrays));
    accumulate_size(sizeof(struct WithCallbacks));
    accumulate_size(sizeof(struct ComplexNode));
    accumulate_size(sizeof(struct CircularList));
    accumulate_size(sizeof(struct Tree));
    accumulate_size(sizeof(struct WithBitfields));
    
    /* Take addresses of all instances and their members */
    take_address(&scalars);
    take_address(&scalars.int_field);
    take_address(&strings);
    take_address(&strings.static_string);
    take_address(&nested);
    take_address(&nested.scalars.float_field);
    take_address(&data_union);
    take_address(&pointers);
    take_address(&pointers.func_ptr);
    take_address(&arrays);
    take_address(&arrays.multi_dim);
    take_address(&callbacks);
    take_address(&circular_list);
    take_address(&tree);
    take_address(&bitfields);
    
    /* Create pointer chains */
    pointers.self = &pointers;
    pointers.struct_ptr = &scalars;
    pointers.union_ptr = &data_union;
    
    /* Reference external type */
    take_address(external_ref);
    
    /* Print result to prevent optimization */
    printf("Total size accumulated: %zu\n", global_size_sum);
    printf("Address of complex_node: %p\n", (void*)&complex_node);
    
    return 0;
}

/* External struct definition (after main to test ordering) */
struct ExternalStruct {
    int external_field;
    struct ComplexNode* node_ref;
};

/* Global variable definition */
struct ExternalStruct* external_ref = NULL;
