#pragma once
//主要用来数据交互，对外提供访问数据的model接口，负责加载题库数据，提供查询题库数据的接口。
#include "../comm/LOG.hpp"
#include "../comm/util.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ns_model
{
    using ns_util::FileUtil;
    using namespace ns_log;

    struct Question
    {
        std::string number;
        std::string title;
        std::string star;
        int cpu_limit;
        int mem_limit;
        std::string desc;
        std::string header;
        std::string tail;
    };

    class Model
    {
    private:
        std::unordered_map<std::string, Question> questions_;
        std::vector<std::string> order_;
        std::string question_root_;

    private:
        static bool SplitLine(const std::string& line, std::vector<std::string>* tokens)
        {
            if(tokens == nullptr)
            {
                return false;
            }

            std::istringstream iss(line);
            std::string token;
            while(iss >> token)
            {
                tokens->push_back(token);
            }
            return !tokens->empty();
        }

        static std::string ResolveQuestionRoot()
        {
            const std::string candidates[] = {
                "./oj_serve/questions/",
                "./questions/",
                "../oj_serve/questions/"
            };

            for(const auto& candidate : candidates)
            {
                std::ifstream in(candidate + "questions.list");
                if(in.is_open())
                {
                    return candidate;
                }
            }

            return "./oj_serve/questions/";
        }

    public:
        bool Load()
        {
            question_root_ = ResolveQuestionRoot();
            std::ifstream in(question_root_ + "questions.list");
            if(!in.is_open())
            {
                LOG(ERROR) << "加载题库失败: " << question_root_ << "questions.list" << std::endl;
                return false;
            }

            std::string line;
            while(std::getline(in, line))
            {
                if(line.empty())
                {
                    continue;
                }

                std::vector<std::string> tokens;
                SplitLine(line, &tokens);
                if(tokens.size() != 5)
                {
                    LOG(WARNING) << "题目列表格式错误: " << line << std::endl;
                    continue;
                }

                Question q;
                q.number = tokens[0];
                q.title = tokens[1];
                q.star = tokens[2];
                q.cpu_limit = std::atoi(tokens[3].c_str());
                q.mem_limit = std::atoi(tokens[4].c_str());

                std::string base = question_root_ + q.number + "/";
                if(!FileUtil::ReadFile(base + "desc.txt", &q.desc)
                    || !FileUtil::ReadFile(base + "header.cpp", &q.header)
                    || !FileUtil::ReadFile(base + "tail.cpp", &q.tail))
                {
                    LOG(WARNING) << "加载题目文件失败: " << q.number << std::endl;
                    continue;
                }

                order_.push_back(q.number);
                questions_.insert({q.number, q});
            }

            LOG(INFO) << "题库加载成功, size=" << questions_.size() << std::endl;
            return !questions_.empty();
        }

        bool GetAllQuestions(std::vector<Question>* out) const
        {
            if(out == nullptr)
            {
                return false;
            }

            out->clear();
            for(const auto& number : order_)
            {
                auto it = questions_.find(number);
                if(it != questions_.end())
                {
                    out->push_back(it->second);
                }
            }

            return true;
        }

        bool GetQuestion(const std::string& number, Question* q) const
        {
            if(q == nullptr)
            {
                return false;
            }

            auto it = questions_.find(number);
            if(it == questions_.end())
            {
                return false;
            }

            *q = it->second;
            return true;
        }
    };
}
