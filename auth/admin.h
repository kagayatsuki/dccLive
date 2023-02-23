/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_ADMIN_H
#define DCCLIVE_ADMIN_H

#include "token.h"

#include <ctime>

class Admin: public Token{
private:
    std::string token_str;
public:
    time_t lastStep;
    Admin(char *_uid, char *_name, char *_time, uint32_t lifetime, std::string &token_s);
    bool StepLife(int16_t delta) override;
    // 对于管理员身份，支持更新Token生命时间
    void ResetLife(uint32_t time);
    std::string &GetTokenStr();
};


#endif //DCCLIVE_ADMIN_H
