#pragma once

#include "mockutils/constexpr_hash.hpp"

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif

#define COUNTER_STRINGIFY( counter ) #counter

#define STUB_ID_STR( counter ) \
    __FILE__ COUNTER_STRINGIFY(counter)

#define STUB_ID(counter) \
    fakeit::constExprHash(STUB_ID_STR(counter))

#define MOCK_TYPE(mock) \
    std::remove_reference<decltype((mock).get())>::type

#define OVERLOADED_METHOD_PTR(mock, method, prototype) \
    fakeit::Prototype<prototype>::template MemberType<typename MOCK_TYPE(mock)>::get(&MOCK_TYPE(mock)::method)

#define CONST_OVERLOADED_METHOD_PTR(mock, method, prototype) \
    fakeit::Prototype<prototype>::template MemberType<typename MOCK_TYPE(mock)>::getconst(&MOCK_TYPE(mock)::method)

#define Dtor(mock) \
    (mock).dtor().setMethodDetails(#mock,"destructor")

#define Method(mock, method) \
    (mock).template stub<STUB_ID(__COUNTER__)>(&MOCK_TYPE(mock)::method).setMethodDetails(#mock,#method)

#define OverloadedMethod(mock, method, prototype) \
    (mock).template stub<STUB_ID(__COUNTER__)>(OVERLOADED_METHOD_PTR( mock , method, prototype )).setMethodDetails(#mock,#method)

#define ConstOverloadedMethod(mock, method, prototype) \
    (mock).template stub<STUB_ID(__COUNTER__)>(CONST_OVERLOADED_METHOD_PTR( mock , method, prototype )).setMethodDetails(#mock,#method)

#define Verify(...) \
        Verify( __VA_ARGS__ ).setFileInfo(__FILE__, __LINE__, __func__)

#define Using(...) \
        Using( __VA_ARGS__ )

#define VerifyNoOtherInvocations(...) \
    VerifyNoOtherInvocations( __VA_ARGS__ ).setFileInfo(__FILE__, __LINE__, __func__)

#define Fake(...) \
    Fake( __VA_ARGS__ )

#define When(call) \
    When(call)

