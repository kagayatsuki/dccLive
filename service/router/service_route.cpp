/*
 * Copyright (c) shinsya.G 
 */
#include "router.h"
#include "../live/room.h"
#include "../../log/env.h"


int svc_publish(RequestRouter &router);
int svc_publish_done(RequestRouter &router);


// 对接nginx的RTMP事件，仅接收POST请求
int route_service(RequestRouter &router) {
#define SVC_ROUTE(a,b)  if (path == (a)) return b(router)
    if (router.GetRequest().GetMethod() != POST)
        return 1;
    std::string &path = router.GetPath();
    SVC_ROUTE("/publish", svc_publish);
    SVC_ROUTE("/publish_done", svc_publish_done);
    return 0;
#undef SVC_ROUTE
}

int svc_publish(RequestRouter &router) {
    auto raw = router.GetRequest().GetPayload();
    URIEncodedPayload form(raw);
    if (form.Get("call") != "publish")
        return 1;
    std::string id = form.Get("name");
    std::string token = form.Get("tk");
    if (id.empty() || token.empty())
        return 1;
    auto room = LiveRoom::TryGetRoom(id);
    if (!room || room->GetKey() != token)
        return 1;
    room->LogAdd("Live on");
    LOG2SVR("Live [%s](%s) is published", room->GetID().c_str(), room->GetTitle().c_str());
    router.GetResponse().AppendPayload("ok");
    return 0;
}

int svc_publish_done(RequestRouter &router) {
    auto raw = router.GetRequest().GetPayload();
    URIEncodedPayload form(raw);
    if (form.Get("call") != "publish_done")
        return 1;
    std::string id = form.Get("name");
    std::string token = form.Get("tk");
    if (id.empty() || token.empty())
        return 1;
    auto room = LiveRoom::TryGetRoom(id);
    if (!room || room->GetKey() != token)
        return 1;
    room->LogAdd("Live off");
    LOG2SVR("Live [%s](%s) publish done", room->GetID().c_str(), room->GetTitle().c_str());
    router.GetResponse().AppendPayload("ok");
    return 0;
}