#include "ui.h"
#include <ncurses/ncurses.h>
#include <thread>
#include <stdexcept>
#include <iostream>

namespace ms {

    static solver* active_window = nullptr;
    static bool window_started = false;
    static int cursorx = 0;
    static int cursory = 0;

    static char cell_to_char(ms::grid::cell c) {
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

    static void go_to_cursor() {
        int maxx = getmaxx(stdscr);
        int maxy = getmaxy(stdscr);
        int height = active_window->height();
        int width = active_window->width();
        int basex = (maxx - (width * 2 + 3)) / 2;
        int basey = (maxy - (height + 2)) / 2;
        wmove(stdscr,basey + cursory + 1,basex + 2*cursorx + 2);
    }

    static void draw_ui(bool redraw) {
        int maxx = getmaxx(stdscr);
        int maxy = getmaxy(stdscr);
        int height = active_window->height();
        int width = active_window->width();

        if(maxx <  width * 2 + 2 || maxy < height + 2 - 1) {
            clear();
            mvaddstr(0, 0, "Window too small to display grid");
            return;
        }
        int basex = (maxx - (width * 2 + 3)) / 2;
        int basey = (maxy - (height + 2)) / 2;
        if(redraw) {
            clear();
            for(int row = 0; row < height; ++row) {
                mvaddch(basey + row + 1,basex,'|');
                mvaddch(basey + row + 1,basex + 2 * width + 2,'|');
            }
            for(int col = 0; col < 2 * width + 1; ++col) {
                mvaddch(basey, basex + col + 1, '-');
                mvaddch(basey + height + 1, basex + col + 1, '-');
            }
        }
        for(int row = 0; row < height; ++row) {
            for(int col = 0; col < width; ++col) {
                mvaddch(basey + row + 1, basex + col * 2 + 2, cell_to_char(active_window->get(row,col)));
            }
        }
        wmove(stdscr,basey + cursory + 1,basex + 2*cursorx + 2);
    }

    static void on_interrupt(int arg) {
        endwin();
        exit(arg);
    }
    static void on_resize(int) {
        endwin();
        refresh();
        clear();
        if(active_window != nullptr) {
            draw_ui(true);
        }
    }


    static void ui_loop() {
        while(1) {
            int ch = getch();
            switch(ch) {
            case ERR:
                std::this_thread::yield();
                break;
            case 'q':
            case 'Q':
                delete active_window;
                active_window = nullptr;
                return;
            case 'r':
            case 'R':
                {
                    solver* temp = active_window;
                    active_window = new solver(temp->height(), temp->width(), temp->bombs());
                    delete temp;
                }
                draw_ui(false);
                break;
            case KEY_DOWN:
                if(cursory + 1 < (int)active_window->height())
                    ++cursory;
                else
                    cursory = active_window->height() - 1;
                go_to_cursor();
                break;
            case KEY_UP:
                if(cursory - 1 > 0)
                    --cursory;
                else
                    cursory = 0;
                go_to_cursor();
                break;
            case KEY_RIGHT:
                if(cursorx + 1 < (int)active_window->width())
                    ++cursorx;
                else
                    cursorx = active_window->width() - 1;
                go_to_cursor();
                break;
            case KEY_LEFT:
                if(cursorx - 1 > 0)
                    --cursorx;
                else
                    cursorx = 0;
                go_to_cursor();
                break;
            case 'z':
                active_window->step_certain();
                draw_ui(false);
                break;
            case 'Z':
                active_window->solve_certain();
                draw_ui(false);
                break;
            case 'x':
                active_window->step();
                draw_ui(false);
                break;
            case 'X':
                active_window->solve();
                draw_ui(false);
                break;
            case 'f':
            case 'F':
                active_window->manual_flag(rc_coord(cursory, cursorx));
                draw_ui(false);
                break;
            case 'o':
            case 'O':
            case '\n':
                active_window->manual_open(rc_coord(cursory, cursorx));
                draw_ui(false);
                break;
            }
        }
    }

    void start_ui(int height, int width, int bombs) {
        if(!window_started) {
            signal(SIGINT, &on_interrupt);
            signal(SIGWINCH, &on_resize);
            window_started = true;
        }
        if(active_window == nullptr)
            active_window = new solver(width,height,bombs);
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, true);
        if(nodelay(stdscr, true) == ERR)
            throw std::runtime_error("nodelay error");
        draw_ui(true);
        try {
            ui_loop();
        } catch (std::exception&) {
            endwin();
            std::cout << "Encountered an error\n";
            return;
        }
        endwin();
    }

}