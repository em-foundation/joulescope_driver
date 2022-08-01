/*
 * Copyright 2014-2021 Jetperch LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include "jsdrv/error_code.h"
#include "jsdrv_prv/platform.h"
#include "jsdrv/union.h"

#include <stdio.h>
#include <stdlib.h>

static void test_numeric_eq(void ** state) {
    (void) state;
    assert_true(jsdrv_union_eq(&jsdrv_union_u16(8), &jsdrv_union_u16(8)));
    assert_false(jsdrv_union_eq(&jsdrv_union_u16(8), &jsdrv_union_u16(9)));
    assert_false(jsdrv_union_eq(&jsdrv_union_u16(8), &jsdrv_union_u32(8)));
    assert_false(jsdrv_union_eq(&jsdrv_union_i16(8), &jsdrv_union_u32(8)));
    assert_true(jsdrv_union_eq(&jsdrv_union_i16(8), &jsdrv_union_i16(8)));
}

static void test_cstr_eq(void ** state) {
    (void) state;
    assert_true(jsdrv_union_eq(&jsdrv_union_cstr("hello"), &jsdrv_union_cstr("hello")));
    assert_false(jsdrv_union_eq(&jsdrv_union_cstr("hello"), &jsdrv_union_cstr("world")));
}

#define ASSERT_VALUE_ERROR(v)  do { \
    bool k;                        \
    assert_int_equal(JSDRV_ERROR_PARAMETER_INVALID, jsdrv_union_to_bool(v, &k)); \
    } while (0)

#define ASSERT_VALUE_TRUE(v)  do { \
    bool k;                        \
    assert_int_equal(0, jsdrv_union_to_bool(v, &k)); \
    assert_true(k);                \
    } while (0)

#define ASSERT_VALUE_FALSE(v)  do { \
    bool k;                        \
    assert_int_equal(0, jsdrv_union_to_bool(v, &k)); \
    assert_false(k);               \
    } while (0)

static void test_str_to_bool(void ** state) {
    (void) state;
    ASSERT_VALUE_FALSE(&jsdrv_union_cstr("off"));
    ASSERT_VALUE_FALSE(&jsdrv_union_cstr("Off"));
    ASSERT_VALUE_FALSE(&jsdrv_union_cstr("OFF"));
    ASSERT_VALUE_FALSE(&jsdrv_union_cstr("disable"));
    ASSERT_VALUE_FALSE(&jsdrv_union_cstr("disabled"));
    ASSERT_VALUE_FALSE(&jsdrv_union_cstr("false"));
    ASSERT_VALUE_FALSE(&jsdrv_union_cstr("no"));
    ASSERT_VALUE_FALSE(&jsdrv_union_cstr("0"));

    ASSERT_VALUE_TRUE(&jsdrv_union_cstr("on"));
    ASSERT_VALUE_TRUE(&jsdrv_union_cstr("On"));
    ASSERT_VALUE_TRUE(&jsdrv_union_cstr("ON"));
    ASSERT_VALUE_TRUE(&jsdrv_union_cstr("enable"));
    ASSERT_VALUE_TRUE(&jsdrv_union_cstr("enabled"));
    ASSERT_VALUE_TRUE(&jsdrv_union_cstr("true"));
    ASSERT_VALUE_TRUE(&jsdrv_union_cstr("yes"));
    ASSERT_VALUE_TRUE(&jsdrv_union_cstr("1"));

    ASSERT_VALUE_ERROR(&jsdrv_union_cstr("hello?"));

    ASSERT_VALUE_TRUE(&jsdrv_union_json("true"));
    ASSERT_VALUE_FALSE(&jsdrv_union_json("false"));
    ASSERT_VALUE_ERROR(&jsdrv_union_json("invalid"));
}

static void test_json_to_bool(void ** state) {
    (void) state;
    ASSERT_VALUE_TRUE(&jsdrv_union_json("true"));
    ASSERT_VALUE_TRUE(&jsdrv_union_json("1"));
    ASSERT_VALUE_FALSE(&jsdrv_union_json("false"));
    ASSERT_VALUE_FALSE(&jsdrv_union_json("0"));
    ASSERT_VALUE_ERROR(&jsdrv_union_json("invalid"));
}

static void test_numeric_to_bool(void ** state) {
    (void) state;
    ASSERT_VALUE_FALSE(&jsdrv_union_f32(0.0f));
    ASSERT_VALUE_TRUE(&jsdrv_union_f32(1.0f));
    ASSERT_VALUE_FALSE(&jsdrv_union_f64(0.0));
    ASSERT_VALUE_TRUE(&jsdrv_union_f64(1.0));

    ASSERT_VALUE_FALSE(&jsdrv_union_u8(0));
    ASSERT_VALUE_TRUE(&jsdrv_union_u8(1));
    ASSERT_VALUE_FALSE(&jsdrv_union_u16(0));
    ASSERT_VALUE_TRUE(&jsdrv_union_u16(0x1000));
    ASSERT_VALUE_FALSE(&jsdrv_union_u32(0));
    ASSERT_VALUE_TRUE(&jsdrv_union_u32(0x10000000));
    ASSERT_VALUE_FALSE(&jsdrv_union_u64(0));
    ASSERT_VALUE_TRUE(&jsdrv_union_u64(0x1000000000000000LL));

    ASSERT_VALUE_FALSE(&jsdrv_union_i8(0));
    ASSERT_VALUE_TRUE(&jsdrv_union_i8(1));
    ASSERT_VALUE_FALSE(&jsdrv_union_i16(0));
    ASSERT_VALUE_TRUE(&jsdrv_union_i16(0x1000));
    ASSERT_VALUE_FALSE(&jsdrv_union_i32(0));
    ASSERT_VALUE_TRUE(&jsdrv_union_i32(0x10000000));
    ASSERT_VALUE_FALSE(&jsdrv_union_i64(0));
    ASSERT_VALUE_TRUE(&jsdrv_union_i64(0x1000000000000000LL));
}

static void test_eq_exact(void ** state) {
    (void) state;
    struct jsdrv_union_s v1 = jsdrv_union_u16(8);
    struct jsdrv_union_s v2 = v1;
    assert_true(jsdrv_union_eq_exact(&v1, &v2));
    v2 = v1; v2.flags = 1;
    assert_false(jsdrv_union_eq_exact(&v1, &v2));
    v2 = v1; v2.op = 1;
    assert_false(jsdrv_union_eq_exact(&v1, &v2));
    v2 = v1; v2.app = 1;
    assert_false(jsdrv_union_eq_exact(&v1, &v2));
    v2 = v1; v2.size = 1;
    assert_false(jsdrv_union_eq_exact(&v1, &v2));
}

static void test_widen(void ** state) {
    (void) state;
    struct jsdrv_union_s v;
    v = jsdrv_union_u8(8);  jsdrv_union_widen(&v); assert_true(jsdrv_union_eq(&jsdrv_union_u64(8), &v));
    v = jsdrv_union_u16(8); jsdrv_union_widen(&v); assert_true(jsdrv_union_eq(&jsdrv_union_u64(8), &v));
    v = jsdrv_union_u32(8); jsdrv_union_widen(&v); assert_true(jsdrv_union_eq(&jsdrv_union_u64(8), &v));
    v = jsdrv_union_u64(8); jsdrv_union_widen(&v); assert_true(jsdrv_union_eq(&jsdrv_union_u64(8), &v));
    v = jsdrv_union_i8(-127);  jsdrv_union_widen(&v); assert_true(jsdrv_union_eq(&jsdrv_union_i64(-127), &v));
    v = jsdrv_union_i16(-127); jsdrv_union_widen(&v); assert_true(jsdrv_union_eq(&jsdrv_union_i64(-127), &v));
    v = jsdrv_union_i32(-127); jsdrv_union_widen(&v); assert_true(jsdrv_union_eq(&jsdrv_union_i64(-127), &v));
    v = jsdrv_union_i64(-127); jsdrv_union_widen(&v); assert_true(jsdrv_union_eq(&jsdrv_union_i64(-127), &v));
}

static void test_equiv(void ** state) {
    (void) state;
    assert_true(jsdrv_union_equiv(&jsdrv_union_u8(8), &jsdrv_union_u16(8)));
    assert_true(jsdrv_union_equiv(&jsdrv_union_u8(8), &jsdrv_union_i16(8)));
    assert_true(jsdrv_union_equiv(&jsdrv_union_i16(8), &jsdrv_union_u8(8)));
    assert_true(jsdrv_union_equiv(&jsdrv_union_i8(-1), &jsdrv_union_i64(-1)));

    assert_false(jsdrv_union_equiv(&jsdrv_union_i8(-1), &jsdrv_union_u8(0xff)));
    struct jsdrv_union_s x = jsdrv_union_u64(0);
    x.value.i64 = -1;
    assert_false(jsdrv_union_equiv(&jsdrv_union_i64(-1), &x));
}

static void test_as_type(void ** state) {
    (void) state;
    struct jsdrv_union_s x;
    x = jsdrv_union_i8(-1); assert_int_equal(0, jsdrv_union_as_type(&x, JSDRV_UNION_I64));  assert_true(jsdrv_union_eq(&x, &jsdrv_union_i64(-1)));
    x = jsdrv_union_u8(8);  assert_int_equal(0, jsdrv_union_as_type(&x, JSDRV_UNION_I64));  assert_true(jsdrv_union_eq(&x, &jsdrv_union_i64(8)));
    x = jsdrv_union_i8(-8);  assert_int_equal(0, jsdrv_union_as_type(&x, JSDRV_UNION_F64));  assert_true(jsdrv_union_eq(&x, &jsdrv_union_f64(-8.0)));
    x = jsdrv_union_u8(8);  assert_int_equal(0, jsdrv_union_as_type(&x, JSDRV_UNION_F64));  assert_true(jsdrv_union_eq(&x, &jsdrv_union_f64(8.0)));
}

static void test_type_to_str(void ** state) {
    (void) state;
    assert_string_equal("nul", jsdrv_union_type_to_str(JSDRV_UNION_NULL));
    assert_string_equal("str", jsdrv_union_type_to_str(JSDRV_UNION_STR));
    assert_string_equal("jsn", jsdrv_union_type_to_str(JSDRV_UNION_JSON));
    assert_string_equal("bin", jsdrv_union_type_to_str(JSDRV_UNION_BIN));
    assert_string_equal("rsv", jsdrv_union_type_to_str(JSDRV_UNION_RSV0));
    assert_string_equal("rsv", jsdrv_union_type_to_str(JSDRV_UNION_RSV1));
    assert_string_equal("f32", jsdrv_union_type_to_str(JSDRV_UNION_F32));
    assert_string_equal("f64", jsdrv_union_type_to_str(JSDRV_UNION_F64));
    assert_string_equal(" u8", jsdrv_union_type_to_str(JSDRV_UNION_U8));
    assert_string_equal("u16", jsdrv_union_type_to_str(JSDRV_UNION_U16));
    assert_string_equal("u32", jsdrv_union_type_to_str(JSDRV_UNION_U32));
    assert_string_equal("u64", jsdrv_union_type_to_str(JSDRV_UNION_U64));
    assert_string_equal(" i8", jsdrv_union_type_to_str(JSDRV_UNION_I8));
    assert_string_equal("i16", jsdrv_union_type_to_str(JSDRV_UNION_I16));
    assert_string_equal("i32", jsdrv_union_type_to_str(JSDRV_UNION_I32));
    assert_string_equal("i64", jsdrv_union_type_to_str(JSDRV_UNION_I64));
    assert_string_equal("inv", jsdrv_union_type_to_str(255));
}

#define assert_str(expect__, value__, opts__) do {  \
    char buf__[32];                         \
    assert_int_equal(0, jsdrv_union_value_to_str((value__),  buf__, (uint32_t) sizeof(buf__), (opts__))); \
    assert_string_equal((expect__), buf__);         \
    } while (0)

static void test_value_to_str(void ** state) {
    (void) state;
    assert_str("1", &jsdrv_union_u8(1), 0);
    assert_str("1", &jsdrv_union_u16(1), 0);
    assert_str("1", &jsdrv_union_u32(1), 0);
    assert_str("1", &jsdrv_union_u64(1), 0);
    assert_str("-1", &jsdrv_union_i8(-1), 0);
    assert_str("-1", &jsdrv_union_i16(-1), 0);
    assert_str("-1", &jsdrv_union_i32(-1), 0);
    assert_str("-1", &jsdrv_union_i64(-1), 0);
    assert_str(" u8    0", &jsdrv_union_u8(0), 1);
    assert_str("u32.R  1", &jsdrv_union_u32_r(1), 1);
    assert_str("str.RC hello", &jsdrv_union_cstr_r("hello"), 1);
    assert_str("jsn.C  hello", &jsdrv_union_cjson("hello"), 1);
    assert_str("bin.C  size=4", &jsdrv_union_cbin((const uint8_t *) "one", 4), 1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_numeric_eq),
            cmocka_unit_test(test_cstr_eq),
            cmocka_unit_test(test_numeric_to_bool),
            cmocka_unit_test(test_str_to_bool),
            cmocka_unit_test(test_json_to_bool),
            cmocka_unit_test(test_eq_exact),
            cmocka_unit_test(test_widen),
            cmocka_unit_test(test_equiv),
            cmocka_unit_test(test_as_type),
            cmocka_unit_test(test_type_to_str),
            cmocka_unit_test(test_value_to_str),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
