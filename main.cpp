#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <random>
#include <set>
#include <stdio.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 850

#define GRID_SIZE 4
#define CELL_SIZE 200
#define OFFSET_Y 50

#define BLACK sf::Color::Black
#define WHITE sf::Color::White

std::vector<sf::SoundBuffer> soundBuffers;
std::vector<sf::Sound> sounds;

int initializationCount = 0;
float averageGameTime = 1.0f;
std::string gameStatus = "Game initialization...";

struct Record {
    std::string date;
    int score;
};

//Загрузка всех рекордов
std::vector<Record> loadRecords(const std::string& filename) {
    std::vector<Record> records;
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "file not exists" << std::endl;
        return records;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Record record;
        std::getline(ss, record.date, ':');
        ss >> record.score;
        records.push_back(record);
    }
    return records;
}

//сохраняем все рекорды
//из глобальных перменных
//новый должен быть предварительно добавлен
void saveRecords(const std::vector<Record>& records, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "file not exists" << std::endl;
        return;
    }

    for (const auto& record : records) {
        file << record.date << ": " << record.score << "\n";
    }
}

//добавление нового рекорда
void addRecord(std::vector<Record>& records, const Record& newRecord) {
    records.push_back(newRecord);
}


// сортировка рекордов
void sortRecords(std::vector<Record>& records) {
    int size = records.size();
    for(int i = 0; i < size - 1; i++) {
        for(int j = 0; j < size - i - 1; j++) {
            if(records[j].score < records[j+1].score) {
                std::swap(records[j], records[j+1]);
            }
        }
    }
}

//получаем текущую дату
std::string getCurrentDate() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%y");
    return oss.str();
}

//загружаем звуки
void loadSounds() {
    std::string soundFiles1[] = {
        "zvuk-nopki-v-kompyuternoy-igre1.wav"
    };
    std::string soundFiles[] = {
        "zvuk-nopki-v-kompyuternoy-igre1.wav"
        "knopka-klik-zvonkii-myagkaya.wav"
        "elektronnyiy-zvuk-knopki1.wav"
        "knopka-klik-glubokii-myagkii-blizkii-priglushennyii.wav"
    };
    for(const auto& file : soundFiles1) {
        sf::SoundBuffer buffer;
        if(!buffer.loadFromFile(file)) {
            printf("[ERROR] load sound file: %s\n", file);
            continue;
        }
        soundBuffers.push_back(buffer);
        sounds.emplace_back(soundBuffers.back());
    }
}

//когда вызывается, играет случайный
// звук из пачки заранее загруженных
void play(){
    int randomSoundIndex = rand() % sounds.size();
    sounds[randomSoundIndex].play();
}


