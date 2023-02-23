/*
 * Copyright (c) shinsya.G 
 */
#include "proto.h"
#include "../utils/string.h"

#include <cstring>
#include <unistd.h>

ResponsePayload::~ResponsePayload() {
    for (auto block : buffer)
        delete[] block;
}

char *ResponsePayload::allocBlock() {
    if (totalSize + RESPONSE_PAYLOAD_BLOCK > RESPONSE_PAYLOAD_MAX)
        return nullptr;
    char *block = new char[RESPONSE_PAYLOAD_BLOCK];
    memset(block, 0, RESPONSE_PAYLOAD_BLOCK);
    lastBufferBlock = block;
    buffer.push_back(block);
    lastBufferCapacity = RESPONSE_PAYLOAD_BLOCK;
    return block;
}

ResponsePayload::ResponsePayload(): lastBufferBlock(nullptr), lastBufferCapacity(0), totalSize(0)
{}

ResponsePayload::ResponsePayload(void *src, size_t len): ResponsePayload() {
    Append(src, len);
}

size_t ResponsePayload::Append(void *src, size_t len) {
    if (!len || !src)
        return 0;
    char *buf = (char *)src;
    size_t writen = 0;
    size_t remain;
    // 复制源数据到链式缓冲区
    while ((remain = len - writen)) {
        if (lastBufferCapacity > 0) {
            size_t copy_size = std::min(remain, lastBufferCapacity);
            memcpy(lastBufferBlock + (RESPONSE_PAYLOAD_BLOCK - lastBufferCapacity), buf, copy_size);
            lastBufferCapacity -= copy_size;
            writen += copy_size;
            buf += copy_size;
            continue;
        }
        if (allocBlock())
            continue;
        break;
    }
    totalSize += writen;
    return writen;
}

size_t ResponsePayload::Length() const {
    return totalSize;
}

size_t ResponsePayload::Copy(const char *buf, size_t max_len) {
    if (!buf || !max_len)
        return 0;
    size_t remain = std::min(max_len, totalSize), writen = 0;
    auto iter = buffer.begin();
    // 复制链式缓冲区数据到指定连续缓冲区
    while (remain) {
        size_t currentSize = std::min<size_t>(remain, RESPONSE_PAYLOAD_BLOCK);
        memcpy((void *)(buf + writen), *iter, currentSize);
        remain -= currentSize;
        writen += currentSize;
        iter++;
    }
    return writen;
}

Response::Response(): message(&response_ok)
{}

void Response::SetMsg(const std::pair<const char *, size_t> *msg) {
    if (!msg)
        return;
    message = msg;
}

size_t Response::Write(int fd) {
    if (fd <= 0)
        return 0;
    size_t payload_size = payload.Length();
    // payload 长度
    if (payload_size)
        options["Content-Length"] = std::to_string(payload_size);
    size_t header_size = 13 + message->second + optionsLength();
    char *buf_ptr = new char[header_size + payload_size];
    std::shared_ptr<char> buf(buf_ptr);
    // 组合回复数据
    size_t offset = 9;
    // 头
    memcpy(buf.get(), "HTTP/1.1 ", 9);
    memcpy(buf.get() + 9, message->first, message->second);
    offset += message->second;
    buf.get()[offset ++] = '\r'; buf.get()[offset ++] = '\n';
    // Options
    for (auto &pair : options) {
        size_t key_len = pair.first.length(), value_len = pair.second.length();
        memcpy(buf.get() + offset, pair.first.c_str(), key_len);
        offset += key_len;
        buf.get()[offset ++] = ':'; buf.get()[offset ++] = ' ';
        memcpy(buf.get() + offset, pair.second.c_str(), value_len);
        offset += value_len;
        buf.get()[offset ++] = '\r'; buf.get()[offset ++] = '\n';
    }
    buf.get()[offset ++] = '\r'; buf.get()[offset ++] = '\n';
    // Payload
    payload.Copy(buf.get() + offset, payload_size);
    size_t write_length = 0;
    while (write_length != payload_size + header_size) {
        ssize_t cur = write(fd, buf.get() + write_length, header_size + payload_size - write_length);
        if (cur <= 0) {
            if (errno == EAGAIN)
                continue;
            break;
        }
        write_length += cur;
    }
    return write_length;
}

size_t Response::optionsLength() {
    size_t len = 0;
    for (auto &pair : options) {
        len += 4 + pair.first.length() + pair.second.length();
    }
    return len;
}

void Response::SetOption(std::string &key, std::string &value) {
    options.insert({key, value});
}

size_t Response::AppendPayload(void *buf, size_t len) {
    return payload.Append(buf, len);
}

bool Response::KeepAlive(bool on, bool set) {
    if (!set) {
        try {
            if (options.at("Connection") == "keep-alive")
                return true;
            return false;
        } catch (std::out_of_range &ex) {
            return false;
        }
    }
    if (on) {
        options["Connection"] = "keep-alive";
    } else {
        options["Connection"] = "close";
    }
    return on;
}

void Response::SetOption(const char *key, const char *value) {
    options.insert({key, value});
}

size_t Response::AppendPayload(const std::string &content) {
    return AppendPayload((void *)content.c_str(), content.length());
}
