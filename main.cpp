#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <random>
#include <queue>
#include <algorithm>
#include <fstream>

using namespace std;

typedef pair<char, pair<int, int>> cell;
typedef map<int, pair<int, int>> bombs;

//необходимо для формирования ключа
const string CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

vector<string> Words(string &s)
{
    //получаем из строки массив слов
    string cur = "";
    vector<string> res;
    for (int i = 0; i < s.length(); i++)
    {
        if (s[i] != ' ')
        {
            cur += s[i];
        }
        else
        {
            res.push_back(cur);
            cur = "";
        }
    }
    res.push_back(cur);
    return res;
}

bombs GetBombs(int n_bombs, int width, int height)
{
    bombs all_bombs;
    vector<pair<int, int>> random_coord;
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            random_coord.push_back({i, j});
        }
    }
    random_shuffle(random_coord.begin(), random_coord.end());
    //получили пары случайных координат
    for (int bomb = 0; bomb < n_bombs; bomb++)
    {
        all_bombs.insert({bomb, random_coord[bomb]});
    }
    return all_bombs;
}

void SetBombs(vector<vector<cell>> &field, int n_bombs, bombs &game_bombs, int height, int width)
{
    for (int bomb = 0; bomb < n_bombs; bomb++)
    {
        int x = game_bombs[bomb].first;
        int y = game_bombs[bomb].second;

        field[x][y].second.first = 1;

        vector<pair<int, int>> delta = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
        for (auto d : delta)
        {
            int nx = x + d.first;
            int ny = y + d.second;
            if (nx >= 0 && nx < width && ny >= 0 && ny < height)
            {
                field[nx][ny].second.second++;
            }
        }
    }
    return;
}

void SeeField(vector<vector<cell>> &field, int &bomb_counter, bool &all_bombs_find, bool show_bombs)
{
    bomb_counter = 0;
    all_bombs_find = true;
    for (auto i : field)
    {
        for (auto j : i)
        {
            if (show_bombs && j.second.first == 1)
            {
                cout << '!';
            }
            else if (!show_bombs || show_bombs && j.second.first != 1)
            {
                cout << j.first;
            }
            if (j.second.first == 1 && j.first != '?')
            {
                bomb_counter++;
            }
            if (j.second.first == 0 && j.first != '_')
            {
                all_bombs_find = false;
            }
        }
        cout << '\n';
    }
    if (!show_bombs)
    {
        cout << "Bombs num: " << bomb_counter << '\n';
    }
}

void bfs(vector<vector<cell>> &field, int start_x, int start_y, int width, int height)
{
    //ищем ячейки, которые можно открыть обходом в ширину

    vector<vector<bool>> visited(width, vector<bool>(height, false));
    vector<pair<int, int>> delta = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

    queue<pair<int, int>> q;
    q.push({start_x, start_y});
    visited[start_x][start_y] = true;

    while (!q.empty())
    {
        auto cur = q.front();
        int x = cur.first;
        int y = cur.second;
        bool all_neighbors_free = true;
        q.pop();
        for (auto d : delta)
        {

            int nx = x + d.first;
            int ny = y + d.second;

            if (nx >= 0 && nx < width && ny >= 0 && ny < height)
            {
                if (!visited[nx][ny] && field[nx][ny].first != '_')
                {
                    if (field[nx][ny].second.first == 0 && field[x][y].second.first == 0)
                    {
                        if (field[nx][ny].second.second == 0)
                        {
                            q.push({nx, ny});
                            visited[nx][ny] = true;
                        }
                        else
                        {
                            field[nx][ny].first = '0' + field[nx][ny].second.second;
                        }
                    }
                    else
                    {
                        all_neighbors_free = false;
                    }
                }
            }
        }
        if (all_neighbors_free || (x == start_x && y == start_y))
        {
            field[x][y].first = '_';
        }
    }
}

void SaveGame(vector<vector<cell>> &field, string &key)
{
    string filename;
    ifstream load;

    while (true)
    {
        cout << "Save-file name in format - name.txt: \n";
        getline(cin, filename);
        if (filename.length() > 4 && filename.find('.') != string::npos)
        {
            if (filename.substr(filename.find('.'), 4) == ".txt")
            {
                break;
            }
        }
        cout << "Please, follow the format!\n";
    }

    ofstream save;
    save.open(filename);
    save << key << '\n';
    save << field.size() << ' ' << field[0].size() << '\n';
    int index = 0;
    char safe = key[index];
    for (auto i : field)
    {
        for (auto j : i)
        {
            //зашифровываем сохранение
            char cell_design = (j.first) ^ safe;
            char cell_type = (j.second.first + '0') ^ safe;
            char near_mines = (j.second.second + '0') ^ safe;

            save << cell_design << ' ' << cell_type << ' ' << near_mines << '\n';
            safe = key[++index];
        }
    }
    save.close();
}

