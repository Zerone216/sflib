#include "sf_json.hpp"
#include <sf_stdc++.h>

using namespace skyfire;
using namespace std;


struct Time
{
    int h;
    int m;
    int s;
};

SF_JSONIFY(Time,h,m,s)

struct People
{
    int age;
    string name;
    string address;
    Time t;
};

SF_JSONIFY(People, age, name ,address, t)

int main()
{
    // 1. 使用字面值操作符解析json
    auto json1 = R"({"name": "wmb","data": [1,2,3,"666"]})"_json;
    auto json2 = R"({"age":25, "address":"西安"})"_json;
    auto json3 = R"({"company":"saming"})"_json;
    // 2. 输出json
    cout<<json1<<endl;
    cout<<json2<<endl;
    cout<<json3<<endl;
    cout<<"---------"<<endl;
    // 3. 合并json
    json1.join(json2);
    json1.join(json3.clone());
    cout<<json1<<endl;
    cout<<json2<<endl;
    cout<<json3<<endl;
    cout<<"---------"<<endl;
    // 4. sf_json底层使用指针，数据属于浅拷贝，所以改变json2的值同样会影响json1的值
    json2["address"] = "北京";
    cout<<json1<<endl;
    cout<<json2<<endl;
    cout<<json3<<endl;
    cout<<"---------"<<endl;
    // 5. json3与json1进行合并时，使用clone()创建了副本（深拷贝）,操作json3不会影响json1
    json3["company"]="lenovo";
    cout<<json1<<endl;
    cout<<json2<<endl;
    cout<<json3<<endl;
    cout<<"---------"<<endl;

    std::tuple<int,string,double> t{5,"hello"s, 5.2};
    auto t_js = to_json(t);
    cout<<t_js<<endl;

    std::tuple<int,string,double> t2;
    from_json(t_js, t2);
    cout<<get<0>(t2)<<endl;
    cout<<get<1>(t2)<<endl;
    cout<<get<2>(t2)<<endl;

    People p;
    p.name = "skyfire";
    p.age = 100;
    p.address = "China";
    p.t.h = 24;
    p.t.m = 60;
    p.t.s = 25;
    auto js = to_json(p);
    cout<<js<<endl;

    People p2;
    from_json(js, p2);
    cout<<to_json(p2)<<endl;
}