//основной класс с игрой
class Game {
public:
    Game(bool noFailMode, int scnds):
            //режим бессмертия
            noFailMode(noFailMode),
            window(
                sf::VideoMode(
                    WINDOW_WIDTH,
                    WINDOW_HEIGHT
                ), 
            "Game"), 
            button_count(0), 
            score(0), 
            isGameOver(false),
            //сколько длится режим
            remainingTime(sf::seconds(scnds))
    {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                sf::RectangleShape button(sf::Vector2f(CELL_SIZE - 10, CELL_SIZE - 10));
                button.setPosition(
                    j * CELL_SIZE + 5, 
                    i * CELL_SIZE + 5 + OFFSET_Y
                );
                button.setFillColor(sf::Color::White);
                buttons.push_back(button);
            }
        }
        timer = sf::seconds(scnds);
        clock.restart();
        resetButtons(4);
        font.loadFromFile("ARIAL.TTF");
        scoreText.setFont(font);
        scoreText.setCharacterSize(30);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10, 10);

        timerText.setFont(font);
        timerText.setCharacterSize(30);
        timerText.setFillColor(sf::Color::White);
        timerText.setPosition(WINDOW_WIDTH - 150, 10);

        if (!buffer.loadFromFile("knopka-klik-glubokii-myagkii-blizkii-priglushennyii.wav")) {
        }
        sound.setBuffer(buffer);

    }

    //для второго режима
    // добавляем новую кнопку, если любая старая пропала
    void spawnButton() {
        int emptyCell;
        do {
            emptyCell = rand() % (GRID_SIZE * GRID_SIZE);
        } while (buttons[emptyCell].getFillColor() == BLACK);

        buttons[emptyCell].setFillColor(BLACK);
    }

    //проверка что игрок попал в клетку с кнопкой
    bool handleCellClick(int row, int col) {
        if(row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE) {
            if(buttons[row * GRID_SIZE + col].getFillColor() == BLACK) {
                buttons[row * GRID_SIZE + col].setFillColor(WHITE);
                play();
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    int gameover(){
        std::vector<Record> records = loadRecords("records.txt");

        Record newRecord;
        newRecord.date = getCurrentDate();
        newRecord.score = score;

        addRecord(records, newRecord);
        sortRecords(records);
        saveRecords(records, "records.txt");


        sf::Text gameOverText;
        gameOverText.setFont(font);
        gameOverText.setCharacterSize(60);
        gameOverText.setFillColor(WHITE);
        gameOverText.setString("Game Over\nScore: " + std::to_string(score));
        gameOverText.setPosition(20, WINDOW_HEIGHT / 2 - gameOverText.getGlobalBounds().height / 2);

        sf::RectangleShape menuButton(sf::Vector2f(200.f, 50.f));
        menuButton.setFillColor(sf::Color::Blue);
        menuButton.setPosition(WINDOW_WIDTH / 2 - menuButton.getSize().x / 2, gameOverText.getPosition().y + gameOverText.getGlobalBounds().height + 20);

        sf::Text menuButtonText;
        menuButtonText.setFont(font);
        menuButtonText.setString("Return to menu");
        menuButtonText.setCharacterSize(20);
        menuButtonText.setFillColor(sf::Color::White);
        menuButtonText.setPosition(
        menuButton.getPosition().x + menuButton.getSize().x / 2 - menuButtonText.getGlobalBounds().width / 2,
        menuButton.getPosition().y + menuButton.getSize().y / 2 - menuButtonText.getGlobalBounds().height / 2);

        sf::Text topScoresText;
        topScoresText.setFont(font);
        topScoresText.setCharacterSize(20);
        topScoresText.setFillColor(sf::Color::White);
        topScoresText.setPosition(20, 20);

        std::string topScoresStr = "Top Scores:\n";
        records = loadRecords("records.txt");
        for (int i = 0; i < std::min(5, static_cast<int>(records.size())); i++) {
            topScoresStr += std::to_string(i+1) + ". " + records[i].date + ": " + std::to_string(records[i].score) + "\n";
        }
        topScoresText.setString(topScoresStr);

        while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return 0;
            }
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (menuButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
                    printf("qwe");
                    return 1; //возвращаем в меню если нажал кнопку домой
                }
            }
        }

        window.clear();
        window.draw(gameOverText);
        window.draw(menuButton);
        window.draw(menuButtonText);
        window.draw(topScoresText);
        window.display();
        }
        return 0;
    }
    
    //ПЕРВЫЙ ИГРОВОЙ РЕЖИМ
    //в нём появляется сразу 4 кнопки в виде "узора" и повторяются до окончания времени
    //в режиме бессмертия за промах вычитается 5 секунд
    void run1() {
        resetButtons(4);
        while (window.isOpen()) {
            sf::Event event;
            
            
            //--------------------ловим нажатия мышкой
            while (window.pollEvent(event)) {
                bool hit;
                if (event.type == sf::Event::Closed)
                    window.close();
                if (event.type == sf::Event::MouseButtonPressed) {
                    if(event.mouseButton.button == sf::Mouse::Left) {
                        int row = (event.mouseButton.y - OFFSET_Y) / CELL_SIZE;
                        int col = event.mouseButton.x / CELL_SIZE;
                        hit = handleCellClick(row, col);
                        if (hit){
                            score++;
                            button_count++;
                        } else if (noFailMode) {
                            remainingTime -= sf::seconds(5);
                        } else {
                            isGameOver = true;
                        }
                    }
                } 
                // и нажатия кнопкой D если игра идёт с графического планшета
                else if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::D || event.key.code == sf::Keyboard::S || event.key.code == sf::Keyboard::A)) {
                    int row = (sf::Mouse::getPosition(window).y - OFFSET_Y) / CELL_SIZE;
                    int col = (sf::Mouse::getPosition(window).x) / CELL_SIZE;
                    hit = handleCellClick(row, col);
                    if (hit){
                           score++;
                           button_count++;
                    } 
                    else if (noFailMode) {
                        remainingTime -= sf::seconds(5);
                    } 
                    else {
                        isGameOver = true;
                    }
                }

                
            }

            // уменьшаем оставшееся время
            remainingTime -= clock.restart();
            if (remainingTime <= sf::Time::Zero) {
                isGameOver = true;
            }

            if(clock.getElapsedTime() > timer) {
                window.close();
            }

            // вывод времени и счёта сверху во время игры
            scoreText.setString("Score: " + std::to_string(score));
            timerText.setString("Time: " + std::to_string((int)remainingTime.asSeconds()));
            
            window.clear();
            for(const auto& button: buttons) {
                window.draw(button);
            }
            window.draw(scoreText);
            window.draw(timerText);
            window.display();
            //если проигрышь - вывод экрана
            // +сохранение рекорда
            if (isGameOver) {
                if (gameover() == 1){
                    return;
                } 
            }

            if(button_count >= 4) {
                resetButtons(4);
                button_count = 0;
            }
            
        }
    }

    void run2() {
        resetButtons(3);
        while (window.isOpen()) {
            sf::Clock clock;
            sf::Time remainingTime = timer - clock.getElapsedTime();
            while (window.isOpen() && !isGameOver) {
                sf::Event event;
                //-----------------
                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed) {
                        window.close();
                        return;
                    } 
                    else if (event.type == sf::Event::MouseButtonPressed){
                        if(event.mouseButton.button == sf::Mouse::Left) {
                            int row = (event.mouseButton.y - OFFSET_Y) / CELL_SIZE;
                            int col = event.mouseButton.x / CELL_SIZE;
                            printf("M: %d %d\n", row, col);
                            bool hit = handleCellClick(row, col);
                            if (hit) {
                                score++;
                                remainingTime += sf::seconds(0.3);
                                spawnButton();
                            }
                            else if(noFailMode){
                                remainingTime -= sf::seconds(3);
                            }
                            else {
                                isGameOver = true;
                            }
                        }
                    }
                    else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D) {
                        int row = (sf::Mouse::getPosition(window).y - OFFSET_Y) / CELL_SIZE;
                        int col = (sf::Mouse::getPosition(window).x) / CELL_SIZE;
                        bool hit = handleCellClick(row, col);
                        if (hit) {
                            score++;
                            remainingTime += sf::seconds(0.3);
                            spawnButton();
                        }
                        else if(noFailMode){
                            remainingTime -= sf::seconds(3);
                        }
                        else {
                            isGameOver = true;
                            gameover();
                        }
                    }
                }
                //-----------------

                
                //-----------------
                remainingTime -= clock.restart();
                if (remainingTime <= sf::Time::Zero) {
                    isGameOver = true;
                }

                scoreText.setString("Score: " + std::to_string(score));
                timerText.setString("Time: " + std::to_string(static_cast<int>(remainingTime.asSeconds())));

                window.clear();
                for(const auto& button: buttons) {
                    window.draw(button);
                }
                window.draw(scoreText);
                window.draw(timerText);
                window.display();
            }

            if (isGameOver) {
                if (gameover() == 1){
                    return;
                }
            }
        }
    }

    

