//
// Created by hariharan on 8/14/19.
//

#ifndef PKV_SERVER_H
#define PKV_SERVER_H

#include <pkv/data_structure.h>
#include <pkv/common/common.h>
#include <basket/unordered_map/unordered_map.h>
#include <basket/set/set.h>
#include <basket/common/macros.h>

namespace pkv{
    class Server {
    private:
        basket::unordered_map<Event,bip::string,boost::hash<Event>> data_map;
        basket::set<Event,boost::hash<Event>> meta_set;
        std::promise<void> exit_server_signal;
        std::thread worker;
        CharStruct name;
        std::shared_ptr<RPC> rpc;
        void RunInternal(std::future<void> futureObj){
            bool count=true;
            while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
                usleep(10000);
                if(count){
                    if(BASKET_CONF->MPI_RANK == 0) printf("Started the PKV server\n");
                    count=false;
                }
            }
        }
    public:
        Server(CharStruct name_):name(name_),data_map(name_+"_DATA"), meta_set(name_+"_META"), exit_server_signal(),worker(){
            rpc=basket::Singleton<RPCFactory>::GetInstance()->GetRPC(BASKET_CONF->RPC_PORT);
            std::function<bool(Event&,std::string&)> callBackFunction(std::bind(&Server::PutCallBack,this,std::placeholders::_1,std::placeholders::_2));
            rpc->bind(name+"_PutCallBack", callBackFunction);
            std::function<std::vector<std::pair<Event,std::string>>(Event&,Event&)> functionGetRangeLocal(std::bind(&Server::GetRangeLocal,this,
                    std::placeholders::_1,std::placeholders::_2));
            rpc->bind(name+"_GetRangeLocal", functionGetRangeLocal);
            std::function<std::vector<std::pair<Event,std::string>>(Event&,Event&)> functionGetRange(std::bind(&Server::GetRange,this,
                    std::placeholders::_1,std::placeholders::_2));
            rpc->bind(name+"_GetRange",functionGetRange);

        }
        bool PutCallBack(Event& key,std::string& value){
            auto b_str=bip::string(value.c_str());
            return data_map.Put(key,b_str) && meta_set.Put(key);
        }
        std::vector<std::pair<Event,std::string>> GetRange(Event& start_key,Event& end_key){
            auto final_values=std::vector<std::pair<Event,std::string>>();
            auto current_server = GetRangeLocal(start_key,end_key);
            final_values.insert(final_values.end(), current_server.begin(), current_server.end());
            for (int i = 0; i < BASKET_CONF->COMM_SIZE; ++i) {
                if (i != BASKET_CONF->MPI_RANK) {
                    typedef std::vector<std::pair<Event,std::string>> ret_type;
                    auto func_prefix = name;
                    auto server = RPC_CALL_WRAPPER("_GetRangeLocal", i, ret_type, start_key,end_key);
                    final_values.insert(final_values.end(), server.begin(), server.end());
                }
            }
            return final_values;
        }
        std::vector<std::pair<Event,std::string>> GetRangeLocal(Event& start_key,Event& end_key){
            auto final_values=std::vector<std::pair<Event,std::string>>();
            auto matched_keys = meta_set.ContainsInServer(start_key,end_key);
            for(auto matched_key:matched_keys){
                final_values.insert(final_values.end(), std::pair<Event,std::string>(matched_key,data_map.Get(matched_key).second));
            }
            return final_values;
        }

        void Run(){
            std::future<void> futureObj = exit_server_signal.get_future();
            worker=std::thread (&Server::RunInternal, this, std::move(futureObj));
        }
        void Stop(){
            exit_server_signal.set_value();
            worker.join();
            if(BASKET_CONF->MPI_RANK == 0) printf("Stopped the PKV server\n");
        }
    };
}
#endif //PKV_SERVER_H
