serenity_component(
    ProgressBar95
    RECOMMENDED
    TARGETS ProgressBar95
)

# Мы пока не используем GML, поэтому строка compile_gml не нужна.
# Если в будущем переведешь интерфейс на GML, добавишь её сюда.

set(SOURCES
    main.cpp
    GameWidget.cpp
)

# ICON app-progress-game указывает системе искать иконку /res/icons/16x16/app-progress-game.png
# Если иконки пока нет, можно временно поставить ICON app-games или оставить как есть (будет дефолтная)
serenity_app(ProgressBar95 ICON app-progressbar95)

# Подключаем только то, что реально используем:
# LibCore (Таймеры), LibGfx (Графика), LibGUI (Окна), LibMain (Точка входа)
target_link_libraries(ProgressBar95 PRIVATE LibCore LibGfx LibGUI LibMain)