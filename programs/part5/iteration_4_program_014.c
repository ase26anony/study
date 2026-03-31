struct S {
    unsigned int bf : 4;  // bitfield
};

void test_invalid_address_of_bitfield() {
    struct S s;
    unsigned int* ptr = &s.bf;  // ERROR: cannot take address of bitfield
}
