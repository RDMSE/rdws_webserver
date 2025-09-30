#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>

struct Order
{
    int id;
    int userId;
    std::string product;
    double amount;
    std::string status;
};

std::string orderToJson(const Order &order)
{
    std::ostringstream oss;
    oss << "{"
        << "\"id\":" << order.id << ","
        << "\"userId\":" << order.userId << ","
        << "\"product\":\"" << order.product << "\","
        << "\"amount\":" << order.amount << ","
        << "\"status\":\"" << order.status << "\""
        << "}";
    return oss.str();
}

std::string ordersToJson(const std::vector<Order> &orders)
{
    std::ostringstream oss;
    oss << "{"
        << "\"orders\":[";

    for (size_t i = 0; i < orders.size(); ++i)
    {
        oss << orderToJson(orders[i]);
        if (i < orders.size() - 1)
        {
            oss << ",";
        }
    }

    oss << "],"
        << "\"total\":" << orders.size() << ","
        << "\"source\":\"orders_service C++ executable\","
        << "\"endpoint\":\"/orders\","
        << "\"timestamp\":" << std::time(nullptr)
        << "}";

    return oss.str();
}

int main(int argc, char *argv[])
{
    // Simular base de dados de pedidos
    std::vector<Order> orders = {
        {1, 1, "Laptop Dell", 2500.00, "completed"},
        {2, 2, "Mouse Logitech", 150.00, "pending"},
        {3, 1, "Teclado Mecânico", 400.00, "shipped"},
        {4, 3, "Monitor 4K", 1200.00, "completed"}};

    // Processar argumentos
    std::string method = "GET";
    std::string path = "/orders";

    if (argc > 1)
    {
        method = argv[1];
    }
    if (argc > 2)
    {
        path = argv[2];
    }

    // Processar requisição
    if (method == "GET")
    {
        if (path == "/orders" || path == "/")
        {
            // Listar todos os pedidos
            std::cout << ordersToJson(orders) << std::endl;
            return 0;
        }
        else if (path.find("/orders/") == 0)
        {
            // Buscar pedido específico
            try
            {
                int orderId = std::stoi(path.substr(8)); // Remove "/orders/"

                for (const auto &order : orders)
                {
                    if (order.id == orderId)
                    {
                        std::cout << "{"
                                  << "\"order\":" << orderToJson(order) << ","
                                  << "\"source\":\"orders_service C++ executable\","
                                  << "\"endpoint\":\"" << path << "\","
                                  << "\"timestamp\":" << std::time(nullptr)
                                  << "}" << std::endl;
                        return 0;
                    }
                }

                // Pedido não encontrado
                std::cout << "{"
                          << "\"error\":\"Order not found\","
                          << "\"orderId\":" << orderId << ","
                          << "\"source\":\"orders_service C++ executable\""
                          << "}" << std::endl;
                return 1;
            }
            catch (const std::exception &e)
            {
                std::cout << "{"
                          << "\"error\":\"Invalid order ID\","
                          << "\"path\":\"" << path << "\","
                          << "\"source\":\"orders_service C++ executable\""
                          << "}" << std::endl;
                return 1;
            }
        }
    }

    // Método não suportado
    std::cout << "{"
              << "\"error\":\"Method not allowed\","
              << "\"method\":\"" << method << "\","
              << "\"path\":\"" << path << "\","
              << "\"source\":\"orders_service C++ executable\""
              << "}" << std::endl;
    return 1;
}
