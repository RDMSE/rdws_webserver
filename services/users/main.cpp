#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>

struct User
{
    int id;
    std::string name;
    std::string email;
};

std::string userToJson(const User &user)
{
    std::ostringstream oss;
    oss << "{"
        << "\"id\":" << user.id << ","
        << "\"name\":\"" << user.name << "\","
        << "\"email\":\"" << user.email << "\""
        << "}";
    return oss.str();
}

std::string usersToJson(const std::vector<User> &users)
{
    std::ostringstream oss;
    oss << "{"
        << "\"users\":[";

    for (size_t i = 0; i < users.size(); ++i)
    {
        oss << userToJson(users[i]);
        if (i < users.size() - 1)
        {
            oss << ",";
        }
    }

    oss << "],"
        << "\"total\":" << users.size() << ","
        << "\"source\":\"users_service C++ executable\","
        << "\"endpoint\":\"/users\","
        << "\"timestamp\":" << std::time(nullptr)
        << "}";

    return oss.str();
}

int main(int argc, char *argv[])
{
    // Simular base de dados de usuários
    std::vector<User> users = {
        {1, "João Silva", "joao@example.com"},
        {2, "Maria Santos", "maria@example.com"},
        {3, "Pedro Costa", "pedro@example.com"},
        {4, "Ana Oliveira", "ana@example.com"},
        {5, "Carlos Ferreira", "carlos@example.com"}};

    // Processar argumentos da linha de comando
    std::string method = "GET";
    std::string path = "/users";

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
        if (path == "/users" || path == "/")
        {
            // Listar todos os usuários
            std::cout << usersToJson(users) << std::endl;
            return 0;
        }
        else if (path.find("/users/") == 0)
        {
            // Buscar usuário específico
            try
            {
                int userId = std::stoi(path.substr(7)); // Remove "/users/"

                for (const auto &user : users)
                {
                    if (user.id == userId)
                    {
                        std::cout << "{"
                                  << "\"user\":" << userToJson(user) << ","
                                  << "\"source\":\"users_service C++ executable\","
                                  << "\"endpoint\":\"" << path << "\","
                                  << "\"timestamp\":" << std::time(nullptr)
                                  << "}" << std::endl;
                        return 0;
                    }
                }

                // Usuário não encontrado
                std::cout << "{"
                          << "\"error\":\"User not found\","
                          << "\"userId\":" << userId << ","
                          << "\"source\":\"users_service C++ executable\""
                          << "}" << std::endl;
                return 1;
            }
            catch (const std::exception &e)
            {
                std::cout << "{"
                          << "\"error\":\"Invalid user ID\","
                          << "\"path\":\"" << path << "\","
                          << "\"source\":\"users_service C++ executable\""
                          << "}" << std::endl;
                return 1;
            }
        }
    }
    else if (method == "POST")
    {
        // Simular criação de usuário
        int newId = users.size() + 1;
        User newUser = {newId, "Novo Usuário", "novo@example.com"};

        std::cout << "{"
                  << "\"message\":\"User created successfully\","
                  << "\"user\":" << userToJson(newUser) << ","
                  << "\"source\":\"users_service C++ executable\","
                  << "\"timestamp\":" << std::time(nullptr)
                  << "}" << std::endl;
        return 0;
    }

    // Método não suportado
    std::cout << "{"
              << "\"error\":\"Method not allowed\","
              << "\"method\":\"" << method << "\","
              << "\"path\":\"" << path << "\","
              << "\"source\":\"users_service C++ executable\""
              << "}" << std::endl;
    return 1;
}
