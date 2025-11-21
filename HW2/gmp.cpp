#include <stdio.h>
#include <gmp.h>

int main() {
    mpz_t a, b, result;
    mpz_init_set_str(a, "123456789123456789123456789", 10);
    mpz_init_set_str(b, "987654321987654321987654321", 10);
    mpz_init(result);

    mpz_add(result, a, b);

    gmp_printf("a + b = %Zd\n", result);

    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(result);
    return 0;
}
