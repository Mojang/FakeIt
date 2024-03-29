/*
 * Copyright (c) 2014 Eran Pe'er.
 *
 * This program is made available under the terms of the MIT License.
 *
 * Created on Mar 10, 2014
 */
#pragma once

#undef max
#include <functional>
#include <type_traits>
#include <vector>
#include <array>
#include <new>
#include <limits>

#include "mockutils/VirtualTable.hpp"
#include "mockutils/union_cast.hpp"
#include "mockutils/MethodInvocationHandler.hpp"
#include "mockutils/VTUtils.hpp"
#include "mockutils/FakeObject.hpp"
#include "mockutils/MethodProxy.hpp"
#include "mockutils/MethodProxyCreator.hpp"

namespace fakeit {

    class InvocationHandlers : public InvocationHandlerCollection {
        std::vector<std::shared_ptr<Destructible>> &_methodMocks;
        std::vector<size_t> &_offsets;

        unsigned int getOffset(size_t id) const
        {
            unsigned int offset = 0;
            for (; offset < _offsets.size(); offset++) {
                if (_offsets[offset] == id) {
                    break;
                }
            }
            return offset;
        }

    public:
        InvocationHandlers(
                std::vector<std::shared_ptr<Destructible>> &methodMocks,
                std::vector<size_t> &offsets) :
                _methodMocks(methodMocks), _offsets(offsets) {
			for (auto it = _offsets.begin(); it != _offsets.end(); ++it)
			{
				*it = std::numeric_limits<int>::max();
			}
        }

        Destructible *getInvocatoinHandlerPtrById(size_t id) override {
            unsigned int offset = getOffset(id);
            std::shared_ptr<Destructible> ptr = _methodMocks[offset];
            return ptr.get();
        }

    };

    template<typename C, typename ... baseclasses>
    struct DynamicProxy {

        static_assert(std::is_polymorphic<C>::value, "DynamicProxy requires a polymorphic type");

        DynamicProxy(C &inst, bool isUsingSpy) :
                instance(inst),
                originalVtHandle(VirtualTable<C, baseclasses...>::getVTable(instance).createHandle()),
                _methodMocks(VTUtils::getVTSize<C>()),
                _offsets(VTUtils::getVTSize<C>()),
                _invocationHandlers(_methodMocks, _offsets),
                _isUsingSpy(isUsingSpy) {
            _cloneVt.copyFrom(originalVtHandle.restore(), isUsingSpy);
            _cloneVt.setCookie(InvocationHandlerCollection::VtCookieIndex, &_invocationHandlers);
            getFake().setVirtualTable(_cloneVt);
        }

        void detach() {
            getFake().setVirtualTable(originalVtHandle.restore());
        }

        ~DynamicProxy() {
            _cloneVt.dispose();
        }

        C &get() {
            return instance;
        }

        void Reset() {
			_methodMocks = {};
            _methodMocks.resize(VTUtils::getVTSize<C>());
            _members = {};
			_offsets = {};
            _offsets.resize(VTUtils::getVTSize<C>());
            _cloneVt.copyFrom(originalVtHandle.restore(), _isUsingSpy);
        }

		void Clear()
        {
        }

        template<size_t id, typename R, typename ... arglist>
        void stubMethod(R(C::*vMethod)(arglist...), MethodInvocationHandler<R, arglist...> *methodInvocationHandler) {
            auto offset = VTUtils::getOffset(vMethod);
            MethodProxyCreator<R, arglist...> creator;
            bind(creator.template createMethodProxy<id + 1>(offset), methodInvocationHandler);
        }

        void stubDtor(MethodInvocationHandler<void> *methodInvocationHandler) {
            auto offset = VTUtils::getDestructorOffset<C>();
            MethodProxyCreator<void> creator;
            bindDtor(creator.createMethodProxy<0>(offset), methodInvocationHandler);
        }

        template<typename R, typename ... arglist>
        bool isMethodStubbed(R(C::*vMethod)(arglist...)) {
            unsigned int offset = VTUtils::getOffset(vMethod);
            return isBinded(offset);
        }

