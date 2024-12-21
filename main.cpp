#include <iostream>
#include <thread>
#include <vector>
#include "NameServer.h"
#include "DataServer.h"


const int replicate_num = 3;

int main(){
    NameServer ns(replicate_num);
    DataServer ds_1("DataServer1");
    DataServer ds_2("DataServer2");
    DataServer ds_3("DataServer3");
    DataServer ds_4("DataServer4");
    ns.add(&ds_1);
    ns.add(&ds_2);
    ns.add(&ds_3);
    ns.add(&ds_4);
    std::thread th1(std::ref(ds_1));
    std::thread th2(std::ref(ds_2));
    std::thread th3(std::ref(ds_3));
    std::thread th4(std::ref(ds_4));
    th1.detach();
    th2.detach();
    th3.detach();
    th4.detach();
    ns();
    return 0;
}
