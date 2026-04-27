#pragma once

#include "runner.hpp"
#include "compiler.hpp"
#include <jsoncpp/json/json.h>
#include "../comm/util.hpp"
#include "../comm/LOG.hpp"

namespace ns_compile_and_run
{
    using ns_compiler::Compiler;
    using ns_runner::Runner;
    using ns_util::FileUtil;
    using ns_util::PathUtil;
    using namespace ns_log;

    class CompileAndRun
    {
    public:
        static void RemoveTempFile(const std::string& file_name)
        {
            const std::string temp_files[] = {
                PathUtil::Src(file_name),
                PathUtil::CompilerError(file_name),
                PathUtil::Stdin(file_name),
                PathUtil::Stdout(file_name),
                PathUtil::Stderr(file_name),
                PathUtil::Exe(file_name)
            };

            for(const auto& temp_file : temp_files)
            {
                if(FileUtil::IsfileExist(temp_file))
                {
                    ::unlink(temp_file.c_str());
                }
            }
        }

        enum StatusCode
        {
            kSuccess = 0,
            kInputError = -1,
            kUnknownError = -2,
            kCompileError = -3
        };

        static std::string CodeToDesc(int code)
        {
            std::string desc;
            switch(code)
            {
                case kSuccess:
                    desc = "编译运行成功";
                    break;
                case kInputError:
                    desc = "提交的代码为空";
                    break;
                case kUnknownError:
                    desc = "未知错误";
                    break;
                case kCompileError:
                    desc = "代码编译的时候发生了错误";
                    break;
                case SIGABRT:
                    desc = "内存超过范围";
                    break;
                case SIGXCPU:
                    desc = "CPU使用超时";
                    break;
                case SIGSEGV:
                    desc = "访问非法内存";
                    break;
                case SIGFPE:
                    desc = "浮点数异常";
                    break;
                case SIGILL:
                    desc = "执行了非法指令";
                    break;
                case SIGBUS:
                    desc = "总线错误";
                    break;
                case SIGKILL:
                    desc = "进程被强制终止";
                    break;
                case SIGSYS:
                    desc = "非法系统调用";
                    break;
                case SIGTERM:
                    desc = "进程被终止";
                    break;
                case SIGXFSZ:
                    desc = "输出文件过大";
                    break;
                default:
                    if(code > 0)
                    {
                        desc = "程序异常终止, 收到信号: " + std::to_string(code);
                    }
                    else
                    {
                        desc = "未知错误";
                    }
                    break;
            }
            return desc;
        }

        static void Start(const std::string& in_json, std::string* out_json)
        {
            // out_json 是输出参数，最终结果 JSON 会写回到它指向的字符串里。
            if(out_json == nullptr)
            {
                LOG(ERROR) << "out_json 为空指针，无法写回结果!" << std::endl;
                return;
            }

            Json::Value in_value;
            Json::Value out_value;
            Json::Reader reader;
            Json::StreamWriterBuilder writer;
            writer["indentation"] = "";
            // 输出 JSON 约定:
            // status 放机器可判断的状态码, reason 放对状态码的简短分类说明。
            // compiler_error/stdout/stderr 按场景补充详细信息, 不和 reason 混用。

            // 1. 先把输入字符串解析成 JSON 对象。
            if(!reader.parse(in_json, in_value))
            {
                out_value["status"] = kUnknownError;
                out_value["reason"] = CodeToDesc(kUnknownError);
                *out_json = Json::writeString(writer, out_value);
                return;
            }

            // 2. 从输入 JSON 里取出代码、标准输入、资源限制。
            std::string code = in_value["code"].asString();
            std::string input = in_value["input"].asString();
            int cpu_limit = in_value["cpu_limit"].asInt();
            int mem_limit = in_value["mem_limit"].asInt();

            if(code.empty())
            {
                out_value["status"] = kInputError;
                out_value["reason"] = CodeToDesc(kInputError);
                *out_json = Json::writeString(writer, out_value);
                return;
            }

            // 3. 生成唯一文件名，再拼出 temp 目录下的源码和输入文件路径。
            std::string file_name = FileUtil::UniqFileName();
            // 同名临时文件理论上很少冲突，但先清理一次能避免残留文件干扰本次结果。
            RemoveTempFile(file_name);
            std::string src_file = PathUtil::Src(file_name);
            std::string stdin_file = PathUtil::Stdin(file_name);

            // 4. 把代码写进 .cpp，把输入写进 .stdin，供编译器和运行器继续使用。
            bool write_src_ok = FileUtil::WriteFile(src_file, code);
            bool write_stdin_ok = FileUtil::WriteFile(stdin_file, input);
            if(!write_src_ok || !write_stdin_ok)
            {
                out_value["status"] = kUnknownError;
                out_value["reason"] = CodeToDesc(kUnknownError);
                *out_json = Json::writeString(writer, out_value);
                RemoveTempFile(file_name);
                return;
            }

            // 5. 编译 temp 目录下的源码，失败时把编译错误回填到输出 JSON。
            if(!Compiler::Compile(file_name))
            {
                std::string compile_error;
                FileUtil::ReadFile(PathUtil::CompilerError(file_name), &compile_error);

                out_value["status"] = kCompileError;
                out_value["reason"] = CodeToDesc(kCompileError);
                out_value["compiler_error"] = compile_error;
                *out_json = Json::writeString(writer, out_value);
                RemoveTempFile(file_name);
                return;
            }

            // 6. 运行刚编译出来的程序，再把标准输出和标准错误读回来。
            int run_result = Runner::Run(file_name, cpu_limit, mem_limit);
            std::string stdout_str;
            std::string stderr_str;

            FileUtil::ReadFile(PathUtil::Stdout(file_name), &stdout_str);
            FileUtil::ReadFile(PathUtil::Stderr(file_name), &stderr_str);

            out_value["status"] = run_result;
            out_value["stdout"] = stdout_str;
            out_value["stderr"] = stderr_str;
            out_value["reason"] = CodeToDesc(run_result);

            *out_json = Json::writeString(writer, out_value);
            RemoveTempFile(file_name);
        }

        static void start(const std::string& in_json, std::string* out_json)
        {
            Start(in_json, out_json);
        }
    };
}
