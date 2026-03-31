i_0 = 0;
i_1 = φ(i_0, i_2);
while (i_1 < n) {
    if (i_1 == 0) {
        // body - executes only on first iteration
    }
    i_2 = i_1 + 1;
    i_1 = φ(i_0, i_2); // Next iteration's Phi gets updated value
}
