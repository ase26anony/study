/* test-gty.h - Header file with GTY annotations for gengtype testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct forward_declared_struct;
union forward_declared_union;

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct basic_struct GTY(()) {
    int id;
    char tag;
};

/* TYPE_UNION: Basic union with GTY annotation */
union basic_union GTY(()) {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* TYPE_POINTER: Struct containing pointer members */
struct pointer_container GTY(()) {
    /* Regular pointer */
    struct basic_struct* GTY((skip)) regular_ptr;
    
    /* Pointer to forward declared type */
    struct forward_declared_struct* GTY((skip)) forward_ptr;
    
    /* Pointer to union */
    union basic_union* GTY((skip)) union_ptr;
    
    /* Self-referential pointer */
    struct pointer_container* GTY((skip)) self_ptr;
};

/* TYPE_ARRAY: Struct with array members */
struct array_container GTY(()) {
    /* Fixed-size array */
    int GTY((length("10"))) fixed_array[10];
    
    /* Array of pointers */
    struct basic_struct* GTY((skip)) pointer_array[5];
    
    /* Multi-dimensional array */
    char GTY((length("5*20"))) matrix[5][20];
};

/* TYPE_SCALAR: Struct with scalar types */
struct scalar_container GTY(()) {
    /* Various scalar types with GTY */
    long GTY((skip)) counter;
    unsigned int GTY((skip)) flags;
    double GTY((skip)) value;
    _Bool GTY((skip)) enabled;
};

/* TYPE_STRING: Struct with string members */
struct string_container GTY(()) {
    /* String pointer */
    const char* GTY((skip)) name;
    
    /* Non-const string */
    char* GTY((skip)) buffer;
    
    /* Array of strings */
    const char* GTY((skip)) string_array[3];
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

/* Struct using callback */
struct callback_container GTY(()) {
    callback_func GTY((skip)) handler;
    void* GTY((skip)) user_data;
};

/* Complex nested type for type graph testing */
struct complex_node GTY(()) {
    int value;
    struct complex_node* GTY((skip)) next;
    struct complex_node* GTY((skip)) prev;
    union basic_union GTY((tag("0"))) data;
};

/* Template-like macro for generating multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

/* Instantiate template-like types */
DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct basic_struct*);

/* Forward declared struct definition */
struct forward_declared_struct GTY(()) {
    int magic;
    struct pointer_container* GTY((skip)) container;
};

/* Forward declared union definition */
union forward_declared_union GTY(()) {
    int x;
    struct basic_struct s;
    struct forward_declared_struct* GTY((skip)) p;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