private:
    void resetButtons(int n) {
        for(auto& button: buttons) {
            button.setFillColor(sf::Color::White);
        }
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, GRID_SIZE * GRID_SIZE - 1);
        std::set<int> selectedIndices;
        while (selectedIndices.size() < n) {
            int index = distrib(gen);
            if (selectedIndices.find(index) == selectedIndices.end()) {
                buttons[index].setFillColor(sf::Color::Black);
                selectedIndices.insert(index);
            }
        }
    }

private:
    sf::SoundBuffer buffer;
    sf::Sound sound;
    std::vector<sf::RectangleShape> buttons;
    sf::RenderWindow window;
    sf::Font font;
    sf::Text scoreText;
    sf::Text timerText;
    int button_count;
    int score;
    bool noFailMode;
    bool isGameOver;
    sf::Time remainingTime;
    sf::Time timer;
    sf::Clock clock;
};


class MainMenu {
public:
    MainMenu() {
        font.loadFromFile("ARIAL.TTF");

        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(30);
        text.setFillColor(sf::Color::White);
        
        text.setString("Mode 1");
        text.setPosition(
            WINDOW_WIDTH / 2, 
            WINDOW_HEIGHT / 2 - 100);
        menuButtons.push_back(text);
        
        text.setString("Mode 2");
        text.setPosition(
            WINDOW_WIDTH / 2, 
            WINDOW_HEIGHT / 2);
        menuButtons.push_back(text);
        

        // чекбокс режима бессмертия
        checkbox.setSize(sf::Vector2f(20, 20));
        checkbox.setOutlineThickness(2);
        checkbox.setOutlineColor(sf::Color::White);
        checkbox.setPosition(
            WINDOW_WIDTH / 2 - 10, 
            WINDOW_HEIGHT - 100);
        checkbox.setFillColor(sf::Color::Black);
        
        checkboxText.setFont(font);
        checkboxText.setString("No fail mode");
        checkboxText.setCharacterSize(20);
        checkboxText.setFillColor(sf::Color::White);
        checkboxText.setPosition(
            WINDOW_WIDTH / 2 + 20,
            WINDOW_HEIGHT - 100);
    }

