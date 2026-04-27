#include "../comm/httplib.h"
#include "oj_control.hpp"
#include "oj_model.hpp"
#include <cstdlib>
#include <iostream>

using namespace httplib;
using namespace ns_control;
using namespace ns_model;

void Usage(const std::string& proc)
{
    std::cerr << "Usage:\n\t" << proc << " port" << std::endl;
}

// ./oj_server 8081
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

    Model model;
    if(!model.Load())
    {
        return 2;
    }

    Control control(&model);
    Server svr;

    // 用户请求的服务路由功能
    svr.Get("/", [&](const Request&, Response& resp)
    {
        control.AllQuestions(resp);
    });

    svr.Get("/all_questions", [&](const Request&, Response& resp)
    {
        control.AllQuestions(resp);
    });

    // /question/1000
    svr.Get(R"(/question/(\d+))", [&](const Request& req, Response& resp)
    {
        control.OneQuestion(req.matches[1], resp);
    });

    // 用户提交代码，进入判题逻辑
    svr.Post(R"(/judge/(\d+))", [&](const Request& req, Response& resp)
    {
        control.Judge(req.matches[1], req, resp);
    });

    svr.Get("/healthz", [](const Request&, Response& resp)
    {
        resp.set_content("oj server is running\n", "text/plain;charset=utf-8");
    });

    std::cout << "oj_server listen on 0.0.0.0:" << port << std::endl;
    std::cout << "GET /all_questions" << std::endl;
    std::cout << "GET /question/{number}" << std::endl;
    std::cout << "POST /judge/{number}" << std::endl;

    svr.listen("0.0.0.0", port);
    return 0;
}
