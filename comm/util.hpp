#pragma once

#include <atomic>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include<fcntl.h>


namespace ns_util
{
    class PathUtil
    {
        private:
        static std::string TempDir()
        {
            const std::string candidates[] = {
                "./temp/",
                "./compile_serve/temp/",
                "../compile_serve/temp/",
                "./../compile_serve/temp/"
            };
            struct stat st;
            for (const auto &candidate : candidates)
            {
                if (stat(candidate.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
                {
                    return candidate;
                }
            }
            return "./compile_serve/temp/";
        }

        public:
        static std::string Suffix(const std::string& file_name,const std::string& suffix)
        {
            std::string path_name=TempDir();
            path_name+=file_name;
            path_name+=suffix;
            return path_name;
        }
        //构建源文件+后缀的完整文件名
        static std::string Src(const std::string& file_name)
        {            
            return Suffix(file_name,".cpp");
        }
        //构建可执行程序的完整路径+文件名
        static std::string Exe(const std::string& file_name)
        {
            return Suffix(file_name,".exe");
        }
        //构建该程序对应的编译错误文件完整路径
        static std::string CompilerError(const std::string& file_name)
        {
            return Suffix(file_name,".compiler_error");
        }
        //兼容旧接口
        static std::string Err(const std::string& file_name)
        {
            return CompilerError(file_name);
        }
        //构建运行时标准输入文件完整路径
        static std::string Stdin(const std::string& file_name)
        {
            return Suffix(file_name,".stdin");
        }
        //构建运行时标准输出文件完整路径
        static std::string Stdout(const std::string& file_name)
        {
            return Suffix(file_name,".stdout");
        }
        //构建运行时标准错误文件完整路径
        static std::string Stderr(const std::string& file_name)
        {
            return Suffix(file_name,".stderr");
        }
    };
    class FileUtil
    {
        public:
        // 形成唯一文件名，只返回名字本身，不包含路径和后缀。
        static std::string UniqFileName()
        {
            struct timeval tv;
            gettimeofday(&tv,nullptr);

            static std::atomic<unsigned long long> seq(0);
            unsigned long long uniq_id = ++seq;
            long long usec = static_cast<long long>(tv.tv_sec) * 1000000 + tv.tv_usec;

            return std::to_string(usec) + "_" + std::to_string(getpid()) + "_" + std::to_string(uniq_id);
        }

        static bool WriteFile(const std::string& file_name,const std::string& content)
        {
            // ofstream 是 C++ 的“输出文件流”，这里表示把内容写到 file_name 对应的文件里。
            // binary 表示按原始字节写，trunc 表示如果文件已存在就先清空再重写。
            std::ofstream ofs(file_name,std::ios::binary | std::ios::trunc);
            if(!ofs.is_open())
            {
                return false;
            }

            // 像往 cout 里输出一样，把整个字符串写进文件。
            ofs<<content;
            // good() 用来判断写入过程中流状态是否正常。
            return ofs.good();
        }

        static bool ReadFile(const std::string& file_name,std::string* content)
        {
            if(content==nullptr)
            {
                return false;
            }

            // ifstream 是 C++ 的“输入文件流”，这里表示从 file_name 对应的文件里读内容。
            std::ifstream ifs(file_name,std::ios::binary);
            if(!ifs.is_open())
            {
                return false;
            }

            std::stringstream buffer;
            // rdbuf() 拿到文件流内部缓冲区，再整体倒进 stringstream，适合一次性读完整文件。
            buffer<<ifs.rdbuf();
            *content=buffer.str();
            return true;
        }

        //判断一个文件是否存在
        static bool IsfileExist(const std::string& file_name)
        {
            struct stat st;
            if(stat(file_name.c_str(),&st)==0)
            {
                return true;
                //获取文件属性成功会返回0，代表文件存在
            }
            else
            {
                return false;
            }
        }
    };
    class Timeutil
    {
        public:
        //获取当前时间戳
        static std::string GetTimeStamp()
        {
            struct timeval tv;
            gettimeofday(&tv,nullptr);
            //获取当前时间，单位是秒和微秒
            time_t sec=tv.tv_sec;
            suseconds_t usec=tv.tv_usec;
            //将秒和微秒转换成毫秒
            long long msec=sec*1000+usec/1000;
            return std::to_string(msec);
        }
    };
}
