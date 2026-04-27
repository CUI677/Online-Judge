#pragma once
#include <cstdlib>
#include <iostream>
#include "../comm/util.hpp"
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "../comm/LOG.hpp"


//只负责代码编译

namespace ns_compiler
{
    //引入路径拼接功能
    using ns_util::PathUtil;
    using namespace ns_log;


    class Compiler{
        public:
        Compiler()
        {
            std::cout<<"Compiler()"<<std::endl;
        }
        ~Compiler()
        {
            std::cout<<"~Compiler()"<<std::endl;
        }
        //返回值表示编译成功与否
        //输入的参数是待编译的代码
        static bool Compile(const std::string& file_name)
        {
            return compile(file_name);
        }

        static bool compile(const std::string& file_name)
        {
            std::string exe_file = PathUtil::Exe(file_name);
            std::string err_file = PathUtil::CompilerError(file_name);
            std::string src_file = PathUtil::Src(file_name);
            // 每次编译前先删掉旧的可执行文件，避免本次编译失败时误用上一次的结果。
            ::unlink(exe_file.c_str());

            pid_t pid=fork();
            if(pid==0)
            {
                int _stderror=open(err_file.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0644);
                if(_stderror<0)                
                {
                    LOG(WARNING)<<"没有形成stderror文件!"<<std::endl;
                    _exit(1);
                }
                if(dup2(_stderror,STDERR_FILENO)<0)
                {
                    LOG(ERROR)<<"dup2 error!"<<std::endl;
                    close(_stderror);
                    _exit(1);
                }
                close(_stderror);
                //将标准错误重定向到文件中
                execlp("g++","g++","-o"
                ,exe_file.c_str()
                ,src_file.c_str()
                ,"-std=c++11",
                "-DCOMPILER_ONLINE",
                nullptr//参数结束标志
                );
                LOG(ERROR)<<"execlp error!"<<std::endl;
                _exit(1);//防止替换失败
            }
            else if(pid<0){
            LOG(ERROR)<<"fork error!"<<std::endl;
            return false;
            }
            else
            {
                int status=0;
                if(waitpid(pid,&status,0)<0)
                {
                    LOG(ERROR)<<"waitpid error!"<<std::endl;
                    return false;
                }
                //第二个参数是推出结果，不关心，第三个参数是选项，0表示阻塞等待
                //编译是否成功,不能只看文件是否存在, 还要看子进程退出状态
                if(!WIFEXITED(status) || WEXITSTATUS(status)!=0)
                {
                    LOG(ERROR)<<"编译失败, 请查看 "<<err_file<<std::endl;
                    return false;
                }

                if(ns_util::FileUtil::IsfileExist(exe_file))
                {
                    LOG(INFO)<<"编译成功!"<<std::endl;
                    return true;
                }
                else
                {
                    LOG(ERROR)<<"编译结束但没有生成可执行文件!"<<std::endl;
                    return false;
                }
            }
            return false;
        }
    };
}
