union {
    struct type *pointer_to;           // TYPE_POINTER
    struct type *element_type;         // TYPE_ARRAY
    struct struct_type *struct_info;   // TYPE_STRUCT/TYPE_USER_STRUCT
    struct union_type *union_info;     // TYPE_UNION
    struct callback_type *callback;    // TYPE_CALLBACK
    // ... scalar might have size/alignment info
    // ... string might have length info
} u;
