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


int main() {
    ms::solver ai(9,9,10);

    ai.manual_open(ms::rc_coord{5,5});
    printms(ai.view_grid());

    std::string line;
    std::string row;
    std::string col;

    while(ai.view_grid().gamestate() == ms::grid::RUNNING) {
        ms::rc_coord opened = ai.step_certain();


        if(opened == ms::rc_coord{0xffff,0xffff}) {
            std::cout << "enter input of the form 'row, column':\n";

            std::getline(std::cin, line);
            std::stringstream ss(line);
            std::getline(ss,row,',');
            std::getline(ss,col,',');

            int r, c;
            bool parse_fail = false;

            try {
                r = std::stoi(row);
                c = std::stoi(col);
                std::cout << "openening cell (" << r << "," << c << ")\n";
            } catch (std::invalid_argument e) { //let the ai decide
                std::cout << "invalid entry: " << e.what() << "\n";
                parse_fail = true;
            }

            if(!parse_fail) {
                try {
                    ai.manual_open(ms::rc_coord{(unsigned int)r,(unsigned int)c});
                } catch (std::exception e) {
                    std::cout << "error: Could not open the cell -- " << e.what() << "\n";
                    return 1;
                }
            }
        }




        
        printms(ai.view_grid());
    }

    return 0;
}



