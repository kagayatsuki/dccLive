/*
 * Copyright (c) shinsya.G 
 */
#include "room.h"

#include <unordered_map>
#include <random>


const char RollAlphaBet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

std::unordered_map<std::string, LiveRoom *> roomMap;


LiveRoom::LiveRoom(const char *_title, const char *_note, const char *_applier, const char *_start_time)
:title(_title), note(_note), applier(_applier), start_time(_start_time), watched(0)
{
    rollId();
    rollKey();
    LogAdd("Room created");
}

void LiveRoom::rollKey() {
    char key_buf[18];
    srandom(time(nullptr) + 0xdd220b72);
    for (auto &i : key_buf) {
        i = RollAlphaBet[random() % 62];
    }
    this->key = std::string(key_buf, 18);
}

void LiveRoom::rollId() {
    char id_buf[10];
    srandom(time(nullptr) + 0xbb770d27);
    for (auto &i : id_buf) {
        i = RollAlphaBet[random() % 62];
    }
    this->id = std::string(id_buf, 10);
    try {       // 以防万一发生碰撞
        LiveRoom *tmp = roomMap.at(id);
        rollId();
    } catch (std::out_of_range &ex){
        roomMap[id] = this;
        return;
    }
}

void LiveRoom::ResetRoomMap() {
    for (auto &i : roomMap) {
        delete i.second;
    }
    roomMap.clear();
}

LiveRoom *LiveRoom::TryGetRoom(std::string &id) {
    try {
        auto t = roomMap.at(id);
        return t;
    } catch (std::out_of_range &ex) {}
    return nullptr;
}

std::string &LiveRoom::GetTitle() {
    return title;
}

std::string &LiveRoom::GetNote() {
    return note;
}

std::string &LiveRoom::GetApplier() {
    return applier;
}

size_t LiveRoom::GetWatched() const {
    return watched;
}

void LiveRoom::IncreaseWatched() {
    watched++;
}

std::string &LiveRoom::GetStart() {
    return start_time;
}

std::string &LiveRoom::GetID() {
    return id;
}

std::string &LiveRoom::GetKey() {
    return key;
}

LiveRoom *LiveRoom::TryAlloc(std::string &title, std::string &note, std::string &applier, std::string &start_time) {
    if (roomMap.size() >= LIVE_ROOM_MAX)
        return nullptr;
    auto room = new LiveRoom(title.c_str(), note.c_str(), applier.c_str(), start_time.c_str());
    return room;
}

LiveRoom::~LiveRoom() {
    roomMap.erase(id);
}

void LiveRoom::LogAdd(const std::string &msg) {
    if (log.size() >= LIVE_ROOM_LOG_MAX) {
        log.erase(log.begin());
    }
    char timestamp[22];
    time_t now = time(nullptr);
    struct tm *now_tm = localtime(&now);
    timestamp[sprintf(timestamp, "[%d/%d/%d %02d:%02d:%02d] ",
                      now_tm->tm_year + 1900,
                      now_tm->tm_mon + 1,
                      now_tm->tm_mday,
                      now_tm->tm_hour,
                      now_tm->tm_min,
                      now_tm->tm_sec)] = 0;
    std::string tmp(timestamp);
    tmp += msg;
    log.push_back(tmp);
}

size_t LiveRoom::LogRead(std::string &buf) {
    size_t count = 0;
    for (auto &msg: log) {
        buf += msg;
        buf += '\n';
        count++;
    }
    return count;
}

