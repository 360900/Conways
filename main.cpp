/*
    author: @fikret0
    date: November 13, 2021

    GitHub: @fikret0
*/

#include <bits/stdc++.h>
#include <SFML/Graphics.hpp>

#define WIDTH 700
#define HEIGHT 700

#define XCELLS 35
#define YCELLS 35

int cellw = WIDTH / XCELLS, cellh = HEIGHT / YCELLS;

std::map<std::pair<int, int>, int> grid;

int camX = 0, camY = 0;

sf::RenderWindow mwindow(sf::VideoMode({WIDTH, HEIGHT}), "Conway's Game of Life");

bool darkmode = false;

int getneighbours(int ln, int col){
    int c = 0;
    for(int y = ln - 1; y <= ln + 1; y++){
        for(int x = col - 1; x <= col + 1; x++){
            if(x == col && y == ln) continue;
            c += grid[{x, y}];
        }
    }

    return c;
}

float defaultdelay = 0.5f;
float fastdelay = 0.05f;
float delay = defaultdelay;

sf::Clock gameclock;

enum Direction { STOPPED, FORWARD, BACKWARD };
Direction direction = STOPPED;

std::vector<std::map<std::pair<int, int>, int>> history;
int histPos = 0;
int histBase = 0;

void pushHistory(const std::map<std::pair<int, int>, int>& state){
    history.resize(histPos + 1);
    history.push_back(state);
    histPos++;
}

std::map<std::pair<int, int>, int> computeNext(){
    std::set<std::pair<int, int>> active;
    for(const auto& kv : grid){
        if(kv.second){
            for(int y = kv.first.second - 1; y <= kv.first.second + 1; y++){
                for(int x = kv.first.first - 1; x <= kv.first.first + 1; x++){
                    active.insert({x, y});
                }
            }
        }
    }

    std::map<std::pair<int, int>, int> tmp = grid;
    for(const auto& p : active){
        int col = grid[p];
        int nb = getneighbours(p.second, p.first);

        if(col == 1){
            if(nb < 2 || nb > 3){
                tmp[p] = 0;
            }
        }
        else{
            if(nb == 3){
                tmp[p] = 1;
            }
        }
    }

    return tmp;
}

void timemanager(){
    float elapsed = gameclock.getElapsedTime().asSeconds();

    if (elapsed >= delay){
        if(direction == FORWARD){
            grid = computeNext();
            pushHistory(grid);
        }
        else if(direction == BACKWARD){
            if(histPos > histBase){
                histPos--;
                grid = history[histPos];
            }
            else{
                mwindow.setTitle("Conway's Game of Life - Start of History");
            }
        }

        gameclock.restart();
    }
}

void cleargrid(){
    grid.clear();
    histBase = histPos;
    history[histBase] = grid;
}

void togglecell(int ln, int col){
    std::pair<int, int> p = {col, ln};
    if(grid[p]){
        grid.erase(p);
    }
    else{
        grid[p] = 1;
    }
}

void setcell(int ln, int col, int v){
    std::pair<int, int> p = {col, ln};
    if(v) grid[p] = 1;
    else grid.erase(p);
    history[histPos] = grid;
}

bool painting = false;
int paintTarget = 1;
std::pair<int, int> lastPaint = {0, 0};

