#include <cstdlib>
#include <iostream>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include "json.hpp"
#include "Common.hpp"
#include "Server.h"
#include "Tests.h"

using boost::asio::ip::tcp;


bool DataStorage::UserNameExist(const std::string& name, int* id) {
    for (const auto& [user_name, user_id] : users_ids_) {
        if (name == user_name) {
            *id = user_id;
            return true;
        }
    }
    return false;
}

int DataStorage::RegisterUser(const std::string& name) {
    UserInfo u_i;
    users_stats_.push_back(u_i);
    users_ids_[name] = (users_stats_.size() - 1);
    return users_stats_.size() - 1;
}

std::string DataStorage::GetActiveRequests(int id) {
    std::string sell = "Requests to sell: ";
    for (const auto& [price, user_info_v] : sell_m_) {
        for (int i = 0; i < user_info_v.size(); i++) {
            if (user_info_v[i].user_id == id) {
                if (sell == "Requests to sell: ")
                    sell += std::to_string(user_info_v[i].amount) + " USD for " + std::to_string(price) + " RUB";
                else
                    sell += ", " + std::to_string(user_info_v[i].amount) + " USD for " + std::to_string(price) + " RUB";
            }
        }
    }
    std::string buy = "\nRequests to buy: ";
    for (const auto& [price, user_info_v] : buy_m_) {
        for (int i = 0; i < user_info_v.size(); i++) {
            if (user_info_v[i].user_id == id) {
                if (buy == "\nRequests to buy: ")
                    buy += std::to_string(user_info_v[i].amount) + " USD for " + std::to_string(-1 * price) + " RUB";
                else
                    buy += ", " + std::to_string(user_info_v[i].amount) + " USD for " + std::to_string(-1 * price) + " RUB";
            }
        }
    }
    std::string reply;
    if (sell != "Requests to sell: ")
        reply += sell;
    if (buy != "\nRequests to buy: ")
        reply += buy;
    if (reply.empty())
        reply = "No active requests.\n";
    else
        reply += ".\n";
    return reply;
}
std::string DataStorage::GetComplitedRequests(int id) {
    std::string reply;
    if (comp_req_m_.count(id) > 0) {
        for (int i = 0; i < comp_req_m_[id].size(); i++) {
            if (comp_req_m_[id].at(i).side == 's')
                reply += "Request to sell " + std::to_string(comp_req_m_[id].at(i).amount) + " USD for " + std::to_string(comp_req_m_[id].at(i).price) + " RUB is completed.\n";
            else if (comp_req_m_[id].at(i).side == 'b')
                reply += "Request to buy " + std::to_string(comp_req_m_[id].at(i).amount) + " USD for " + std::to_string(comp_req_m_[id].at(i).price) + " RUB is completed.\n";
            else
                reply += "Broken request.\n";
        }
    }
    else
        reply = "No complited requests.\n";
    return reply;
}
std::string DataStorage::GetBalanceByID(int id) {
    std::string reply;
    if (id < users_stats_.size()) {
        if ((users_stats_[id].dol_cur == 0) && (users_stats_[id].rub_cur == 0)) {
            reply = "You don`t have a balance yet.\n";
            return reply;
        }
        else {
            reply = "Current balance: " + std::to_string(users_stats_[id].dol_cur) + " USD, " + std::to_string(users_stats_[id].rub_cur) + " RUB.\n";
            return reply;
        }
    }
    reply = "You don`t have balance yet.\n";
    return reply;
}
std::map<int, std::vector<TradeInfo>>* DataStorage::GetBuyRequests() {
    return &buy_m_;
}
std::map<int, std::vector<TradeInfo>>* DataStorage::GetSellRequests() {
    return &sell_m_;
}
std::string DataStorage::StoreSellRequest(int id, int price, int amount) {
    std::string reply;
    users_stats_[id].dol_cur += amount;
    TradeInfo t_i;
    t_i.amount = amount;
    t_i.user_id = id;
    sell_m_[price].push_back(t_i);
    reply = "User " + std::to_string(id) +
        " made a trade request to sell " + std::to_string(amount) +
        " USD.\n";
    return reply;
}
std::string DataStorage::StoreBuyRequest(int id, int price, int amount) {
    std::string reply;
    users_stats_[id].rub_cur += amount * price;
    TradeInfo t_i;
    t_i.amount = amount;
    t_i.user_id = id;
    buy_m_[-1 * price].push_back(t_i);
    reply = "User " + std::to_string(id) +
        " made a trade request to buy " + std::to_string(amount) +
        " USD.\n";
    return reply;
}
void DataStorage::UpdateUserStats(int id, int rub, int usd) {
    users_stats_[id].rub_cur += rub;
    users_stats_[id].dol_cur += usd;
}
void DataStorage::UpdateComplitedRequests(int id, int price, int amount, char side) {
    RequestInfo r_i;
    r_i.amount = amount;
    r_i.price = price;
    r_i.side = side;
    comp_req_m_[id].push_back(r_i);
}

