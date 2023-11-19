#include <iostream>
#include <boost/asio.hpp>

#include "Common.hpp"
#include "json.hpp"

using boost::asio::ip::tcp;

std::string my_id;

// Отправка сообщения на сервер по шаблону.
void SendMessage(
    tcp::socket& aSocket,
    const std::string& aId,
    const std::string& aRequestType,
    const std::string& aMessage)
{
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    req["Message"] = aMessage;

    std::string request = req.dump();
    boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
}

// Возвращает строку с ответом сервера на последний запрос.
std::string ReadMessage(tcp::socket& aSocket)
{
    boost::asio::streambuf b;
    boost::asio::read_until(aSocket, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return line;
}

// "Создаём" пользователя, получаем его ID.
std::string ProcessRegistration(tcp::socket& aSocket)
{
    std::cout << "Hello! Enter your name: ";
    std::string name;
    std::cin >> name;
    // Для регистрации Id не нужен, заполним его нулём
    SendMessage(aSocket, "0", Requests::Registration, name);
    return ReadMessage(aSocket);
}

std::string ProcessTrade(tcp::socket& aSocket)
{
    std::string amount, price, side;
    std::cout << "For creating trading request enter amount of currency: ";
    std::cin >> amount;
    std::cout << "Enter your price: ";
    std::cin >> price;
    std::cout << "Enter your side (sell/buy) in the format \"s\" or \"b\": ";
    std::cin >> side;

    nlohmann::json req;
    req["amount"] = amount;
    req["price"] = price;
    req["side"] = side;
    std::string request = req.dump();
 
    SendMessage(aSocket, my_id, Requests::Trade, request);
    return ReadMessage(aSocket);
}

std::string ActiveRequests(tcp::socket& aSocket) {
    SendMessage(aSocket, my_id, Requests::Active, "");
    return ReadMessage(aSocket);
}

std::string ComplitedRequests(tcp::socket& aSocket) {
    SendMessage(aSocket, my_id, Requests::Completed, "");
    return ReadMessage(aSocket);
}

std::string Balance(tcp::socket& aSocket) {
    SendMessage(aSocket, my_id, Requests::Balance, "");
    return ReadMessage(aSocket);
}

int main()
{
    try
    {
        boost::asio::io_service io_service;
        //host 
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
        tcp::resolver::iterator iterator = resolver.resolve(query);

        tcp::socket s(io_service);
        s.connect(*iterator);

        // Мы предполагаем, что для идентификации пользователя будет использоваться ID.
        // Тут мы "регистрируем" пользователя - отправляем на сервер имя, а сервер возвращает нам ID.
        // Этот ID далее используется при отправке запросов.
        my_id = ProcessRegistration(s);
        std::cout << "Your ID is " << my_id << '\n';
        while (true)
        {
            // Тут реализовано "бесконечное" меню.
            std::cout << "Menu:\n"
                         "1) Create trading request\n"
                         "2) Watch my active trading requests\n"
                         "3) Watch my completed trading requests\n"
                         "4) Watch my balance\n"
                         "5) Exit"
                         << std::endl;

            short menu_option_num;
            std::cin >> menu_option_num;
            switch (menu_option_num)
            {
                case 1:
                {
                    // Отправка торговой заявки
                    std::string res = ProcessTrade(s);
                    std::cout << res;
                    break;
                }
                case 2:
                {
                    std::string res = ActiveRequests(s);
                    std::cout << res;
                    break;
                }
                case 3:
                {
                    std::string res = ComplitedRequests(s);
                    std::cout << res;
                    break;
                }
                case 4:
                {
                    std::string res = Balance(s);
                    std::cout << res;
                    break;
                }
                case 5:
                {
                    exit(0);
                    break;
                }
                default:
                {
                    std::cout << "Unknown menu option\n" << std::endl;
                    exit(0);
                    break;
                }
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}