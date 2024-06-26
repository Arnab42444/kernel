#pragma once

#include "kernel/drivers/keyboard.h"
#include "kernel/drivers/terminal.h"
#include "kernel/libk/alloc.h"
#include "kernel/libk/string.h"
#include "kernel/scheduler.h"

typedef struct {
    char *name;
    void (*function)();
} ShellCommand;

void shell_test() {
    terminal_writestring("Test successful\n");
}

void shell_memusage() {
    char *n = kmalloc(256);
    memset(n, '\0', 256);
    itoa(get_mem_usage(), n);
    terminal_writestring("Current memory usage: ");
    terminal_writestring(n);
    terminal_writestring("\n");
    kmfree(n);
}

void shell_help() {
    terminal_writestring("help - display help menu\n");
    terminal_writestring("clear - clear the screen\n");
    terminal_writestring("test - print test message\n");
    terminal_writestring("mem - display memory usage\n");
}

void test() {
    int i = 0;
    while(true) {
        char n[5];
        itoa(i, n);
        terminal_writestring(n);
        terminal_writestring("         \r");
        yield();

        if (i > 300) {
            exitTask();
        } else {
            i++;
        }
    }
}

void shell_spawn_test() {
    spawnTask(test);
}

ShellCommand commands[] = {
    {.name = "test", .function = shell_test},
    {.name = "clear", .function = terminal_initialize},
    {.name = "mem", .function = shell_memusage},
    {.name = "help", .function = shell_help},
    {.name = "task", .function = shell_spawn_test},
    {.name = "exit", .function = exitTask},
};

char *shell_read_line() {
    terminal_writestring("> ");
    char *cmd = kmalloc(256);
    memset(cmd, '\0', 256);
    size_t i = 0;

    while (true) {
        char key = keyboardSpinLoop();
        if (key == '\b') {
            if (i) { 
                i--;
                cmd[i] = '\0';
                terminal_column--;
                terminal_putchar(' ');
                terminal_column--;
            }
            continue;
        }
        terminal_putchar(key);

        if (key == '\n') break;
        cmd[i] = key;
        i++;
    }

    return cmd;
}

void shell() {
    while (true) {
        char *cmd = shell_read_line();

        if (strcmp(cmd, "") == 0) {
            kmfree(cmd);
            continue;
        }

        bool executed = false;
        for (size_t i = 0; i < sizeof(commands) / sizeof(ShellCommand); i++) {
            if (strcmp(cmd, commands[i].name) == 0) {
                commands[i].function();
                executed = true;
                break;
            }
        }

        if (!executed) {
            terminal_writestring("Unknown command ");
            terminal_writestring(cmd);
            terminal_writestring(". Try typing \"help\".\n");
        }

        kmfree(cmd);
    }
}