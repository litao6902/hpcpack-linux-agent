#ifndef VERSION_H_INCLUDED
#define VERSION_H_INCLUDED

#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace hpc
{
    class Version
    {
        public:
            static const std::map<std::string, std::vector<std::string>>& GetVersionHistory()
            {
                static std::map<std::string, std::vector<std::string>> versionHistory =
                {
                    { "1.1.1.1",
                        {
                            "Node manager main functionality support",
                            "Added version support",
                            "Fixed network card reversed order issue",
                            "Added trace",
                            "Added error codes definition",
                            "Fixed a potential node error issue",
                        }
                    },
                    { "1.1.1.2",
                        {
                            "Fixed a long running issue because of callback failure",
                            "Added version history support",
                        }
                    },
                    { "1.1.1.3",
                        {
                            "Fixed a long running issue because of callback contract mismatch",
                        }
                    },
                    { "1.1.1.4",
                        {
                            "Retry when create cgroup failed",
                            "Return the exit code and error message when PrepareTask",
                            "Record the output to message in Process",
                        }
                    },
                    { "1.1.1.5",
                        {
                            "Print out version history",
                        }
                    },
                    { "1.1.1.6",
                        {
                            "Changed the process chain which will handle the user permission, std streams better",
                            "Added utilities to handle users",
                            "Added unit test framework",
                            "Added test case for some echo Process",
                            "Fixed a bug that a requeued task cannot be canceled",
                            "Fixed a bug that json not-existing value is handled incorrectly",
                        }
                    },
                    { "1.1.1.7",
                        {
                            "Make file dependency auto detect",
                        }
                    },
                    { "1.1.1.8",
                        {
                            "Set timeout to let callback release locks to avoid deadlock",
                            "Request resync when callback fails",
                            "Adjust the maximum log size to avoid flushing important logs",
                            "Fixed the detection method of cgroup in CentOS 6.6",
                            "Make the return message of a task more verbose",
                            "Verify the cgroup creation status at the end of PrepareTask",
                            "Adjusted makefile output format",
                        }
                    },
                    { "1.1.1.9",
                        {
                            "Fixed the default working directory ownership",
                            "Write to the task message and log only when failure happened",
                            "Configure CentOS sudoers to allow non-tty execution",
                            "Changed test user name in unit test case",
                            "Won't delete the user in job ending to avoid multi-job interference",
                        }
                    },
                    { "1.1.1.10",
                        {
                            "Try to fix a thread pool leak issue",
                            "Support CGroup mounting point on CentOS 6.6",
                        }
                    },
                };

                return versionHistory;
            }

            static const std::string& GetVersion()
            {
                auto& h = GetVersionHistory();
                auto it = --h.end();
                return it->first;
            }

            static void PrintVersionHistory()
            {
                auto& h = GetVersionHistory();
                for (auto& v : h)
                {
                    std::cout << v.first << std::endl;
                    std::cout << "================================================================" << std::endl;

                    int number = 0;
                    for (auto& m : v.second)
                    {
                        number++;
                        std::cout << number << ". " << m << std::endl;
                    }

                    std::cout << std::endl;
                }
            }

        private:

    };
}

#endif // VERSION_H_INCLUDED