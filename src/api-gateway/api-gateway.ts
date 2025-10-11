#!/usr/bin/env node

/**
 * API Gateway for C++ Microservices Architecture
 *
 * This gateway provides a unified HTTP interface to multiple
 * C++ microservice executables, handling routing, error handling,
 * and response formatting.
 */

import express, { Request, Response, NextFunction, Application } from 'express';
import { exec } from 'child_process';
import { promisify } from 'util';
import cors from 'cors';
import helmet from 'helmet';
import morgan from 'morgan';

// Import modular routes and types
import {
  UsersRouter,
  OrdersRouter,
  UserOrdersRouter,
  Config,
  ServiceResponse,
  ErrorResponse,
  HealthResponse,
  ApiDocsResponse,
  MicroserviceRouter,
} from '../routes';

const execAsync = promisify(exec);

// Configuration
const config: Config = {
  port: parseInt(process.env.PORT || '8080'),
  buildPath: process.env.BUILD_PATH || './build',
  environment: process.env.NODE_ENV || 'development',
  timeout: parseInt(process.env.SERVICE_TIMEOUT || '5000'),
};

// Initialize Express app
const app: Application = express();

// Middleware stack
app.use(helmet()); // Security headers
app.use(cors()); // Enable CORS
app.use(morgan('combined')); // Request logging
app.use(express.json({ limit: '10mb' }));
app.use(express.urlencoded({ extended: true }));

// Request ID middleware for tracing
app.use((req: Request, res: Response, next: NextFunction) => {
  req.requestId = Date.now().toString(36) + Math.random().toString(36);
  res.set('X-Request-ID', req.requestId);
  next();
});

/**
 * Helper function to execute microservice and handle errors
 */
async function callMicroservice(
  serviceName: string,
  method: string,
  path: string,
  requestId: string
): Promise<ServiceResponse> {
  const startTime = Date.now();

  try {
    const servicePath = `${config.buildPath}/src/services/${serviceName}/${serviceName}_service`;
    const command = `"${servicePath}" "${method}" "${path}"`;

    console.log(`[${requestId}] Calling: ${command}`);

    const { stdout, stderr } = await execAsync(command, {
      timeout: config.timeout,
      encoding: 'utf8',
    });

    if (stderr) {
      console.warn(`[${requestId}] ${serviceName} stderr:`, stderr);
    }

    const duration = Date.now() - startTime;
    console.log(`[${requestId}] ${serviceName} completed in ${duration}ms`);

    // Parse JSON response
    const result: ServiceResponse = JSON.parse(stdout.trim());

    // Add gateway metadata
    result.gateway = {
      requestId,
      serviceName,
      executionTime: duration,
      timestamp: new Date().toISOString(),
    };

    return result;
  } catch (error: any) {
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
function handleServiceError(error: Error, req: Request, res: Response): void {
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

  const errorResponse: ErrorResponse = {
    error: true,
    message: error.message,
    requestId: req.requestId,
    timestamp: new Date().toISOString(),
  };

  res.status(statusCode).json(errorResponse);
}

// ============================================
// MICROSERVICES ROUTER CONFIGURATION
// ============================================

// Initialize all microservice routers
const microserviceRouters: MicroserviceRouter[] = [
  new UsersRouter(),
  new OrdersRouter(),
  new UserOrdersRouter(),
];

// Setup all microservice routes dynamically
microserviceRouters.forEach(routerInstance => {
  const router = routerInstance.setupRoutes(callMicroservice, handleServiceError);
  app.use(routerInstance.routeConfig.basePath, router);

  console.log(`Registered routes for ${routerInstance.routeConfig.serviceName}:`);
  routerInstance.getRouteInfo().endpoints.forEach(endpoint => {
    console.log(`   ${endpoint}`);
  });
});

// ============================================
// UTILITY ROUTES
// ============================================

// Health check endpoint
app.get('/health', async (req: Request, res: Response) => {
  const healthData: HealthResponse = {
    status: 'ok',
    timestamp: new Date().toISOString(),
    requestId: req.requestId,
    config: {
      environment: config.environment,
      buildPath: config.buildPath,
      timeout: config.timeout,
    },
    services: {},
  };

  // Get unique service names from all routers
  const uniqueServices = Array.from(
    new Set(microserviceRouters.map(router => router.routeConfig.serviceName))
  );

  // Test each service
  for (const service of uniqueServices) {
    try {
      const startTime = Date.now();
      await callMicroservice(service, 'GET', `/${service}`, req.requestId);
      healthData.services[service] = {
        status: 'healthy',
        responseTime: Date.now() - startTime,
      };
    } catch (error) {
      healthData.services[service] = {
        status: 'unhealthy',
        error: (error as Error).message,
      };
      healthData.status = 'degraded';
    }
  }

  const statusCode = healthData.status === 'ok' ? 200 : 503;
  res.status(statusCode).json(healthData);
});

// API documentation endpoint
app.get('/api-docs', (req: Request, res: Response) => {
  // Build endpoints dynamically from all routers
  const endpoints: Record<string, string> = {
    'GET /health': 'Health check and service status',
    'GET /api-docs': 'This documentation',
  };

  // Add endpoints from all routers
  microserviceRouters.forEach(router => {
    const routeInfo = router.getRouteInfo();
    routeInfo.endpoints.forEach((endpoint: string) => {
      // Generate description based on endpoint
      const [method, path] = endpoint.split(' ');
      let description = `${router.routeConfig.serviceName} service endpoint`;

      if (path.includes('/:')) {
        description = `Get specific ${router.routeConfig.serviceName} by ID`;
      } else if (path.endsWith(router.routeConfig.basePath)) {
        description = `List all ${router.routeConfig.serviceName}`;
      }

      endpoints[endpoint] = description;
    });
  });

  const docsResponse: ApiDocsResponse = {
    title: 'C++ Microservices API Gateway',
    version: '1.0.0',
    description: 'Gateway for C++ microservice executables',
    endpoints,
    requestId: req.requestId,
  };

  res.json(docsResponse);
});

// 404 handler
app.use((req: Request, res: Response) => {
  // Generate available routes dynamically
  const availableRoutes = ['/health', '/api-docs'];
  microserviceRouters.forEach(router => {
    availableRoutes.push(router.routeConfig.basePath);
  });

  const errorResponse: ErrorResponse = {
    error: true,
    message: `Route ${req.method} ${req.path} not found`,
    requestId: req.requestId,
    availableRoutes,
  };

  res.status(404).json(errorResponse);
});

// Global error handler
app.use((error: Error, req: Request, res: Response, next: NextFunction) => {
  console.error(`[${req.requestId || 'unknown'}] Unhandled error:`, error);

  const errorResponse: ErrorResponse = {
    error: true,
    message: 'Internal server error',
    requestId: req.requestId,
  };

  res.status(500).json(errorResponse);
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
    console.log(`   GET  http://localhost:${config.port}/api-docs`);

    // Show all microservice endpoints
    microserviceRouters.forEach(router => {
      const routeInfo = router.getRouteInfo();
      routeInfo.endpoints.forEach((endpoint: string) => {
        const [method, path] = endpoint.split(' ');
        console.log(`   ${method}  http://localhost:${config.port}${path}`);
      });
    });

    console.log('');
    console.log(`Registered ${microserviceRouters.length} microservice router(s)`);
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
export { app, startServer, callMicroservice };
