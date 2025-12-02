#include <AK/StringView.h>
#include <AK/ByteBuffer.h>
#include <AK/Vector.h>
#include <LibAudio/ConnectionToServer.h>
#include <LibAudio/Loader.h>
#include <LibAudio/Sample.h>
#include <LibCore/ArgsParser.h>
#include <LibCore/File.h>
#include <LibCore/EventLoop.h>
#include <LibMain/Main.h>
#include <unistd.h>

ErrorOr<int> play_sound(AK::StringView sound_name);

ErrorOr<int> serenity_main(Main::Arguments arguments)
{
    Core::EventLoop event_loop;
    StringView sound_name;

    Core::ArgsParser parser;
    parser.add_positional_argument(sound_name, "Sound to play (startup|notify|shutdown)", "sound");
    parser.parse(arguments);

    pid_t pid = fork();
    if (pid < 0) {
        warnln("Failed to fork process");
        return 1;
    }
    
    if (pid > 0) {
        return 0;
    }
    
    setsid();
    
    return play_sound(sound_name);
}

ErrorOr<int> play_sound(AK::StringView sound_name)
{
    StringView path;
    if (sound_name == "startup")
        path = "/res/sound/CawOS_Startup.wav"sv;
    else if (sound_name == "notify")
        path = "/res/sound/CawOS_Notify.wav"sv;
    else if (sound_name == "shutdown")
        path = "/res/sound/CawOS_Shutdown.wav"sv;
    else {
        warnln("Unknown sound: {}. Available: startup, notify, shutdown", sound_name);
        return 1;
    }

    // Загружаем аудио файл
    auto loader_or_error = Audio::Loader::create(path);
    if (loader_or_error.is_error()) {
        warnln("Failed to load audio file: {}", path);
        return 1;
    }
    auto loader = loader_or_error.release_value();
    
    // Пытаемся подключиться к AudioServer с повторами (для случаев раннего запуска)
    RefPtr<Audio::ConnectionToServer> connection;
    constexpr int max_retries = 10;
    constexpr unsigned int retry_delay_us = 200'000; // 200ms
    
    for (int retry = 0; retry < max_retries; retry++) {
        auto connection_or_error = Audio::ConnectionToServer::try_create();
        if (!connection_or_error.is_error()) {
            connection = connection_or_error.release_value();
            break;
        }
        
        if (retry < max_retries - 1) {
            usleep(retry_delay_us);
        }
    }
    
    if (!connection) {
        warnln("Failed to connect to AudioServer after {} retries", max_retries);
        return 1;
    }

    // Загружаем все сэмплы
    Vector<Audio::Sample> samples;
    
    while (true) {
        auto buffer_or_error = loader->get_more_samples();
        if (buffer_or_error.is_error())
            break;
        
        auto buffer = buffer_or_error.release_value();
        if (buffer.is_empty())
            break;
            
        samples.extend(buffer);
    }

    if (samples.is_empty()) {
        warnln("No audio samples loaded from {}", path);
        return 1;
    }

    // Устанавливаем sample rate и отправляем аудио
    connection->set_self_sample_rate(loader->sample_rate());
    
    TRY(connection->async_enqueue(samples));

    // Вычисляем длительность и ждём завершения воспроизведения
    auto sample_rate = loader->sample_rate();
    auto duration_seconds = static_cast<float>(samples.size()) / static_cast<float>(sample_rate);
    
    usleep(static_cast<unsigned int>((duration_seconds + 0.5f) * 1'000'000));

    return 0;
}