User::User(DataStorage* storage) :
    storage_(storage)
{
}

int User::Register(const std::string& name) {
    int new_id = storage_->RegisterUser(name);
    return new_id;
}


Request::Request(const std::string& message, DataStorage* storage, int id) :
    storage_(storage), id_(id)
{
    auto j = nlohmann::json::parse(message);
    amount_ = std::stoi(std::string(j["amount"]));
    price_ = std::stoi(std::string(j["price"]));
    side_ = std::string(j["side"])[0];
}

std::string Request::Process() {
    if (side_ == 's')
        return Sell();
    else if (side_ == 'b')
        return Buy();
    return "Error: wrong trade side.\n";
}

std::string Request::Sell() {
    std::string reply;
    for (auto& [neg_price, trade_info_v] : *storage_->GetBuyRequests()) {
        if (amount_ > 0) {
            if (price_ < (-1 * neg_price)) {
                for (int i = 0; i < trade_info_v.size(); i++) {
                    if (amount_ > trade_info_v[i].amount) {
                        reply += SellMore(trade_info_v[i].user_id, (-1 * neg_price), trade_info_v[i].amount);
                        trade_info_v.erase(trade_info_v.begin() + i);
                        i--;
                    }
                    else if (amount_ < trade_info_v[i].amount) {
                        reply += SellLess(trade_info_v[i].user_id, (-1 * neg_price));
                        trade_info_v[i].amount -= amount_;
                        return reply;
                    }
                    else {
                        reply += SellSame(trade_info_v[i].user_id, (-1 * neg_price), trade_info_v[i].amount);
                        trade_info_v.erase(trade_info_v.begin() + i);
                        i--;
                        return reply;
                    }

                }
            }
        }
    }
    reply += storage_->StoreSellRequest(id_, price_, amount_);
    return reply;
}

std::string Request::Buy() {
    std::string reply;
    for (auto& [price, trade_info_v] : *storage_->GetSellRequests()) {
        if (amount_ > 0) {
            if (price_ > price) {
                for (int i = 0; i < trade_info_v.size(); i++) {
                    if (amount_ > trade_info_v[i].amount) {
                        reply += BuyMore(trade_info_v[i].user_id, price, trade_info_v[i].amount);
                        trade_info_v.erase(trade_info_v.begin() + i);
                        i--;
                    }
                    else if (amount_ < trade_info_v[i].amount) {
                        reply += BuyLess(trade_info_v[i].user_id, price);
                        trade_info_v[i].amount -= amount_;
                        return reply;
                    }
                    else {
                        reply += BuySame(trade_info_v[i].user_id, price, trade_info_v[i].amount);
                        trade_info_v.erase(trade_info_v.begin() + i);
                        i--;
                        return reply;
                    }

                }
            }
        }
    }
    reply += storage_->StoreBuyRequest(id_, price_, amount_);
    return reply;
}

