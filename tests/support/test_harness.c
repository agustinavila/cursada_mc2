#include "test_harness.h"

#include <stdio.h>
#include <string.h>

static int test_harness_fallas_ = 0;

void test_harness_reset(void)
{
    test_harness_fallas_ = 0;
}

void test_harness_run(const char* nombre, test_harness_func_t prueba)
{
    printf("[TEST] %s\n", nombre);
    prueba();
}

void test_harness_fail(const char* archivo, int linea, const char* expresion)
{
    ++test_harness_fallas_;
    fprintf(stderr, "%s:%d assertion failed: %s\n", archivo, linea, expresion);
}

void test_harness_fail_int(const char* archivo,
                           int linea,
                           const char* esperado_texto,
                           intmax_t esperado,
                           const char* actual_texto,
                           intmax_t actual)
{
    ++test_harness_fallas_;
    fprintf(stderr,
            "%s:%d expected %s=%jd but got %s=%jd\n",
            archivo,
            linea,
            esperado_texto,
            esperado,
            actual_texto,
            actual);
}

void test_harness_fail_uint(const char* archivo,
                            int linea,
                            const char* esperado_texto,
                            uintmax_t esperado,
                            const char* actual_texto,
                            uintmax_t actual)
{
    ++test_harness_fallas_;
    fprintf(stderr,
            "%s:%d expected %s=%ju but got %s=%ju\n",
            archivo,
            linea,
            esperado_texto,
            esperado,
            actual_texto,
            actual);
}

void test_harness_fail_mem(const char* archivo,
                           int linea,
                           const char* esperado_texto,
                           const char* actual_texto,
                           size_t longitud)
{
    ++test_harness_fallas_;
    fprintf(stderr,
            "%s:%d expected %s and %s to match for %zu bytes\n",
            archivo,
            linea,
            esperado_texto,
            actual_texto,
            longitud);
}

int test_harness_result(void)
{
    if (test_harness_fallas_ == 0) {
        printf("[RESULT] all tests passed\n");
        return 0;
    }

    fprintf(stderr, "[RESULT] %d test assertion(s) failed\n", test_harness_fallas_);
    return 1;
}
