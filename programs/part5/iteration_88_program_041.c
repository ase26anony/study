#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED ========== */
/* Forward declarations that create undefined types initially */
struct opaque;                    /* TYPE_UNDEFINED */
struct forward_declared_struct;   /* TYPE_UNDEFINED */
union forward_declared_union;     /* TYPE_UNDEFINED */

/* ========== TYPE_SCALAR ========== */
/* Basic scalar types and typedefs */
typedef int my_int;               /* TYPE_SCALAR */
typedef char my_char;             /* TYPE_SCALAR */
typedef float my_float;           /* TYPE_SCALAR */
typedef double my_double;         /* TYPE_SCALAR */
typedef _Bool my_bool;            /* TYPE_SCALAR */

/* GCC extensions for scalar types */
typedef __complex__ double complex_double;    /* TYPE_SCALAR */
typedef __complex__ float complex_float;      /* TYPE_SCALAR */
typedef __builtin_va_list va_list_type;       /* TYPE_SCALAR */
typedef int __attribute__((vector_size(16))) vector_int; /* TYPE_SCALAR */

/* ========== TYPE_STRING ========== */
typedef const char* string_ptr;   /* TYPE_STRING */
typedef char* mutable_string;     /* TYPE_STRING */

/* ========== TYPE_STRUCT ========== */
/* Basic struct */
struct basic_struct {
    int x;
    double y;
    char z;
};

/* Packed struct - TYPE_USER_STRUCT */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(8)));

/* Aligned struct - TYPE_USER_STRUCT */
struct __attribute__((aligned(32))) aligned_struct {
    double data[4];
    int flags;
};

/* Designated init struct - TYPE_USER_STRUCT */
struct __attribute__((designated_init)) designated_struct {
    int field1;
    char field2;
    float field3;
};

/* ========== TYPE_UNION ========== */
union basic_union {
    int as_int;
    float as_float;
    char as_char[4];
};

/* Tagged union */
union tagged_union {
    struct {
        int type;
    } header;
    struct {
        int type;
        int value;
    } int_data;
    struct {
        int type;
        double value;
    } double_data;
};

/* ========== TYPE_POINTER ========== */
typedef int* int_ptr;                     /* TYPE_POINTER */
typedef struct basic_struct* struct_ptr;  /* TYPE_POINTER */
typedef void (*void_func_ptr)(void);      /* TYPE_POINTER */

/* Triple pointer for complexity */
typedef int*** triple_int_ptr;            /* TYPE_POINTER */

/* ========== TYPE_ARRAY ========== */
typedef int fixed_array[10];              /* TYPE_ARRAY */
typedef int multi_array[5][10];           /* TYPE_ARRAY */
extern int incomplete_array[];            /* TYPE_ARRAY */

/* Struct with flexible array member */
struct flex_array_struct {
    int count;
    int data[];  /* Incomplete array */
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types */
typedef int (*binary_op)(int, int);       /* TYPE_CALLBACK */
typedef void (*event_handler)(void*);     /* TYPE_CALLBACK */
typedef int (*comparator)(const void*, const void*);  /* TYPE_CALLBACK */

/* Complex callback with struct parameter */
typedef struct basic_struct* (*struct_factory)(int, char);  /* TYPE_CALLBACK */

/* ========== RECURSIVE AND INTERCONNECTED TYPES ========== */
/* Self-referential struct */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* Pointer to own type */
    struct recursive_node* prev;
};

/* Mutually recursive types */
struct type_a;
struct type_b;

struct type_a {
    int id;
    struct type_b* partner;
};

struct type_b {
    int id;
    struct type_a* partner;
    struct type_a* alternatives[3];  /* Array of pointers */
};

/* Union containing array of pointers */
union pointer_container {
    struct type_a* a_ptrs[5];
    struct type_b* b_ptrs[5];
    void* generic_ptrs[10];
};

/* ========== COMPLEX NESTED TYPE ========== */
struct nested_mess {
    struct {
        int tag;
        union {
            int i;
            float f;
            char* s;  /* String pointer */
        } value;
    } header;
    
    union {
        int int_array[4];
        struct recursive_node* node_ptr;
        binary_op operation;
    } payload;
    
    struct nested_mess* children[3];  /* Array of pointers to self */
};

/* ========== OPAQUE TYPE DEFINITION ========== */
/* Now define the previously opaque struct */
struct opaque {
    void* data;
    int size;
    struct opaque* next;
};

/* ========== C++ SPECIFIC FOR TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    /* This should create TYPE_LANG_STRUCT */
    class CppClass {
    private:
        int private_data;
    public:
        CppClass() : private_data(0) {}
        virtual ~CppClass() {}
        virtual void method() = 0;
    };
    
    /* Class with transaction_safe attribute */
    class __attribute__((transaction_safe)) TransactionClass {
        int value;
    public:
        TransactionClass(int v) : value(v) {}
        int get_value() const { return value; }
    };
}
#endif

/* ========== FUNCTION DECLARATIONS ========== */
void use_types_in_other_file(struct recursive_node* node, union pointer_container* container);

#endif /* VARIED_TYPES_H */
