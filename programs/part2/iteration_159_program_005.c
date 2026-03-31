/* test-gty.h - Header file with various GTY-annotated types */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct my_struct;
union my_union;

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int x;
    float y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int i;
    float f;
    void* p;
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_container GTY(()) {
    /* Pointer to another GTY-annotated struct */
    struct my_struct* GTY((skip)) child;
    
    /* Pointer to self */
    struct pointer_container* GTY((skip)) next;
    
    /* Void pointer */
    void* GTY((skip)) data;
};

/* TYPE_ARRAY: Struct with arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    int GTY((length("10"))) fixed_arr[10];
    
    /* Variable-length array (requires length callback) */
    char* GTY((length("get_vla_length"))) vla;
    
    /* Array of pointers */
    struct my_struct* GTY((skip)) ptr_array[5];
};

/* TYPE_SCALAR: Various scalar types */
struct scalar_container GTY(()) {
    long GTY((skip)) counter;
    unsigned GTY((skip)) flags;
    double GTY((skip)) value;
    enum { RED, GREEN, BLUE } GTY((skip)) color;
};

/* TYPE_STRING: String types */
struct string_container GTY(()) {
    const char* GTY((skip)) name;
    char* GTY((skip)) buffer;
    const char* GTY((skip)) path;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_fn GTY((skip)) handler;
    void* GTY((skip)) user_data;
};

/* Complex nested type for TYPE_STRUCT recursion */
struct complex_node GTY(()) {
    int id;
    struct complex_node* GTY((skip)) left;
    struct complex_node* GTY((skip)) right;
    union my_union GTY((tag("type"))) data;
};

/* Template-like macro for generating multiple struct types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

/* Instantiate template-like structs */
DEF_PAIR(int);
DEF_PAIR(float);
DEF_PAIR(struct my_struct*);

/* Union containing both struct and pointer */
union mixed_union GTY(()) {
    struct my_struct s;
    struct pointer_container* GTY((skip)) pc;
    int array[4];
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
