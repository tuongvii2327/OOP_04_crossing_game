#pragma once
#include <stdexcept>
#include <string>

// Lớp Exception cơ sở cho toàn bộ Game
class GameException : public std::runtime_error {
public:
    explicit GameException(const std::string& msg) : std::runtime_error("[Game Error] " + msg) {}
};

// ==========================================
// Nhóm lỗi về Tài nguyên (Assets)
// ==========================================
class ResourceException : public GameException {
public:
    explicit ResourceException(const std::string& msg) : GameException("[Resource] " + msg) {}
};

class TextureLoadException : public ResourceException {
public:
    explicit TextureLoadException(const std::string& path)
        : ResourceException("Failed to load Texture from: " + path) {}
};

class FontLoadException : public ResourceException {
public:
    explicit FontLoadException(const std::string& path)
        : ResourceException("Failed to load Font from: " + path) {}
};

class AudioLoadException : public ResourceException {
public:
    explicit AudioLoadException(const std::string& path)
        : ResourceException("Failed to load Audio from: " + path) {}
};

// ==========================================
// Nhóm lỗi về Đọc/Ghi dữ liệu (Save/Load)
// ==========================================
class SaveLoadException : public GameException {
public:
    explicit SaveLoadException(const std::string& msg) : GameException("[Save/Load] " + msg) {}
};

class CorruptedSaveException : public SaveLoadException {
public:
    explicit CorruptedSaveException(int slot)
        : SaveLoadException("Save data corrupted or invalid checksum at slot " + std::to_string(slot)) {}
};

// ==========================================
// Nhóm lỗi về Quản lý trạng thái (State Machine)
// ==========================================
class StateException : public GameException {
public:
    explicit StateException(const std::string& msg) : GameException("[StateMachine] " + msg) {}
};

class EmptyStateException : public StateException {
public:
    explicit EmptyStateException(const std::string& msg)
        : StateException("Stack is empty: " + msg) {}
};
