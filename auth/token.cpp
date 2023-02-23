/*
 * Copyright (c) shinsya.G 
 */
#include "token.h"

using namespace std;

Token::Token(char *_uid, char *_name, char *_time)
: uid(make_shared<string>(_uid)), name(make_shared<string>(_name)), time(make_shared<string>(_time)), life(0)
{}

shared_ptr<string> &Token::GetUid() {
    return this->uid;
}

shared_ptr<string> &Token::GetName() {
    return this->name;
}

shared_ptr<string> &Token::GetTime() {
    return this->time;
}

uint32_t Token::GetLife() const {
    return this->life;
}

bool Token::StepLife(int16_t delta) {
    return false;
}
