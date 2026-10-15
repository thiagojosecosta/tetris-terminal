#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

#define w 10
#define h 20

int board[h][w];
int shapes[7][4][2] = {
    {{-1, 0}, {0, 0}, {1, 0}, {2, 0}},
    {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
    {{-1, 0}, {0, 0}, {1, 0}, {0, 1}},
    {{-1, 0}, {0, 0}, {1, 0}, {-1, 1}},
    {{-1, 0}, {0, 0}, {1, 0}, {1, 1}},
    {{0, 0}, {1, 0}, {-1, 1}, {0, 1}},
    {{-1, 0}, {0, 0}, {0, 1}, {1, 1}}};
int cur[4][2];
int kind;
int next_kind;
int show_help = 1;
int score = 0;
int lines = 0;
int best = 0; 
int px = 4;
int py = 0;
int ox = 1;
int oy = 1;

int is_piece(int x, int y)
{
    for (int i = 0; i < 4; i++)
    {
        if (px + cur[i][0] == x && py + cur[i][1] == y)
            return 1;
    }
    return 0;
}

int fits(int p[4][2], int x, int y)
{
    for (int i = 0; i < 4; i++)
    {
        int cx = x + p[i][0];
        int cy = y + p[i][1];

        if (cx < 0 || cx >= w)
            return 0;
        if (cy >= h)
            return 0;
        if (cy >= 0 && board[cy][cx])
            return 0;
    }
    return 1;
}

void rotate(void)
{
    if (kind == 1) return;

    int n[4][2];

    for (int i = 0; i < 4; i++)
    {
        n[i][0] = -cur[i][1];
        n[i][1] = cur[i][0];
    }

    if (fits(n, px, py))
    {
        for (int i = 0; i < 4; i++)
        {
            cur[i][0] = n[i][0];
            cur[i][1] = n[i][1];
        }
    }
}

void new_piece(void)
{
    kind = next_kind;
    next_kind = rand() % 7;

    for (int i = 0; i < 4; i++)
    {
        cur[i][0] = shapes[kind][i][0];
        cur[i][1] = shapes[kind][i][1];
    }

    px = w / 2 - 1;
    py = 0;
}

void prepare_terminal(void)
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(console, &mode);
    SetConsoleMode(console, mode | 0X0004);
}

void goto_xy(int x, int y)
{
    printf("\x1b[%d;%dH", y, x);
}

void center_well(void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

    int cols = info.srWindow.Right - info.srWindow.Left + 1;
    int rows = info.srWindow.Bottom - info.srWindow.Top +1;

    ox = (cols - 24) / 2 + 1;
    oy = (rows - 21) / 2 + 1;

    if(ox < 1) ox = 1;
    if(oy < 1) oy = 1;

}

void lock_piece(void)
{
    for (int i = 0; i < 4; i++)
    {
        int y = py + cur[i][1];
        if (y >= 0)
        board[y][px + cur[i][0]] = 1;
    }
}

int clear_lines(void)
{
    int cleared = 0;
    int y = h -1;

    while (y >= 0)
    {
        int full =1;
        for (int x = 0; x < w; x++)
        {
            if (board[y][x] == 0)
            {
                full = 0;
                break;
            }
        }

        if (full)
        {
            for (int yy = y; yy > 0; yy--)
            for (int x = 0; x < w; x++)
            board[yy][x] = board[yy - 1][x];
            
            for (int x = 0; x < w; x++)
            board[0][x] = 0;

            cleared++;
        }
        else
        {
            y--;
        }
    }

    return cleared;

}

void draw_wll(void)
{
    for (int y = 0; y < h; y++)
    {
        goto_xy(ox, oy + y);
        printf("<!");
        for (int x = 0; x < w; x++)
        {
            if (board[y][x] || is_piece(x, y))
            printf("[]");
            else 
            printf(" .");
        }

        printf("!>");
    }
    
    goto_xy(ox, oy +h);
    printf("<!====================!>");

    int sx = ox + 28;

    goto_xy(sx, oy); printf("PONTOS: %d", score);
    goto_xy(sx, oy + 1); printf("LINHAS: %d", lines);
    goto_xy(sx, oy + 2); printf("RECORDE: %d", best);

    goto_xy(sx, oy + 4); printf("PROXIMA:");
    goto_xy(sx, oy + 5); printf("        ");
    goto_xy(sx, oy + 6); printf("        ");

    if ( show_help)
    {
        for(int i = 0; i < 4; i++)
        {
            int bx = shapes[next_kind][i][0] + 1;
            int by = shapes[next_kind][i][1];
            goto_xy(sx + bx * 2, oy + 5 + by);
            printf("[]");
        }
    }  

    goto_xy(sx, oy + 9); printf("<- -> MOVER               ");
    goto_xy(sx, oy + 10); printf("SETA CIMA GIRA           ");
    goto_xy(sx, oy + 11); printf("SETA BAIXO ACELERAR      ");
    goto_xy(sx, oy + 12); printf("ESPACO DERRUBAR          ");
    goto_xy(sx, oy + 13); printf("Q SAIR                   ");
    goto_xy(sx, oy + 14); printf("H ESCONDER AJUDA         ");
    
}

void reset_game(void)
{
    for(int y = 0; y < h; y++)
    for (int x = 0; x < w; x++)
    board[y][x] = 0;

    score = 0;
    lines = 0;
    printf("\x1b[2J");
    next_kind = rand() %7;
    new_piece();
}

int play(void)
{
    int tick = 0;
    
    while (1)
    {
        draw_wll();
        Sleep(10);
        tick++;

        if (_kbhit())
        {
            int c = _getch();
            if (c == 0 || c == 224)
            {
                c = _getch();
                if (c == 75 && fits(cur, px - 1, py)) px--;
                if (c == 77 && fits(cur, px + 1, py)) px++;
                if (c == 72) rotate();
                if (c == 80) tick = 20;
            }
            else if (c == ' ')
            {
                while (fits(cur, px, py + 1)) py++;
                tick = 20;
            }
            else if (c == 'h' || c == 'H')
                show_help = !show_help;
            else if (c == 'q' || c == 'Q')
            return 1;
        }

        if(tick >=20)
        {
            tick = 0;
            if(fits(cur, px, py + 1))
                py++;
            else 
            {
                lock_piece();
                int n = clear_lines();
                lines += n;
                if (n == 1) score += 100;
                if (n == 2) score += 300;
                if (n == 3) score += 500;
                if (n == 4) score += 800;
                new_piece();
                if (!fits(cur, px, py))
                return 0;

            }
        }
    }
}

int ask_restart(void)
{
    goto_xy(ox + 6, oy + h + 2);
    printf("FIM DE JOGO");
    goto_xy(ox - 2, oy + h + 3);
    printf("R - JOGAR DE NOVO Q - SAIR");

    while (1)
    {
        int c = _getch();
        if(c == 'r' || c == 'R') return 1;
        if(c == 'q' || c == 'Q') return 0;
    }
}

int main(void)
{
    srand(time(NULL));
    prepare_terminal();
    printf("\x1b[32m");      
    printf("\x1b[?25l");         
    center_well();           
    
    while (1)
    {
        reset_game();

        if(play())
        break;

        if(score > best)
        best = score;
        draw_wll();

        if(!ask_restart())
        break;
    }
    
    printf("\x1b[0m");
    printf("\x1b[2J\x1b[H");     
    printf("\x1b[?25h\n");
    return 0;
}