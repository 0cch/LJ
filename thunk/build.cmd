@echo off
set path=%path%;E:\make_your_compiler\bin
win_flex -o lj_scanner.cpp lj_scanner.ll
win_bison -o lj_parser.cpp lj_parser.yy