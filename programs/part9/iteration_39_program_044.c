// Initial
i₀ = 0

// Loop header
i_phi = Φ(i₀, i_next)

while (i_phi < n) {
    if (i_phi == 0) {  // True only on first iteration
        // body
    }
    i_next = i_phi + 1
    // Loop back to header
}