vector<vector<cell>> LoadGame()
{
    string filename;
    ifstream load;

    while (true)
    {
        cout << "Loading file name in format - name.txt: \n";
        getline(cin, filename);
        if (filename.length() > 4 && filename.find('.') != string::npos)
        {
            if (filename.substr(filename.find('.'), 4) == ".txt")
            {
                load.open(filename);
                if (load)
                {
                    break;
                }
                else
                {
                    cout << "File does not exist\n";
                    continue;
                }
            }
        }
        cout << "Please, follow the format!\n";
    }

    string cur_str, key;
    getline(load, key);
    int width, height;
    getline(load, cur_str);
    vector<string> wi_he = Words(cur_str);
    width = stoi(wi_he[0]), height = stoi(wi_he[1]);
    vector<vector<cell>> field(width, vector<cell>(height, {'*', {0, 0}}));

    int index = 0;
    char safe = key[index];
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            getline(load, cur_str);

            //расшифровываем сохранение
            char cell_design = cur_str[0] ^ safe;
            char cell_type = cur_str[2] ^ safe;
            char near_mines = cur_str[4] ^ safe;

            field[i][j].first = cell_design;
            field[i][j].second.first = (cell_type) - '0';
            field[i][j].second.second = (near_mines) - '0';
            safe = key[++index];
        }
    }
    load.close();
    return field;
}

void StartGame(vector<vector<cell>> &field, bool &new_g)
{

    string action;
    int x;
    int y;
    int bomb_counter_display;

    int width = field.size();
    int height = field[0].size();

    //получаем уникальный ключ для игры
    random_device rd;
    mt19937 mt(rd());
    uniform_int_distribution<int> random_chars(0, CHARACTERS.size() - 1);
    string key = "";
    for (int i = 0; i <= width * height; i++)
    {
        key += CHARACTERS[random_chars(mt)];
    }

    bool all_bombs_find;
    SeeField(field, bomb_counter_display, all_bombs_find, false);
    while (!all_bombs_find)
    {
        cout << "Your turn: \n";
        string turn;
        getline(cin, turn);
        vector<string> move = Words(turn);
        if (move.size() > 1)
        {
            x = stoi(move[0]) - 1;
            y = stoi(move[1]) - 1;
            action = move[2];
        }
        else
        {
            action = move[0];
        }
        cout << '\n';
        if (x < 0 || x >= width || y < 0 || y >= height)
        {
            cout << "Wrong x or y\n";
            continue;
        }
        if (action == "Open")
        {

            if (field[x][y].second.first == 1) //наткнулись на мину
            {
                cout << "Bang! You lose\n";
                return;
            }
            else
            {
                bfs(field, x, y, width, height);
            }
        }
        else if (action == "Flag")
        {
            if (field[x][y].first == '?')
            {
                field[x][y].first = '*';
            }
            else
            {
                field[x][y].first = '?';
            }
        }
        else if (action == "Save")
        {
            SaveGame(field, key);
        }
        else if (action == "Load")
        {
            field = LoadGame();
            width = field.size();
            height = field[0].size();
        }
        else if (action == "New")
        {
            string wish;
            cout << "Wish to save? (Y/N)\n";
            getline(cin, wish);
            if (wish == "Y")
            {
                SaveGame(field, key);
            }
            new_g = true;
            break;
        }
        SeeField(field, bomb_counter_display, all_bombs_find, false);
        if (all_bombs_find)
        {
            SeeField(field, bomb_counter_display, all_bombs_find, true);
            cout << "\nCongratulations! All bombs are detected!\n";
        }
    }
    return;
}

bool IsNumber(string &s)
{
    return !s.empty() && std::find_if(s.begin(),
                                      s.end(), [](char c)
                                      { return !std::isdigit(c); }) == s.end();
}

int main()
{
    cout << "Welcome to the Minesweeper!\nCommands list:\n\
    1)x y Open - open cell (x, y)\n\
    2)x y Flag - set flag on cell (x, y)\n\
    3)Save - save game\n\
    4)Load - load game\n\
    5)New - start new game\nWhat's next?\n1.New game\n2.Load game\n";
    string choose;
    while (true)
    {
        getline(cin, choose);
        if (choose == "1" || choose == "2")
        {
            break;
        }
        cout << "Enter 1 or 2\n";
    }

    bool new_g = false;
    string again;
    if (choose == "2")
    {
        vector<vector<cell>> field = LoadGame();
        StartGame(field, new_g);
        cout << "Want to play again? (Y/N)\n";
        getline(cin, again);
        if (again == "N")
        {
            return 0;
        }
    }
    again = "Y";
    while (again == "Y")
    {
        int width, height, n_bombs;
        string enter;
        while (true)
        {
            cout << "Enter the field size (width x height): \n";
            getline(cin, enter);
            vector<string> ent_w = Words(enter);
            if (ent_w.size() == 2)
            {
                if (IsNumber(ent_w[0]) && IsNumber(ent_w[1]))
                {
                    width = stoi(ent_w[0]);
                    height = stoi(ent_w[1]);
                    enter.clear();
                    break;
                }
            }
            cout << "Incorrect width or height\n";
        }
        while (true)
        {
            cout << "Enter num of bombs: \n";
            getline(cin, enter);
            if (IsNumber(enter))
            {
                n_bombs = stoi(enter);
                if (n_bombs > 0 and n_bombs < width * height)
                {
                    break;
                }
            }
            cout << "Incorrect number of bombs";
        }

        bombs game_bombs = GetBombs(n_bombs, width, height);                    //получаем для каждой бомбы координаты
        vector<vector<cell>> field(width, vector<cell>(height, {'*', {0, 0}})); //поле - (изображение,(пусто/бомба, бомб вокруг))
        SetBombs(field, n_bombs, game_bombs, height, width);
        StartGame(field, new_g);

        if (!new_g)
        {
            cout << "Want to play again? (Y/N)\n";
            getline(cin, again);
        }
    }
    return 0;
}