@echo off
if not exist "build" mkdir build
cl /EHsc /W4 /WX- /std:c++17 /Fe:"build\UDPOptimizer.exe" main.cpp network_optimizer.cpp /link user32.lib gdi32.lib comctl32.lib
