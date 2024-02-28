#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>
#include "imgui\\imgui.h"

class DebugLogger {
public:
    void AddLog(const std::string& msg) {
        messages.push_back(msg);
    }

    void Draw(const char* title) {
        if (ImGui::Begin(title)) {
            

            for (const auto& msg : messages) {
                ImGui::TextUnformatted(msg.c_str());
            }

            if (ImGui::Button("Save Log")) {
                SaveLogToFile(); // Call the method to save the log
            }

        }
        ImGui::End();
    }

    void SaveLogToFile() {
        // Get current time
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);

        // Convert time to tm struct
        std::tm now_tm = *std::localtime(&now_time_t);

        // Format time into a string with the format "debug_log{YYYY-MM-DD_HH-MM-SS}.txt"
        char filename[80];
        strftime(filename, sizeof(filename), "DebugLogs\\debug_log%Y-%m-%d_%H-%M-%S.txt", &now_tm);
        
        // Proceed with saving the log to the file
        std::ofstream fileStream(filename, std::ios::out | std::ios::app);
        if (!fileStream.is_open()) {
            std::cerr << "Failed to open log file for writing: " << filename << std::endl;
            return;
        }

        for (const auto& msg : messages) {
            fileStream << msg << std::endl;
        }

        fileStream.close();
    }

private:
    std::vector<std::string> messages;
};