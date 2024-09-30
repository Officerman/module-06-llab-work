#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <memory>
#include <thread>

enum class LogLevel { INFO, WARNING, ERROR };

class Logger {
private:
    static std::shared_ptr<Logger> instance;
    static std::mutex mutex_;
    std::ofstream logFile;
    LogLevel currentLogLevel;

    // Приватный конструктор для одиночи
    Logger() : currentLogLevel(LogLevel::INFO) {
        logFile.open("log.txt", std::ios::app); // Открываем файл для логов
        if (!logFile.is_open()) {
            throw std::runtime_error("Unable to open log file");
        }
    }

public:
    // Удаление возможности копирования и присваивания
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Метод для получения единственного экземпляра
    static std::shared_ptr<Logger> GetInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance) {
            instance = std::shared_ptr<Logger>(new Logger());
        }
        return instance;
    }

    // Метод для логирования сообщений
    void Log(const std::string& message, LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (level >= currentLogLevel) {
            switch (level) {
                case LogLevel::INFO:
                    logFile << "[INFO]: " << message << std::endl;
                    break;
                case LogLevel::WARNING:
                    logFile << "[WARNING]: " << message << std::endl;
                    break;
                case LogLevel::ERROR:
                    logFile << "[ERROR]: " << message << std::endl;
                    break;
            }
        }
    }

    // Установка уровня логирования
    void SetLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        currentLogLevel = level;
    }

    // Изменение пути к файлу логов
    void SetLogFilePath(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        logFile.close();
        logFile.open(path, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Unable to open log file");
        }
    }

    // Закрытие файла
    ~Logger() {
        logFile.close();
    }
};

// Инициализация стат переменных
std::shared_ptr<Logger> Logger::instance = nullptr;
std::mutex Logger::mutex_;

// Функция для многопоточного тестирования
void LogMessages(LogLevel level, const std::string& message) {
    auto logger = Logger::GetInstance();
    logger->Log(message, level);
}

int main() {
    auto logger = Logger::GetInstance();
    logger->SetLogLevel(LogLevel::INFO);

    // Несколько потоков для логирования
    std::thread t1(LogMessages, LogLevel::INFO, "This is an info message");
    std::thread t2(LogMessages, LogLevel::WARNING, "This is a warning message");
    std::thread t3(LogMessages, LogLevel::ERROR, "This is an error message");

    t1.join();
    t2.join();
    t3.join();

    return 0;
}

