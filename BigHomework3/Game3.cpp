#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <codecvt>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <sstream>
#include <random>


struct Terrain {
	std::wstring name;

	Terrain(std::wstring name) : name(name) {}
};


class Loot {
protected:
	std::wstring name; // Название предмета
	int cost;

public:
	static std::wifstream inputFile;
	static std::vector<std::wstring> vecForRandomItems;

	Loot() {
		vecForRandomItems.clear();
		if (!inputFile.is_open()) {
			inputFile.open(L"C:\\Users\\FridayJarvis\\source\\repos\\Game3\\Game3\\propertiesOfLoot.txt");

			if (!inputFile.is_open()) {
				std::wcerr << L"Не удалось открыть файл propertiesOfLoot.txt\n";
				return;
			}

			inputFile.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
		}
	}

	virtual void recordToPropertiesOfLoot(wchar_t ch_start, wchar_t ch_break) {
		vecForRandomItems.clear();

		if (inputFile.is_open()) {
			inputFile.clear(); // Сброс флагов ошибок
			inputFile.seekg(0, std::ios::beg); // Переместить указатель чтения в начало
		}
		else {
			std::wcerr << L"Файл propertiesOfLoot.txt не открыт.\n";
			return;
		}

		std::wstring buffer;

		bool startReading = false;

		while (std::getline(inputFile, buffer)) {
			if (buffer.find(ch_break) != std::wstring::npos) break;

			if (startReading) vecForRandomItems.push_back(buffer);

			if (buffer.find(ch_start) != std::wstring::npos) startReading = true;
		}
	}// Запись свойств лута

	virtual void registrationItems() = 0;       // Регистрация свойств
	virtual void print() = 0;                   // Вывод информации о луте

	std::wstring getName() const { return name; }
	int getCost() const { return cost; }

	virtual ~Loot() {
		vecForRandomItems.clear();
	}

	friend class ExcavationMoon;
};


std::wifstream Loot::inputFile;


class UnderWaterLoot : public Loot {
protected:
	bool isEdible;
	int heal;
public:
	UnderWaterLoot() {
		recordToPropertiesOfLoot(L'$', L'/');
		registrationItems();
	}

	void registrationItems() override {
		if (vecForRandomItems.empty()) {
			std::wcerr << L"Нет данных для регистрации.\n";
			return;
		}

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, vecForRandomItems.size() - 1);

		std::wistringstream iss(vecForRandomItems[dis(gen)]);
		iss >> name >> cost >> std::boolalpha >> isEdible;
		if (isEdible) iss >> heal;
	}

	int getHeal() {
		return heal;
	}

	bool getIsEdible() {
		return isEdible;
	}

	void print() override {
		std::wcout << L"Вы нашли " << name << L" (" << cost << L")!\n";
		if (isEdible) {
			std::wcout << L"Это съедобный ресурс! Восстанавливает " << heal << L" хп.\n";
		}
	}
};


class VolcanoLoot : public Loot {
public:
	bool isDangerous;
	int damage;

	VolcanoLoot() {
		recordToPropertiesOfLoot(L'/', L'\\');
		registrationItems();
	}

	void registrationItems() override {
		if (vecForRandomItems.empty()) {
			std::wcerr << L"Нет данных для регистрации.\n";
			return;
		}

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, vecForRandomItems.size() - 1);

		std::wistringstream iss(vecForRandomItems[dis(gen)]);
		iss >> name >> cost >> std::boolalpha >> isDangerous;
		if (isDangerous) iss >> damage;
	}

	void print() override {
		std::wcout << L"Вы нашли " << name << L" (" << cost << L")!\n";
		if (isDangerous) {
			std::wcout << L"Осторожно, это опасный ресурс! Вы будете получать по " << damage << L" урона\n";
		}
	}
};


class MoonLoot : public Loot {
public:
	int experience;

	MoonLoot() {
		recordToPropertiesOfLoot(L'\\', L'#');
		registrationItems();
	}

	void registrationItems() override {
		if (vecForRandomItems.empty()) {
			std::wcerr << L"Нет данных для регистрации." << '\n';
			return;
		}

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, vecForRandomItems.size() - 1);

