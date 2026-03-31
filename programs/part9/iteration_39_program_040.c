int i = 0;          // i₀ = 0
while (i < n) {     // i here is i₁ (Phi node)
    if (i == 0) {   // i here is also i₁
        // This executes ONLY on first iteration
        // because i₁ = φ(0, i₂) and i₂ is always ≥ 1
    }
    i++;            // i₂ = i₁ + 1
}
