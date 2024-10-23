#ifndef CPP_PROFILE_HPP
#define CPP_PROFILE_HPP
#include "abstract.hpp"

namespace handlers {

    class Cpp final : public IHandler {
    private:
        const string compile_command = utils::get_env("CPP_COMPILE_COMMAND", "g++");
        const string default_args = "-O3 -static -DONLINE_JUDGE -lm -o main.out";

        void compile() override {
            if (submission.status != data::submission_status::Running) return;
            if (submission.content == nullptr) {
                submission.status = data::submission_status::InternalError;
                submission.message = "Submission content is null";
                return;
            }
            std::ofstream source_file(work_dir + "/main.cpp");
            source_file << submission.content;
            source_file.close();
            string standard;
            if (variant == "c++98") standard = "-std=c++98";
            else if (variant == "c++11") standard = "-std=c++11";
            else standard = "-std=c++17";
            const string command = compile_command + ' ' + standard + ' ' + default_args + ' ' + work_dir + "/main.cpp";
            if (system(command.c_str()) != 0) {
                submission.status = data::submission_status::CompilationError;
                submission.message = "Compilation command exit with non-zero status";
            }
        }

        void test(const string& input, const string& output) override {
            utils::RunGuard run_guard(problem.time_limit_secs, problem.memory_limit_mb);
            std::stringstream output_stream;
            run_guard.run((work_dir + "/main.out").c_str(), input, output_stream);
            if (run_guard.status == EXIT_SUCCESS) {
                if (!utils::token_compare(output_stream, output)) {
                    submission.status = data::submission_status::WrongAnswer;
                }
            } else if (run_guard.status == SIGXCPU) {
                submission.status = data::submission_status::TimeLimitExceeded;
            } else if (run_guard.status == SIGXFSZ) {
                submission.status = data::submission_status::MemoryLimitExceeded;
            } else {
                submission.status = data::submission_status::RuntimeError;
            }
        }

    public:
        Cpp(data::Submission submission, data::Problem problem) : IHandler(std::move(submission), std::move(problem)) {}
    };

}

#endif
