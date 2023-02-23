/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_ROUTER_H
#define DCCLIVE_ROUTER_H

#include "../proto.h"


class RequestRouter {
private:
    ProtocolHeader &header;
    Response &resp;
    std::string realPath;
    std::unordered_map<std::string, std::string> args;
public:
    RequestRouter(ProtocolHeader &request, Response &response);
    ProtocolHeader &GetRequest();
    Response &GetResponse();
    std::string &GetPath();
    std::string &GetArg(std::string &key);
    std::string &GetArg(const char *key);
    std::unordered_map<std::string, std::string> &GetArgs();
};

typedef int(*RouteProcess)(RequestRouter &router);

extern void error_page_404(ProtocolHeader &request, Response &response);
extern void server_response(Response &response);
extern void process_route(ProtocolHeader &request, Response &response, RouteProcess router);

extern int route_console(RequestRouter &router);

#endif //DCCLIVE_ROUTER_H
