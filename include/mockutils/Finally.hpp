/*
 * Finally.hpp
 * Copyright (c) 2014 Eran Pe'er.
 *
 * This program is made available under the terms of the MIT License.
 * 
 * Created on Aug 30, 2014
 */
#pragma once

#include <Platform/brstd/functional.h>

namespace fakeit {

    class Finally {
    private:
        brstd::function<void()> _finallyClause;

        Finally(const Finally &);

        Finally &operator=(const Finally &);

    public:
        explicit Finally(brstd::function<void()> f) :
                _finallyClause(f) {
        }

        ~Finally() {
            _finallyClause();
        }
    };
}
