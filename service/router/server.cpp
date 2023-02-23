/*
 * Copyright (c) shinsya.G 
 */
#include "router.h"

#include <string>

void server_response(Response &response) {
    response.SetOption("Server", "dccLive");
}

void process_route(ProtocolHeader &request, Response &response, RouteProcess router) {
    server_response(response);
    RequestRouter route(request, response);
    if (router && !router(route))
        return;
    error_page_404(request, response);
}

RequestRouter::RequestRouter(ProtocolHeader &request, Response &response) : header(request), resp(response) {
    // 取URI参数前路径
    std::string path = request.GetPath();
    size_t pos_arg = path.find('?');
    if (pos_arg == std::string::npos) {
        realPath = path;
        return;
    }
    // 不含参数路径
    realPath = std::string(path, 0, pos_arg);
    // 识别参数表
    const char *args_raw = path.c_str() + pos_arg + 1;
    char key_buf[URL_PARAM_KEY_MAX + 1], value_buf[URL_PARAM_VALUE_MAX + 1];
    uint32_t k_off, v_off;
    while (*args_raw) {
        // key=value : key
        for (k_off = 0; *args_raw && (*args_raw != '=') && (*args_raw != '&') && (k_off < URL_PARAM_KEY_MAX); k_off++, args_raw++) {
            key_buf[k_off] = *args_raw;
        }
        key_buf[k_off] = '\0';
        // key长度超长(URL_PARAM_KEY_MAX)被截断情况
        if (k_off == URL_PARAM_KEY_MAX && *args_raw != '&' && *args_raw != '=' && !*args_raw) {
            for(; *args_raw && (*args_raw != '=') && (*args_raw != '&'); args_raw++);
        }
        args_raw += !!*args_raw;  // 跳过分割符(若字符不为结束符)
        if (k_off == 0) // value可为空，key不可为空
            continue;
        // key=value : value
        for (v_off = 0; *args_raw && (*args_raw != '&') && (v_off < URL_PARAM_VALUE_MAX); v_off++, args_raw++) {
            value_buf[v_off] = *args_raw;
        }
        value_buf[v_off] = '\0';
        args_raw += !!*args_raw;
        args.insert({key_buf, value_buf});
    }
}

ProtocolHeader &RequestRouter::GetRequest() {
    return header;
}

Response &RequestRouter::GetResponse() {
    return resp;
}

std::string &RequestRouter::GetPath() {
    return realPath;
}

std::string &RequestRouter::GetArg(std::string &key) {
    return args[key];
}

std::string &RequestRouter::GetArg(const char *key) {
    return args[key];
}

std::unordered_map<std::string, std::string> &RequestRouter::GetArgs() {
    return args;
}
