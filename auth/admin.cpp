/*
 * Copyright (c) shinsya.G 
 */

#include "admin.h"


Admin::Admin
(char *_uid, char *_name, char *_time, uint32_t lifetime, std::string &token_s
): Token(_uid, _name, _time), token_str(token_s) {
    this->life = lifetime;
    this->lastStep = ::time(nullptr);
}

bool Admin::StepLife(int16_t delta) {
    if (delta > 0) {
        this->life += delta;
        return true;
    }
    if ((-delta) > this->life) {
        this->life = 0;
        return false;
    }
    this->life += delta;
    return true;
}

void Admin::ResetLife(uint32_t time) {
    this->life = time;
}

std::string &Admin::GetTokenStr() {
    return this->token_str;
}
