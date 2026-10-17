#pragma once
// 内置工具（read_file / grep_search / get_datetime 等）的执行实现。
// 与 server-context 解耦，供单体版 llama-server（server.cpp）直接使用。
// 通过 HTTP 暴露：GET /v1/tools（列出 schema）、POST /v1/tools {"tool":name,"params":{}}（执行）。

#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <vector>

using json = nlohmann::ordered_json;

struct server_tool {
    std::string name;
    std::string display_name;
    bool permission_write = false;

    virtual ~server_tool() = default;
    virtual json get_definition() = 0;
    virtual json invoke(json params) = 0;

    json to_json();
};

struct server_tools {
    std::vector<std::unique_ptr<server_tool>> tools;

    // enabled 为空 = 不启用任何工具；含 "all" = 全部。未知名字抛 std::runtime_error。
    void setup(const std::vector<std::string> & enabled_tools);
    json invoke(const std::string & name, const json & params);
};
