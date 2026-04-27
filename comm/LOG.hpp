#pragma once
#include <iostream>
#include <string>
#include "util.hpp"

namespace ns_log
{
    //日志级别
    enum LogLevel
    {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        FATAL
    };

    // LOG(INFO) << "message"
    inline std::ostream &Log(const std::string &level, const std::string &file_name, int line)
    {
        std::string message = "[";
        message += level;
        message += "]";
        //添加报错文件名称
        message += file_name;
        message += ":[";
        //添加报错行号
        message += std::to_string(line);
        message += "][";
        //添加日志时间戳
        message += ns_util::Timeutil::GetTimeStamp();
        message += "]";
        //cout本质内部是包含一个缓冲区的，日志信息先写入缓冲区，等到输出换行或者缓冲区满了才会真正输出
        std::cout << message;//不要endl进行刷新，返回的时候这个流就被拿到，可以开放式的拼接错误信息
        return std::cout;
    }
}

//开放式日志接口
#define LOG(level) ns_log::Log(#level, __FILE__, __LINE__)
