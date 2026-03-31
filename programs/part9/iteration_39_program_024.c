i₀ = 0
while (true) {
    i₁ = φ(i₀, i₂)  // Phi node at loop entry
    if (i₁ >= n) break;
    if (i₁ == 0) {
        // body - only executes on first iteration
    }
    i₂ = i₁ + 1
}
