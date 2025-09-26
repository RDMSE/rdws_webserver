# C++ REST Server with Pistache

Um servidor REST simples em C++ usando o framework Pistache, criado para Fedora Server.

> 🏠➡️🖥️ **Desenvolvimento Local + Deploy Remoto**: Veja [DEVELOPMENT.md](DEVELOPMENT.md) para guia completo de como desenvolver localmente (Linux Mint) e fazer deploy no servidor.

## Funcionalidades

- Servidor HTTP REST básico
- Endpoint GET `/hello` que retorna "Hello World from C++ REST Server!"
- Endpoint GET `/` (raiz) que também retorna a mensagem de Hello World
- Execução em http://localhost:9080
- Shutdown gracioso com Ctrl+C

## Pré-requisitos

### Fedora Server

Instale as dependências necessárias:

```bash
# Atualizar sistema
sudo dnf update -y

# Instalar ferramentas de desenvolvimento essenciais
sudo dnf groupinstall -y "Development Tools" "Development Libraries"

# Instalar CMake
sudo dnf install -y cmake

# Instalar compilador C++ e ferramentas
sudo dnf install -y gcc-c++ make

# Instalar dependências do Pistache
sudo dnf install -y pistache-devel

# Se pistache-devel não estiver disponível, instalar dependências para compilar do código fonte
sudo dnf install -y curl-devel rapidjson-devel
```

### Se Pistache não estiver nos repositórios

Caso o Pistache não esteja disponível via DNF, você pode compilá-lo do código fonte:

```bash
# Clonar e compilar Pistache
git clone https://github.com/pistacheio/pistache.git
cd pistache
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
sudo ldconfig
cd ../..
```

## Estrutura do Projeto

```
server/
├── src/
│   ├── main.cpp           # Arquivo principal com inicialização do servidor
│   └── hello_server.cpp   # Implementação da classe HelloServer
├── include/
│   └── hello_server.h     # Cabeçalho da classe HelloServer
├── CMakeLists.txt         # Configuração do CMake
└── README.md             # Esta documentação
```

## Compilação

1. Crie um diretório de build:
```bash
mkdir build && cd build
```

2. Configure o projeto com CMake:
```bash
cmake ..
```

3. Compile:
```bash
make
```

## Execução

Após a compilação, execute o servidor:

```bash
# A partir do diretório build
./rest_server
```

O servidor iniciará e estará disponível em:
- http://localhost:9080/hello
- http://localhost:9080/

## Testando

Você pode testar os endpoints usando curl:

```bash
# Teste o endpoint /hello
curl http://localhost:9080/hello

# Teste o endpoint raiz
curl http://localhost:9080/

# Ambos devem retornar: "Hello World from C++ REST Server!"
```

## Configuração

O servidor está configurado para:
- **Porta**: 9080 (configurável no main.cpp)
- **Threads**: 2 (configurável no main.cpp)
- **IP**: 0.0.0.0 (aceita conexões de qualquer IP)

Para alterar essas configurações, edite o arquivo `src/main.cpp`.

## Testando

O projeto inclui dois tipos de testes:

### 🧪 **Testes Unitários** (Google Test)
Testam a lógica das classes sem dependências externas.

### 🌐 **Testes de Integração** 
Testam endpoints HTTP reais com requisições completas.

### Executar Testes

```bash
# Compilar e executar apenas testes unitários
cd build
PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig cmake ..
make unit_tests
./tests/unit_tests

# Compilar e executar testes de integração
make integration_tests
./tests/integration_tests

# Executar todos os testes
./tests/unit_tests && ./tests/integration_tests

# Usar as tarefas do VS Code (Recomendado)
# Ctrl+Shift+P -> Tasks: Run Task -> "Run Tests" (unitários)
# Ctrl+Shift+P -> Tasks: Run Task -> "Run Integration Tests"
# Ctrl+Shift+P -> Tasks: Run Task -> "Run All Tests"
```

### Testes Unitários (7 testes)

- **HelloServerTest**: Testa inicialização básica do servidor
- **HelloServerConstructorTest**: Testa diferentes configurações de endereço
- **HelloServerFunctionalTest**: Testes funcionais básicos
- **HelloServerMultipleInstancesTest**: Testa múltiplas instâncias
- **HelloServerConfigTest**: Testa diferentes configurações de threads
- **HelloServerEdgeCasesTest**: Testa casos extremos

### Testes de Integração (4 testes)

- **TestRootEndpoint**: Testa GET / retorna "Hello World"
- **TestHelloEndpoint**: Testa GET /hello retorna "Hello World"
- **TestNonExistentEndpoint**: Testa endpoint inexistente retorna 404
- **TestConcurrentRequests**: Testa 5 requisições simultâneas

### Executar Testes com Saída Detalhada

```bash
cd build
./tests/unit_tests --gtest_verbose
./tests/integration_tests --gtest_verbose
```

## Desenvolvimento

### Adicionando Novos Endpoints

Para adicionar novos endpoints, edite os métodos na classe `HelloServer`:

1. Adicione a declaração no header `include/hello_server.h`
2. Implemente o método em `src/hello_server.cpp`
3. Registre a rota no método `setupRoutes()`

### Exemplo de Novo Endpoint

```cpp
// No header file
void statusHandler(const Pistache::Rest::Request& request, 
                  Pistache::Http::ResponseWriter response);

// Na implementação
void HelloServer::statusHandler(const Pistache::Rest::Request& request, 
                               Pistache::Http::ResponseWriter response) {
    response.send(Pistache::Http::Code::Ok, "Server is running!");
}

// No setupRoutes()
Routes::Get(router, "/status", Routes::bind(&HelloServer::statusHandler, this));
```

## Troubleshooting

### Erro de biblioteca não encontrada
Se encontrar erros relacionados a bibliotecas não encontradas:
```bash
sudo ldconfig
```

### Porta em uso
Se a porta 9080 estiver em uso, altere no `main.cpp`:
```cpp
Port port(8080); // ou outra porta disponível
```

### Firewall
Se não conseguir acessar de outras máquinas, configure o firewall:
```bash
sudo firewall-cmd --permanent --add-port=9080/tcp
sudo firewall-cmd --reload
```