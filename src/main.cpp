#include "core/application.h"
#include "core/logger.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>

int main(int argc, char** argv)
{
    initialize_logging();

    LOG_FATAL("This is a fatal message.");
    LOG_ERROR("This is a error message.");
    LOG_WARN("This is a warn message.");
    LOG_INFO("This is a info message.");
    LOG_DEBUG("This is a debug message.");
    LOG_TRACE("This is a trace message.");

    ApplicationConfig app_config;
    app_config.client_width = 1280;
    app_config.client_height = 720;
    app_config.pos_x = 100;
    app_config.pos_y = 100;
    app_config.name = "D3D12 Renderer";

    Application app;

    initialize(&app, app_config);
    run(&app);
    shutdown(&app);

    return 0;
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{
    main(0, nullptr);

    return 0;
}