		std::wistringstream iss(vecForRandomItems[dis(gen)]);
		iss >> name >> cost >> std::boolalpha >> experience;
	}

	void print() override {
		std::wcout << L"Вы нашли " << name << L" (" << cost << L")!\n";
		std::wcout << L"Это ресурс для археологов! Дает " << experience << L" опыта!\n";
	}
};


std::vector<std::wstring> Loot::vecForRandomItems;


struct Food
{
	std::wstring name;
	int cost;
	int heal;

	Food(std::wstring name, int cost, int heal) : name(name), cost(cost), heal(heal) {}
};


struct ItemAsFood : Food
{
	std::shared_ptr<Loot> loot;
	ItemAsFood(std::shared_ptr<Loot>& loot)
		: Food(loot->getName(), loot->getCost(),
			std::static_pointer_cast<UnderWaterLoot>(loot)->getHeal()),
		loot(loot) {
	}

};


struct Enstrument
{
	std::wstring name;
	std::wstring type;
	std::wstring currency;

	int durability;
	int cost;

	Enstrument(const std::wstring& name, const std::wstring& type, int durability, int cost)
		: name(name), type(type), durability(durability), cost(cost) {
	}
};


class Excavation {
protected:
	int cost; // Cost for the trip

	std::wstring name; // Expedition name

	std::shared_ptr<Terrain> terrain; // Terrain type
	std::shared_ptr<Loot> loot;

public:
	Excavation(const std::wstring& name, int cost)
		: name(name), cost(cost) {
		loot = std::make_shared<UnderWaterLoot>();
		terrain = std::make_shared<Terrain>(name);
	}

	virtual void excavate(std::vector<std::shared_ptr<Loot>>& inventory) {
		loot->print();
		inventory.push_back(loot);
	} // Adds loot to inventory

	virtual void excavate(std::vector<std::shared_ptr<Loot>>& inventory,
		std::vector<std::shared_ptr<ItemAsFood>>& inventoryOfFood) {};

	virtual void effect(std::shared_ptr<Enstrument>& enstrument,
        std::vector<std::shared_ptr<Enstrument>>& inventoryOfEnstruments) = 0; // Effect during excavation
	virtual void printText(int& health) {}// Funny text during excavation
	virtual void printText(int& exp, std::shared_ptr<Enstrument> enstrument) {}

	virtual ~Excavation() = default;

	const std::shared_ptr<Terrain>& getTerrain() const { return terrain; }
	const std::wstring& getName() const { return name; }
	const int getCost() const { return cost; }
};


class ExcavationUnderWater : public Excavation {
public:
	ExcavationUnderWater() : Excavation(L"Подводные раскопки", 500) {}

	void excavate(std::vector<std::shared_ptr<Loot>>& inventory,
		std::vector<std::shared_ptr<ItemAsFood>>& inventoryOfFood)
	{
		loot->print();
		inventory.push_back(loot);
		if (std::static_pointer_cast<UnderWaterLoot>(loot)->getIsEdible())
		{
			std::shared_ptr<ItemAsFood> itemAsFood = std::make_shared<ItemAsFood>(loot);
			inventoryOfFood.push_back(itemAsFood);
		}
	}

	void effect(std::shared_ptr<Enstrument>& enstrument,
        std::vector<std::shared_ptr<Enstrument>>& inventoryOfEnstruments) override {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 100);

		if(dis(gen) <= 30) {
			std::uniform_int_distribution<> dis1(15, 30);
			enstrument->durability -= dis1(gen);
			std::wcout << L"Вы повредили о подводный грунт свой инструмент на " << dis1(gen) << L" единиц\n";
			std::wcout << L"Состояние предмета: " << enstrument->durability << L" единиц\n";
		}
        else if (dis(gen) > 30 and dis(gen) <= 60) {
			if (inventoryOfEnstruments.empty()) {
				std::wcout << L"У вас нет инструментов для замены.\n";
				return;
			}
            std::uniform_int_distribution<> dis1(15, 30);
            enstrument->durability += dis1(gen);
            if (enstrument->durability > 100) std::wcout << L"Вы восстановили свой инструмент.\nПрочность: " << enstrument->durability << L" единиц\n";
            else std::wcout << L"Вы восстановили инструмент на " << dis1(gen) << L" единиц\n";

            std::wcout << L"Вода быстро охладила ваш инструмент, но замедлила работу.\n";
            Sleep(1000);
        }
		else std::wcout << L"Вода оказалась чистой и прозрачной. Вам повезло!\n";
		
	}

	void printText(int& health) override {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 100);

		if(dis(gen) <= 30) {
			std::uniform_int_distribution<> dis(20, 40);
			health -= dis(gen);
			std::wcout << L"Вы нашли акулу! Она укусила вас на " << dis(gen) << L"\nВаше хп: " << health << L"\n";
		}
		else
		{
			std::uniform_int_distribution<> dis(0, 10);
			health -= dis(gen);
			std::wcout << L"Вы устали, но вам надо работать! Вы потеряли: " << dis(gen) << L"\nВаше хп: " << health << L"\n";
		}
		
	}
};


