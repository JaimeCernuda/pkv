//
// Created by hariharan on 8/16/19.
//

#ifndef PKV_CONFIGURATION_MANAGER_H
#define PKV_CONFIGURATION_MANAGER_H

#include <basket/common/singleton.h>
#include <basket/common/typedefs.h>
#include <basket/common/data_structures.h>
#include <basket/common/macros.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/reader.h>

#define CHRONOLOG_CONF basket::Singleton<chronolog::ConfigurationManager>::GetInstance()
namespace chronolog{
    class ConfigurationManager{

    private:
        void config(rapidjson::Document &doc, const char* member, uint16_t &variable){
            assert(doc.HasMember(member));
            assert(doc[member].IsInt());
            variable = doc[member].GetInt();
        }

        void config(rapidjson::Document &doc, const char* member, really_long &variable){
            assert(doc.HasMember(member));
            assert(doc[member].IsUint64());
            variable = doc[member].GetUint64();
        }

        void config(rapidjson::Document &doc, const char* member, CharStruct &variable){
            assert(doc.HasMember(member));
            assert(doc[member].IsString());
            variable = doc[member].GetString();
        }
        int CountServers(CharStruct server_list_path){
            fstream file;
            int total=0;
            file.open(server_list_path.c_str(), ios::in);
            if (file.is_open()) {
                std::string file_line;
                std::string server_node_name;
                int count;
                while (getline(file, file_line)) {
                    if (!file_line.empty()) {
                        int split_loc = file_line.find(':');  // split to node and net
                        if (split_loc != std::string::npos) {
                            server_node_name = file_line.substr(0, split_loc);
                            count = atoi(file_line.substr(split_loc+1, std::string::npos).c_str());
                        } else {
                            // no special network
                            server_node_name = file_line;
                            count = 1;
                        }
                        // server list is list of network interfaces
                        for(int i=0;i<count;++i){
                            total++;
                        }
                    }
                }
            } else {
                printf("Error: Can't open server list file %s\n", server_list_path.c_str());
                exit(EXIT_FAILURE);
            }
            file.close();
            return total;
        }

    public:
        CharStruct PKV_SERVER_LISTS;
        uint16_t PKV_SERVER_PORT;
        uint16_t PKV_RPC_THREADS;
        CharStruct PKV_SERVER_DIR;
        CharStruct CONFIGURATION_FILE;
        really_long NVME_CAPACITY,SSD_CAPACITY;
        size_t PKV_KEY_SPACE_LEN;
        int PKV_SERVERS_COUNT;


        ConfigurationManager(): CONFIGURATION_FILE("/home/hdevarajan/projects/chronolog/conf/config/chronolog.json"),
                                PKV_SERVER_LISTS("/home/hdevarajan/projects/chronolog/server_lists/pkv"),
                                PKV_SERVER_DIR("/mnt/ssd/hdevarajan/ssd"),
                                PKV_SERVER_PORT(11000),
                                PKV_RPC_THREADS(4),
                                NVME_CAPACITY(200ULL*1024ULL*1024ULL*1024ULL),
                                SSD_CAPACITY(1024ULL*1024ULL*1024ULL*1024ULL),
                                PKV_KEY_SPACE_LEN(1024ULL*1024ULL),
                                PKV_SERVERS_COUNT(1),

        void LoadConfiguration(){
            using namespace rapidjson;

            FILE *outfile = fopen(CONFIGURATION_FILE.c_str(), "r");
            if(outfile == NULL){
                std::cout << "HLog configuration not found" << std::endl;
                exit(EXIT_FAILURE);
            }
            char buf[65536];
            FileReadStream instream(outfile, buf, sizeof(buf));
            Document doc;
            doc.ParseStream<kParseStopWhenDoneFlag>(instream);
            if(!doc.IsObject()) {
                std::cout << "HLog - Canfiguration JSON is invalid" << std::endl;
                fclose(outfile);
                exit(EXIT_FAILURE);
            }
            config(doc, "PKV_SERVER_LISTS", PKV_SERVER_LISTS);
            config(doc, "PKV_SERVER_DIR", PKV_SERVER_DIR);
            config(doc, "PKV_SERVER_PORT", PKV_SERVER_PORT);
            config(doc, "PKV_RPC_THREADS", PKV_RPC_THREADS);
            config(doc, "NVME_CAPACITY", NVME_CAPACITY);
            config(doc, "SSD_CAPACITY", SSD_CAPACITY);
            config(doc, "PKV_KEY_SPACE_LEN", PKV_KEY_SPACE_LEN);
            PKV_SERVERS_COUNT = CountServers(PKV_SERVER_LISTS);
        }
        void ConfigurePKVClient(){
            LoadConfiguration();
            BASKET_CONF->ConfigureDefaultClient(PKV_SERVER_LISTS.c_str());
            BASKET_CONF->RPC_PORT=PKV_SERVER_PORT;
        }
        void ConfigurePKVServer(){
            LoadConfiguration();
            BASKET_CONF->RPC_THREADS=PKV_RPC_THREADS;
            BASKET_CONF->MEMORY_ALLOCATED=1024ULL * 1024ULL * 1024ULL * 1ULL;
            BASKET_CONF->ConfigureDefaultServer(PKV_SERVER_LISTS.c_str());
            BASKET_CONF->RPC_PORT=PKV_SERVER_PORT;
        }
    };
}


#endif //PKV_CONFIGURATION_MANAGER_H
