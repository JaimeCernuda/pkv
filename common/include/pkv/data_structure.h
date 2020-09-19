//
// Created by hariharan on 8/14/19.
//

#ifndef PKV_DATA_STRUCTURE_H
#define PKV_DATA_STRUCTURE_H

#include <basket/common/data_structures.h>
#include <rpc/msgpack.hpp>

typedef struct Event{
    CharStruct journal_name_;
    uint32_t event_id;
    bool operator>(const Event &o) const {
        return  journal_name_ >= o.journal_name_ && event_id > o.event_id;
    }
    bool operator>=(const Event &o) const {
        return journal_name_ >= o.journal_name_ &&  event_id >= o.event_id;
    }
    bool operator<(const Event &o) const {
        return journal_name_ <= o.journal_name_ &&  event_id < o.event_id;
    }
    bool operator==(const Event &o) const {
        return o.event_id == event_id && o.journal_name_==journal_name_;
    }
    Event():journal_name_(),event_id(0){}
    Event(const Event &other) : journal_name_(other.journal_name_),
                                event_id(other.event_id) {} /* copy constructor*/
    Event(Event &&other) :journal_name_(other.journal_name_),
                          event_id(other.event_id){} /* move constructor*/
    /**
    * Operators
    */
    Event &operator=(const Event &other) {
        event_id = other.event_id;
        journal_name_ = other.journal_name_;
        return *this;
    }
} Event;

typedef struct Node{
    CharStruct node_name_;
    uint16_t port_;
    Node():node_name_(),port_(0){}
    Node(const Node &other) : node_name_(other.node_name_),
                              port_(other.port_) {} /* copy constructor*/
    Node(Node &&other) :node_name_(other.node_name_),
                        port_(other.port_) {} /* move constructor*/
    /**
    * Operators
    */
    Node &operator=(const Node &other) {
        node_name_ = other.node_name_;
        port_ = other.port_;
        return *this;
    }
}Node;

typedef struct PlaybackEvent{
    CharStruct journal_name_;
    uint32_t start_;
    uint32_t end_;
    Node destination;
    bool operator>(const PlaybackEvent &o) const {
        return  end_ > o.end_;
    }
    bool operator>=(const PlaybackEvent &o) const {
        return end_ >= o.end_;;
    }
    bool operator<(const PlaybackEvent &o) const {
        return start_ < o.start_;
    }
    bool operator==(const PlaybackEvent &o) const {
        return start_==o.start_ && end_ == o.end_;
    }
    bool operator!=(const PlaybackEvent &o) const {
        return !(*this == o);
    }
    PlaybackEvent():journal_name_(),start_(-1),end_(-1),destination(){}
    PlaybackEvent(const PlaybackEvent &other) : journal_name_(other.journal_name_),
                                                start_(other.start_),
                                                end_(other.end_),
                                                destination(other.destination){} /* copy constructor*/
    PlaybackEvent(PlaybackEvent &&other) :journal_name_(other.journal_name_),
                                          start_(other.start_),
                                          end_(other.end_),
                                          destination(other.destination){}  /* move constructor*/
    /**
    * Operators
    */
    PlaybackEvent &operator=(const PlaybackEvent &other) {
        journal_name_=other.journal_name_;
        start_ = other.start_;
        end_ = other.end_;
        destination=other.destination;
        return *this;
    }

    std::vector<PlaybackEvent> Substract(const PlaybackEvent &o) const {
        std::vector<PlaybackEvent> left_over = std::vector<PlaybackEvent>();
        PlaybackEvent myinstance(*this);
        if (start_ < o.start_ && end_ >= o.start_) {
            PlaybackEvent new_matrix(myinstance);
            new_matrix.end_ = o.start_ - 1;
            left_over.push_back(new_matrix);
            myinstance.start_ = o.start_;
        }
        if (start_ <= o.end_ && end_ > o.end_) {
            PlaybackEvent new_matrix(myinstance);

            new_matrix.start_ = o.end_ + 1;
            left_over.push_back(new_matrix);
            myinstance.end_ = o.end_;
        }
        return left_over;
    }
    PlaybackEvent Union(const PlaybackEvent &o) const {
        PlaybackEvent return_instance(*this);
        if (start_ < o.start_) return_instance.start_ = start_;
        else return_instance.start_ = o.start_;
        if (end_ < o.end_) return_instance.end_ = o.end_;
        else return_instance.end_ = end_;
        return return_instance;
    }
} EventRange;

