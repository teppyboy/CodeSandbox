#ifndef PYTHON_PROFILE_HPP
#define PYTHON_PROFILE_HPP
#include "abstract.hpp"

namespace modules {

    class Python final : public IModules {
    private:

        void compile() override {
            if (submission.status != data::submission_status::Running) return;
            std::ofstream source_file(work_dir + "/main.py", std::ios::binary);
            source_file << submission.file_content;
            source_file.close();
        }

        void test(const std::string& input, const std::string& output) override {
            utils::RunGuard run_guard(problem.time_limit_secs, problem.memory_limit_mb);
            std::stringstream output_stream;
            std::string command = "python3";
            if (variant == "python313") command = "python3.13";
            run_guard.run((command + " " + work_dir + "/main.py").c_str(), input, output_stream);
            if (WIFEXITED(run_guard.status)) {
                if (WEXITSTATUS(run_guard.status) == EXIT_SUCCESS) {
                    if (!utils::token_compare(output_stream, output))
                        submission.status = data::submission_status::WrongAnswer;
                    else return;
                } else if (WEXITSTATUS(run_guard.status) == EXIT_FAILURE) {
                    submission.status = data::submission_status::RuntimeError;
                    submission.message = run_guard.message;
                }
            }
            else if (WIFSIGNALED(run_guard.status)) {
                if (WTERMSIG(run_guard.status) == SIGXCPU || WTERMSIG(run_guard.status) == SIGALRM)
                    submission.status = data::submission_status::TimeLimitExceeded;
                else if (WTERMSIG(run_guard.status) == SIGKILL)
                    submission.status = data::submission_status::MemoryLimitExceeded;
            }
            else {
                submission.status = data::submission_status::RuntimeError;
                submission.message = run_guard.message;
            }
        }

    public:
        Python(const data::Submission* submission, const data::Problem* problem) : IModules(submission, problem) {}
    };

}

#endif
