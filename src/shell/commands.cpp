#include "commands.hpp"

#include "memory/physical.hpp"
#include "shell.hpp"
#include "utils.hpp"

namespace cosmos::shell {
    struct Command {
        const char* name;
        const char* description;
        const CommandFn fn;
    };

    void meminfo() {
        print("Total");
        print(GRAY, ": ");
        printf("%d", static_cast<uint64_t>(memory::phys::get_total_pages()) * 4096 / 1024 / 1024);
        print(GRAY, " mB\n");

        print("Free");
        print(GRAY, ": ");
        printf("%d", static_cast<uint64_t>(memory::phys::get_free_pages()) * 4096 / 1024 / 1024);
        print(GRAY, " mB\n");
    }

    void help();

    static constexpr Command commands[] = {
        { "meminfo", "Display memory information", meminfo },
        { "help", "Display all available commands", help },
    };

    void help() {
        for (auto i = 0u; i < sizeof(commands) / sizeof(Command); i++) {
            const auto& cmd = commands[i];

            print(cmd.name);
            printf(GRAY, " - %s\n", cmd.description);
        }
    }

    CommandFn get_command_fn(const char* name) {
        for (auto i = 0u; i < sizeof(commands) / sizeof(Command); i++) {
            const auto& cmd = commands[i];
            if (utils::streq(name, cmd.name)) return cmd.fn;
        }

        return nullptr;
    }
} // namespace cosmos::shell
