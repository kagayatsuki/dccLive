/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_PROTO_H
#define DCCLIVE_PROTO_H

#include <unordered_map>
#include <vector>
#include <memory>

#define HEADER_KEY_MAX              24      // header key 最长长度
#define HEADER_VALUE_MAX            256     // header value 最长长度
#define HEADER_PATH_MAX             256     // 请求路径最长
#define CONTENT_LENGTH_MAX          (1024 * 1024 * 5)      // 最大payload长度 (5MB)

#define URL_PARAM_KEY_MAX           64
#define URL_PARAM_VALUE_MAX         255

#define RESPONSE_PAYLOAD_MAX        (1024 * 1024 * 32)     // 最大响应payload长度 (32MB)
#define RESPONSE_PAYLOAD_BLOCK      (1024 * 1024)
#define RESPONSE_MSG_TYPE           const std::pair<const char *, size_t>

RESPONSE_MSG_TYPE response_not_support  = {"505 HTTP Version Not Supported", 30};
RESPONSE_MSG_TYPE response_not_implement= {"501 Not Implemented", 19};
RESPONSE_MSG_TYPE response_server_error = {"500 Internal Server Error", 25};
RESPONSE_MSG_TYPE response_tool_large   = {"413 Request Entity Too Large", 28};
RESPONSE_MSG_TYPE response_not_found    = {"404 Not Found", 13};
RESPONSE_MSG_TYPE response_forbidden    = {"403 Forbidden", 13};
RESPONSE_MSG_TYPE response_unauthorized = {"401 Unauthorized", 16};
RESPONSE_MSG_TYPE response_ok           = {"200 OK", 6};
RESPONSE_MSG_TYPE response_switch_proto = {"101 Switching Protocols", 23};

#define CONTENT_TYPE                const char *
extern CONTENT_TYPE      mime_type_html;
extern CONTENT_TYPE      mime_type_css;
extern CONTENT_TYPE      mime_type_js;
extern CONTENT_TYPE      mime_type_json;
extern CONTENT_TYPE      mime_type_image;
extern CONTENT_TYPE      mime_type_woff2;
extern CONTENT_TYPE      mime_type_svg;

enum ProtocolRequestMethod {
    GET,
    POST,
    UNSUPPORTED
};

class ProtocolPayload {
private:
    size_t size;
    char *data;
public:
    const char *GetData();
    size_t GetSize();
public:
    ProtocolPayload();
    ProtocolPayload(const char *buf, size_t data_size);
    ~ProtocolPayload();
};

class ProtocolHeader {
private:
    ProtocolRequestMethod method;
    char path[HEADER_PATH_MAX + 1]{};
    char version[4]{};
    std::unordered_map<std::string, std::string> options;
    std::unordered_map<std::string, std::string> cookies;
    std::shared_ptr<ProtocolPayload> payload;
private:
    void parseCookie();
public:
    ProtocolHeader(const char *buf, size_t size);
    std::shared_ptr<ProtocolPayload> GetPayload();
    ProtocolRequestMethod GetMethod();
    size_t GetPath(char *buf, size_t buf_max);
    std::string GetPath();
    size_t GetVersion(char *buf, size_t buf_max);
//    int GetOption(const char *key, char *buf, size_t buf_max);
    std::unordered_map<std::string, std::string> &GetOptions();
    std::unordered_map<std::string, std::string> &GetCookies();
};


class URIEncodedPayload: public ProtocolPayload {
private:
    std::unordered_map<std::string, std::string> form;
private:
    void parseForm(const char *raw, size_t size);
public:
    explicit URIEncodedPayload(ProtocolPayload &payload);
    explicit URIEncodedPayload(std::shared_ptr<ProtocolPayload> &payload);
    std::string Get(const std::string &key);
    std::unordered_map<std::string, std::string> &Gets();
public:
    static std::string UrlEncode(const char *origin);
    static std::string UrlDecode(const std::string &origin);
};


class ResponsePayload {
private:
    std::vector<char *> buffer;
    char *lastBufferBlock;
    size_t lastBufferCapacity;
    size_t totalSize;
private:
    char *allocBlock();
public:
    ~ResponsePayload();
    ResponsePayload();
    ResponsePayload(void *src, size_t len);
    size_t Append(void *src, size_t len);
    size_t Length() const;
    size_t Copy(const char *buf, size_t max_len);
};


class Response {
private:
    std::unordered_map<std::string, std::string> options;
    RESPONSE_MSG_TYPE *message;
    ResponsePayload payload;
private:
    size_t optionsLength();
public:
    Response();
    void SetMsg(RESPONSE_MSG_TYPE *msg);
    size_t Write(int fd);
    void SetOption(std::string &key, std::string &value);
    void SetOption(const char *key, const char *value);
    bool KeepAlive(bool on, bool set);
    size_t AppendPayload(void *buf, size_t len);
    size_t AppendPayload(const std::string &content);
};

#endif //DCCLIVE_PROTO_H
