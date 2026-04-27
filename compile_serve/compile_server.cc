#include "compile_run.hpp"
#include "../comm/httplib.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace ns_compile_and_run;
using namespace httplib;

void Usage(const std::string& proc)
{
    std::cerr << "Usage:\n\t" << proc << " port" << std::endl;
}

// 编译服务启动时可能被多个人同时请求，所以真正区分每次提交的临时文件名
// 的工作放在 CompileAndRun 内部处理，这里只负责接收 HTTP 请求并转发。
// ./compile_server 8080
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        return 1;
    }

    int port = std::atoi(argv[1]);
    if(port <= 0 || port > 65535)
    {
        std::cerr << "invalid port: " << argv[1] << std::endl;
        return 1;
    }

    Server svr;

    svr.Post("/compile_and_run", [](const Request& req, Response& resp)
    {
        // 用户通过 HTTP POST 上传一个 JSON 字符串，请求体就在 req.body 里。
        std::string in_json = req.body;
        std::string out_json;

        if(!in_json.empty())
        {
            CompileAndRun::Start(in_json, &out_json);
            resp.set_content(out_json, "application/json;charset=utf-8");
        }
        else
        {
            resp.status = 400;
            resp.set_content("{\"status\":-1,\"reason\":\"empty request body\"}",
                             "application/json;charset=utf-8");
        }
    });

    std::cout << "compile_server listen on 0.0.0.0:" << port << std::endl;
    std::cout << "POST /compile_and_run" << std::endl;

    svr.listen("0.0.0.0", port);
    return 0;
}
