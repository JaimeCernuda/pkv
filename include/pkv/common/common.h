//
// Created by hariharan on 11/13/19.
//

#ifndef CHRONOLOG_PKV_COMMON_H
#define CHRONOLOG_PKV_COMMON_H


#include <pkv/data_structure.h>
#include <pkv/configuration_manager.h>

static std::size_t hash_value(Event const& k)
{
    size_t hash_val = hash<CharStruct>()(k.journal_name_);
    size_t val = ((k.event_id / CHRONOLOG_CONF->PKV_KEY_SPACE_LEN) + hash_val) % CHRONOLOG_CONF->PKV_SERVERS_COUNT;
    return val;
}
#endif //CHRONOLOG_PKV_COMMON_H
