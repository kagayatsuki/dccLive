/*
 * Copyright (c) shinsya.G 
 */
#include "service.h"
#include "pool.h"
#include "proto.h"
#include "../auth/admin.h"
#include "env.h"
#include "../utils/socket.h"
#include "../utils/string.h"
#include "../utils/sha1.h"
#include "router/router.h"

#include <unordered_map>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>

extern int process_service(void *);

#ifndef DEFAULT_ROOT_PASSWORD
#define DEFAULT_ROOT_PASSWORD           "dcclivetopdev"
#endif
#ifndef DEFAULT_CLIENT_BUFFER_SIZE
#define DEFAULT_CLIENT_BUFFER_SIZE      (4096 + CONTENT_LENGTH_MAX)     // 默认缓冲区大小 4KB(header) + PayloadMAX(payload)
#endif

uint32_t session_expire = DEFAULT_SESSION_EXPIRE;
uint32_t client_buffer_size = DEFAULT_CLIENT_BUFFER_SIZE;

typedef struct AdminSecret {
    uint16_t uid;
    char passwd[32];      // 经过综合考虑，认为管理员密码增加保密是没有必要的，除非能直接登录到服务器，对于这种情况，保密也无济于事
} AdminSecret;

Pool *connPool;

std::unordered_map<std::string, AdminSecret *> authorityTable;  // 身份验证表
std::unordered_map<std::string, Admin *> sessionTable;          // session表

time_t lastDecreaseSession = 0;
char *client_buffer = nullptr;

void loadUserTable();
void changePassword(std::string &name, const char *new_pass);
void rewriteUserTable();

int connectionProcess(PoolEvent *e);
int dataInProcess(PoolEvent *e);

void decreaseSessionLife();

int init_console(Pool *pool) {
    connPool = pool;
    client_buffer = new char[client_buffer_size];
    loadUserTable();
    int fd = createNonBlockSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        LOG2SVR("open console socket failed");
        return -1;
    }
    sockaddr_in addr{};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(CONSOLE_PORT);
    addr.sin_family = AF_INET;
    if (bind(fd, (sockaddr *)&addr, sizeof(addr)) != 0) {
        LOG2SVR("Cannot bind console socket to port %d", (int)CONSOLE_PORT);
        return -1;
    }
    if (listen(fd, CONSOLE_BACKLOG) != 0) {
        LOG2SVR("Cannot listening on console socket");
        return -1;
    }
    if (pool->Reg(fd, connectionProcess, nullptr)) {
        LOG2SVR("Pool exception while register console socket");
        return -1;
    }

    return 0;
}

void shut_console(Pool *) {
    delete[] client_buffer;
    LOG2SYS("console shutdown");
}

int process_console(void *d) {
    decreaseSessionLife();
    setNextAction(process_service, d);
    return 0;
}

