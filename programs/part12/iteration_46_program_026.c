struct type int_type = {TYPE_SCALAR, NULL};
struct type ptr_type = {TYPE_POINTER, NULL};
ptr_type.u.pointer_to = &int_type;
