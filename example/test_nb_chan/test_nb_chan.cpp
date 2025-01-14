//
// Created by skyfire on 19-9-26.
//

#include <iostream>
#include "core/sf_nb_chan.hpp"
#include "tools/sf_thread_pool.hpp"

using namespace std;
using namespace skyfire;

mutex mu;

void product(const shared_ptr<sf_nb_chan<int>>& ch){
    for(int i=0;i<100;++i){
        this_thread::sleep_for(chrono::seconds(3));
        ch<<i;
        {
            lock_guard<mutex> lck(mu);
            cout << "thread:" << this_thread::get_id() << " product:" << i << endl;
        }
    }
    ch->close();
}

void custom(const shared_ptr<sf_nb_chan<int>>& ch){
    try {
        while (true) {
            int i;
            this_thread::sleep_for(chrono::seconds(1));
            i << ch;
            {
                lock_guard<mutex> lck(mu);
                cout << "\t\tthread:" << this_thread::get_id() << " custom:" << i << endl;
            }
        }
    }catch (sf_chan_close_exception &e){
        cout<<e.what()<<endl;
    }
}


int main(){
    auto pool = sf_thread_pool::make_instance(8);
    auto ch = sf_nb_chan<int>::make_instance();
    pool->add_task(product, ch);
    pool->add_task(product, ch);
    pool->add_task(product, ch);
    pool->add_task(product, ch);

    pool->add_task(custom, ch);
    pool->wait_all_task_finished();
}