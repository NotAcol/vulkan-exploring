static void ShishuaGen(shishua_state *restrict S, u64 *restrict Buffer, u64 Length) {
    __m256i S0 = S->State[0], S1 = S->State[1], Output = S->Output, Counter = S->Counter;

    // TODO(acol): make this deal with arrays of length different than multiples of 4
    // Assert(((Length % 4) == 0));

    for (u64 i = 0; i < Length; i += 4) {
        __m256i Increment = _mm256_set_epi64x(1, 3, 5, 7);
        S1 = _mm256_add_epi64(S1, Counter);
        Counter = _mm256_add_epi64(Counter, Increment);

        _mm256_store_si256((__m256i *)&Buffer[i], Output);

        __m256i U0 = _mm256_srli_epi64(S0, 1);
        __m256i U1 = _mm256_srli_epi64(S1, 3);

        __m256i Shu0 = _mm256_set_epi32(4, 3, 2, 1, 0, 7, 6, 5);
        __m256i Shu1 = _mm256_set_epi32(2, 1, 0, 7, 6, 5, 4, 3);
        __m256i T0 = _mm256_permutevar8x32_epi32(S0, Shu0);
        __m256i T1 = _mm256_permutevar8x32_epi32(S0, Shu1);

        S0 = _mm256_add_epi64(T0, U0);
        S1 = _mm256_add_epi64(T1, U1);

        Output = _mm256_xor_si256(U0, T1);
    }

    S->State[0] = S0;
    S->State[1] = S1;
    S->Output = Output;
    S->Counter = Counter;
}

static shishua_state ShishuaSeed(u64 Value[4]) {
    shishua_state S;
    u64 phi[8] = {
        0x9E3779B97F4A7C15, 0xF39CC0605CEDC834, 0x1082276BF3A27251, 0xF86C6A11D0C18E95,
        0x2767F0B153D27B7F, 0x0347045B5BF1827F, 0x01886F0928403002, 0xC1D64BA40F335E36,
    };

    S.State[0] = _mm256_set_epi64x(phi[3], phi[2] ^ Value[1], phi[1], phi[0] ^ Value[0]);
    S.State[1] = _mm256_set_epi64x(phi[7], phi[6] ^ Value[3], phi[5], phi[4] ^ Value[2]);

    u64 Buffer[4 * 2];

    for (int i = 0; i < 10; i++) {
        ShishuaGen(&S, Buffer, 4 * 2);
        S.State[0] = S.State[1];
        S.State[1] = S.Output;
    }

    return S;
}

static u64 RotateLeftU64(u64 X, u8 K) {
    X = (X << K) | (X >> (64 - (K)));
    return X;
}

static u64 JsfGen(jsf_state *Series) {
    u64 A = Series->A;
    u64 B = Series->B;
    u64 C = Series->C;
    u64 D = Series->D;

    u64 E = A - RotateLeftU64(B, 7);

    A = B ^ RotateLeftU64(C, 13);
    B = C + RotateLeftU64(D, 37);
    C = D + E;
    D = E + A;
    return D;
}

static jsf_state JsfSeed(u8 Value) {
    jsf_state Series = {0};

    Series.A = 0xF1EA5EED;
    Series.B = Series.C = Series.D = Value;

    for (u32 i = 20; i != 0; i--) {
        JsfGen(&Series);
    }

    return Series;
}
