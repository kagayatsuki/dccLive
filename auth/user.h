/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_USER_HPP
#define DCCLIVE_USER_HPP

#include "token.h"

class User: public Token {
private:
    std::shared_ptr<std::string> live_uuid;
public:
    User(char *_uid, char *_name, char *_time, char *_uuid);
    bool StepLife(int16_t delta) override;
    std::shared_ptr<std::string> &GetUUID();
};


#endif //DCCLIVE_USER_HPP
