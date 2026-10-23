/*
 * Copyright (c) 2014 Eran Pe'er.
 *
 * This program is made available under the terms of the MIT License.
 *
 * Created on Mar 10, 2014
 */
#pragma once

#if defined (__GNUG__) || _MSC_VER >= 1900
#define FAKEIT_THROWS noexcept(false)
#define FAKEIT_NO_THROWS noexcept(true)
#elif defined (_MSC_VER)
#define FAKEIT_THROWS throw(...)
#define FAKEIT_NO_THROWS
#endif

// Not __cpp_rtti: clang-cl defines it even with /GR-, which emits no RTTI data.
#if defined (__GXX_RTTI) || defined (_CPPRTTI)
#define FAKEIT_HAS_RTTI 1
#else
#define FAKEIT_HAS_RTTI 0
#endif
