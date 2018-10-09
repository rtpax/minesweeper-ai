#include "grid.h"
#include "solver.h"
#include "ui.h"
#include <stdio.h>
#include <iostream>
#include <stdexcept>
#include <sstream>

int main(int argc, char ** argv) {
    int height = 0, width = 0, bombs = 0;

    if(argc > 3) {
        try {
            height = std::stoi(argv[1]);
            width  = std::stoi(argv[2]);
            bombs  = std::stoi(argv[3]);
        } catch (std::exception) {
            goto bad_arguments;
        }
    } else {
    bad_arguments:
        height = 9;
        width = 9;
        bombs = 10;
    }

    ms::start_ui(height, width, bombs);
}