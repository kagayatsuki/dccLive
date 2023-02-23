/*
 * Copyright (c) shinsya.G 
 */

#include "user.h"

bool User::StepLife(int16_t delta) {
    if (delta > 0) {
        this->life += delta;
        return true;
    }
    return false;
}

User::User
(char *_uid, char *_name, char *_time, char *_uuid
):Token(_uid, _name, _time),
live_uuid(std::make_shared<std::string>(_uuid)){
    this->life = 0;
}

std::shared_ptr<std::string> &User::GetUUID() {
    return this->live_uuid;
}
