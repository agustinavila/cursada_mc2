#if !defined(TESTS_SUPPORT_TEST_HARNESS_H_)
#define TESTS_SUPPORT_TEST_HARNESS_H_

#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>

typedef void (*test_harness_func_t)(void);

void test_harness_reset(void);
void test_harness_run(const char* nombre, test_harness_func_t prueba);
void test_harness_fail(const char* archivo, int linea, const char* expresion);
void test_harness_fail_int(const char* archivo,
                           int linea,
                           const char* esperado_texto,
                           intmax_t esperado,
                           const char* actual_texto,
                           intmax_t actual);
void test_harness_fail_uint(const char* archivo,
                            int linea,
                            const char* esperado_texto,
                            uintmax_t esperado,
                            const char* actual_texto,
                            uintmax_t actual);
void test_harness_fail_mem(const char* archivo,
                           int linea,
                           const char* esperado_texto,
                           const char* actual_texto,
                           size_t longitud);
int test_harness_result(void);

#define RUN_TEST(prueba) test_harness_run(#prueba, prueba)

#define TEST_ASSERT_TRUE(expresion) \
    do { \
        if (!(expresion)) { \
            test_harness_fail(__FILE__, __LINE__, #expresion); \
        } \
    } while (0)

#define TEST_ASSERT_FALSE(expresion) TEST_ASSERT_TRUE(!(expresion))

#define TEST_ASSERT_EQ_INT(esperado, actual) \
    do { \
        const intmax_t esperado__ = (intmax_t) (esperado); \
        const intmax_t actual__ = (intmax_t) (actual); \
        if (esperado__ != actual__) { \
            test_harness_fail_int(__FILE__, __LINE__, #esperado, esperado__, #actual, actual__); \
        } \
    } while (0)

#define TEST_ASSERT_EQ_UINT(esperado, actual) \
    do { \
        const uintmax_t esperado__ = (uintmax_t) (esperado); \
        const uintmax_t actual__ = (uintmax_t) (actual); \
        if (esperado__ != actual__) { \
            test_harness_fail_uint(__FILE__, __LINE__, #esperado, esperado__, #actual, actual__); \
        } \
    } while (0)

#define TEST_ASSERT_EQ_BOOL(esperado, actual) TEST_ASSERT_EQ_UINT((esperado), (actual))

#define TEST_ASSERT_MEM_EQ(esperado, actual, longitud) \
    do { \
        if (memcmp((esperado), (actual), (longitud)) != 0) { \
            test_harness_fail_mem(__FILE__, __LINE__, #esperado, #actual, (longitud)); \
        } \
    } while (0)

#endif // TESTS_SUPPORT_TEST_HARNESS_H_
