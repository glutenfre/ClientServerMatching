#include "Server.h"

std::string MakeMessage(int id, int price, int amount, std::string side) {
    nlohmann::json req_data;
    req_data["amount"] = std::to_string(amount);
    req_data["price"] = std::to_string(price);
    req_data["side"] = side;
    /*nlohmann::json req;
    req["UserId"] = id;
    req["ReqType"] = Requests::Trade;
    req["Message"] = req_data.dump();*/
    std::string request = req_data.dump();
    return request;
}

void TestUserRegistration() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    assert(req_center.RegisterNewUser("Ivan") == 0);
    assert(req_center.RegisterNewUser("Masha") == 1);
    assert(req_center.RegisterNewUser("Lena") == 2);
    assert(req_center.RegisterNewUser("Ivan") == 0);
    assert(req_center.RegisterNewUser("Lena") == 2);
}

void TestManySellOneBuy() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "s";
    std::string request = MakeMessage(id, price, amount, side);
    std::string reply = "User " + std::to_string(id) +
        " made a trade request to sell " + std::to_string(amount) +
        " USD.\n";
    assert(req_center.RegisterTradeRequest(request, id) == reply);

    amount = 2;
    price = 63;
    side = "s";
    request = MakeMessage(id, price, amount, side);
    reply = "User " + std::to_string(id) +
        " made a trade request to sell " + std::to_string(amount) +
        " USD.\n";
    assert(req_center.RegisterTradeRequest(request, id) == reply);

    id = 1;
    amount = 1;
    price = 67;
    side = "b";
    request = MakeMessage(id, price, amount, side);
    reply = "User 1 bought 1 USD for 67 RUB from User 0 .\n";
    std::string test_reply = req_center.RegisterTradeRequest(request, id);
    assert(test_reply == reply);

    std::cout << "Tests passed successfully!\n";
}
void TestManySellOneBuyOne() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "s";
    std::string request = MakeMessage(id, price, amount, side);
    std::string reply = "User " + std::to_string(id) +
        " made a trade request to sell " + std::to_string(amount) +
        " USD.\n";
    assert(req_center.RegisterTradeRequest(request, id) == reply);

    amount = 2;
    price = 63;
    request = MakeMessage(id, price, amount, side);
    reply = "User " + std::to_string(id) +
        " made a trade request to sell " + std::to_string(amount) +
        " USD.\n";
    assert(req_center.RegisterTradeRequest(request, id) == reply);

    id = 1;
    amount = 1;
    price = 67;
    side = "b";
    request = MakeMessage(id, price, amount, side);
    reply = "User 1 bought 1 USD for 67 RUB from User 0 .\n";
    std::string test_reply = req_center.RegisterTradeRequest(request, id);
    assert(test_reply == reply);
}

void TestManySellOneBuyMany() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "s";
    std::string request = MakeMessage(id, price, amount, side);

    req_center.RegisterTradeRequest(request, id);

    amount = 2;
    price = 63;
    request = MakeMessage(id, price, amount, side);

    req_center.RegisterTradeRequest(request, id);

    id = 1;
    amount = 5;
    price = 67;
    side = "b";
    request = MakeMessage(id, price, amount, side);
    std::string reply = "User 1 bought 1 USD for 67 RUB from User 0 .\n"
        "User 1 bought 2 USD for 134 RUB from User 0 .\n"
        "User 1 made a trade request to buy 2 USD.\n";
    std::string test_reply = req_center.RegisterTradeRequest(request, id);
    assert(test_reply == reply);
}

void TestManyBuyOneSellOne() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "b";
    std::string request = MakeMessage(id, price, amount, side);
    std::string reply = "User " + std::to_string(id) +
        " made a trade request to buy " + std::to_string(amount) +
        " USD.\n";
    std::string r = req_center.RegisterTradeRequest(request, id);
    assert(r == reply);

    amount = 2;
    price = 63;
    request = MakeMessage(id, price, amount, side);
    reply = "User " + std::to_string(id) +
        " made a trade request to buy " + std::to_string(amount) +
        " USD.\n";
    assert(req_center.RegisterTradeRequest(request, id) == reply);

    id = 1;
    amount = 1;
    price = 60;
    side = "s";
    request = MakeMessage(id, price, amount, side);
    reply = "User 1 sold 1 USD for 63 RUB to User 0 .\n";
    std::string test_reply = req_center.RegisterTradeRequest(request, id);
    assert(test_reply == reply);
}

void TestManyBuyOneSellMany() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "b";
    std::string request = MakeMessage(id, price, amount, side);

    req_center.RegisterTradeRequest(request, id);

    amount = 2;
    price = 63;
    request = MakeMessage(id, price, amount, side);

    req_center.RegisterTradeRequest(request, id);

    id = 1;
    amount = 5;
    price = 57;
    side = "s";
    request = MakeMessage(id, price, amount, side);
    std::string reply = "User 1 sold 2 USD for 126 RUB to User 0 .\n"
        "User 1 sold 1 USD for 61 RUB to User 0 .\n"
        "User 1 made a trade request to sell 2 USD.\n";
    std::string test_reply = req_center.RegisterTradeRequest(request, id);
    assert(test_reply == reply);
}

