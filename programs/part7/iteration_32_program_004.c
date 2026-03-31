uint64_t a[4], b[4], result[4];
uint64_t carry = 0;

for (int i = 0; i < 4; i++) {
    asm volatile ("addc %0, %1, %2, %3"
                  : "=r"(result[i]), "=@ccc"(carry)
                  : "r"(a[i]), "r"(b[i]), "0"(carry));
}
