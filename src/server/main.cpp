//
// Created by hariharan on 8/14/19.
//

#ifndef PKV_MAIN_H
#define PKV_MAIN_H


#include <csignal>
#include <basket/common/singleton.h>
#include <pkv/server/server.h>
#include <basket/common/data_structures.h>
#include <pkv/data_structure.h>
#include <pkv/configuration_manager.h>
#include <pkv/daemonize.h>

void finalize(){
    MPI_Barrier(MPI_COMM_WORLD);
    auto server = basket::Singleton<pkv::Server>::GetInstance("SSD_KV");
    server->Stop();
    MPI_Finalize();
}

int main(int argc, char* argv[]){
    MPI_Init(&argc,&argv);
    MPI_Barrier(MPI_COMM_WORLD);
    if(argc > 1) CHRONOLOG_CONF->CONFIGURATION_FILE=argv[1];
    BASKET_CONF->BACKED_FILE_DIR=CHRONOLOG_CONF->PKV_SERVER_DIR;
    CHRONOLOG_CONF->ConfigurePKVServer();
    auto server = basket::Singleton<pkv::Server>::GetInstance("SSD_KV");
    server->Run();
    strcpy(main_log_file,"pkv.log");
    catch_all_signals();
    while(true) sleep(1);
}
#endif //PKV_MAIN_H
