/*
 * Copyright (c) shinsya.G 
 */
#include "proto.h"
#include "../utils/string.h"

#include <cstring>


ProtocolPayload::ProtocolPayload(const char *buf, size_t data_size) :
        data(new char[std::max<size_t>(data_size + 1, CONTENT_LENGTH_MAX + 1)]),
        size(std::max<size_t>(data_size, CONTENT_LENGTH_MAX)) {
    memcpy(data, buf, size);
    data[size] = '\0';
}

ProtocolPayload::~ProtocolPayload() {
    delete[] this->data;
}

ProtocolPayload::ProtocolPayload() {
    this->data = nullptr;
    this->size = 0;
}

const char *ProtocolPayload::GetData() {
    return data;
}

size_t ProtocolPayload::GetSize() {
    return size;
}

std::shared_ptr<ProtocolPayload> ProtocolHeader::GetPayload() {
    return this->payload;
}

ProtocolHeader::ProtocolHeader(const char *buf, size_t size) : method(UNSUPPORTED) {
    if (size < 17)
        return;

    size_t i;   // 原始数据读取游标
    // 请求方式
    if (strstr(buf, "GET ") == buf) {
        method = GET;
        i = 4;
    } else if (strstr(buf, "POST ") == buf) {
        method = POST;
        i = 5;
    } else {
        path[0] = '\0';
        version[0] = '\0';
        return;
    }

    // 请求路径
    char path_buf[HEADER_PATH_MAX + 1];
    int j = 0, x = 0, last_pre = 0;
    for (; buf[i] != ' ' && buf[i] != '\0' && j < HEADER_PATH_MAX; i++, j++)
        path_buf[j] = buf[i];
    path_buf[j] = '\0';
    // 路径后处理(防止xss攻击, 类似 '/api/../../a' 将处理为 '/a')
    j = 0;
    if (strstr(path_buf, "../") == path_buf) {
        j = 2;
    }
    for (; path_buf[j];) {
        if (path_buf[j] != '/') {
            path_buf[x++] = path_buf[j++];
            continue;
        }
        if (path_buf[j + 1] == '.' && path_buf[j + 2] == '.' && path_buf[j + 3] == '/') {
            j += 3;
            x = last_pre;
            // 向前寻找更高一级目录位置
            int m = x - 1;
            for (; m > 0; m--) {
                if (path_buf[m] == '/') break;
            }
            last_pre = (m > 0) ? m : 0;
        } else {
            path_buf[last_pre = x] = '/';
            x++;
            j++;
        }
    }
    i += 1;
    path_buf[x] = '\0';
    strcpy(this->path, path_buf);
    // 协议版本
    if (strstr(buf + i, "HTTP/") == buf + i) {
        i += 5;
        for (int m = 0; buf[i] && buf[i] != '\r' && m < 3; m++, i++)
            this->version[m] = buf[i];
        if (buf[i] != '\r' && buf[i + 1] != '\n') {
            this->method = UNSUPPORTED;
            return;
        }
    } else {
        this->method = UNSUPPORTED;
        return;
    }
    // header携带键值
    char opt_buf[HEADER_KEY_MAX + HEADER_VALUE_MAX + 2];
    size_t opt_pos = 0, sep_pos = 0;
    for (i += 2; i < size; i++) {
        // 结算一条option
        if (buf[i] == '\r' && buf[i + 1] == '\n') {
            if (sep_pos == 0 || (opt_pos - sep_pos < 2)) {
                sep_pos = 0;
                opt_pos = 0;
                i++;
                continue;
            }
            for (int k = 0; k < sep_pos; k++)
                opt_buf[k] = (char) tolower(opt_buf[k]);
            std::string key(opt_buf, sep_pos);
            std::string value(opt_buf + sep_pos + 2, opt_pos - sep_pos - 2);
            this->options.insert({key, value});
            sep_pos = 0;
            opt_pos = 0;
            if (buf[i + 2] == '\r' && buf[i + 3] == '\n') {
                i += 4;
                break;
            } else {
                i++;
                continue;
            }
        }
        // 键:值分割位置
        if (buf[i] == ':' && buf[i + 1] == ' ' && sep_pos == 0) {
            sep_pos = opt_pos;
        }
        opt_buf[opt_pos++] = buf[i];
    }
    // 解析携带cookie
    parseCookie();

    // 原始payload数据(如果是POST)
    if (this->method != POST)
        return;
    try {
        std::string &content_len_str = this->options.at("content-length");
        char *str_end;
        long content_len = strtol(content_len_str.c_str(), &str_end, 10);
        // 若content-length不是合法数字或者与实际数据剩余长度不符，则不继续
        if (*str_end != '\0' || content_len != size - i)
            return;
        auto payload_ptr = new ProtocolPayload(buf + i, size - i);
        this->payload.reset(payload_ptr);
    } catch (std::out_of_range &ex) {
        return;
    }
}

