/*
 * Copyright (c) 2014 Eran Pe'er.
 *
 * This program is made available under the terms of the MIT License.
 *
 * Created on Mar 10, 2014
 */
#pragma once

#include "fakeit/DomainObjects.hpp"
#include "fakeit/MockImpl.hpp"
#include "fakeit/ActualInvocation.hpp"
#include "fakeit/Prototype.hpp"

namespace fakeit {
    namespace internal {
    }
    using namespace fakeit::internal;

    template<typename C, typename ... baseclasses>
    class Mock : public ActualInvocationsSource {
        MockImpl<C, baseclasses...> impl;
    public:
        virtual ~Mock() = default;

        static_assert(std::is_polymorphic<C>::value, "Can only mock a polymorphic type");

        Mock() : impl(Fakeit) {
        }

        explicit Mock(C &obj) : impl(Fakeit, obj) {
        }

        // Copying or moving a Mock causes issues where mocked methods don't copy or move with it
        // Delete both the copy and move constructors, and the assignment and move operators
        Mock(const Mock&) = delete;
        Mock(Mock&&) = delete;
        Mock& operator=(const Mock&) = delete;
        Mock& operator=(Mock&&) = delete;

        virtual C &get() {
            return impl.get();
        }

//		std::shared_ptr<C> getShared() {
//			return impl.getShared();
//		}
        
		C &operator()() {
            return get();
        }

        void Reset() {
            impl.reset();
        }

		void ClearInvocationHistory() {
			impl.clear();
		}

        template<class DataType, typename ... arglist,
                class = typename std::enable_if<std::is_member_object_pointer<DataType C::*>::value>::type>
        DataMemberStubbingRoot<C, DataType> Stub(DataType C::* member, const arglist &... ctorargs) {
            return impl.stubDataMember(member, ctorargs...);
        }

        template<size_t id, typename R, typename T, typename ... arglist, class = typename std::enable_if<
                !std::is_void<R>::value && std::is_base_of<T, C>::value>::type>
        MockingContext<R, arglist...> stub(R (T::*vMethod)(arglist...) const) {
            auto methodWithoutConstVolatile = reinterpret_cast<R (T::*)(arglist...)>(vMethod);
            return impl.template stubMethod<id>(methodWithoutConstVolatile);
        }

        template<size_t id, typename R, typename T, typename... arglist, class = typename std::enable_if<
                !std::is_void<R>::value && std::is_base_of<T, C>::value>::type>
        MockingContext<R, arglist...> stub(R(T::*vMethod)(arglist...) volatile) {
            auto methodWithoutConstVolatile = reinterpret_cast<R(T::*)(arglist...)>(vMethod);
            return impl.template stubMethod<id>(methodWithoutConstVolatile);
        }

        template<size_t id, typename R, typename T, typename... arglist, class = typename std::enable_if<
                !std::is_void<R>::value && std::is_base_of<T, C>::value>::type>
        MockingContext<R, arglist...> stub(R(T::*vMethod)(arglist...) const volatile) {
            auto methodWithoutConstVolatile = reinterpret_cast<R(T::*)(arglist...)>(vMethod);
            return impl.template stubMethod<id>(methodWithoutConstVolatile);
        }

        template<size_t id, typename R, typename T, typename... arglist, class = typename std::enable_if<
                !std::is_void<R>::value && std::is_base_of<T, C>::value>::type>
        MockingContext<R, arglist...> stub(R(T::*vMethod)(arglist...)) {
            return impl.template stubMethod<id>(vMethod);
        }

        template<size_t id, typename R, typename T, typename... arglist, class = typename std::enable_if<
                std::is_void<R>::value && std::is_base_of<T, C>::value>::type>
        MockingContext<void, arglist...> stub(R(T::*vMethod)(arglist...) const) {
            auto methodWithoutConstVolatile = reinterpret_cast<void (T::*)(arglist...)>(vMethod);
            return impl.template stubMethod<id>(methodWithoutConstVolatile);
        }

        template<size_t id, typename R, typename T, typename... arglist, class = typename std::enable_if<
                std::is_void<R>::value && std::is_base_of<T, C>::value>::type>
        MockingContext<void, arglist...> stub(R(T::*vMethod)(arglist...) volatile) {
            auto methodWithoutConstVolatile = reinterpret_cast<void (T::*)(arglist...)>(vMethod);
            return impl.template stubMethod<id>(methodWithoutConstVolatile);
        }

        template<size_t id, typename R, typename T, typename... arglist, class = typename std::enable_if<
                std::is_void<R>::value && std::is_base_of<T, C>::value>::type>
        MockingContext<void, arglist...> stub(R(T::*vMethod)(arglist...) const volatile) {
            auto methodWithoutConstVolatile = reinterpret_cast<void (T::*)(arglist...)>(vMethod);
            return impl.template stubMethod<id>(methodWithoutConstVolatile);
        }

        template<size_t id, typename R, typename T, typename... arglist, class = typename std::enable_if<
                std::is_void<R>::value && std::is_base_of<T, C>::value>::type>
        MockingContext<void, arglist...> stub(R(T::*vMethod)(arglist...)) {
            auto methodWithoutConstVolatile = reinterpret_cast<void (T::*)(arglist...)>(vMethod);
            return impl.template stubMethod<id>(methodWithoutConstVolatile);
        }

        DtorMockingContext dtor() {
            return impl.stubDtor();
        }

        void getActualInvocations(std::unordered_set<Invocation *> &into) const override {
            impl.getActualInvocations(into);
        }

    };

}