int main(int argc, char **argv){
    std::cout << "GitHub: @fikret0 (c) 2021" << std::endl << "Conway's Game of Life" << std::endl << "--dark for dark mode" << std::endl << std::endl
        << "Space: Play / pause (forward)" << std::endl
        << "Z: Play backwards / pause" << std::endl
        << "C: Clear the grid" << std::endl
        << "Tab: Fast forward" << std::endl
        << "Arrows: Navigate canvas" << std::endl;

    for(int ai = 0; ai < argc; ai++){
        std::string arg = argv[ai];
        if(arg == "--dark"){
            darkmode = true;
        }
    }

    sf::Color gridcolor(200, 200, 200);
    sf::Color cellcolor(0, 0, 0);
    sf::Color bgcolor(255, 255, 255);

    if(darkmode){
        gridcolor = sf::Color(55, 55, 55);
        cellcolor = sf::Color(255, 255, 255);
        bgcolor = sf::Color(0, 0, 0);
    }

    history.push_back(grid);
    histPos = 0;
    histBase = 0;

    while (mwindow.isOpen()){
        while (const auto event = mwindow.pollEvent()){
            if (event->is<sf::Event::Closed>()){
                mwindow.close();
            }
            else if(const auto* key = event->getIf<sf::Event::KeyPressed>()){
                if(key->code == sf::Keyboard::Key::Space){ // play / pause forward
                    if(direction == BACKWARD) continue; // must stop reverse with Z first
                    if(direction == FORWARD) direction = STOPPED;
                    else direction = FORWARD;
                    mwindow.setTitle(direction == FORWARD ? "Conway's Game of Life - Playing" : "Conway's Game of Life");
                    gameclock.restart();
                }
                else if(key->code == sf::Keyboard::Key::Z){ // play backwards
                    if(direction == FORWARD) continue; // can't reverse while going forwards
                    if(direction == BACKWARD) direction = STOPPED;
                    else direction = BACKWARD;
                    mwindow.setTitle(direction == BACKWARD ? "Conway's Game of Life - Reversing" : "Conway's Game of Life");
                    gameclock.restart();
                }
                else if(key->code == sf::Keyboard::Key::C){ // clear
                    cleargrid();
                }
                else if(key->code == sf::Keyboard::Key::Tab){ // fast forward
                    if(delay != fastdelay){
                        delay = fastdelay;
                        mwindow.setTitle("Conway's Game of Life - Fast Forward");
                    }
                }
                else if(key->code == sf::Keyboard::Key::Up){
                    camY -= 1;
                }
                else if(key->code == sf::Keyboard::Key::Down){
                    camY += 1;
                }
                else if(key->code == sf::Keyboard::Key::Left){
                    camX -= 1;
                }
                else if(key->code == sf::Keyboard::Key::Right){
                    camX += 1;
                }
            }
            else if(const auto* key = event->getIf<sf::Event::KeyReleased>()){
                if(key->code == sf::Keyboard::Key::Tab){ // restore default speed
                    delay = defaultdelay;
                    mwindow.setTitle("Conway's Game of Life");
                }
            }
            else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()){
                if (mouse->button == sf::Mouse::Button::Left){
                    int col = floor(mouse->position.x / cellw) + camX;
                    int ln = floor(mouse->position.y / cellh) + camY;

                    painting = true;
                    paintTarget = grid[{col, ln}] ? 0 : 1;
                    lastPaint = {col, ln};
                    histBase = histPos;
                    history[histBase] = grid;
                    gameclock.restart();
                    setcell(ln, col, paintTarget);
                }
            }
            else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()){
                if (mouse->button == sf::Mouse::Button::Left){
                    painting = false;
                }
            }
            else if (const auto* mouse = event->getIf<sf::Event::MouseMoved>()){
                if (painting){
                    int col = floor(mouse->position.x / cellw) + camX;
                    int ln = floor(mouse->position.y / cellh) + camY;

                    if (lastPaint != std::make_pair(col, ln)){
                        lastPaint = {col, ln};
                        gameclock.restart();
                        setcell(ln, col, paintTarget);
                    }
                }
            }
        }

        mwindow.clear(bgcolor);

        if(direction != STOPPED){
            timemanager();
        }
        else{
            gameclock.restart();
        }

        float th = 1.f;

        int startX = camX - 1;
        int startY = camY - 1;
        int endX = camX + XCELLS + 1;
        int endY = camY + YCELLS + 1;

        /* Draw the grid */
        for(int xc = startX; xc <= endX; xc++){
            int c = (xc - camX) * cellw;

            sf::RectangleShape line;
            line.setPosition(sf::Vector2f(c, 0));
            line.setSize(sf::Vector2f(th, HEIGHT));

            line.setFillColor(gridcolor);
            line.setOutlineThickness(0);

            mwindow.draw(line);
        }

        for(int yc = startY; yc <= endY; yc++){
            int c = (yc - camY) * cellh;

            sf::RectangleShape line;
            line.setPosition(sf::Vector2f(0, c));
            line.setSize(sf::Vector2f(WIDTH, th));

            line.setFillColor(gridcolor);
            line.setOutlineThickness(0);

            mwindow.draw(line);
        }

        /* Draw the cells */
        for(const auto& kv : grid){
            if(!kv.second) continue;

            int cx = kv.first.first;
            int cy = kv.first.second;

            if(cx < camX || cx >= camX + XCELLS || cy < camY || cy >= camY + YCELLS){
                continue;
            }

            sf::RectangleShape cell;
            cell.setPosition(sf::Vector2f((cx - camX) * cellw + th, (cy - camY) * cellh + th));
            cell.setSize(sf::Vector2f(cellw - th, cellh - th));

            cell.setFillColor(cellcolor);
            cell.setOutlineThickness(0);

            mwindow.draw(cell);
        }

        mwindow.display();
    }
}
