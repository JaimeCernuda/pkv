//
// Created by hariharan on 8/14/19.
//

#ifndef PKV_CLIENT_H
#define PKV_CLIENT_H

#include <basket/unordered_map/unordered_map.h>
#include <basket/set/set.h>
#include <pkv/common/common.h>
#include <pkv/data_structure.h>
#include <boost/thread/mutex.hpp>

namespace pkv{
    class Client {
    private:
        basket::unordered_map<Event,bip::string,boost::hash<Event>> data_map;
        basket::set<Event,boost::hash<Event>> meta_set;
        std::string func_prefix;
        std::shared_ptr<RPC> rpc;
        int num_servers,my_server;
        boost::mutex mtx;
    public:
        Client(std::string name):data_map(name+"_DATA"), meta_set(name+"_META"), func_prefix(name),num_servers(BASKET_CONF->NUM_SERVERS),my_server(BASKET_CONF->MY_SERVER){
            rpc=basket::Singleton<RPCFactory>::GetInstance()->GetRPC(BASKET_CONF->RPC_PORT);
        }
        bool Put(Event& key,std::string& value){
            mtx.lock();
            auto b_str=bip::string(value.c_str());
            bool status = data_map.Put(key,b_str) && meta_set.Put(key);
            mtx.unlock();
            return status;
        }
        std::pair<bool, std::string> Get(Event& key){
            mtx.lock();
            auto p=data_map.Get(key);
            auto status = std::pair<bool, std::string>(p.first,p.second.c_str());
            mtx.unlock();
            return status;
        }
        bool Delete(Event& key){
            mtx.lock();
            auto ret_1=data_map.Erase(key);
            auto ret_2=meta_set.Erase(key);
            auto status = ret_1.first && ret_2;
            mtx.unlock();
            return status;
        }
        std::vector<std::pair<Event,std::string>> GetRange(Event& start_key,Event& end_key) {

            int start_server = start_key.event_id/CHRONOLOG_CONF->PKV_KEY_SPACE_LEN;
            int end_server = end_key.event_id/CHRONOLOG_CONF->PKV_KEY_SPACE_LEN;
            auto events = std::vector<std::pair<Event, std::string>>();
            if(end_server - start_server > CHRONOLOG_CONF->PKV_SERVERS_COUNT){
                /**
                 * I will anyways touch all servers.
                 */
                events = rpc->call<RPCLIB_MSGPACK::object_handle>(my_server,func_prefix+"_GetRange",start_key,end_key).template as<std::vector<std::pair<Event,
                        std::string>>>();
            }
            else{
                uint32_t start_event_id=start_key.event_id;
                while(start_event_id < end_key.event_id){
                    uint32_t end_event_id = (start_event_id/CHRONOLOG_CONF->PKV_KEY_SPACE_LEN + 1)*CHRONOLOG_CONF->PKV_KEY_SPACE_LEN - 1;
                    if(end_event_id > end_key.event_id) end_event_id = end_key.event_id;
                    auto events_temp = rpc->call<RPCLIB_MSGPACK::object_handle>(my_server,func_prefix+"_GetRangeLocal",start_key,end_key).template as<std::vector<std::pair<Event,
                            std::string>>>();
                    events.insert(events.end(),events_temp.begin(),events_temp.end());
                    start_event_id = end_event_id + 1;
                }

            }
            sort(events.begin(), events.end(),
                 [](const std::pair<Event, std::string> & a, const std::pair<Event, std::string> & b) -> bool{
                     return a.first < b.first;
                 });
            return events;
        }
    };
}


#endif //PKV_CLIENT_H
