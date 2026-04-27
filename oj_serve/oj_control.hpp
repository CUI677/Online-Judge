#pragma once

#include "oj_model.hpp"
#include "oj_view.hpp"
#include "../comm/httplib.h"
#include "../comm/LOG.hpp"
#include "../compile_serve/compile_run.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <limits>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace ns_control
{
    using ns_compile_and_run::CompileAndRun;
    using ns_model::Model;
    using ns_model::Question;
    using ns_view::View;
    using namespace ns_log;

    class Machine
    {
    public:
        std::string ip;
        int port;

    private:
        uint64_t load_;
        mutable std::mutex mtx_;

    public:
        // 初始化一台编译机，初始负载为 0。
        Machine(const std::string& machine_ip, int machine_port)
            : ip(machine_ip)
            , port(machine_port)
            , load_(0)
        {
        }

        // 为当前机器增加一个正在处理的请求计数。
        void IncLoad()
        {
            std::lock_guard<std::mutex> lock(mtx_);
            ++load_;
        }

        // 当前机器完成一个请求后减少负载计数。
        void DecLoad()
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if(load_ > 0)
            {
                --load_;
            }
        }

        // 获取当前机器的负载值。
        uint64_t Load() const
        {
            std::lock_guard<std::mutex> lock(mtx_);
            return load_;
        }

        // 返回 ip:port 形式的机器地址，便于日志和网络请求使用。
        std::string Address() const
        {
            return ip + ":" + std::to_string(port);
        }
    };

    class LoadBalance
    {
    private:
        std::vector<std::shared_ptr<Machine>> machines_;
        std::vector<int> online_;
        std::vector<int> offline_;
        mutable std::mutex mtx_;
        std::string conf_path_;

    private:
        // 去掉字符串首尾空白字符，方便后续解析配置项。
        static std::string Trim(const std::string& value)
        {
            const std::string whitespace = " \t\r\n";
            const std::string::size_type begin = value.find_first_not_of(whitespace);
            if(begin == std::string::npos)
            {
                return "";
            }

            const std::string::size_type end = value.find_last_not_of(whitespace);
            return value.substr(begin, end - begin + 1);
        }

        // 依次尝试候选路径，找到第一个存在的编译机配置文件。
        static std::string ResolveMachineConf()
        {
            const std::string candidates[] = {
                "./oj_serve/conf/service_machine.conf",
                "./conf/service_machine.conf",
                "../oj_serve/conf/service_machine.conf"
            };

            for(const auto& candidate : candidates)
            {
                std::ifstream in(candidate);
                if(in.is_open())
                {
                    return candidate;
                }
            }

            return "./oj_serve/conf/service_machine.conf";
        }

        // 将一行机器配置解析为 ip 和 port；空行、注释行或格式错误都会返回 false。
        static bool ParseMachineLine(const std::string& line, std::string* ip, int* port)
        {
            if(ip == nullptr || port == nullptr)
            {
                return false;
            }

            const std::string trimmed = Trim(line);
            if(trimmed.empty() || trimmed[0] == '#')
            {
                return false;
            }

            const std::string::size_type pos = trimmed.rfind(':');
            if(pos == std::string::npos || pos == 0 || pos + 1 >= trimmed.size())
            {
                return false;
            }

            std::string port_str = Trim(trimmed.substr(pos + 1));
            char* end = nullptr;
            long machine_port = std::strtol(port_str.c_str(), &end, 10);
            if(end == port_str.c_str() || *end != '\0' || machine_port <= 0 || machine_port > 65535)
            {
                return false;
            }

            *ip = Trim(trimmed.substr(0, pos));
            *port = static_cast<int>(machine_port);
            return !ip->empty();
        }

        // 读取机器配置文件，解析出所有可用编译机，并加入在线机器列表。
        bool LoadConf(const std::string& machine_conf)
        {
            std::ifstream in(machine_conf);
            if(!in.is_open())
            {
                LOG(WARNING) << "负载均衡配置不存在: " << machine_conf
                             << ", 将退回本地 CompileAndRun 模式" << std::endl;
                return false;
            }

            std::string line;
            int line_no = 0;
            while(std::getline(in, line))
            {
                ++line_no;
                std::string ip;
                int port = 0;
                if(!ParseMachineLine(line, &ip, &port))
                {
                    const std::string trimmed = Trim(line);
                    if(!trimmed.empty() && trimmed[0] != '#')
                    {
                        LOG(WARNING) << "机器配置格式错误, line=" << line_no << ": " << line << std::endl;
                    }
                    continue;
                }

                const int id = static_cast<int>(machines_.size());
                machines_.push_back(std::make_shared<Machine>(ip, port));
                online_.push_back(id);
            }

            return !machines_.empty();
        }

        // 返回当前在线编译机数量，作为分发尝试次数上限。
        std::size_t OnlineSize() const
        {
            std::lock_guard<std::mutex> lock(mtx_);
            return online_.size();
        }

        // 从在线机器中挑选当前负载最小的一台，并先记一次占用。
        bool SmartChoice(int* id, std::shared_ptr<Machine>* machine)
        {
            if(id == nullptr || machine == nullptr)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(mtx_);
            if(online_.empty())
            {
                return false;
            }

            int best_id = online_[0];
            uint64_t min_load = machines_[best_id]->Load();
            for(std::size_t i = 1; i < online_.size(); ++i)
            {
                const int current_id = online_[i];
                const uint64_t current_load = machines_[current_id]->Load();
                if(current_load < min_load)
                {
                    min_load = current_load;
                    best_id = current_id;
                }
            }

            *id = best_id;
            *machine = machines_[best_id];
            (*machine)->IncLoad();
            return true;
        }

        // 请求结束后释放对应机器的占用计数。
        void ReleaseMachine(int id)
        {
            if(id >= 0 && id < static_cast<int>(machines_.size()))
            {
                machines_[id]->DecLoad();
            }
        }

        // 将请求失败的机器从在线列表移到离线列表。
        void OfflineMachine(int id)
        {
            std::lock_guard<std::mutex> lock(mtx_);
            std::vector<int>::iterator it = std::find(online_.begin(), online_.end(), id);
            if(it == online_.end())
            {
                return;
            }

            online_.erase(it);
            if(std::find(offline_.begin(), offline_.end(), id) == offline_.end())
            {
                offline_.push_back(id);
            }
        }

        // 向指定编译机发送编译运行请求，并返回其响应结果。
        bool PostToMachine(const std::shared_ptr<Machine>& machine, const std::string& in_json, std::string* out_json)
        {
            if(machine == nullptr || out_json == nullptr)
            {
                return false;
            }

            httplib::Client cli(machine->ip, machine->port);
            cli.set_connection_timeout(1, 0);
            cli.set_read_timeout(60, 0);
            cli.set_write_timeout(5, 0);

            httplib::Result res = cli.Post("/compile_and_run",
                                           in_json,
                                           "application/json;charset=utf-8");
            if(res && res->status == 200)
            {
                *out_json = res->body;
                return true;
            }

            if(res)
            {
                LOG(ERROR) << "请求编译主机失败, host=" << machine->Address()
                           << ", http_status=" << res->status << std::endl;
            }
            else
            {
                LOG(ERROR) << "请求编译主机失败, host=" << machine->Address()
                           << ", 无法建立连接" << std::endl;
            }
            return false;
        }

    public:
        // 初始化负载均衡器，并在启动时尝试加载编译机配置。
        LoadBalance()
            : conf_path_(ResolveMachineConf())
        {
            if(LoadConf(conf_path_))
            {
                LOG(INFO) << "加载编译主机配置成功: " << conf_path_
                          << ", size=" << machines_.size() << std::endl;
            }
        }

        // 判断当前是否启用了远端编译机负载均衡。
        bool Enabled() const
        {
            return !machines_.empty();
        }

        // 将一次编译请求分发给在线机器，失败时依次尝试其他机器。
        bool Dispatch(const std::string& in_json, std::string* out_json)
        {
            const std::size_t attempts = OnlineSize();
            for(std::size_t i = 0; i < attempts; ++i)
            {
                int id = -1;
                std::shared_ptr<Machine> machine;
                if(!SmartChoice(&id, &machine))
                {
                    return false;
                }

                const bool ok = PostToMachine(machine, in_json, out_json);
                ReleaseMachine(id);
                if(ok)
                {
                    return true;
                }

                OfflineMachine(id);
                LOG(WARNING) << "将编译主机标记为离线: " << machine->Address() << std::endl;
            }

            return false;
        }
    };

    class Control
    {
    private:
        Model* model_;
        LoadBalance load_balance_;

        // 统一输出简单 JSON 响应，便于前端处理状态和错误信息。
        static void WriteJson(httplib::Response& resp, int status, const std::string& reason)
        {
            Json::Value value;
            Json::StreamWriterBuilder writer;
            writer["indentation"] = "";
            value["status"] = status;
            value["reason"] = reason;
            resp.set_content(Json::writeString(writer, value), "application/json;charset=utf-8");
        }

    public:
        // 绑定题库模型，并初始化判题使用的负载均衡器。
        explicit Control(Model* model)
            : model_(model)
            , load_balance_()
        {
        }

        // 获取题目列表并渲染为题库首页。
        void AllQuestions(httplib::Response& resp)
        {
            std::vector<Question> questions;
            model_->GetAllQuestions(&questions);

            std::string html;
            View::RenderAllQuestions(questions, &html);
            resp.set_content(html, "text/html;charset=utf-8");
        }

        // 获取单道题目并渲染详情页，不存在则返回 404。
        void OneQuestion(const std::string& number, httplib::Response& resp)
        {
            Question q;
            if(!model_->GetQuestion(number, &q))
            {
                resp.status = 404;
                resp.set_content("question not found\n", "text/plain;charset=utf-8");
                return;
            }

            std::string html;
            View::RenderQuestion(q, &html);
            resp.set_content(html, "text/html;charset=utf-8");
        }

        // 处理一次判题请求，组装编译参数并选择远端或本地执行。
        void Judge(const std::string& number, const httplib::Request& req, httplib::Response& resp)
        {
            Question q;
            if(!model_->GetQuestion(number, &q))
            {
                resp.status = 404;
                WriteJson(resp, -1, "question not found");
                return;
            }

            Json::Value in_value;
            Json::Reader reader;
            if(!reader.parse(req.body, in_value))
            {
                resp.status = 400;
                WriteJson(resp, -1, "bad json");
                return;
            }

            Json::Value compile_value;
            Json::StreamWriterBuilder writer;
            writer["indentation"] = "";

            std::string user_code = in_value["code"].asString();
            compile_value["code"] = q.header + "\n" + user_code + "\n" + q.tail;
            compile_value["input"] = in_value["input"].asString();
            compile_value["cpu_limit"] = q.cpu_limit;
            compile_value["mem_limit"] = q.mem_limit;

            std::string compile_json = Json::writeString(writer, compile_value);
            std::string result_json;

            if(load_balance_.Enabled())
            {
                if(!load_balance_.Dispatch(compile_json, &result_json))
                {
                    resp.status = 503;
                    WriteJson(resp, -2, "backend compile service unavailable");
                    return;
                }
            }
            else
            {
                CompileAndRun::Start(compile_json, &result_json);
            }

            resp.set_content(result_json, "application/json;charset=utf-8");
        }
    };
}
