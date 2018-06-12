#include "grid.h"
#include "stdio.h"

char cell_to_char(ms::cell c) {
    switch(c) {
    case ms::ms_0:
        return '0';
    case ms::ms_1:
        return '1';
    case ms::ms_2:
        return '2';
    case ms::ms_3:
        return '3';
    case ms::ms_4:
        return '4';
    case ms::ms_5:
        return '5';
    case ms::ms_6:
        return '6';
    case ms::ms_7:
        return '7';
    case ms::ms_8:
        return '8';
    case ms::ms_hidden:
        return ' ';
    case ms::ms_bomb:
        return '*';
    case ms::ms_flag:
        return 'F';
    case ms::ms_question:
        return '?';
    default:
        return '#';
    }
}

void printms(const ms::grid * const g) {
    printf(" ");
    for(int w = 0; w < g->width(); ++w) {
        printf("--");
    }
    printf("  \n");

    for(int h = 0; h < g->height(); ++h) {
        printf("|");
        for(int w = 0; w < g->width(); ++w) {
            printf(" %c",cell_to_char(g->get(h, w)));
        }
        printf(" |\n");
    }

    printf(" ");
    for(int w = 0; w < g->width(); ++w) {
        printf("--");
    }
    printf("  \n");
}


int main() {
    ms::grid mine(9,9,10);
    printf("%d\n", mine.open(3,2));
    printf("%d\n", mine.open(2,2));
    printms(&mine);
}



