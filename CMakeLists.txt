cmake_minimum_required(VERSION 3.23)
project(dccLive)

set(CMAKE_CXX_STANDARD 11)

include_directories(log)

add_executable(dccLive main.cpp log/log.cpp runtime.cpp
        auth/token.cpp auth/user.cpp auth/admin.cpp service/pool.cpp service/http.cpp service/console.cpp utils/utils.cpp service/core.cpp utils/sha1.cpp service/response.cpp service/header.cpp service/router/error.cpp service/router/server.cpp service/router/console_route.cpp service/router/console_service.cpp service/mime.cpp)