std::string Request::SellMore(int id_to_buy, int price_to_buy, int amount_to_buy) {
    std::string reply;
    amount_ -= amount_to_buy;
    //Client selling
    // <RUB, USD>
    storage_->UpdateUserStats(id_, price_to_buy * amount_to_buy, 0);
    //Client buying
    storage_->UpdateUserStats(id_to_buy, (-1 * price_to_buy * amount_to_buy), amount_to_buy);

    storage_->UpdateComplitedRequests(id_to_buy, price_to_buy, amount_to_buy, 'b');

    reply = "User " + std::to_string(id_) +
        " sold " + std::to_string(amount_to_buy) +
        " USD for " + std::to_string(price_to_buy * amount_to_buy) +
        " RUB to User " + std::to_string(id_to_buy) +
        " .\n";
    return reply;
}

std::string Request::SellLess(int id_to_buy, int price_to_buy) {
    std::string reply;
    //Client selling
    // <RUB, USD>
    storage_->UpdateUserStats(id_, price_to_buy * amount_, 0);
    //Client buying
    storage_->UpdateUserStats(id_to_buy, (-1 * price_to_buy * amount_), amount_);

    reply = "User " + std::to_string(id_) +
        " sold " + std::to_string(amount_) +
        " USD for " + std::to_string(price_to_buy * amount_) +
        " RUB to User " + std::to_string(id_to_buy) +
        " .\n";
    return reply;
}

std::string Request::SellSame(int id_to_buy, int price_to_buy,int amount_to_buy) {
    std::string reply;
    //Client selling
    // <RUB, USD>
    storage_->UpdateUserStats(id_, price_to_buy * amount_, 0);
    //Client buying
    storage_->UpdateUserStats(id_to_buy, (-1 * price_to_buy * amount_), amount_);

    storage_->UpdateComplitedRequests(id_to_buy, price_to_buy, amount_to_buy, 'b');
    storage_->UpdateComplitedRequests(id_, price_, amount_, 's');

    reply = "User " + std::to_string(id_) +
        " sold " + std::to_string(amount_) +
        " USD for " + std::to_string(price_to_buy * amount_) +
        " RUB to User " + std::to_string(id_to_buy) +
        " .\n";
    return reply;
}

std::string Request::BuyMore(int id_to_sell, int price_to_sell, int amount_to_sell) {
    std::string reply;
    amount_ -= amount_to_sell;
    //Client selling
    // <RUB, USD>
    storage_->UpdateUserStats(id_to_sell, price_to_sell * amount_to_sell, (-1 * amount_to_sell));
    //Client buying
    storage_->UpdateUserStats(id_, 0, amount_to_sell);

    storage_->UpdateComplitedRequests(id_to_sell, price_to_sell, amount_to_sell, 's');

    reply = "User " + std::to_string(id_) +
        " bought " + std::to_string(amount_to_sell) +
        " USD for " + std::to_string(price_ * amount_to_sell) +
        " RUB from User " + std::to_string(id_to_sell) +
        " .\n";
    return reply;
}

std::string Request::BuyLess(int id_to_sell, int price_to_sell) {
    std::string reply;
    //Client selling
    // <RUB, USD>
    storage_->UpdateUserStats(id_to_sell, price_to_sell * amount_, (-1 * amount_));
    //Client buying
    storage_->UpdateUserStats(id_, 0, amount_);

    reply = "User " + std::to_string(id_) +
        " bought " + std::to_string(amount_) +
        " USD for " + std::to_string(price_ * amount_) +
        " RUB from User " + std::to_string(id_to_sell) +
        " .\n";
    return reply;
}

