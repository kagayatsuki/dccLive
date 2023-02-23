/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_TOKEN_H
#define DCCLIVE_TOKEN_H

#include <string>
#include <memory>

class Token {
private:
    std::shared_ptr<std::string> uid;
    std::shared_ptr<std::string> name;
    std::shared_ptr<std::string> time;
protected:
    uint32_t life;
public:
    Token(char *_uid, char *_name, char *_time);
    std::shared_ptr<std::string> &GetUid();
    std::shared_ptr<std::string> &GetName();
    std::shared_ptr<std::string> &GetTime();
    uint32_t GetLife() const;
public:
    virtual bool StepLife(int16_t delta);
};

#endif //DCCLIVE_TOKEN_H
