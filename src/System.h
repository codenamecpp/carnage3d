#pragma once

// Common system specific stuff collected in System class
class System final: public cxx::noncopyable
{
public:
    // Initialize game subsystems and run main loop
    void Run(int argc, char *argv[]);

    // Abnormal application shutdown due to critical failure
    void Terminate();

    // Set application exit request flag, execution will be interrupted soon
    void QuitRequest();

    // Get real time seconds since system started
    double GetSystemSeconds() const;

private:
    void Initialize(int argc, char *argv[]);
    void Deinit(bool isTermination);
    bool ExecuteFrame();
    void ParseStartupParams(int argc, char *argv[]);

    // Save/Load configuration to/from external file
    bool LoadConfiguration();
    bool SaveConfiguration();

private:
    bool mQuitRequested;
};

extern System gSystem;