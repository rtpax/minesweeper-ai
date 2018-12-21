#include "ui.h"
#include <ncurses.h>
#include <thread>
#include <stdexcept>
#include <iostream>
#include <signal.h>

namespace ms {

    static solver* active_window = nullptr;
    static bool window_started = false;
    static int cursorx = 0;
    static int cursory = 0;
    static bool had_stopped = false; //redrawing within the sigcont handler failed, this is a workaround

    static bool use_color = false;

    void ui_use_color(bool color) {
        use_color = color;
    }

    bool ui_using_color() {
        return use_color;
    }

    static char cell_to_char(ms::grid::cell c) {
        switch(c) {
        case ms::grid::ms_0:
            return '.';
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
            return 'x';
        case ms::grid::ms_unopened_bomb:
            return '*';
        case ms::grid::ms_flag:
            return 'F';
        case ms::grid::ms_question:
            return '?';
        default:
            return '#';
        }
    }

    static void set_cell_color(ms::grid::cell c) {
        if(!use_color) {
            attron(COLOR_PAIR(100));
            return;
        }

        switch(c) {
        case ms::grid::ms_0:
            attron(COLOR_PAIR(6));
            break;
        case ms::grid::ms_1:
            attron(COLOR_PAIR(1));
            break;
        case ms::grid::ms_2:
            attron(COLOR_PAIR(2));
            break;
        case ms::grid::ms_3:
            attron(COLOR_PAIR(3));
            break;
        case ms::grid::ms_4:
            attron(COLOR_PAIR(4));
            break;
        case ms::grid::ms_5:
            attron(COLOR_PAIR(5));
            break;
        case ms::grid::ms_6:
            attron(COLOR_PAIR(6));
            break;
        case ms::grid::ms_7:
            attron(COLOR_PAIR(7));
            break;
        case ms::grid::ms_8:
            attron(COLOR_PAIR(8));
            break;
        case ms::grid::ms_hidden:
            attron(COLOR_PAIR(100));
            break;
        case ms::grid::ms_bomb:
            attron(COLOR_PAIR(10));
            break;
        case ms::grid::ms_unopened_bomb:
            attron(COLOR_PAIR(100));
            break;
        case ms::grid::ms_flag:
            attron(COLOR_PAIR(100));
            break;
        case ms::grid::ms_question:
            attron(COLOR_PAIR(100));
            break;
        default:
            attron(COLOR_PAIR(9));
        }
    }

    static void go_to_cursor() {
        int maxx = getmaxx(stdscr);
        int maxy = getmaxy(stdscr);
        int height = active_window->height();
        int width = active_window->width();
        int basex = (maxx - (width * 2 + 3)) / 2;
        int basey = (maxy - (height + 2)) / 2 + 1;
        wmove(stdscr,basey + cursory + 1,basex + 2*cursorx + 2);
    }

    static void draw_ui(bool redraw) {
        int maxx = getmaxx(stdscr);
        int maxy = getmaxy(stdscr);
        int height = active_window->height();
        int width = active_window->width();

        attron(COLOR_PAIR(100));

        if(maxx <  width * 2 + 2 || maxy < height + 2 - 1 + 1) {
            clear();
            mvaddstr(0, 0, "Window too small to display grid");
            return;
        }
        int basex = (maxx - (width * 2 + 3)) / 2;
        int basey = (maxy - (height + 2)) / 2 + 1;
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
        mvaddstr(basey-1,basex,"    ");
        mvaddstr(basey-1,basex,std::to_string(active_window->remaining_bombs()).c_str());
        char ch = '#';
        switch(active_window->gamestate()) {
        case grid::RUNNING:
            ch = 'R';
            break;
        case grid::WON:
            ch = 'W';
            break;
        case grid::LOST:
            ch = 'L';
            break;
        case grid::NEW:
            ch = 'N';
            break;
        }
        mvaddch(basey-1,basex + width + 1,ch);
        for(int row = 0; row < height; ++row) {
            for(int col = 0; col < width; ++col) {
                set_cell_color(active_window->get(row,col));
                mvaddch(basey + row + 1, basex + col * 2 + 2, cell_to_char(active_window->get(row,col)));
            }
        }
        attron(COLOR_PAIR(100));
        wmove(stdscr,basey + cursory + 1,basex + 2*cursorx + 2);
        refresh();
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
    static void on_cont(int) {
        had_stopped = true;
    }


    static void ui_loop() {
        using clock = std::chrono::high_resolution_clock;
        using ms = std::chrono::milliseconds;
        clock::time_point cycle_time;

        while(1) {
            if(had_stopped) {
                draw_ui(true);
                had_stopped = false;
            }
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
                draw_ui(true);
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
                cycle_time = clock::now() + ms(10);
                while(active_window->step_certain() != BAD_RC_COORD) {
                    if(getch() != ERR)
                        break;
                    if(clock::now() > cycle_time) {
                        cycle_time = clock::now() + ms(10);
                        draw_ui(false);
                    }
                }
                draw_ui(false);
                break;
            case 'x':
                active_window->step();
                draw_ui(false);
                break;
            case 'X':
                cycle_time = clock::now() + ms(10);
                while(active_window->step() != BAD_RC_COORD) {
                    if(getch() != ERR)
                        break;
                    if(clock::now() > cycle_time) {//displaying too often slows program down
                        cycle_time = clock::now() + ms(10);
                        draw_ui(false);
                    }
                }
                draw_ui(false);
                break;
            case 'c':
            case 'C':
                use_color = !use_color;
                draw_ui(false);
                break;
            case 'f':
            case 'F':
                if(active_window->gamestate() != grid::NEW && active_window->gamestate() != grid::RUNNING)
                    break; 
                active_window->manual_flag(rc_coord(cursory, cursorx));
                draw_ui(false);
                break;
            case 'g':
            case 'G':
                if(active_window->gamestate() != grid::NEW && active_window->gamestate() != grid::RUNNING)
                    break; 
                active_window->manual_unflag(rc_coord(cursory, cursorx));
                draw_ui(false);
                break;
            case 'o':
            case 'O':
            case '\n':
                if(active_window->gamestate() != grid::NEW && active_window->gamestate() != grid::RUNNING)
                    break;
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
            signal(SIGCONT, &on_cont);
            window_started = true;
        }
        if(active_window == nullptr)
            active_window = new solver(width,height,bombs);
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, true);
        start_color();

        init_pair(1, COLOR_BLUE, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_RED, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6, COLOR_YELLOW, COLOR_BLACK);
        init_pair(7, COLOR_CYAN, COLOR_BLACK);
        init_pair(8, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(9, COLOR_BLACK, COLOR_RED);
        init_pair(10, COLOR_BLACK, COLOR_WHITE);

        init_pair(100, COLOR_WHITE, COLOR_BLACK);

        if(nodelay(stdscr, true) == ERR)
            throw std::runtime_error("nodelay error");
        draw_ui(true);
        try {
            ui_loop();
        } catch (std::exception& e) {
            endwin();
            std::cout << "Encountered an error: " << e.what() << "\n";
            return;
        }
        endwin();
    }

}
