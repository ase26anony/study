union {
    struct type *pointer_to;           // TYPE_POINTER
    struct type *element_type;         // TYPE_ARRAY
    struct type *return_type;          // TYPE_CALLBACK
    struct struct_type *struct_info;   // TYPE_STRUCT/TYPE_USER_STRUCT
    struct union_type *union_info;     // TYPE_UNION
    // For scalar types, might have:
    enum scalar_type scalar_kind;      // TYPE_SCALAR
    // For strings:
    size_t string_length;              // TYPE_STRING (if fixed length)
    // For arrays:
    size_t array_size;                 // TYPE_ARRAY
} u;
