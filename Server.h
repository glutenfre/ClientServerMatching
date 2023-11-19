#pragma once
#include <cstdlib>
#include <iostream>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <map>
#include "json.hpp"
#include "Common.hpp"

using boost::asio::ip::tcp;

struct TradeInfo {
    size_t user_id = -1;
    int amount = 0;
};

struct UserInfo {
    int rub_cur = 0;
    int dol_cur = 0;
};

struct RequestInfo {
    int price = 0;
    int amount = 0;
    char side = 'n'; // 'n' - no side
};

class DataStorage {
public:
    bool UserNameExist(const std::string& name, int* id);

    int RegisterUser(const std::string& name);

    std::string GetActiveRequests(int id);
    std::string GetComplitedRequests(int id);
    std::string GetBalanceByID(int id);
    std::map<int, std::vector<TradeInfo>>* GetBuyRequests();
    std::map<int, std::vector<TradeInfo>>* GetSellRequests();
    std::string StoreSellRequest(int id, int price, int amount);
    std::string StoreBuyRequest(int id, int price, int amount);
    void UpdateUserStats(int id, int rub, int usd);
    void UpdateComplitedRequests(int id, int price, int amount, char side);
private:
    std::map<std::string, size_t> users_ids_;
    std::vector<UserInfo> users_stats_;
    std::map<int, std::vector<TradeInfo>> sell_m_;
    std::map<int, std::vector<TradeInfo>> buy_m_;
    std::map<int, std::vector<RequestInfo>> comp_req_m_;

};

class User {
public:
    User(DataStorage* storage);

    int Register(const std::string& name);

private:
    DataStorage* storage_;
};

class Request {
public:
    Request(const std::string& message, DataStorage* storage, int id);

    std::string Process();

private:

    std::string Sell();

    std::string Buy();

    std::string SellMore(int id_to_buy, int price_to_buy, int amount_to_buy);

    std::string SellLess(int id_to_buy, int price_to_buy);

    std::string SellSame(int id_to_buy, int price_to_buy, int amount_to_buy);

    std::string BuyMore(int id_to_sell, int price_to_sell, int amount_to_sell);

    std::string BuyLess(int id_to_sell, int price_to_sell);

    std::string BuySame(int id_to_sell, int price_to_sell, int amount_to_sell);

    DataStorage* storage_;
    int id_ = -1;
    int amount_ = 0;
    int price_ = 0;
    char side_ = 'n';
};

class RequestCenter {
public:
    RequestCenter(DataStorage* storage);

    int RegisterNewUser(const std::string& name);

    std::string RegisterTradeRequest(const std::string& message, int id);

    std::string GetActive(int id);

    std::string GetComplited(int id);

    std::string GetBalance(int id);

private:
    DataStorage* storage_;
};

class session : public boost::enable_shared_from_this<session>
{
public:
    static boost::shared_ptr<session> Create(boost::asio::io_service& io_service, DataStorage* core);

    tcp::socket& socket();

    void start();

    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred);

    void handle_write(const boost::system::error_code& error,
        size_t bytes_transferred);

private:
    session(boost::asio::io_service& io_service, DataStorage* storage);

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
    std::string reply;
    DataStorage* storage_;
};

class server
{
public:
    server(boost::asio::io_service& io_service);

    void handle_accept(boost::shared_ptr<session> new_session,
        const boost::system::error_code& error);

private:
    void start_accept();

    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
    DataStorage storage_;
};