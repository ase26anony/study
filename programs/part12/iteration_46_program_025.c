// Creating a pointer to int type
struct type *int_type = create_type(TYPE_SCALAR);
struct type *ptr_type = create_type(TYPE_POINTER);
ptr_type->u.pointer_to = int_type;

// Creating an array of pointers
struct type *arr_type = create_type(TYPE_ARRAY);
arr_type->u.element_type = ptr_type;
