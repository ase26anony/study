/* test-gty.h - Header file with various GTY-annotated types */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#ifdef __cplusplus
extern "C" {
#endif

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int x;
    double y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int int_val;
    double double_val;
    void* ptr_val;
};

/* TYPE_POINTER: Will be used within another struct */
struct pointer_container GTY(()) {
    /* TYPE_POINTER case */
    struct my_struct* GTY((skip)) struct_ptr;
    
    /* TYPE_ARRAY case - fixed size array */
    int GTY((length("10"))) fixed_array[10];
    
    /* TYPE_SCALAR case */
    long GTY((skip)) counter;
    
    /* TYPE_STRING case */
    const char* GTY((skip)) name;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));

/* Complex nested structure for type graph */
struct nested_struct GTY(()) {
    struct my_struct* GTY((skip)) child;
    struct nested_struct* GTY((skip)) next;  /* Self-referential pointer */
    union my_union data;
};

/* Template-like macro to generate multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Language-specific structure hook (simulating Tree nodes) */
struct lang_specific GTY((tag("TS_VAR_DECL"))) {
    int decl_uid;
    const char* GTY((skip)) decl_name;
    struct lang_specific* GTY((skip)) chain;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
