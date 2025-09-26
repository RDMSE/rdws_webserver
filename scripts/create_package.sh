#!/bin/bash

# Script simples para criar um pacote compactado do projeto
# Útil para backup ou transferência manual

echo "📦 Criando pacote do projeto C++ REST Server..."

cd /home/rdias/sources/lab/server

# Criar arquivo tar compactado excluindo arquivos de build
tar czf /tmp/cpp-rest-server-$(date +%Y%m%d-%H%M).tar.gz \
    --exclude='build' \
    --exclude='.git' \
    --exclude='*.log' \
    --exclude='server.pid' \
    --exclude='test_results.xml' \
    .

PACKAGE_FILE=$(ls -t /tmp/cpp-rest-server-*.tar.gz | head -1)

echo "✅ Pacote criado: $PACKAGE_FILE"
echo "💾 Tamanho: $(du -h "$PACKAGE_FILE" | cut -f1)"

echo ""
echo "Para usar:"
echo "  scp $PACKAGE_FILE sua-maquina:~/"