typedef struct Location{
    CharStruct path;
    size_t offset;
    size_t event_size;
    bool operator==(const Location &o) const {
        return o.path == path && o.offset==offset && o.event_size==event_size;
    }
    Location():path(),offset(0),event_size(0){}
    Location(const Location &other) : path(other.path),
                                      offset(other.offset),
                                      event_size(other.event_size){} /* copy constructor*/
    Location(Location &&other) :path(other.path),
                                offset(other.offset),
                                event_size(other.event_size){} /* move constructor*/
    Location &operator=(const Location &other) {
        path = other.path;
        offset = other.offset;
        event_size = other.event_size;
        return *this;
    }
} Location;


namespace std {
    template<>
    struct hash<Node> {
        int operator()(const Node &k) const {
            size_t hash_val = hash<CharStruct>()(k.node_name_);
            hash_val ^= hash<uint16_t>()(k.port_);
            return hash_val;
        }
    };
    template<>
    struct hash<PlaybackEvent> {
        int operator()(const PlaybackEvent &k) const {
            size_t hash_val = hash<CharStruct>()(k.journal_name_);
            hash_val ^= hash<uint32_t>()(k.start_);
            hash_val ^= hash<uint32_t>()(k.end_);
            hash_val ^= hash<Node>()(k.destination);
            return hash_val;
        }
    };
}
#ifdef BASKET_ENABLE_RPCLIB
namespace clmdep_msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
    namespace mv1 = clmdep_msgpack::v1;
        template<>
        struct convert<Event> {
            mv1::object const &operator()(mv1::object const &o, Event &input) const {
                input.journal_name_ = o.via.array.ptr[0].as<CharStruct>();
                input.event_id = o.via.array.ptr[1].as<uint32_t>();
                return o;
            }
        };

        template<>
        struct pack<Event> {
            template<typename Stream>
            packer <Stream> &operator()(mv1::packer <Stream> &o, Event const &input) const {
                o.pack_array(2);
                o.pack(input.journal_name_);
                o.pack(input.event_id);
                return o;
            }
        };

        template<>
        struct object_with_zone<Event> {
            void operator()(mv1::object::with_zone &o, Event const &input) const {
                o.type = type::ARRAY;
                o.via.array.size = 2;
                o.via.array.ptr = static_cast<clmdep_msgpack::object *>(o.zone.allocate_align(
                        sizeof(mv1::object) * o.via.array.size, MSGPACK_ZONE_ALIGNOF(mv1::object)));
                o.via.array.ptr[0] = mv1::object(input.journal_name_, o.zone);
                o.via.array.ptr[1] = mv1::object(input.event_id, o.zone);
            }
        };

        template<>
        struct convert<Location> {
            mv1::object const &operator()(mv1::object const &o, Location &input) const {
                input.path = o.via.array.ptr[0].as<CharStruct>();
                input.event_size = o.via.array.ptr[1].as<size_t>();
                input.offset = o.via.array.ptr[2].as<size_t>();
                return o;
            }
        };

        template<>
        struct pack<Location> {
            template<typename Stream>
            packer <Stream> &operator()(mv1::packer <Stream> &o, Location const &input) const {
                o.pack_array(3);
                o.pack(input.path);
                o.pack(input.event_size);
                o.pack(input.offset);
                return o;
            }
        };

        template<>
        struct object_with_zone<Location> {
            void operator()(mv1::object::with_zone &o, Location const &input) const {
                o.type = type::ARRAY;
                o.via.array.size = 3;
                o.via.array.ptr = static_cast<clmdep_msgpack::object *>(o.zone.allocate_align(
                        sizeof(mv1::object) * o.via.array.size, MSGPACK_ZONE_ALIGNOF(mv1::object)));
                o.via.array.ptr[0] = mv1::object(input.path, o.zone);
                o.via.array.ptr[1] = mv1::object(input.event_size, o.zone);
                o.via.array.ptr[2] = mv1::object(input.offset, o.zone);
            }
        };

        template<>
        struct convert<bip::string> {
            clmdep_msgpack::object const &operator()(clmdep_msgpack::object const &o, bip::string &v) const {
                switch (o.type) {
                    case clmdep_msgpack::type::BIN:
                        v.assign(o.via.bin.ptr, o.via.bin.size);
                        break;
                    case clmdep_msgpack::type::STR:
                        v.assign(o.via.str.ptr, o.via.str.size);
                        break;
                    default:
                        throw clmdep_msgpack::type_error();
                        break;
                }
                return o;
            }
        };

        template<>
        struct pack<bip::string> {
            template<typename Stream>
            clmdep_msgpack::packer<Stream> &
            operator()(clmdep_msgpack::packer<Stream> &o, const bip::string &v) const {
                uint32_t size = checked_get_container_size(v.size());
                o.pack_str(size);
                o.pack_str_body(v.data(), size);
                return o;
            }
        };

        template<>
        struct object<bip::string> {
            void operator()(clmdep_msgpack::object &o, const bip::string &v) const {
                uint32_t size = checked_get_container_size(v.size());
                o.type = clmdep_msgpack::type::STR;
                o.via.str.ptr = v.data();
                o.via.str.size = size;
            }
        };

        template<>
        struct object_with_zone<bip::string> {
            void operator()(clmdep_msgpack::object::with_zone &o, const bip::string &v) const {
                uint32_t size = checked_get_container_size(v.size());
                o.type = clmdep_msgpack::type::STR;
                char *ptr = static_cast<char *>(o.zone.allocate_align(size, MSGPACK_ZONE_ALIGNOF(char)));
                o.via.str.ptr = ptr;
                o.via.str.size = size;
                std::memcpy(ptr, v.data(), v.size());
            }
        };

        template<>
        struct convert<PlaybackEvent> {
            mv1::object const &operator()(mv1::object const &o, PlaybackEvent &input) const {
                input.journal_name_ = o.via.array.ptr[0].as<CharStruct>();
                input.start_ = o.via.array.ptr[1].as<uint32_t>();
                input.end_ = o.via.array.ptr[2].as<uint32_t>();
                input.destination = o.via.array.ptr[3].as<Node>();
                return o;
            }
        };

        template<>
        struct pack<PlaybackEvent> {
            template<typename Stream>
            packer <Stream> &operator()(mv1::packer <Stream> &o, PlaybackEvent const &input) const {
                o.pack_array(4);
                o.pack(input.journal_name_);
                o.pack(input.start_);
                o.pack(input.end_);
                o.pack(input.destination);
                return o;
            }
        };

        template<>
        struct object_with_zone<PlaybackEvent> {
            void operator()(mv1::object::with_zone &o, PlaybackEvent const &input) const {
                o.type = type::ARRAY;
                o.via.array.size = 4;
                o.via.array.ptr = static_cast<clmdep_msgpack::object *>(o.zone.allocate_align(
                        sizeof(mv1::object) * o.via.array.size, MSGPACK_ZONE_ALIGNOF(mv1::object)));
                o.via.array.ptr[0] = mv1::object(input.journal_name_, o.zone);
                o.via.array.ptr[1] = mv1::object(input.start_, o.zone);
                o.via.array.ptr[2] = mv1::object(input.end_, o.zone);
                o.via.array.ptr[3] = mv1::object(input.destination, o.zone);
            }
        };

        template<>
        struct convert<Node> {
            mv1::object const &operator()(mv1::object const &o, Node &input) const {
                input.node_name_ = o.via.array.ptr[0].as<CharStruct>();
                input.port_ = o.via.array.ptr[1].as<uint16_t>();
                return o;
            }
        };

        template<>
        struct pack<Node> {
            template<typename Stream>
            packer <Stream> &operator()(mv1::packer <Stream> &o, Node const &input) const {
                o.pack_array(2);
                o.pack(input.node_name_);
                o.pack(input.port_);
                return o;
            }
        };

        template<>
        struct object_with_zone<Node> {
            void operator()(mv1::object::with_zone &o, Node const &input) const {
                o.type = type::ARRAY;
                o.via.array.size = 2;
                o.via.array.ptr = static_cast<clmdep_msgpack::object *>(o.zone.allocate_align(
                        sizeof(mv1::object) * o.via.array.size, MSGPACK_ZONE_ALIGNOF(mv1::object)));
                o.via.array.ptr[0] = mv1::object(input.node_name_, o.zone);
                o.via.array.ptr[1] = mv1::object(input.port_, o.zone);
            }
        };

    }  // namespace adaptor
}
}  // namespace clmdep_msgpack
#endif

#endif //PKV_DATA_STRUCTURE_H
