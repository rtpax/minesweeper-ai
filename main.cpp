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
    ms::solver ai(9,13,32);

    ai.manual_open(ms::rc_coord{5,5});
    printms(ai.view_grid());

    std::string line;
    std::string row;
    std::string col;
    

    while(ai.view_grid().gamestate() == ms::grid::RUNNING) {
        std::cout << "enter input of the form 'row, column':\n";

        std::getline(std::cin, line);

        if(line == "sweep") {
            int opened = ai.solve_certain();
            std::cout << opened << " cells opened\n";
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
                std::cout << "invalid entry: " << e.what() << "\n";
                parse_fail = true;
            }

            if(parse_fail) {
                ms::rc_coord opened = ai.step_certain();
                std::cout << "solver opening cell (" << opened.row << "," << opened.col << ")\n";
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

/*
I don't know how it (correctly) determined the flag at (4,1).

 --------------------------
|               F 1 0 0 0 0 |
|   5     2     3 2 0 1 1 1 |
|     2   F 2 3 F 4 2 2 F 1 |
|     3 2 3 2 3 F F F 4 4 3 |
|   F 3   2 F 2 2 3 4 F F F |
|     3   3 3 4 2 1 2 F F 3 |
|     2 1 2 F F F 1 1 2 2 1 |
|   3 2   2 2 3 2 1 1 1 2 1 |
|         1 0 0 0 0 1 F 2 F |
 --------------------------
 */

