#include <LibGUI/Application.h>
#include <LibGUI/Window.h>
#include <LibGUI/Icon.h>
#include <LibMain/Main.h>

#include "GameWidget.h"

ErrorOr<int> serenity_main(Main::Arguments arguments)
{
    auto app = TRY(GUI::Application::create(arguments));

    auto window = TRY(GUI::Window::try_create());
    window->set_title("ProgressBar95 (ProgressOS)"sv);
    window->resize(640, 480);
    window->set_icon(GUI::Icon::default_icon("app-progressbar95"sv).bitmap_for_size(16));

    auto widget = TRY(ProgressOSGame::GameWidget::try_create());
    window->set_main_widget(widget);

    window->show();
    return app->exec();
}
