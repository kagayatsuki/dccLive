/*
 * Copyright (c) shinsya.G 
 */
#include "room.h"
#include "handler.h"
#include "env.h"

#include <unordered_map>

extern std::unordered_map<std::string, LiveRoom *> roomMap;
extern size_t console_access_count;

int live_info(RequestRouter &router, Admin *auth) {
    std::string builder = R"({"code":"success","data":{)";
    builder += R"("access_count":)";
    builder += std::to_string(console_access_count);
    builder += R"(,"room_count":)";
    builder += std::to_string(roomMap.size());
    builder += R"(,"room_max":)";
    builder += std::to_string(LIVE_ROOM_MAX);
    builder += R"(,"current_user":")";
    builder += *auth->GetName();
    builder += R"("}})";
    router.GetResponse().AppendPayload(builder);
    return 0;
}

int live_list(RequestRouter &router, Admin *auth) {
    auto &response = router.GetResponse();
    if (roomMap.empty()) {
        response.AppendPayload(R"({"code":"success","data":[]})");
        return 0;
    }
    std::string builder = R"({"code":"success","data":[)";
    int i = 0;
    for (auto &room : roomMap) {
        builder += '{';
        builder += R"("title":")";
        builder += URIEncodedPayload::UrlEncode(room.second->GetTitle().c_str());
        builder += R"(","note":")";
        builder += URIEncodedPayload::UrlEncode(room.second->GetNote().c_str());
        builder += R"(","applier_mark":")";
        builder += URIEncodedPayload::UrlEncode(room.second->GetApplier().c_str());
        builder += R"(","watched":)";
        builder += std::to_string(room.second->GetWatched());
        builder += R"(,"time_start":")";
        builder += URIEncodedPayload::UrlEncode(room.second->GetStart().c_str());
        builder += R"(","id":")";
        builder += room.second->GetID();
        builder += R"(","push_key":"tk=)";
        builder += URIEncodedPayload::UrlEncode(room.second->GetKey().c_str());
        builder += "&id=room";
        builder += URIEncodedPayload::UrlEncode(room.second->GetID().c_str());
        builder += "\"}";
        if (i != roomMap.size() - 1)
            builder += ',';
        i++;
    }
    builder += "]}";
    response.AppendPayload(builder);
    return 0;
}

int live_new(RequestRouter &router, Admin *auth) {
    const std::string formField[] = {"title", "dep", "start_time", "note"};
    auto raw = router.GetRequest().GetPayload();
    auto &response = router.GetResponse();
    URIEncodedPayload form(raw);
    auto &map = form.Gets();
    // 检查form字段
    for (auto &field : formField) {
        try {
            std::string &v = map.at(field);
            if (v.empty())
                throw std::out_of_range("");
        } catch (std::out_of_range &ex) {
            std::string error_msg = R"({"code":"fail","msg":"Invalid field ')";
            error_msg += field;
            error_msg += R"('"})";
            response.AppendPayload(error_msg);
            return 0;
        }
    }
    auto room = LiveRoom::TryAlloc(map["title"], map["note"], map["dep"], map["start_time"]);
    if (!room) {
        const std::string msg = R"({"code":"fail","msg":"room count over limitation"})";
        response.AppendPayload(msg);
        return 0;
    }
    LOG2ADM("[%s] create new room {name: %s; id: %s; key: %s}", auth->GetName()->c_str(),
            room->GetTitle().c_str(), room->GetID().c_str(), room->GetKey().c_str());
    const std::string msg = R"({"code":"success"})";
    response.AppendPayload(msg);
    return 0;
}

int live_delete(RequestRouter &router, Admin *auth) {
    auto &response = router.GetResponse();
    auto raw = router.GetRequest().GetPayload();
    auto form = URIEncodedPayload(raw);
    try {
        std::string &id = form.Gets().at("id");
        LiveRoom *room = LiveRoom::TryGetRoom(id);
        if (!room)
            throw std::out_of_range("");
        LOG2ADM("[%s] delete room {title: %s; id: %s; key: %s}", auth->GetName()->c_str(),
                room->GetTitle().c_str(), room->GetID().c_str(), room->GetKey().c_str());
        delete room;
        response.AppendPayload(R"({"code":"success"})");
    } catch (std::out_of_range &ex) {
        response.AppendPayload(R"({"code":"fail","msg":"room unavailable"})");
    }
    return 0;
}

int live_export_access(RequestRouter &router, Admin *auth) {
    auto &response = router.GetResponse();
    auto &params = router.GetArgs();
    try {
        std::string &id = params.at("id");
        LiveRoom *room = LiveRoom::TryGetRoom(id);
        if (!room)
            throw std::out_of_range("");
        std::string log;
        room->LogRead(log);
        response.AppendPayload(log);
    } catch (std::out_of_range &ex) {
        response.AppendPayload("room unavailable");
    }
    return 0;
}