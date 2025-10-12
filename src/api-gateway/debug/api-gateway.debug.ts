#!/usr/bin/env node

/**
 * Enhanced Debug Version of API Gateway
 * 
 * This version includes additional debugging features:
 * - Detailed request/response logging
 * - Microservice execution tracing
 * - Environment variable inspection
 * - Performance metrics
 */

// Enhanced debugging
process.env.DEBUG = process.env.DEBUG || '*';
process.env.NODE_ENV = process.env.NODE_ENV || 'development';

// Add timestamps to console logs
const originalLog = console.log;
const originalError = console.error;
const originalWarn = console.warn;

console.log = (...args) => {
  originalLog(`[${new Date().toISOString()}] [LOG]`, ...args);
};

console.error = (...args) => {
  originalError(`[${new Date().toISOString()}] [ERROR]`, ...args);
};

console.warn = (...args) => {
  originalWarn(`[${new Date().toISOString()}] [WARN]`, ...args);
};

// Import the original API Gateway
import { app, startServer, callMicroservice } from '../api-gateway';
import express from 'express';

// Add debug middleware
app.use((req, res, next) => {
  const startTime = Date.now();
  
  console.log(`[DEBUG] Incoming Request:`);
  console.log(`   Method: ${req.method}`);
  console.log(`   Path: ${req.path}`);
  console.log(`   Headers:`, JSON.stringify(req.headers, null, 2));
  console.log(`   Body:`, req.body);
  console.log(`   Query:`, req.query);
  console.log(`   Params:`, req.params);
  
  // Intercept response
  const originalSend = res.send;
  res.send = function(body) {
    const duration = Date.now() - startTime;
    console.log(`[DEBUG] Outgoing Response (${duration}ms):`);
    console.log(`   Status: ${res.statusCode}`);
    console.log(`   Headers:`, res.getHeaders());
    console.log(`   Body:`, typeof body === 'string' ? body.substring(0, 500) : body);
    
    return originalSend.call(this, body);
  };
  
  next();
});

// Enhanced microservice debugging
const originalCallMicroservice = callMicroservice;

// Add debug route for inspecting environment
app.get('/debug/env', (req, res) => {
  const debugInfo = {
    nodeVersion: process.version,
    platform: process.platform,
    arch: process.arch,
    cwd: process.cwd(),
    environment: {
      NODE_ENV: process.env.NODE_ENV,
      PORT: process.env.PORT,
      BUILD_PATH: process.env.BUILD_PATH,
      DB_HOST: process.env.DB_HOST,
      DB_PORT: process.env.DB_PORT,
      DB_NAME: process.env.DB_NAME,
      DB_USER: process.env.DB_USER,
      // Don't expose password
      DB_PASSWORD: process.env.DB_PASSWORD ? '***HIDDEN***' : 'not set',
    },
    microservicePaths: {
      users: `./build/src/services/users/users_service`,
      orders: `./build/src/services/orders/orders_service`,
    }
  };
  
  res.json(debugInfo);
});

// Add debug route for testing microservice execution
app.get('/debug/microservice/:service', async (req, res) => {
  const { service } = req.params;
  const method = req.query.method as string || 'GET';
  const path = req.query.path as string || `/${service}`;
  
  try {
    console.log(`[DEBUG] Testing microservice directly:`);
    console.log(`   Service: ${service}`);
    console.log(`   Method: ${method}`);
    console.log(`   Path: ${path}`);
    
    const result = await originalCallMicroservice(service, method, path, req.requestId);
    
    res.json({
      success: true,
      result,
      debug: {
        service,
        method,
        path,
        requestId: req.requestId
      }
    });
  } catch (error) {
    console.error(`[DEBUG] Microservice test failed:`, error);
    
    res.status(500).json({
      success: false,
      error: (error as Error).message,
      debug: {
        service,
        method,
        path,
        requestId: req.requestId
      }
    });
  }
});

// Debug route for inspecting build files
app.get('/debug/build', (req, res) => {
  const fs = require('fs');
  const path = require('path');
  
  try {
    const buildPath = './build';
    const servicesPath = path.join(buildPath, 'src/services');
    
    const buildInfo: {
      buildExists: boolean;
      servicesExists: boolean;
      services: Record<string, any>;
    } = {
      buildExists: fs.existsSync(buildPath),
      servicesExists: fs.existsSync(servicesPath),
      services: {}
    };
    
    if (buildInfo.servicesExists) {
      const services = fs.readdirSync(servicesPath);
      
      for (const service of services) {
        const servicePath = path.join(servicesPath, service);
        const executablePath = path.join(servicePath, `${service}_service`);
        
        buildInfo.services[service] = {
          directoryExists: fs.existsSync(servicePath),
          executableExists: fs.existsSync(executablePath),
          executablePath: executablePath,
          isExecutable: fs.existsSync(executablePath) ? 
            fs.accessSync(executablePath, fs.constants.F_OK | fs.constants.X_OK) === undefined : false
        };
      }
    }
    
    res.json(buildInfo);
  } catch (error) {
    res.status(500).json({
      error: (error as Error).message
    });
  }
});

console.log('Enhanced Debug API Gateway Starting...');
console.log('Additional debug endpoints:');
console.log('   GET /debug/env - Environment inspection');
console.log('   GET /debug/microservice/:service - Test microservice directly');
console.log('   GET /debug/build - Build files inspection');

export { app as debugApp, startServer };

// Start if run directly
if (require.main === module) {
  startServer();
}