    int run(sf::RenderWindow& window) {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                    return -1;
                } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {

                    // если нажата кнопка меню, запускает игру в соответствующем режиме
                    for (int i = 0; i < menuButtons.size(); i++) {
                        if (menuButtons[i].getGlobalBounds().contains(
                            event.mouseButton.x,
                            event.mouseButton.y
                        )) {
                            return i + 1;
                        }
                    }
                    
                    // если прожат чекбокс, включается режим "бессмертия"
                    if (checkbox.getGlobalBounds().contains(
                        event.mouseButton.x, 
                        event.mouseButton.y)
                    ) {
                        noFailMode = !noFailMode;
                        if (noFailMode) {
                            checkbox.setFillColor(WHITE);
                        } else {
                            checkbox.setFillColor(BLACK);
                        }
                    }
                }
            }
            
            window.clear();
            for(const auto& button: menuButtons) {
                window.draw(button);
            }
            window.draw(checkbox);
            window.draw(checkboxText);
            window.display();
        }
        
        // пользователь закрыл окно, так что можно завершить программу
        return -1; 
    }
    void stop(sf::RenderWindow& window){
        window.close();
    }

    bool getNoFailMode() const {
        return noFailMode;
    }

private:
    sf::Font font;
    std::vector<sf::Text> menuButtons;
    sf::RectangleShape checkbox;
    sf::Text checkboxText;
    bool noFailMode = false;
};

int main() {
    srand(time(0));
    loadSounds();
    MainMenu menu;
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Game");
    int mode = menu.run(window);
    menu.stop(window);
    if (mode == 1) {
        Game game(menu.getNoFailMode(), 30);
        game.run1();
    } else if(mode == 2){
        Game game(menu.getNoFailMode(), 10);
        game.run2();
    }
    return 0;
}