std::string Request::BuySame(int id_to_sell, int price_to_sell, int amount_to_sell) {
    std::string reply;
    //Client selling
    // <RUB, USD>
    storage_->UpdateUserStats(id_to_sell, price_to_sell * amount_, (-1 * amount_));
    //Client buying
    storage_->UpdateUserStats(id_, 0, amount_);

    storage_->UpdateComplitedRequests(id_to_sell, price_to_sell, amount_to_sell, 's');
    storage_->UpdateComplitedRequests(id_, price_, amount_, 'b');

    reply = "User " + std::to_string(id_) +
        " bought " + std::to_string(amount_) +
        " USD for " + std::to_string(price_ * amount_) +
        " RUB from User " + std::to_string(id_to_sell) +
        " .\n";
    return reply;
}

RequestCenter::RequestCenter(DataStorage* storage) :
storage_(storage)
{
}

int RequestCenter::RegisterNewUser(const std::string& name)
{
    int id;
    if (storage_->UserNameExist(name, &id)) {
        return id;
    }
    User user(storage_);
    return user.Register(name);
}

std::string RequestCenter::RegisterTradeRequest(const std::string& message, int id) {
    Request request(message, storage_, id);
    std::string reply = request.Process();
    return reply;
}

std::string RequestCenter::GetActive(int id) {
    std::string reply;
    reply = storage_->GetActiveRequests(id);
    return reply;
}

std::string RequestCenter::GetComplited(int id) {
    std::string reply;
    reply = storage_->GetComplitedRequests(id);
    return reply;
}

std::string RequestCenter::GetBalance(int id) {
    std::string reply;
    reply = storage_->GetBalanceByID(id);
    return reply;
}

boost::shared_ptr<session> session::Create(boost::asio::io_service& io_service, DataStorage* core) {
    return boost::shared_ptr<session>(new session(io_service, core));
}

tcp::socket& session::socket()
{
    return socket_;
}

void session::start()
{
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

// Обработка полученного сообщения.
void session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred)
{
    if (!error)
    {
        data_[bytes_transferred] = '\0';

        // Парсим json, который пришёл нам в сообщении.
        auto j = nlohmann::json::parse(data_);
        auto reqType = j["ReqType"];

        RequestCenter req_center(storage_);

        reply = "Error! Unknown request type";
        if (reqType == Requests::Registration)
        {
            reply = std::to_string(req_center.RegisterNewUser(j["Message"]));
        }
        else if (reqType == Requests::Trade)
        {
            reply = req_center.RegisterTradeRequest(j["Message"], std::stoi(std::string(j["UserId"])));
        }
        else if (reqType == Requests::Active)
        {
            reply = req_center.GetActive(std::stoi(std::string(j["UserId"])));
        }
        else if (reqType == Requests::Completed)
        {
            reply = req_center.GetComplited(std::stoi(std::string(j["UserId"])));
        }
        else if (reqType == Requests::Balance)
        {
            reply = req_center.GetBalance(std::stoi(std::string(j["UserId"])));
        }
        boost::asio::async_write(socket_,
            boost::asio::buffer(reply, reply.size()),
            boost::bind(&session::handle_write, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
}

void session::handle_write(const boost::system::error_code& error,
    size_t bytes_transferred)
{
    if (!error)
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
}

session::session(boost::asio::io_service& io_service, DataStorage* storage)
    : socket_(io_service),
    storage_(storage)
{
}

server::server(boost::asio::io_service& io_service)
    : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "Server started! Listen " << port << " port" << std::endl;
    start_accept();
}

void server::handle_accept(boost::shared_ptr<session> new_session,
    const boost::system::error_code& error)
{
    if (!error)
    {
        new_session->start();
    }
    start_accept();
}

void server::start_accept()
{
    boost::shared_ptr<session> new_session_ = session::Create(io_service_, &storage_);
    acceptor_.async_accept(new_session_->socket(),
        boost::bind(&server::handle_accept, this, new_session_,
            boost::asio::placeholders::error));
}

int main()
{
    try
    {
        boost::asio::io_service io_service;

        server s(io_service);

        Test();
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}