ProtocolRequestMethod ProtocolHeader::GetMethod() {
    return this->method;
}

size_t ProtocolHeader::GetPath(char *buf, size_t buf_max) {
    return str_copy(buf, this->path, buf_max);
}

size_t ProtocolHeader::GetVersion(char *buf, size_t buf_max) {
    return str_copy(buf, this->version, buf_max);
}

std::unordered_map<std::string, std::string> &ProtocolHeader::GetOptions() {
    return this->options;
}

std::string ProtocolHeader::GetPath() {
    std::string tmp(this->path);
    return tmp;
}

void ProtocolHeader::parseCookie() {
    const char *cookie_raw;
    try {
        std::string &tmp = options.at("cookie");
        cookie_raw = tmp.c_str();
    } catch (std::out_of_range &ex) {
        return;
    }
    char item[4096];
    size_t item_len, key_len;
    while (*cookie_raw) {
        for (item_len = 0, key_len = 0;
             *cookie_raw && (*cookie_raw != '=') && (*cookie_raw != ';' && *(cookie_raw + 1) != ' ') && item_len < 4096;
             item_len++, key_len++, cookie_raw++) {
            item[item_len] = *cookie_raw;
        }
        if (key_len == 0 && *cookie_raw && *cookie_raw == ';' && *(cookie_raw + 1) == ' ') {
            cookie_raw += 2;
            continue;
        }
        cookie_raw += !!*cookie_raw;
        for (; *cookie_raw && (*cookie_raw != ';' && *(cookie_raw + 1) != ' ') &&
               item_len < 4096; item_len++, cookie_raw++) {
            item[item_len] = *cookie_raw;
        }
        cookies.insert({std::string(item, key_len), std::string(item + key_len, item_len - key_len)});
        if (*cookie_raw && *(cookie_raw + 1))
            cookie_raw += 2;
        else
            cookie_raw += !!*cookie_raw ;
    }
}

std::unordered_map<std::string, std::string> &ProtocolHeader::GetCookies() {
    return cookies;
}

char toHex(unsigned char x) {
    return (char)((x > 9) ? (x + 55) : (x + 48));
}

char fromHex(char x) {
    char y = 0;
    if (x >= 'A' && x <= 'Z')
        y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z')
        y = x - 'a' + 10;
    else if (x >= '0' && x <= '9')
        y = x - '0';
    return y;
}

std::string URIEncodedPayload::UrlEncode(const char *origin) {
    std::string str;
    if (!origin)
        return str;
    unsigned char ch;
    for (; (ch = *origin); origin++) {
        if (std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~' || ch == ' ')
            str += ch;
        else {
            str += '%';
            str += toHex(ch >> 4);
            str += toHex(ch % 16);
        }
    }
    return str;
}

std::string URIEncodedPayload::UrlDecode(const std::string &origin) {
    std::string str;
    size_t len = origin.length();
    const char *cstr = origin.c_str();
    for (size_t i = 0; i < len; i++) {
        if (cstr[i] == '+')
            str += ' ';
        else if (cstr[i] == '%') {
            if (!(i + 2 < len))
                break;
            str += (char)(fromHex(cstr[i + 1]) * 16 + fromHex(cstr[i + 2]));
            i += 2;
        } else
            str += cstr[i];
    }
    return str;
}

std::string URIEncodedPayload::Get(const std::string &key) {
    std::string tmp;
    try {
        return form.at(key);
    } catch (std::out_of_range &ex) {
        return tmp;
    }
}

URIEncodedPayload::URIEncodedPayload(ProtocolPayload &payload) {
    parseForm(payload.GetData(), payload.GetSize());
}

void URIEncodedPayload::parseForm(const char *raw, size_t size) {
    const char *ptr = raw;
    size_t sep, len, count = 0;
    while (*ptr && count < size) {
        for (sep = 0, len = 0; ptr[len] && (ptr[len] != '=') && (ptr[len] != '&') && count < size; sep++, len++, count++);
        if (!len)
            break;
        for (; ptr[len] && (ptr[len] != '&') && count < size; len++, count++);
        std::string key(ptr, sep);
        if (len == sep) {
            form.insert({UrlDecode(key), std::string("")});
//            printf("value: [empty]\n");
        } else {
            std::string value(ptr + sep + 1, len - sep - 1);
            form.insert({UrlDecode(key), UrlDecode(value)});
//            printf("value: %s\n", value.c_str());
        }
        count += !!ptr[len];
        ptr += len + (!!ptr[len]);
    }
}

std::unordered_map<std::string, std::string> &URIEncodedPayload::Gets() {
    return form;
}

URIEncodedPayload::URIEncodedPayload(std::shared_ptr<ProtocolPayload> &payload) {
    parseForm(payload->GetData(), payload->GetSize());
}

