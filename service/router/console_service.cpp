/*
 * Copyright (c) shinsya.G 
 */
#include "router.h"
#include "../../auth/admin.h"
#include "../service.h"

#include <fcntl.h>
#include <unistd.h>

char file_buffer[RESPONSE_PAYLOAD_MAX];

extern Admin *tryCompareAuth(const char *name, const char *pwd);
extern Admin *getAuthSession(const std::string &sess_id);
extern void changePassword(std::string &name, const char *new_pass);

int api_login(RequestRouter &router);
Admin *api_auth_check(RequestRouter &);
int api_change_pwd(RequestRouter &router, Admin *auth);
int api_web_auth_check(RequestRouter &router);

const char *mime_compare(const std::string &suffix);

int route_console_api(RequestRouter &router) {
    Response &response = router.GetResponse();
    std::string url = router.GetPath().substr(4);
    response.SetOption("Content-Type", mime_type_json);
    if (url == "/login")
        return api_login(router);
    // 匹配其它api请求
    // 登录态验证
    Admin *auth = api_auth_check(router);
    if (!auth)
        return 0;
    // 继续匹配
    if (url == "/changepwd")
        return api_change_pwd(router, auth);
    if (url == "/authcheck")
        return api_web_auth_check(router);
    return 0;
}

int api_web_auth_check(RequestRouter &router) {
    const std::string authorized = R"({"code":"success"})";
    router.GetResponse().AppendPayload(authorized);
    return 0;
}

Admin *api_auth_check(RequestRouter &router) {
    const std::string unauthorized = R"({"code":"fail","msg":"unauthorized"})";
//    Debug
//    for (auto &i : router.GetRequest().GetCookies()) {
//        printf("%s:%s\n", i.first.c_str(), i.second.c_str());
//    }
    try {
        std::string &token = router.GetRequest().GetCookies().at("console_session");
        Admin *auth = getAuthSession(token);
        if (auth) {
            auth->ResetLife(DEFAULT_SESSION_EXPIRE);
            return auth;
        }
        throw std::out_of_range("");
    } catch (std::out_of_range &) {
        router.GetResponse().KeepAlive(false, true);
        router.GetResponse().AppendPayload(unauthorized);
        return nullptr;
    }
}

// 尝试登录验证请求，仅限POST
int api_login(RequestRouter &router) {
    std::string name;
    Response &response = router.GetResponse();
    if (router.GetRequest().GetMethod() != POST)
        return 1;   // GET 404
    auto raw = router.GetRequest().GetPayload();
    URIEncodedPayload payload(raw);
    auto &form = payload.Gets();
    try {
        name = form.at("user");
    } catch (std::out_of_range &ex) {
        const std::string empty_username = R"({"code":"fail","msg":"empty username"})";
        response.AppendPayload(empty_username);
        return 0;
    }
    std::string pwd;
    try {
        pwd = form.at("pwd");
    } catch (std::out_of_range &ex) {
        const std::string empty_pwd = R"({"code":"fail","msg":"empty password"})";
        response.AppendPayload(empty_pwd);
        return 0;
    }
    Admin *sess = tryCompareAuth(name.c_str(), pwd.c_str());
    if (!sess) {
        const std::string not_matched = R"({"code":"fail","msg":"unverified"})";
        response.AppendPayload(not_matched);
        return 0;
    }
    std::string ret = R"({"code":"success","data":")";
    ret.append(sess->GetTokenStr());
    ret.append("\"}");
    response.AppendPayload(ret);
    return 0;
}

int api_change_pwd(RequestRouter &router, Admin *auth) {
    // TODO: 修改密码
    return 0;
}

// 静态资源请求，仅GET方法
int route_console_web(RequestRouter &router) {
    if (router.GetRequest().GetMethod() != GET)
        return 1;   // POST 404
    std::string path("./web");
    path.append(router.GetPath());
    int fd = open(path.c_str(), O_RDONLY);
    if (fd <= 0)
        return 1;
    size_t data_len = read(fd, file_buffer, RESPONSE_PAYLOAD_MAX);
    Response &response = router.GetResponse();
    response.KeepAlive(false, true);
    if (data_len == 0) {
        response.SetMsg(&response_forbidden);
        close(fd);
        return 0;
    }
    response.AppendPayload(file_buffer, data_len);
    auto &options = router.GetRequest().GetOptions();
    // 目前启用keep-alive会有bug
//    if (options["connection"] == "keep-alive")
//        response.KeepAlive(true, true);
    std::string suffix;
    size_t point_pos = path.rfind('.');
    if (point_pos != std::string::npos) {
        suffix = path.substr(point_pos + 1);
    }
    response.SetOption("Content-Type", mime_compare(suffix));
    close(fd);
    return 0;
}

const char *mime_compare(const std::string &suffix) {
    if (suffix == "png" || suffix == "jpg" || suffix == "bmp" || suffix == "jpeg" || suffix == "gif")
        return mime_type_image;
    else if (suffix == "svg")
        return mime_type_svg;
    else if (suffix == "js")
        return mime_type_js;
    else if (suffix == "json")
        return mime_type_json;
    else if (suffix == "css")
        return mime_type_css;
    else if (suffix == "woff2")
        return mime_type_woff2;
    return mime_type_html;
}