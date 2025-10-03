#!/usr/bin/env node

// Exemplo de API Gateway único que chama todos os microserviços
const express = require('express');
const { exec } = require('child_process');
const util = require('util');
const execAsync = util.promisify(exec);

const app = express();
const PORT = 8080;

// Middleware
app.use(express.json());

// Função helper para executar microserviços
async function callMicroservice(serviceName, method, path) {
    try {
        const command = `./services/${serviceName}/${serviceName}_service "${method}" "${path}"`;
        console.log(`Calling: ${command}`);
        
        const { stdout, stderr } = await execAsync(command);
        
        if (stderr) {
            console.error(`${serviceName} stderr:`, stderr);
        }
        
        return JSON.parse(stdout.trim());
    } catch (error) {
        console.error(`Error calling ${serviceName}:`, error);
        throw error;
    }
}

// Rotas para Users
app.get('/users', async (req, res) => {
    try {
        const result = await callMicroservice('users', 'GET', '/users');
        res.json(result);
    } catch (error) {
        res.status(500).json({ error: 'Users service error', details: error.message });
    }
});

app.get('/users/:id', async (req, res) => {
    try {
        const result = await callMicroservice('users', 'GET', `/users/${req.params.id}`);
        res.json(result);
    } catch (error) {
        res.status(500).json({ error: 'Users service error', details: error.message });
    }
});

// Rotas para Orders
app.get('/orders', async (req, res) => {
    try {
        const result = await callMicroservice('orders', 'GET', '/orders');
        res.json(result);
    } catch (error) {
        res.status(500).json({ error: 'Orders service error', details: error.message });
    }
});

app.get('/orders/:id', async (req, res) => {
    try {
        const result = await callMicroservice('orders', 'GET', `/orders/${req.params.id}`);
        res.json(result);
    } catch (error) {
        res.status(500).json({ error: 'Orders service error', details: error.message });
    }
});

// Health check
app.get('/health', (req, res) => {
    res.json({ 
        status: 'ok', 
        timestamp: new Date().toISOString(),
        services: ['users', 'orders']
    });
});

// Iniciar servidor
app.listen(PORT, () => {
    console.log(`🚀 API Gateway running on port ${PORT}`);
    console.log(`📡 Available endpoints:`);
    console.log(`   GET  http://localhost:${PORT}/users`);
    console.log(`   GET  http://localhost:${PORT}/users/:id`);
    console.log(`   GET  http://localhost:${PORT}/orders`);
    console.log(`   GET  http://localhost:${PORT}/orders/:id`);
    console.log(`   GET  http://localhost:${PORT}/health`);
});

module.exports = app;