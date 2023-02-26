/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_ROOM_H
#define DCCLIVE_ROOM_H

#include <unistd.h>
#include <string>
#include <vector>

#define LIVE_ROOM_MAX           6       // 最大允许的房间数
#define LIVE_ROOM_LOG_MAX       300     // ROOM LOG记录最大条数

extern size_t live_api_access_count;    // 直播间接口访问计数

class LiveRoom {
private:
    std::string title;
    std::string note;
    std::string applier;
    size_t watched;
    std::string start_time;
    std::string id;
    std::string key;
    std::vector<std::string> log;
private:
    void rollKey();
    void rollId();
public:
    LiveRoom(const char *_title, const char *_note, const char *_applier, const char *_start_time);
    ~LiveRoom();
    std::string &GetTitle();
    std::string &GetNote();
    std::string &GetApplier();
    size_t GetWatched() const;
    void IncreaseWatched();
    std::string &GetStart();
    std::string &GetID();
    std::string &GetKey();
    void LogAdd(const std::string &msg);
    size_t LogRead(std::string &buf);
public:
    static LiveRoom *TryGetRoom(std::string &id);
    static LiveRoom *TryAlloc(std::string &title, std::string &note, std::string &applier, std::string &start_time);
    static void ResetRoomMap();
};


#endif //DCCLIVE_ROOM_H
