//
// Created by hariharan on 8/14/19.
//

#include <mpi.h>
#include <basket/common/macros.h>
#include <c++/pkv/client/client.h>
#include <pkv/data_structure.h>
#include <pkv/configuration_manager.h>

int main(int argc, char* argv[]){
    MPI_Init(&argc,&argv);
    /*if(BASKET_CONF->MPI_RANK==0){
        printf("%d ready for attach\n", BASKET_CONF->COMM_SIZE);
        fflush(stdout);
        getchar();
    }
    MPI_Barrier(MPI_COMM_WORLD);*/
    if(argc > 1) CHRONOLOG_CONF->CONFIGURATION_FILE=argv[1];
    CHRONOLOG_CONF->ConfigurePKVClient();
    auto client = basket::Singleton<pkv::Client>::GetInstance("SSD_KV");
    Event e[4];
    char journal_name[20];
    snprintf(journal_name, 20, "test_%d",BASKET_CONF->MPI_RANK);
    e[0].journal_name_=CharStruct(journal_name);
    e[0].event_id=1+BASKET_CONF->MPI_RANK*10;
    e[1].journal_name_=CharStruct(journal_name);
    e[1].event_id=2+BASKET_CONF->MPI_RANK*10;
    e[2].journal_name_=CharStruct(journal_name);
    e[2].event_id=3+BASKET_CONF->MPI_RANK*10;
    e[3].journal_name_=CharStruct(journal_name);
    e[3].event_id=4+BASKET_CONF->MPI_RANK*10;

    std::string data="Hello World";

    // Do three puts and check statuses
    bool status;
    for (int i = 0; i < 3; i++) {
        status = client->Put(e[i],data);
        assert(status);
    }

    // Do gets and check results
    std::string get_data;
    std::pair<bool, std::string> iter;
    for (int i = 0; i < 3; i++) {
        iter = client->Get(e[i]);
        assert(iter.first);
        assert(iter.second==data);
    }

    // Check results on range get
    auto results = client->GetRange(e[1],e[2]);
    assert(results.size() == 2);
    for(int i = 0; i < results.size(); i++){
        assert(results[i].first==e[i+1]);
        assert(results[i].second==data);
    }

    // Check results on get of item not put yet
    auto fail = client->Get(e[3]);
    assert(!fail.first);

    // Check results on range get including item not put yet on rhs
    auto results_ob_right = client->GetRange(e[1],e[3]);
    assert(results_ob_right.size() == 2);

    for(int i = 0; i < results_ob_right.size(); i++){
        assert(results_ob_right[i].first==e[i+1]);
        assert(results_ob_right[i].second==data);
    }

    // Check results on range get with greater start than end
    auto nonsense_results = client->GetRange(e[2], e[1]);
    assert(nonsense_results.size() == 0);    

    // Check status of delete on item
    status = client->Delete(e[0]);
    assert(status);

    // Check results on range get including deleted item on lhs
    auto results_ob_left = client->GetRange(e[0], e[2]);
    assert(results_ob_left.size() == 2);
    for(int i = 0; i < results_ob_left.size(); i++){
        assert(results_ob_left[i].first==e[i+1]);
        assert(results_ob_left[i].second==data);
    }

    // Check results on range with "hole" in middle
    status = client->Put(e[0], data);
    assert(status);    
    status = client->Delete(e[1]);
    assert(status);
    auto results_hole = client->GetRange(e[0], e[2]);
    assert(results_hole.size() == 2);
    assert(results_hole[0].first == e[0]);
    assert(results_hole[0].second==data);
    assert(results_hole[1].first == e[2]);
    assert(results_hole[1].second==data);
    
    // Check statuses of more deletes (including after being put a second time)
    status = client->Delete(e[0]);
    assert(status);
    status = client->Delete(e[2]);
    assert(status);

    // Check status of delete on item never put
    status = client->Delete(e[3]);
    assert(!status);
    MPI_Finalize();
    return 0;
}
