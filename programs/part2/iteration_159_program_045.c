/* test-gty.h - Header file with various GTY-annotated types */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) base_struct {
    int id;
    const char* GTY((skip)) description;
};

/* TYPE_UNION: Basic union with GTY annotation */
union GTY(()) data_union {
    int int_val;
    double double_val;
    void* GTY((skip)) ptr_val;
};

/* TYPE_POINTER: Struct containing pointers */
struct GTY(()) pointer_container {
    struct base_struct* GTY((skip)) base_ptr;
    void* GTY((skip)) generic_ptr;
    int* GTY((skip)) int_ptr;
};

/* TYPE_ARRAY: Struct with fixed-size array */
struct GTY(()) array_container {
    int GTY((length("10"))) fixed_array[10];
    struct base_struct* GTY((length("5"))) ptr_array[5];
};

/* TYPE_SCALAR: Explicit scalar types */
struct GTY(()) scalar_types {
    long GTY((skip)) counter;
    unsigned long GTY((skip)) flags;
    short GTY((skip)) small_value;
};

/* TYPE_STRING: String types */
struct GTY(()) string_container {
    const char* GTY((skip)) name;
    char* GTY((skip)) mutable_string;
    const char* GTY((skip)) path;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct GTY(()) callback_container {
    callback_func GTY((skip)) handler;
    void* GTY((skip)) user_data;
};

/* Complex nested structure for type graph */
struct GTY(()) complex_node {
    int value;
    struct complex_node* GTY((skip)) next;
    struct complex_node* GTY((skip)) prev;
    union data_union GTY((skip)) node_data;
};

/* Template-like macro for generating multiple struct types */
#define DEF_PAIR(T, NAME) \
    struct GTY(()) NAME { \
        T first; \
        T second; \
    }

DEF_PAIR(int, int_pair);
DEF_PAIR(double, double_pair);
DEF_PAIR(struct base_struct*, struct_pair);

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;
struct GTY(()) tree_node {
    int data;
    struct tree_node* GTY((skip)) left;
    struct tree_node* GTY((skip)) right;
};

/* Language-specific structure simulation */
struct GTY((tag("TS_VAR_DECL"))) lang_specific {
    int decl_type;
    const char* GTY((skip)) identifier;
    void* GTY((skip)) lang_specific_data;
};

#endif /* TEST_GTY_H */
