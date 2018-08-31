#include "grid.h"
#include "solver.h"
#include <stdio.h>
#include <iostream>
#include <stdexcept>
#include <sstream>

char cell_to_char(ms::grid::cell c) {
    switch(c) {
    case ms::grid::ms_0:
        return '0';
    case ms::grid::ms_1:
        return '1';
    case ms::grid::ms_2:
        return '2';
    case ms::grid::ms_3:
        return '3';
    case ms::grid::ms_4:
        return '4';
    case ms::grid::ms_5:
        return '5';
    case ms::grid::ms_6:
        return '6';
    case ms::grid::ms_7:
        return '7';
    case ms::grid::ms_8:
        return '8';
    case ms::grid::ms_hidden:
        return ' ';
    case ms::grid::ms_bomb:
        return '*';
    case ms::grid::ms_flag:
        return 'F';
    case ms::grid::ms_question:
        return '?';
    default:
        return '#';
    }
}

void printms(const ms::grid& g) {
    printf(" ");
    for(unsigned int w = 0; w < g.width(); ++w) {
        printf("--");
    }
    printf("  \n");

    for(unsigned int h = 0; h < g.height(); ++h) {
        printf("|");
        for(unsigned int w = 0; w < g.width(); ++w) {
            printf(" %c",cell_to_char(g.get(h, w)));
        }
        printf(" |\n");
    }

    printf(" ");
    for(unsigned int w = 0; w < g.width(); ++w) {
        printf("--");
    }
    printf("  \n");
}

int main(int argc, char ** argv) { try {
    std::cout << time(NULL) << "\n";
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

    ms::solver ai(height, width, bombs);

    std::string line;
    std::string row;
    std::string col;


    while(ai.view_grid().gamestate() == ms::grid::RUNNING || ai.view_grid().gamestate() == ms::grid::NEW) {
        std::cout << "enter input of the form 'row, column':\n";

        std::getline(std::cin, line);

        if(line == "sweep") {
            int opened = ai.solve_certain();
            std::cout << opened << " steps taken\n";
        } else if (line == "print") {
            printms(ai.view_grid());
        } else if (line == "solve") {
            int opened = ai.solve();
            std::cout << opened << " steps taken\n";
        } else if (line == "step") {
            ms::rc_coord opened = ai.step();
            if(opened != ms::BAD_RC_COORD)
                std::cout << "solver opened cell (" << opened.row << "," << opened.col << ")\n";
            else
                std::cout << "no cells opened\n";
        } else {
            int r, c;
            bool parse_fail = false;

            std::stringstream ss(line);
            std::getline(ss,row,',');
            std::getline(ss,col,',');

            try {
                r = std::stoi(row);
                c = std::stoi(col);
                std::cout << "manual openening cell (" << r << "," << c << ")\n";
            } catch (std::invalid_argument e) { //let the ai decide
                parse_fail = true;
            }

            if(parse_fail) {
                ms::rc_coord opened = ai.step_certain();
                if(opened != ms::BAD_RC_COORD)
                    std::cout << "solver opened cell (" << opened.row << "," << opened.col << ")\n";
                else
                    std::cout << "no cells opened\n";
            }
            if(!parse_fail) {
                try {
                    ai.manual_open(ms::rc_coord{(unsigned int)r,(unsigned int)c});
                } catch (std::exception e) {
                    std::cout << "could not open cell (" << r << "," << c << ")\n";
                }
            }
        }
    }

    printms(ai.view_grid());

    switch(ai.view_grid().gamestate()) {
    case ms::grid::WON:
        std::cout << "WON\n";
        break;
    case ms::grid::LOST:
        std::cout << "LOST\n";
        break;
    case ms::grid::RUNNING:
        std::cout << "RUNNING\n";
        break;
    case ms::grid::NEW:
        std::cout << "NEW\n";
        break;
    default:
        std::cout << "gamestate error\n";
    }

    return 0;
} catch(const ms::bad_region_error& e) {
    std::cerr << e.what();
    return 1;
}}

