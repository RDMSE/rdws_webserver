#!/usr/bin/env node

/**
 * API Gateway for C++ Microservices Architecture
 * 
 * This gateway provides a unified HTTP interface to multiple
 * C++ microservice executables, handling routing, error handling,
 * and response formatting.
 */

const express = require('express');
const { exec } = require('child_process');
const util = require('util');
const cors = require('cors');
const helmet = require('helmet');
const morgan = require('morgan');

const execAsync = util.promisify(exec);

// Configuration
const config = {
    port: process.env.PORT || 8080,
    buildPath: process.env.BUILD_PATH || './build',
    environment: process.env.NODE_ENV || 'development',
    timeout: parseInt(process.env.SERVICE_TIMEOUT) || 5000
};

// Initialize Express app
const app = express();

// Middleware stack
app.use(helmet()); // Security headers
app.use(cors()); // Enable CORS
app.use(morgan('combined')); // Request logging
app.use(express.json({ limit: '10mb' }));
app.use(express.urlencoded({ extended: true }));

// Request ID middleware for tracing
app.use((req, res, next) => {
    req.requestId = Date.now().toString(36) + Math.random().toString(36);
    res.set('X-Request-ID', req.requestId);
    next();
});

/**
 * Helper function to execute microservice and handle errors
 */
async function callMicroservice(serviceName, method, path, requestId) {
    const startTime = Date.now();
    
    try {
        const servicePath = `${config.buildPath}/services/${serviceName}/${serviceName}_service`;
        const command = `"${servicePath}" "${method}" "${path}"`;
        
        console.log(`[${requestId}] Calling: ${command}`);
        
        const { stdout, stderr } = await execAsync(command, {
            timeout: config.timeout,
            encoding: 'utf8'
        });
        
        if (stderr) {
            console.warn(`[${requestId}] ${serviceName} stderr:`, stderr);
        }
        
        const duration = Date.now() - startTime;
        console.log(`[${requestId}] ${serviceName} completed in ${duration}ms`);
        
        // Parse JSON response
        const result = JSON.parse(stdout.trim());
        
        // Add gateway metadata
        result.gateway = {
            requestId,
            serviceName,
            executionTime: duration,
            timestamp: new Date().toISOString()
        };
        
        return result;
        
    } catch (error) {
        const duration = Date.now() - startTime;
        console.error(`[${requestId}] Error calling ${serviceName} (${duration}ms):`, error.message);
        
        // Handle different types of errors
        if (error.killed && error.signal === 'SIGTERM') {
            throw new Error(`Service ${serviceName} timed out after ${config.timeout}ms`);
        }
        
        if (error.code === 'ENOENT') {
            throw new Error(`Service ${serviceName} executable not found`);
        }
        
        if (error.message.includes('JSON')) {
            throw new Error(`Service ${serviceName} returned invalid JSON`);
        }
        
        throw new Error(`Service ${serviceName} error: ${error.message}`);
    }
}

/**
 * Error handling middleware
 */
function handleServiceError(error, req, res) {
    console.error(`[${req.requestId}] Service error:`, error.message);
    
    // Determine appropriate HTTP status code
    let statusCode = 500;
    if (error.message.includes('not found')) {
        statusCode = 404;
    } else if (error.message.includes('timed out')) {
        statusCode = 408;
    } else if (error.message.includes('invalid JSON')) {
        statusCode = 502;
    }
    
    res.status(statusCode).json({
        error: true,
        message: error.message,
        requestId: req.requestId,
        timestamp: new Date().toISOString()
    });
}

// ============================================
// ROUTES - Users Service
// ============================================

app.get('/users', async (req, res) => {
    try {
        const result = await callMicroservice('users', 'GET', '/users', req.requestId);
        res.json(result);
    } catch (error) {
        handleServiceError(error, req, res);
    }
});

app.get('/users/:id', async (req, res) => {
    try {
        const userId = req.params.id;
        
        // Basic validation
        if (!/^\d+$/.test(userId)) {
            return res.status(400).json({
                error: true,
                message: 'User ID must be a number',
                requestId: req.requestId
            });
        }
        
        const result = await callMicroservice('users', 'GET', `/users/${userId}`, req.requestId);
        res.json(result);
    } catch (error) {
        handleServiceError(error, req, res);
    }
});

// ============================================
// ROUTES - Orders Service
// ============================================

app.get('/orders', async (req, res) => {
    try {
        const result = await callMicroservice('orders', 'GET', '/orders', req.requestId);
        res.json(result);
    } catch (error) {
        handleServiceError(error, req, res);
    }
});

