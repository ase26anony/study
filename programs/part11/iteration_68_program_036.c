/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED/opaque pointer */
struct opaque;

/* TYPE_STRUCT - Regular struct with GTY annotation */
struct GTY(()) my_struct {
    /* TYPE_SCALAR */
    int scalar_member;
    
    /* TYPE_STRING */
    const char *string_member;
    
    /* TYPE_POINTER */
    struct my_struct *next;
    
    /* TYPE_ARRAY - Fixed size array */
    int array_member[10];
    
    /* Pointer to undefined/opaque type */
    struct opaque *opaque_ptr;
};

/* TYPE_UNION - Union with GTY annotation */
union GTY(()) my_union {
    int as_int;
    float as_float;
    struct my_struct *as_struct;
};

/* TYPE_USER_STRUCT - Using typedef with GTY */
typedef struct GTY(()) {
    int id;
    char name[32];
} user_struct_t;

/* TYPE_CALLBACK - Function pointer typedef with GTY */
typedef void GTY((callback)) (*callback_func)(int, const char*);

/* Structure containing callback */
struct GTY(()) has_callback {
    callback_func cb;
    int data;
};

/* TYPE_LANG_STRUCT - Simulating language-specific structure */
#ifdef GENERATOR_FILE
/* This would be in a language-specific header in real GCC */
struct GTY(()) lang_struct {
    int lang_specific;
    void *lang_data;
};
#endif

/* Another struct to ensure multiple instances are counted */
struct GTY(()) another_struct {
    double value;
    struct my_struct *link;
};

/* Union with pointer members */
union GTY(()) ptr_union {
    struct my_struct *sptr;
    user_struct_t *uptr;
    void *vptr;
};

#endif /* GTY_TEST_H */
