/*
 * Copyright (c) shinsya.G 
 */
#include "router.h"

// route '/api/'
extern int route_console_api(RequestRouter &router);
// route '/
extern int route_console_web(RequestRouter &router);


int route_console(RequestRouter &router) {
    std::string &path = router.GetPath();
    if (path.find("/api/") == 0)
        return route_console_api(router);
    if (path.find('/') == 0)
        return route_console_web(router);
    return 1;
}