class ExcavationVolcano : public Excavation {
public:
	ExcavationVolcano() : Excavation(L"Вулканические раскопки", 1500) {}

	void effect(std::shared_ptr<Enstrument>& enstrument,
        std::vector<std::shared_ptr<Enstrument>>& inventoryOfEnstruments) override {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 100);
		if (dis(gen) <= 30) {
			std::uniform_int_distribution<> dis(15, 30);
			enstrument->durability -= dis(gen);
			std::wcout << L"Экстремальная температура повредила ваш инструмент для вулканических раскопок на " << dis(gen) << L" единиц\n" << L"Ваше хп: " << enstrument->durability << L"\n";;
		}else
		{
			std::uniform_int_distribution<> dis(5, 15);
			
			enstrument->durability -= dis(gen);
			std::wcout << L"Высокая температура повредила ваш инструмент для вулканических раскопок на " << dis(gen) << L" единиц\n";
			std::wcout << L"Состояние предмета: " << enstrument->durability << L" единиц\n";
		}

	}

	void printText(int& health) override
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 100);

		if(dis(gen)<= 30)
		{
			std::uniform_int_distribution<> dis(10, 30);
			std::wcout << L"Вы наступили в лаву! Урон по вам составил: " << dis(gen) << L" единиц.\nВаше хп : " << health << L"\n";
			return;
		}
		std::wcout << L"Осторожно, лава очень горячая!\n";

	}
};


class ExcavationMoon : public Excavation {
public:
	ExcavationMoon() : Excavation(L"Лунные раскопки", 1000) {}

	void effect(std::shared_ptr<Enstrument>& enstrument,
        std::vector<std::shared_ptr<Enstrument>>& inventoryOfEnstruments) override {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 1);
		std::uniform_int_distribution<> dis1(5,  20);
		if(dis(gen))
		{
			loot->cost *= 2;
			std::wcout << L"Космическая аномалия увеличила количество лута!\n";
			return;
		}

		std::wcout << L"Прочность уменьшена на " << dis1(gen) << L"\n";
		enstrument->durability -= dis1(gen);
	}

	void printText(int& health) override
	{
		std::wcout << L"Это невероятно! Лунные находки бесценны.\n";
	}
};


class Game {
private:
    int money;
    int health;
    int exp;

    int const FOOD_START = 3;

    std::vector<std::shared_ptr<Excavation>> expeditions;
    std::vector<std::shared_ptr<Loot>> inventory;
    std::vector<std::shared_ptr<Food>> inventoryOfFood;
    std::vector<std::shared_ptr<Enstrument>> inventoryOfEnstruments;

public:
    Game() : money(1000), inventoryOfFood(FOOD_START), health(100), exp(0) {
        expeditions.push_back(std::make_shared<ExcavationUnderWater>());
        expeditions.push_back(std::make_shared<ExcavationMoon>());
        expeditions.push_back(std::make_shared<ExcavationVolcano>());

        inventoryOfEnstruments.push_back(std::make_shared<Enstrument>(L"Инструмент для подводных раскопок",
            L"Подводные раскопки", 100, 100));

        for (int i = 0; i < FOOD_START; ++i) inventoryOfFood[i] = std::make_shared<Food>(L"Яблоко", 50, 10);
    }