app.get('/orders/:id', async (req, res) => {
    try {
        const orderId = req.params.id;
        
        // Basic validation
        if (!/^\d+$/.test(orderId)) {
            return res.status(400).json({
                error: true,
                message: 'Order ID must be a number',
                requestId: req.requestId
            });
        }
        
        const result = await callMicroservice('orders', 'GET', `/orders/${orderId}`, req.requestId);
        res.json(result);
    } catch (error) {
        handleServiceError(error, req, res);
    }
});

// Get orders by user ID
app.get('/users/:userId/orders', async (req, res) => {
    try {
        const userId = req.params.userId;
        
        if (!/^\d+$/.test(userId)) {
            return res.status(400).json({
                error: true,
                message: 'User ID must be a number',
                requestId: req.requestId
            });
        }
        
        const result = await callMicroservice('orders', 'GET', `/users/${userId}/orders`, req.requestId);
        res.json(result);
    } catch (error) {
        handleServiceError(error, req, res);
    }
});

// ============================================
// UTILITY ROUTES
// ============================================

// Health check endpoint
app.get('/health', async (req, res) => {
    const healthData = {
        status: 'ok',
        timestamp: new Date().toISOString(),
        requestId: req.requestId,
        config: {
            environment: config.environment,
            buildPath: config.buildPath,
            timeout: config.timeout
        },
        services: {}
    };
    
    // Test each service
    const services = ['users', 'orders'];
    
    for (const service of services) {
        try {
            const startTime = Date.now();
            await callMicroservice(service, 'GET', `/${service}`, req.requestId);
            healthData.services[service] = {
                status: 'healthy',
                responseTime: Date.now() - startTime
            };
        } catch (error) {
            healthData.services[service] = {
                status: 'unhealthy',
                error: error.message
            };
            healthData.status = 'degraded';
        }
    }
    
    const statusCode = healthData.status === 'ok' ? 200 : 503;
    res.status(statusCode).json(healthData);
});

// API documentation endpoint
app.get('/api-docs', (req, res) => {
    res.json({
        title: 'C++ Microservices API Gateway',
        version: '1.0.0',
        description: 'Gateway for C++ microservice executables',
        endpoints: {
            'GET /health': 'Health check and service status',
            'GET /users': 'List all users',
            'GET /users/:id': 'Get user by ID',
            'GET /orders': 'List all orders',
            'GET /orders/:id': 'Get order by ID',
            'GET /users/:userId/orders': 'Get orders for a specific user',
            'GET /api-docs': 'This documentation'
        },
        requestId: req.requestId
    });
});

// 404 handler
app.use((req, res) => {
    res.status(404).json({
        error: true,
        message: `Route ${req.method} ${req.path} not found`,
        requestId: req.requestId,
        availableRoutes: ['/health', '/users', '/orders', '/api-docs']
    });
});

// Global error handler
app.use((error, req, res, next) => {
    console.error(`[${req.requestId || 'unknown'}] Unhandled error:`, error);
    res.status(500).json({
        error: true,
        message: 'Internal server error',
        requestId: req.requestId
    });
});

// ============================================
// SERVER STARTUP
// ============================================

function startServer() {
    const server = app.listen(config.port, () => {
        console.log('API Gateway started successfully!');
        console.log(`Server running on port ${config.port}`);
        console.log(`Environment: ${config.environment}`);
        console.log(`Build path: ${config.buildPath}`);
        console.log(`Service timeout: ${config.timeout}ms`);
        console.log('');
        console.log('Available endpoints:');
        console.log(`   GET  http://localhost:${config.port}/health`);
        console.log(`   GET  http://localhost:${config.port}/users`);
        console.log(`   GET  http://localhost:${config.port}/users/:id`);
        console.log(`   GET  http://localhost:${config.port}/orders`);
        console.log(`   GET  http://localhost:${config.port}/orders/:id`);
        console.log(`   GET  http://localhost:${config.port}/users/:userId/orders`);
        console.log(`   GET  http://localhost:${config.port}/api-docs`);
        console.log('');
    });
    
    // Graceful shutdown
    process.on('SIGTERM', () => {
        console.log('SIGTERM received, shutting down gracefully...');
        server.close(() => {
            console.log('Server closed');
            process.exit(0);
        });
    });
    
    process.on('SIGINT', () => {
        console.log('SIGINT received, shutting down gracefully...');
        server.close(() => {
            console.log('Server closed');
            process.exit(0);
        });
    });
    
    return server;
}

// Start server if this file is run directly
if (require.main === module) {
    startServer();
}

// Export for testing
module.exports = { app, startServer, callMicroservice };