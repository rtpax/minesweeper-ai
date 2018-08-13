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

int notmain() {
    ms::region a;
    a.addcell(ms::rc_coord{1,2});
    a.addcell(ms::rc_coord{3,4});
    a.addcell(ms::rc_coord{5,6});
    a.set_range(1,2);

    ms::region b = a;

	for(unsigned int i = 0; i < a.size(); ++i)
		printf("(%d,%d)",a[i].row,a[i].col);
    printf("\nmin: %d\nmax: %d\n",a.min(),a.max());    

    for(unsigned int i = 0; i < b.size(); ++i)
		printf("(%d,%d)",b[i].row,b[i].col);
    printf("\nmin: %d\nmax: %d\n",b.min(),b.max());

    printf("samearea1: %s\n",a.samearea(b) ? "true" : "false");
    printf("samearea2: %s\n",b.samearea(a) ? "true" : "false");

    printf("a == b: %s\n",a == b ? "true" : "false");    
    printf("b == a: %s\n",b == a ? "true" : "false"); 

    return 0;
}

int main() {
    ms::solver ai(9,10,20);

    std::string line;
    std::string row;
    std::string col;
    

    while(ai.view_grid().gamestate() == ms::grid::RUNNING || ai.view_grid().gamestate() == ms::grid::NEW) {
        //printms(ai.view_grid());

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
            if(opened != ms::rc_coord{0xffff,0xffff})
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
                if(opened != ms::rc_coord{0xffff,0xffff})
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

    return 0;
}