    void saveGame(const std::wstring& filename) {
        std::wofstream outFile(filename);
        if (!outFile) {
            std::wcout << L"Не удалось открыть файл для сохранения.\n";
            return;
        }

        outFile << money << L'\n' << health << L'\n' << exp << L'\n';

        outFile << inventoryOfFood.size() << L'\n';
        for (const auto& food : inventoryOfFood) {
            outFile << food->name << L' ' << food->cost << L' ' << food->heal << L'\n';
        }

        outFile << inventoryOfEnstruments.size() << L'\n';
        for (const auto& enstrument : inventoryOfEnstruments) {
            outFile << enstrument->name << L' ' << enstrument->type << L' ' << enstrument->durability << L' ' << enstrument->cost << L'\n';
        }

        outFile << inventory.size() << L'\n';
        for (const auto& loot : inventory) {
            outFile << loot->getName() << L' ' << loot->getCost() << L'\n';
        }

        outFile.close();
    }

    void loadGame(const std::wstring& filename) {
        std::wifstream inFile(filename);
        if (!inFile) {
            std::wcerr << L"Не удалось открыть файл для загрузки.\n";
            return;
        }

        inFile >> money >> health >> exp;

        int foodCount;
        inFile >> foodCount;
        inventoryOfFood.clear();
        for (int i = 0; i < foodCount; ++i) {
            std::wstring name;
            int cost, heal;
            inFile >> name >> cost >> heal;
            inventoryOfFood.push_back(std::make_shared<Food>(name, cost, heal));
        }

        int enstrumentCount;
        inFile >> enstrumentCount;
        inventoryOfEnstruments.clear();
        for (int i = 0; i < enstrumentCount; ++i) {
            std::wstring name, type;
            int durability, cost;
            inFile >> name >> type >> durability >> cost;
            inventoryOfEnstruments.push_back(std::make_shared<Enstrument>(name, type, durability, cost));
        }

        int lootCount;
        inFile >> lootCount;
        inventory.clear();
        for (int i = 0; i < lootCount; ++i) {
            std::wstring name;
            int cost;
            inFile >> name >> cost;
            // Здесь можно добавить логику для создания конкретного типа лута
            inventory.push_back(std::make_shared<UnderWaterLoot>()); // Пример
        }

        inFile.close();
    }

    template<class T1, class T2>
    int getPosition(const T1& obj, const std::vector<T2>& listOfObjs) {
        for (int i = 0; i < listOfObjs.size(); ++i) {
            if (obj == listOfObjs[i]->type) return i;
        }
        return -1; // Возвращаем -1, если объект не найден
    }

