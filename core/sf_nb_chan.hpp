#pragma once
#include "sf_nb_chan.h"

namespace skyfire {
    template <typename  T>
    void sf_nb_chan<T>::close(){
        closed__ = true;
        cond_pop__.notify_all();
    }
}