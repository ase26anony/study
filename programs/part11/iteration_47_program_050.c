x₀ = 0
for (int i = 0; i < n; i++) {
    x₁ = φ(x₀, x₃)  // Phi node at loop entry
    if (x₁ == 0) {
        x₂ = 1;
    } else {
        x₃ = 0;
    }
    // Implicit merge: x₄ = φ(x₂, x₃)
}