    void shop() {
        std::wcout << L"Добро пожаловать в магазин! У вас: " << money << L" денег.\n";
        std::wcout << L"1. Купить еду (1 ед. за 50 денег).\n"
                   << L"2. Продать еду.\n"
                   << L"3. Продать ресурс.\n"
                   << L"4. Купить инструмент\n"
                   << L"5. Продать инструмент\n"
                   << L"6. Выйти из магазина.\n"
                   << L"7. Поесть.\n";

        int choice;
        std::wcin >> choice;

        switch (choice) {
        case 1:
            if (money >= 50) {
                inventoryOfFood.push_back(std::make_shared<Food>(L"Яблоко", 50, 10));
                money -= 50;
                std::wcout << L"Вы купили 1 ед. еды. Осталось денег: " << money << L'\n';
            }
            else {
                std::wcout << L"Недостаточно денег!\n";
            }
            break;

        case 2: {
            if (!inventoryOfFood.empty()) {
                int choiceForSell;

                std::wcout << L"Ваша еда:\n";
                for (int i = 0; i < inventoryOfFood.size(); ++i) std::wcout << i + 1 << ". " << inventoryOfFood[i]->name << L'\n';

                std::wcout << L"Выберите еду, которую хотите продать (0 для выхода): ";
                std::wcin >> choiceForSell;

                if (choiceForSell > 0 && choiceForSell <= inventoryOfFood.size()) {
                    money += inventoryOfFood[choiceForSell - 1]->cost;
                    std::wcout << L"Вы продали " << inventoryOfFood[choiceForSell - 1]->name << L" за " << inventoryOfFood[choiceForSell - 1]->cost << L"руб .\n";
                    std::wcout << L"Ваш баланс: " << money << L'\n';
                    inventoryOfFood.erase(inventoryOfFood.begin() + choiceForSell - 1);
                }
                else {
                    std::wcout << L"Неверный выбор!\n";
                }
            }
            else std::wcout << L"У вас нет еды\n";
            break;
        }
        case 3:
            if (!inventory.empty()) {
                int choiceForSell;

                std::wcout << L"Ваш лут:\n";
                for (int i = 0; i < inventory.size(); ++i) {
                    std::wcout << i + 1 << ". " << inventory[i]->getName() << L"(" << inventory[i]->getCost() << L"рублей)\n";
                }

                std::wcout << L"Выберите лут, который хотите продать (0 для выхода): ";
                std::wcin >> choiceForSell;

                if (choiceForSell > 0 && choiceForSell <= inventory.size()) {
                    money += inventory[choiceForSell - 1]->getCost();
                    std::wcout << L"Вы продали " << inventory[choiceForSell - 1]->getName() << L" за " << inventory[choiceForSell - 1]->getCost() << L"руб .\n";
                    std::wcout << L"Ваш баланс: " << money << L'\n';
                    inventory.erase(inventory.begin() + choiceForSell - 1);
                }
                else {
                    std::wcout << L"Неверный выбор!\n";
                }
            }
            else std::wcout << L"У вас нет лута\n";
            break;

        case 4: {
            std::wcout << L"Какие инструменты вы хотите купить:\n"
                << L"1. Инструмент для подводных раскопок (100 руб)\n"
                << L"2. Инструмент для лунных раскопок (500 руб)\n"
                << L"3. Инструмент для вулканических раскопок (500 exp)\n";

            int choiceEnstrument;
            std::wcin >> choiceEnstrument;

            switch (choiceEnstrument) {
            case 1:
                if (money >= 100) {
                    std::shared_ptr<Enstrument> instrument = std::make_shared<Enstrument>(
                        L"Инструмент для подводных раскопок", L"Подводные раскопки", 100, 100);

                    inventoryOfEnstruments.push_back(instrument);
                    money -= 100;
                }
                else std::wcout << L"У вас не хватает денег!";
                break;
            case 2:
                if (money >= 500) {
                    std::shared_ptr<Enstrument> instrument = std::make_shared<Enstrument>(
                        L"Инструмент для лунных раскопок", L"Лунные раскопки", 100, 100);

                    inventoryOfEnstruments.push_back(instrument);
                    money -= 500;
                }
                else std::wcout << L"У вас не хватает денег!";
                break;
            case 3:
                if (exp >= 500) {
                    std::shared_ptr<Enstrument> instrument = std::make_shared<Enstrument>(
                        L"Инструмент для вулканических раскопок", L"Вулканические раскопки", 100, 100);

                    inventoryOfEnstruments.push_back(instrument);
                    exp -= 500;
                }
                else std::wcout << L"У вас не хватает опыта!";
                break;
            default:
                std::wcout << L"Неправильный ввод!";
            }
            break;
        }

        case 5:
            if (!inventoryOfEnstruments.empty()) {
                int choiceForSell;

                std::wcout << L"Ваши инструменты:\n";
                for (int i = 0; i < inventoryOfEnstruments.size(); ++i) {
                    std::wcout << i + 1 << ". " << inventoryOfEnstruments[i]->name << L'\n';
                }

                std::wcout << L"Выберите инструмент, который хотите продать (0 для выхода): ";
                std::wcin >> choiceForSell;

                if (choiceForSell > 0 && choiceForSell <= inventoryOfEnstruments.size()) {
                    std::wcout << L"Вы продали " << inventoryOfEnstruments[choiceForSell - 1]->name << L" за " << inventoryOfEnstruments[choiceForSell - 1]->cost << L"руб .\n";
                    money += inventoryOfEnstruments[choiceForSell - 1]->cost;
                    std::wcout << L"Ваш баланс: " << money << L'\n';
                    inventoryOfEnstruments.erase(inventoryOfEnstruments.begin() + choiceForSell - 1);
                }
                else {
                    std::wcout << L"Неверный выбор!\n";
                }
            }
            else std::wcout << L"У вас нет инструментов\n";
            break;

        case 6:
            std::wcout << L"Вы вышли из магазина.\n";
            break;

        case 7:
            if (health < 100){
                if (inventoryOfFood.size() > 0) {
                    int choiceOfFood;

                    std::wcout << L"Что вы хотите съесть:\n";
                    for (int i = 0; i < inventoryOfFood.size(); ++i) {
                        std::wcout << i + 1 << L". " << inventoryOfFood[i]->name << L"(" << inventoryOfFood[i]->heal << L")\n";
                    }

                    std::wcin >> choiceOfFood;

                    if (choiceOfFood > 0 && choiceOfFood <= inventoryOfFood.size()) {
                        health += inventoryOfFood[choiceOfFood - 1]->heal;
                        inventoryOfFood.erase(inventoryOfFood.begin() + choiceOfFood - 1);
                        if (health > 100) {
                            health = 100;
                            std::wcout << L"У вас полное здоровье: " << health << L" хп\n";
                            break;
                        }
                        std::wcout << L"Вы поели и восстановили здоровье! Ваше хп: " << health << L'\n';
                    }
                    else std::wcout << L"Неверный выбор!\n";
                }
                else std::wcout << L"У вас нет еды!" << '\n';
			}
			else std::wcout << L"У вас полное здоровье!" << '\n';
            break;

        default:
            std::wcout << L"Неверный выбор!\n";
        }
    }