void TestCantBuy() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "s";
    std::string request = MakeMessage(id, price, amount, side);

    req_center.RegisterTradeRequest(request, id);

    id = 1;
    amount = 5;
    price = 57;
    side = "b";
    request = MakeMessage(id, price, amount, side);
    std::string reply = "User 1 made a trade request to buy 5 USD.\n";
    std::string test_reply = req_center.RegisterTradeRequest(request, id);
    assert(test_reply == reply);
}

void TestCantSell() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "b";
    std::string request = MakeMessage(id, price, amount, side);

    req_center.RegisterTradeRequest(request, id);

    id = 1;
    amount = 5;
    price = 80;
    side = "s";
    request = MakeMessage(id, price, amount, side);
    std::string reply = "User 1 made a trade request to sell 5 USD.\n";
    std::string test_reply = req_center.RegisterTradeRequest(request, id);
    assert(test_reply == reply);
}

void TestWrongSide() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "p";
    std::string request = MakeMessage(id, price, amount, side);

    std::string test_reply = req_center.RegisterTradeRequest(request, id);

    std::string reply = "Error: wrong trade side.\n";
    assert(test_reply == reply);
}

void TestNoBalance() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    int id = 0;

    std::string test_reply = req_center.GetBalance(id);
    std::string reply = "You don`t have a balance yet.\n";
    assert(test_reply == reply);
}

void TestBalance() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "s";
    std::string request = MakeMessage(id, price, amount, side);

    req_center.RegisterTradeRequest(request, id);
    std::string test_reply = req_center.GetBalance(id);
    std::string reply = "Current balance: 1 USD, 0 RUB.\n";
    assert(test_reply == reply);
}

void TestBalanceChanged() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "s";
    std::string request = MakeMessage(id, price, amount, side);

    req_center.RegisterTradeRequest(request, id);
    std::string test_reply = req_center.GetBalance(id);
    std::string reply = "Current balance: 1 USD, 0 RUB.\n";
    assert(test_reply == reply);

    id = 1;
    amount = 2;
    price = 63;
    side = "b";
    request = MakeMessage(id, price, amount, side);
    req_center.RegisterTradeRequest(request, id);
    test_reply = req_center.GetBalance(id);
    reply = "Current balance: 1 USD, 63 RUB.\n";
    assert(test_reply == reply);
}

void TestNoActiveRequests() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "s";
    std::string request = MakeMessage(id, price, amount, side);

    req_center.RegisterTradeRequest(request, id);

    id = 1;
    amount = 1;
    price = 63;
    side = "b";
    request = MakeMessage(id, price, amount, side);
    req_center.RegisterTradeRequest(request, id);
    std::string test_reply = req_center.GetActive(id);
    std::string reply = "No active requests.\n";
    assert(test_reply == reply);
}

void TestActiveRequests() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "s";
    std::string request = MakeMessage(id, price, amount, side);

    req_center.RegisterTradeRequest(request, id);

    std::string test_reply = req_center.GetActive(id);
    std::string reply = "Requests to sell: 1 USD for 61 RUB.\n";
    assert(test_reply == reply);
}

void TestComplitedRequests() {
    DataStorage storage;
    RequestCenter req_center(&storage);
    req_center.RegisterNewUser("Ivan");
    req_center.RegisterNewUser("Masha");

    int id = 0;
    int amount = 1;
    int price = 61;
    std::string side = "s";
    std::string request = MakeMessage(id, price, amount, side);
    req_center.RegisterTradeRequest(request, id);
    std::string r = req_center.GetComplited(id);
    std::string reply = "No complited requests.\n";
    assert(r == reply);

    id = 1;
    amount = 1;
    price = 67;
    side = "b";
    request = MakeMessage(id, price, amount, side);
    req_center.RegisterTradeRequest(request, id);
    id = 0;
    r = req_center.GetComplited(id);
    reply = "Request to sell 1 USD for 61 RUB is completed.\n";
    assert(r == reply);
    id = 1;
    r = req_center.GetComplited(id);
    reply = "Request to buy 1 USD for 67 RUB is completed.\n";
    assert(r == reply);
}

void Test() {
    TestUserRegistration();
    TestManySellOneBuyOne();
    TestManySellOneBuyMany();
    TestManyBuyOneSellOne();
    TestManyBuyOneSellMany();
    TestCantBuy();
    TestCantSell();
    TestWrongSide();
    TestNoBalance();
    TestBalance();
    TestBalanceChanged();
    TestNoActiveRequests();
    TestActiveRequests();
    TestComplitedRequests();
    std::cout << "Tests passed successfully!\n";
}