/*
    Copyright (c) 2007-2017 Contributors as noted in the AUTHORS file

    This file is part of libzmq, the ZeroMQ core engine in C++.

    libzmq is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    As a special exception, the Contributors give you permission to link
    this library with independent modules to produce an executable,
    regardless of the license terms of these independent modules, and to
    copy and distribute the resulting executable under terms of your choice,
    provided that you also meet, for each linked independent module, the
    terms and conditions of the license of that module. An independent
    module is a module which is not derived from or based on this library.
    If you modify this library, you must extend this exception to your
    version of the library.

    libzmq is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "precompiled.hpp"
#include <stdlib.h>

#if !defined ZMQ_HAVE_WINDOWS
#include <unistd.h>
#endif

#include "random.hpp"
#include "stdint.hpp"
#include "clock.hpp"
#include "mutex.hpp"
#include "macros.hpp"

#if defined(ZMQ_USE_LIBSODIUM)
#include "sodium.h"
#endif

void zmq::seed_random ()
{
#if defined ZMQ_HAVE_WINDOWS
    const int pid = static_cast<int> (GetCurrentProcessId ());
#else
    int pid = static_cast<int> (getpid ());
#endif
    srand (static_cast<unsigned int> (clock_t::now_us () + pid));
}

uint32_t zmq::generate_random ()
{
    //  Compensate for the fact that rand() returns signed integer.
    const uint32_t low = static_cast<uint32_t> (rand ());
    uint32_t high = static_cast<uint32_t> (rand ());
    high <<= (sizeof (int) * 8 - 1);
    return high | low;
}

static void manage_random (bool init_)
{
#if defined(ZMQ_USE_LIBSODIUM)
    if (init_) {
        //  sodium_init() is now documented as thread-safe in recent versions
        int rc = sodium_init ();
        zmq_assert (rc != -1);
#if defined(ZMQ_LIBSODIUM_RANDOMBYTES_CLOSE)
    } else {
        // randombytes_close either a no-op or not threadsafe
        // doing this without refcounting can cause crashes
        // if called while a context is active
        randombytes_close ();
#endif
    }
#else
    LIBZMQ_UNUSED (init_);
#endif
}

void zmq::random_open ()
{
    manage_random (true);
}

void zmq::random_close ()
{
    manage_random (false);
}
