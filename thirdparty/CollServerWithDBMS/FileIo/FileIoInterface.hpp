#pragma once

template<typename T>
class FileIoInterface {
public:
    virtual ~FileIoInterface() = default;

    virtual void ReadFromFile() = 0;

    virtual bool WriteToFile() = 0;

    virtual void setValue(T&value) = 0;

    virtual T& getValue() = 0;
};
