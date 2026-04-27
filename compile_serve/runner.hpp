#pragma once

#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <string>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include<sys/time.h>
#include "../comm/LOG.hpp"
#include "../comm/util.hpp"

namespace ns_runner
{
    using ns_util::FileUtil;
    using ns_util::PathUtil;
    using namespace ns_log;

    class Runner
    {
        public:
        enum StatusCode
        {
            kRunSuccess = 0,
            kRunUnknownError = -2
        };

        Runner()
        {
            std::cout<<"Runner()"<<std::endl;
        }
        ~Runner()
        {
            std::cout<<"~Runner()"<<std::endl;
        }

        private:
        static bool SetProcLimit(int cpu_limit, int mem_limit)
        {
            if(cpu_limit > 0)
            {
                struct rlimit cpu_rlimit;
                cpu_rlimit.rlim_cur = static_cast<rlim_t>(cpu_limit);
                cpu_rlimit.rlim_max = static_cast<rlim_t>(cpu_limit);
                if(setrlimit(RLIMIT_CPU, &cpu_rlimit) < 0)
                {
                    LOG(ERROR)<<"setrlimit RLIMIT_CPU error!"<<std::endl;
                    return false;
                }
            }

            if(mem_limit > 0)
            {
                struct rlimit mem_rlimit;
                rlim_t mem_bytes = static_cast<rlim_t>(mem_limit) * 1024 * 1024;
                mem_rlimit.rlim_cur = mem_bytes;
                mem_rlimit.rlim_max = mem_bytes;
                if(setrlimit(RLIMIT_AS, &mem_rlimit) < 0)
                {
                    LOG(ERROR)<<"setrlimit RLIMIT_AS error!"<<std::endl;
                    return false;
                }
            }
            return true;
        }

        public:
        //返回值表示运行结果，非0表示不同类型的运行错误
        //cpu_limit 单位是秒，mem_limit 单位是 MB
        static int Run(const std::string& file_name, int cpu_limit, int mem_limit)
        {
            //结果对不对不用管，测试用例来决定；
            std::string execute = PathUtil::Exe(file_name);
            std::string stdin_file = PathUtil::Stdin(file_name);
            std::string stdout_file = PathUtil::Stdout(file_name);
            std::string stderr_file = PathUtil::Stderr(file_name);

            if(!FileUtil::IsfileExist(execute))
            {
                LOG(ERROR)<<"可执行程序不存在: "<<execute<<std::endl;
                return kRunUnknownError;
            }

            pid_t pid = fork();
            if(pid==0)
            {
                // 这里把“记录日志并退出”提成一个局部 lambda，避免每个错误分支都重复写同样代码。
                auto report_error = [&](const std::string& message)
                // auto report_error = 定义一个名为 report_error 的局部“匿名函数对象”
                // [&] 表示按引用捕获当前作用域里用到的外部变量
                // (const std::string& message) 是这个 lambda 的参数列表
                {
                    // 记录具体失败原因
                    LOG(ERROR)<<message<<std::endl;
                    // 直接结束子进程。这里不用保留退出码做细分，统一按运行失败处理即可。
                    _exit(1);
                };

                int stdin_fd = open(stdin_file.c_str(), O_RDONLY | O_CREAT, 0644);
                if(stdin_fd < 0)
                {
                    report_error("open stdin file error!");
                }

                int stdout_fd = open(stdout_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(stdout_fd < 0)
                {
                    close(stdin_fd);
                    report_error("open stdout file error!");
                }

                int stderr_fd = open(stderr_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(stderr_fd < 0)
                {
                    close(stdin_fd);
                    close(stdout_fd);
                    report_error("open stderr file error!");
                }

                if(dup2(stdin_fd, STDIN_FILENO) < 0
                    || dup2(stdout_fd, STDOUT_FILENO) < 0
                    || dup2(stderr_fd, STDERR_FILENO) < 0)
                {
                    close(stdin_fd);
                    close(stdout_fd);
                    close(stderr_fd);
                    report_error("dup2 error!");
                }

                close(stdin_fd);
                close(stdout_fd);
                close(stderr_fd);

                if(!SetProcLimit(cpu_limit, mem_limit))
                {
                    _exit(1);
                }

                execlp(execute.c_str(), execute.c_str(), nullptr);
                report_error("execlp error!");
            }
            else if(pid>0)
            {
                int status = 0;
                if(waitpid(pid, &status, 0) < 0)
                {
                    LOG(ERROR)<<"waitpid error!"<<std::endl;
                    return kRunUnknownError;
                }

                // WIFEXITED(status) 为真，表示子进程是“正常退出”的。
                // 正常退出包括：main里 return、或者代码里主动调用 exit。
                if(WIFEXITED(status))
                {
                    int exit_code = WEXITSTATUS(status);
                    // 退出码为0，表示用户程序正常运行结束。
                    if(exit_code == 0)
                    {
                        return kRunSuccess;
                    }
                    // 其它非0退出码，统一看作用户程序运行出错。
                    // 这里统一返回负数，和“收到信号导致退出”的正数语义区分开。
                    return kRunUnknownError;
                }

                // WIFSIGNALED(status) 为真，表示子进程不是正常 return/exit 结束，
                // 而是被某个信号异常终止了，例如：
                // SIGSEGV: 段错误，比如空指针解引用、非法内存访问
                // SIGFPE: 算术异常，比如整数除0
                if(WIFSIGNALED(status))
                {
                    int signal_number = WTERMSIG(status);
                    if(signal_number == SIGXCPU)
                    {
                        LOG(ERROR)<<"program timeout by RLIMIT_CPU!"<<std::endl;
                        return signal_number;
                    }
                    LOG(ERROR)<<"program was terminated by signal: "<<signal_number<<std::endl;
                    return signal_number;
                }

                // 兜底分支：如果既不是正常退出，也不是信号终止，先统一按未知错误处理。
                return kRunUnknownError;
            }
            else
            {
                LOG(ERROR)<<"fork error!"<<std::endl;
                return kRunUnknownError;
            }
            return kRunUnknownError;
        }

        static int run(const std::string& file_name, int cpu_limit, int mem_limit)
        {
            return Run(file_name, cpu_limit, mem_limit);
        }
    };
}