        bool isDtorStubbed() {
            unsigned int offset = VTUtils::getDestructorOffset<C>();
            return isBinded(offset);
        }

        template<typename R, typename ... arglist>
        Destructible *getMethodMock(R(C::*vMethod)(arglist...)) {
            auto offset = VTUtils::getOffset(vMethod);
            std::shared_ptr<Destructible> ptr = _methodMocks[offset];
            return ptr.get();
        }

        Destructible *getDtorMock() {
            auto offset = VTUtils::getDestructorOffset<C>();
            std::shared_ptr<Destructible> ptr = _methodMocks[offset];
            return ptr.get();
        }

        template<typename DataType, typename ... arglist>
        void stubDataMember(DataType C::*member, const arglist &... initargs) {
            DataType C::*theMember = (DataType C::*) member;
            C &mock = get();
            DataType *memberPtr = &(mock.*theMember);
            _members.push_back(
                    std::shared_ptr<DataMemeberWrapper < DataType, arglist...> >
                    {new DataMemeberWrapper < DataType, arglist...>(memberPtr,
                    initargs...)});
        }

        template<typename DataType>
        void getMethodMocks(std::vector<DataType> &into) const {
            for (std::shared_ptr<Destructible> ptr : _methodMocks) {
                DataType p = dynamic_cast<DataType>(ptr.get());
                if (p) {
                    into.push_back(p);
                }
            }
        }

        VirtualTable<C, baseclasses...> &getOriginalVT() {
            VirtualTable<C, baseclasses...> &vt = originalVtHandle.restore();
            return vt;
        }

    private:

        template<typename DataType, typename ... arglist>
        class DataMemeberWrapper : public Destructible {
        private:
            DataType *dataMember;
        public:
            DataMemeberWrapper(DataType *dataMem, const arglist &... initargs) :
                    dataMember(dataMem) {
                new(dataMember) DataType{initargs ...};
            }

            ~DataMemeberWrapper() override
            {
                dataMember->~DataType();
            }
        };

        static_assert(sizeof(C) == sizeof(FakeObject<C, baseclasses...>), "This is a problem");

        C &instance;
        typename VirtualTable<C, baseclasses...>::Handle originalVtHandle; // avoid delete!! this is the original!
        VirtualTable<C, baseclasses...> _cloneVt;
        //
        std::vector<std::shared_ptr<Destructible>> _methodMocks;
        std::vector<std::shared_ptr<Destructible>> _members;
        std::vector<size_t> _offsets;
        InvocationHandlers _invocationHandlers;
        bool _isUsingSpy = false;

        FakeObject<C, baseclasses...> &getFake() {
            return reinterpret_cast<FakeObject<C, baseclasses...> &>(instance);
        }

        void bind(const MethodProxy &methodProxy, Destructible *invocationHandler) {
            getFake().setMethod(methodProxy.getOffset(), methodProxy.getProxy());
            _methodMocks[methodProxy.getOffset()].reset(invocationHandler);
            _offsets[methodProxy.getOffset()] = methodProxy.getId();
        }

        void bindDtor(const MethodProxy &methodProxy, Destructible *invocationHandler) {
            getFake().setDtor(methodProxy.getProxy());
            _methodMocks[methodProxy.getOffset()].reset(invocationHandler);
            _offsets[methodProxy.getOffset()] = methodProxy.getId();
        }

        template<typename DataType>
        DataType getMethodMock(unsigned int offset) {
            std::shared_ptr<Destructible> ptr = _methodMocks[offset];
            return dynamic_cast<DataType>(ptr.get());
        }

        template<typename BaseClass>
        void checkMultipleInheritance() {
            C *ptr = (C *) (unsigned int) 1;
            BaseClass *basePtr = ptr;
            int delta = (unsigned long) basePtr - (unsigned long) ptr;
            if (delta > 0) {
                // base class does not start on same position as derived class.
                // this is multiple inheritance.
                throw std::invalid_argument(std::string("multiple inheritance is not supported"));
            }
        }

        bool isBinded(unsigned int offset) {
            std::shared_ptr<Destructible> ptr = _methodMocks[offset];
            return ptr.get() != nullptr;
        }

    };
}