    void museum() {
        std::wcout << L"Вы посетили музей.\n";
        if (!inventory.empty()) {
            std::wcout << L"Ваши находки:\n";
            for (const auto& item : inventory) std::wcout << L"- " << item->getName() << L'\n';
        }
        else {
            std::wcout << L"У вас пока нет находок." << '\n';
        }
    }

    void expedition() {
        std::wcout << L"Выберите экспедицию:\n";
        for (int i = 0; i < expeditions.size(); ++i) {
            std::wcout << i + 1 << L". " << expeditions[i]->getName() << L" (Цена: " << expeditions[i]->getCost() << L")\n";
        }

        int choice;
        std::wcin >> choice;

        if (choice > 0 && choice <= expeditions.size()) {
            std::shared_ptr<Excavation> expedition = expeditions[choice - 1];

            if (money >= expedition->getCost()) {
                money -= expedition->getCost();

                int instrumentIndex = getPosition(expedition->getTerrain()->name, inventoryOfEnstruments);
                if (instrumentIndex != -1) {
                    std::shared_ptr<Enstrument> instrument = inventoryOfEnstruments[instrumentIndex];

                    if (expedition->getTerrain()->name == L"Подводные раскопки") expedition->printText(exp, instrument);
                    else expedition->printText(health);

                    instrumentIndex = getPosition(expedition->getTerrain()->name, inventoryOfEnstruments);
                    expedition->effect(instrument, inventoryOfEnstruments);

                    if (instrument->type == expedition->getTerrain()->name) {
                        expedition->excavate(inventory);
                    }
                    else std::wcout << L"У вас нет подходящего инструмента для этой местности!\n";
                }
                else std::wcout << L"У вас нет подходящего инструмента для этой местности!\n";
            }
            else std::wcout << L"Недостаточно денег или еды для экспедиции!\n";
        }
        else std::wcout << L"Неверный выбор!\n";
    }

    void play() {
        while (health > 0 && money >= 0 && inventory.size() >= 0 && inventoryOfFood.size() >= 0 && inventoryOfEnstruments.size() >= 0) {

            std::wcout << L"Ваши ресурсы: Деньги: " << money << L", Еды: " << inventoryOfFood.size() << L", Хп: " << health << L", Опыта " << exp << L'\n';

            std::wcout << L"1. Магазин\n"
                       << L"2. Экспедиция\n"
                       << L"3. Музей\n"
                       << L"4. Сохранить игру\n"
                       << L"5. Загрузить игру\n"
                       << L"6. Выйти из игры\n";

            int choice;
            std::wcin >> choice;

            switch (choice) {
            case 1:
                shop();
                break;
            case 2:
                expedition();
                break;
            case 3:
                museum();
                break;
            case 4:
                saveGame(L"savegame.txt");
                break;
            case 5:
                loadGame(L"savegame.txt");
                break;
            case 6:
                std::wcout << L"Спасибо за игру!\n";
                return;
            default:
                std::wcout << L"Неверный выбор!\n";
            }
        }

        std::wcout << L"Игра окончена. У вас закончились деньги или еда.\n";
    }
};

int main() {
	_setmode(_fileno(stdout), _O_U8TEXT);
	_setmode(_fileno(stdin), _O_U8TEXT);
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	setlocale(LC_ALL, "en_US.UTF-8");

	Game game;
	game.play();

	if (Loot::inputFile.is_open()) {
		Loot::inputFile.close();
	}
	return 0;
}