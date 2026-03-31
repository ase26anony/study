i_initial = 0
i_phi = Φ(i_initial, i_next)  // Loop Phi node
while (i_phi < n) {
    if (i_phi == 0) {  // True only when i_phi comes from i_initial
        // body - executes once
    }
    i_next = i_phi + 1
    i_phi = Φ(i_initial, i_next)  // Next iteration
}
