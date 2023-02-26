/*
 * Copyright (c) shinsya.G 
 */
#include "router.h"
#include "../../auth/admin.h"
#include "../service.h"
#include "../live/handler.h"
#include "../../log/env.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

size_t console_access_count = 0;        // 控制台接口访问计数
char file_buffer[RESPONSE_PAYLOAD_MAX];

extern Admin *tryCompareAuth(const char *name, const char *pwd);
extern Admin *getAuthSession(const std::string &sess_id);
extern void changePassword(std::string &name, const char *new_pass);

int live_route(RequestRouter &router, Admin *auth);
int log_route(RequestRouter &router);

int api_login(RequestRouter &router);
Admin *api_auth_check(RequestRouter &);
int api_change_pwd(RequestRouter &router, Admin *auth);
int api_web_auth_check(RequestRouter &router);

const char *mime_compare(const std::string &suffix);

int route_console_api(RequestRouter &router) {
    Response &response = router.GetResponse();
    std::string url = router.GetPath().substr(4);
    response.SetOption("Content-Type", mime_type_json);
    console_access_count++;
    if (url == "/login")
        return api_login(router);
    // 匹配其它api请求
    // 登录态验证
    Admin *auth = api_auth_check(router);
    if (!auth)
        return 0;
    response.KeepAlive(true, true);
    // 继续匹配
    if (url == "/changepwd")
        return api_change_pwd(router, auth);
    if (url == "/authcheck")
        return api_web_auth_check(router);
    if (url.find("/live/") == 0)
        return live_route(router, auth);
    if (url.find("/log/") == 0)
        return log_route(router);
    return 1;
}

int live_route(RequestRouter &router, Admin *auth) {
    std::string url = router.GetPath().substr(10);
#define LIVE_ROUTE(a,b) if(url == (a))return b(router, auth);
    LIVE_ROUTE("info", live_info);
    LIVE_ROUTE("list", live_list);
    LIVE_ROUTE("new", live_new);
    LIVE_ROUTE("delete", live_delete);
    LIVE_ROUTE("exportAccess", live_export_access);
    return 1;
#undef LIVE_ROUTE
}

void api_log_read(RequestRouter &router, const char *log_file) {
    auto &response = router.GetResponse();
    int fd = open(log_file, O_RDONLY);
    if (fd <= 0) {
        response.AppendPayload(R"({"code":"success","data":"log empty"})");
        return;
    }
    lseek(fd, -4096, SEEK_END);
    char buf[4097];
    size_t len = read(fd, buf, 4096);
    close(fd);
    buf[len] = 0;
    response.AppendPayload(URIEncodedPayload::UrlEncode(buf));
}

int log_route(RequestRouter &router) {
    std::string type = router.GetPath().substr(9);
#define LOG_ROUTE(a,b) if(type == (a)) {api_log_read(router, (b)); return 0;}
    LOG_ROUTE("admin", LOG_FILE_ADMIN);
    LOG_ROUTE("svr", LOG_FILE_LIVE_SVR);
    LOG_ROUTE("cli", LOG_FILE_LIVE_CLI);
    LOG_ROUTE("sys", LOG_FILE_SYSTEM);
    return 1;
#undef LOG_ROUTE
}

int api_web_auth_check(RequestRouter &router) {
    const std::string authorized = R"({"code":"success"})";
    router.GetResponse().AppendPayload(authorized);
    return 0;
}

Admin *api_auth_check(RequestRouter &router) {
    const std::string unauthorized = R"({"code":"fail","msg":"unauthorized"})";
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
    Response &response = router.GetResponse();
    response.KeepAlive(true, true);
    if (router.GetRequest().GetMethod() != GET)
        return 1;   // POST 404
    std::string path("./web");
    std::string &access_path = router.GetPath();
    if (access_path == "/")
        path.append("/index.html");
    else if (access_path.empty() || access_path[access_path.length() - 1] == '/')
        return 1;
    else
        path.append(access_path);

    // 避免对目录进行open
    struct stat f_stat{};
    stat(path.c_str(), &f_stat);
    if (!S_ISREG(f_stat.st_mode))
        return 1;

    // 尝试读取并发送文件数据
    int fd = open(path.c_str(), O_RDONLY);
    if (fd <= 0)
        return 1;
    size_t data_len = read(fd, file_buffer, RESPONSE_PAYLOAD_MAX);
    if (data_len == 0) {
        response.SetMsg(&response_forbidden);
        return 0;
    } else if (data_len > RESPONSE_PAYLOAD_MAX) {       // 文件过大
        response.SetMsg(&response_server_error);
        response.AppendPayload("Resource too large");
        return 0;
    }
    response.AppendPayload(file_buffer, data_len);
    std::string suffix;
    size_t point_pos = path.rfind('.');
    if (point_pos != std::string::npos) {
        suffix = path.substr(point_pos + 1);
    }
    response.SetOption("Content-Type", mime_compare(suffix));
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
