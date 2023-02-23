/*
 * Copyright (c) shinsya.G 
 */
#include "router.h"

void error_page_404(ProtocolHeader &request, Response &response) {
    response.SetMsg(&response_not_found);
    std::string content("Service ");
    content.append(request.GetPath());
    content.append(" not found");
    response.AppendPayload((void *)content.c_str(), content.length());
    response.KeepAlive(false, true);
}