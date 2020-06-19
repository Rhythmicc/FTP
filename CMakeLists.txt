cmake_minimum_required(VERSION 3.16)
project(FTP C)

set(CMAKE_C_STANDARD 11)

add_executable(FtpServer FtpServer/main.c FtpServer protocol utils configure common.h priv)
add_executable(FtpClient FtpClient/main.c FtpClient protocol utils configure common.h priv)