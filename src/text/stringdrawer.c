#include "header/text/stringdrawer.h"

uint16_t char_to_color(char c){
    if(c == 'a') return Green;
    switch (c)
    {
        case ' ': return Magenta;
        case '.': return White;
        case '@': return Black;
        case '#': return LightMagenta;
        case ',': return Yellow;
        case '+': return Yellow;
    }
    return White;
}
void drawBg(uint16_t bgColor, int y, int x){
    framebuffer_write(y, x, ' ', Black, bgColor);
}
void drawBgFromStringList(const char* stringList[]){
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            drawBg(char_to_color(stringList[i][j]), i, j);
        }
    }
}

void drawTextWhite(char* string, int y, int x){
    for(int i = 0; string[i] != '\0'; i++){
        framebuffer_write(y, x + i, string[i], White, Black);
    }
}


const char *welcomeScreen[] = {
"                                             @                                  ",
"                                           @@@                                  ",
"                                     @@@@@@@@                                   ",
"                    @@@       @@@@@@@@@@@@@@@@@@       @@                       ",
"                      @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                        ",
"                       @@@@@@@@@@@@@@@@@@@..##..@@@@@@@@@                       ",
"                      @@@@@@@@@@@@@@@@@@@.........@@@@@@@ @@                    ",
"                  @@@@@@@@@@@@@@@@@@@@@@@..@@@@@@@@@@@@@@@@                     ",
"                 @@@@@@@@@@@@@@@@@@@@@@@@@@@@.....@@@@@@@                       ",
"             @@@@@@@@@@@@@@@@@.........@@@@...##....@@@@@                       ",
"           @@@@@@@@@@@@@@@@@...@@@@.@...@@...##....@@@@@@                       ",
"               @@@....@@@@...@.......@...@@....@@@@@@@@@@@@                     ",
"               @@...@@@.....@...##..@....@@@@@@@....@@@@@@@@@                   ",
"               .......@@.....@@@@@@@....@@@@...##..@@@@                         ",
"                 ......@@.................@@@@..@@@@@                           ",
"                 @@@@@@@@@....@......@@@...@@@@@@@@..                           ",
"               @@@@@@@@@@@@@...@@@@@@...............                            ",
"                  @@@@@@@@@   ...........                                       ",
"                                                                                ",
"                                     +++     ++++                               ",
"                                   +     +  +      +                            ",
"                           ++ ++  +       +  ++++  + +                          ",
"                          +  +  +  +     +       + ++                           ",
"                          +     +    +++     ++++  + +                          ",
"                                                                                ",
}
;
void drawWelcomeScreen() {
    drawBgFromStringList(welcomeScreen);
}