void loadUserTable() {
    int fd = open("./admin", O_RDWR);
    char buf[66] = {};
    memset(buf, 0, 66);
    if (fd == -1) {
        // 生成默认root用户
        fd = open("./admin", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        auto default_root = new AdminSecret;
        default_root->uid = 0;
        memset(default_root->passwd, 0, 32);
        memcpy(buf + 2, "root", 4);
        strcpy(default_root->passwd, DEFAULT_ROOT_PASSWORD);
        strcpy(buf + 34, DEFAULT_ROOT_PASSWORD);
        authorityTable.insert({std::string("root"), default_root});
        if (fd == -1) {
            LOG2SYS("WARN: Cannot open authority record file './admin'");
            return;
        }
        LOG2SYS("There was no user record. default root user password is '%s'", DEFAULT_ROOT_PASSWORD);
        if (write(fd, buf, 66) != 66)
            LOG2SYS("WARN: Cannot write default root user to './admin'");
    }
    while (read(fd, buf, 66) == 66) {
        auto user = new AdminSecret;
        memcpy(user->passwd, buf + 34, 32);
        user->uid = *((uint16_t *)buf);
        buf[34] = 0;
        std::string name(buf + 2);
        authorityTable.insert({name, user});
        LOG2SYS("User: %s", name.c_str());
    }
    close(fd);
}

void decreaseSessionLife() {
    time_t currentTime = time(nullptr);
    // 至少5秒才进行一次session维护
    if (currentTime - lastDecreaseSession < 5)
        return;
    for (auto it = sessionTable.begin(); it != sessionTable.end();) {
        if ((*it).second->StepLife((int16_t)((*it).second->lastStep - currentTime))) {
            (*it).second->lastStep = currentTime;
            it++;
            continue;
        }
        LOG2ADM("admin session expired (%s:%s)", (*it).first.c_str(), (*it).second->GetUid()->c_str());
        it = sessionTable.erase(it);
    }
    lastDecreaseSession = currentTime;
}

Admin *tryCompareAuth(const char *name, const char *pwd) {
    std::string username(name);
    // 用户存在
    AdminSecret *secret;
    try {
        secret = authorityTable.at(name);
    } catch (std::out_of_range &ex) {
        return nullptr;
    }
    LOG2CLI("Try to login user [%s](%x) by pwd(%s)", name, secret, pwd);
    if (!secret)
        return nullptr;
    // 密码对比
    if (strcmp(secret->passwd, pwd) != 0)
        return nullptr;
    // 生成session数据
    char uid[8];
    memset(uid, 0, 8);
    uint2string(secret->uid, uid, 7);
    // 登录时间
    char current_time[32];
    memset(current_time, 0, 32);
    time_t cur_sec = time(nullptr);
    tm *datetime = localtime(&cur_sec);
    sprintf(current_time, "%d-%d-%d %d:%d:%d", datetime->tm_year + 1900, datetime->tm_mon + 1, datetime->tm_mday,
            datetime->tm_hour, datetime->tm_min, datetime->tm_sec);
    // 生成session id
    // sha1(name + uid + time) => session_id
    char sid[40];
    char origin[42], hash_buf[20];
    size_t name_len = username.length(), uid_len = sizeof(secret->uid), time_len = sizeof(time_t);
    hash_sha1_ctx sha1_ctx;
    memset(origin, 0, 42);
    memcpy(origin, username.c_str(), name_len);
    memcpy(origin + name_len, &(secret->uid), uid_len);
    memcpy(origin + name_len + uid_len, &cur_sec, sizeof(time_t));
    sha1_init(&sha1_ctx);
    sha1_update(&sha1_ctx, origin, name_len + uid_len + time_len);
    sha1_final(&sha1_ctx, (uint8_t *)hash_buf);
    sha1_string((uint8_t *)hash_buf, sid, false);
    // session生效
    std::string session_id(sid, 40);
    auto *token = new Admin(uid, (char *)name, current_time, session_expire, session_id);
    sessionTable.insert({session_id, token});
    LOG2ADM("User[%s] login success. Token: %s", name, session_id.c_str());
    return token;
}

Admin *getAuthSession(const std::string &sess_id) {
    try {
        Admin *auth = sessionTable.at(sess_id);
        return auth;
    } catch (std::out_of_range &ex) {
        return nullptr;
    }
}

void changePassword(std::string &name, const char *new_pass) {
    AdminSecret *secret = authorityTable[name];
    if (!secret)
        return;
    memset(secret->passwd, 0, 32);
    size_t len = strlen(new_pass);
    memcpy(secret->passwd, new_pass, (len > 32) ? 32 : len);
    LOG2ADM("user(%s)'s password has been changed", name.c_str());
    rewriteUserTable();
}

void rewriteUserTable() {
    int fd = open("./admin", O_RDWR | O_TRUNC);
    if (fd == -1) {
        LOG2ADM("Cannot open authority table file. Rewrite user table failed");
        return;
    }
    char buf[66];
    for (auto &pair: authorityTable) {
        memcpy(buf, &(pair.second->uid), 2);
        strcpy(buf + 2, pair.first.c_str());
        memcpy(buf + 34, pair.second->passwd, 32);
        write(fd, buf, 66);
    }
    close(fd);
    LOG2ADM("Authority table rewrite");
}

int connectionProcess(PoolEvent *e) {
    sockaddr_in cli_addr{};
    socklen_t cli_len;
    while (true) {
        int cli = accept(e->fd, (sockaddr *)&cli_addr, &cli_len);
        if (cli > 0) {
            setNonBlock(cli);
            connPool->Reg(cli, dataInProcess, nullptr);
            continue;
        }
        break;
    }
    return 0;
}

int dataInProcess(PoolEvent *e) {
    memset(client_buffer, 0, DEFAULT_CLIENT_BUFFER_SIZE);
    size_t len = read(e->fd, client_buffer, DEFAULT_CLIENT_BUFFER_SIZE);
    char tmp;
    if (len == 0 || (len == -1 && errno != EAGAIN))
        return 1;
    bool too_large = false;
    while (read(e->fd, &tmp, 1) == 1) {too_large = true;}
    ProtocolHeader request(client_buffer, len);
    Response res;
    std::string path_str = request.GetPath();
    // 未知请求
    if (request.GetMethod() == UNSUPPORTED) {
        res.SetMsg(&response_not_implement);
        res.Write(e->fd);
        LOG2CLI("Access %s %s", path_str.c_str(), "Unsupported request");
        return 1;
    }
    // 请求过大
    if (too_large) {
        res.SetMsg(&response_tool_large);
        res.Write(e->fd);
        LOG2CLI("Access %s %s", path_str.c_str(), "Request tool large");
        return 1;
    }
    // 路由
    process_route(request, res, route_console);
    size_t write_len = res.Write(e->fd);
    if (write_len <= 0) {
        LOG2CLI("Access %s %s", path_str.c_str(), "Written failed");
        return 1;
    }
    if (!res.KeepAlive(false, false)) {
        LOG2CLI("Access %s %s", path_str.c_str(), "Done");
        return 1;
    }
    LOG2CLI("Access %s %s", path_str.c_str(), "Keep-alive");
    return 0;
}