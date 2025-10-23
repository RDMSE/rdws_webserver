/**
 * API Gateway Tests
 *
 * Tests the API Gateway functionality including routing,
 * error handling, and mocked microservice responses.
 * Uses mocks to isolate gateway logic from microservice dependencies.
 */

import request from 'supertest';
import { app } from '../api-gateway';
import * as childProcess from 'child_process';

// Mock the child_process module to intercept microservice calls
jest.mock('child_process');
const mockExec = childProcess.exec as jest.MockedFunction<typeof childProcess.exec>;

interface ApiResponse {
  body: any;
  status: number;
  headers: any;
}

// Mock data for different microservices
const mockUsersResponse = {
  users: [
    { id: 1, name: 'John Doe', email: 'john@example.com' },
    { id: 2, name: 'Jane Smith', email: 'jane@example.com' },
  ],
  total: 2,
};

const mockUserResponse = {
  user: { id: 1, name: 'John Doe', email: 'john@example.com' },
};

const mockOrdersResponse = {
  orders: [
    { id: 1, userId: 1, product: 'Laptop', amount: 999.99 },
    { id: 2, userId: 2, product: 'Mouse', amount: 29.99 },
  ],
  total: 2,
};

const mockOrderResponse = {
  order: { id: 1, userId: 1, product: 'Laptop', amount: 999.99 },
};

const mockUserOrdersResponse = {
  userId: 1,
  orders: [{ id: 1, product: 'Laptop', amount: 999.99 }],
  total: 1,
};

describe('API Gateway', () => {
  beforeEach(() => {
    // Reset mocks before each test
    jest.clearAllMocks();

    // Default successful mock implementation for JSON Event/Context format
    mockExec.mockImplementation((command: string, options: any, callback?: any) => {
      // Handle promisified exec
      if (typeof options === 'function') {
        callback = options;
        options = {};
      }

      setTimeout(() => {
        let mockData: any = {};
        let stdout = '';

        try {
          // Extract JSON arguments from command
          const jsonMatches = command.match(/'(\{.*?\})'/g);
          
          if (!jsonMatches || jsonMatches.length < 2) {
            return callback(new Error('Invalid command format - missing JSON arguments'));
          }

          // Parse Event and Context JSON
          const eventJson = jsonMatches[0].slice(1, -1); // Remove quotes
          const contextJson = jsonMatches[1].slice(1, -1); // Remove quotes
          
          const lambdaEvent = JSON.parse(eventJson);
          const lambdaContext = JSON.parse(contextJson);

          // Determine service and route from context and event
          const serviceName = lambdaContext.functionName;
          const httpMethod = lambdaEvent.httpMethod;
          const path = lambdaEvent.path;

          // Route based on service and path
          if (serviceName === 'users') {
            if (httpMethod === 'GET' && (path === '/' || path === '/users')) {
              mockData = mockUsersResponse;
            } else if (httpMethod === 'GET' && (path === '/1' || path === '/users/1')) {
              mockData = mockUserResponse;
            } else {
              return callback(new Error('Service users not found'));
            }
          } else if (serviceName === 'orders') {
            if (httpMethod === 'GET' && (path === '/' || path === '/orders')) {
              mockData = mockOrdersResponse;
            } else if (httpMethod === 'GET' && (path === '/1' || path === '/orders/1')) {
              mockData = mockOrderResponse;
            } else if (httpMethod === 'GET' && (path === '/1/orders' || path === '/users/1/orders')) {
              mockData = mockUserOrdersResponse;
            } else {
              return callback(new Error('Service orders not found'));
            }
          } else {
            return callback(new Error('Service not found'));
          }

          stdout = JSON.stringify(mockData);
          callback(null, { stdout, stderr: '' });
        } catch (parseError) {
          console.error('Mock parse error:', parseError, 'Command:', command);
          return callback(new Error('Invalid JSON in command'));
        }
      }, 10);

      return {} as any; // Mock child process
    });
  });
  describe('Health Check', () => {
    test('GET /health returns status information when services are healthy', async () => {
      const response: ApiResponse = await request(app)
        .get('/health')
        .expect(200)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('status', 'ok');
      expect(response.body).toHaveProperty('timestamp');
      expect(response.body).toHaveProperty('requestId');
      expect(response.body).toHaveProperty('services');

      // Should have users and orders services with healthy status
      expect(response.body.services).toHaveProperty('users');
      expect(response.body.services.users).toHaveProperty('status', 'healthy');
      expect(response.body.services.users).toHaveProperty('responseTime');

      expect(response.body.services).toHaveProperty('orders');
      expect(response.body.services.orders).toHaveProperty('status', 'healthy');
      expect(response.body.services.orders).toHaveProperty('responseTime');

      // Verify that exec was called for health check with JSON format
      expect(mockExec).toHaveBeenCalledWith(
        expect.stringContaining('users_service'),
        expect.any(Object),
        expect.any(Function)
      );
      expect(mockExec).toHaveBeenCalledWith(
        expect.stringContaining('orders_service'),
        expect.any(Object),
        expect.any(Function)
      );
    });

    test('GET /health returns degraded status when services are unhealthy', async () => {
      // Mock service failure
      mockExec.mockImplementation((command: string, options: any, callback?: any) => {
        if (typeof options === 'function') {
          callback = options;
        }
        setTimeout(() => {
          callback(new Error('Service unavailable'));
        }, 10);
        return {} as any;
      });

      const response: ApiResponse = await request(app)
        .get('/health')
        .expect(503)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('status', 'degraded');
      expect(response.body.services.users).toHaveProperty('status', 'unhealthy');
      expect(response.body.services.users).toHaveProperty('error');
      expect(response.body.services.users.error).toContain('Service unavailable');
    });
  });

  describe('API Documentation', () => {
    test('GET /api-docs returns documentation', async () => {
      const response: ApiResponse = await request(app)
        .get('/api-docs')
        .expect(200)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('title');
      expect(response.body).toHaveProperty('endpoints');
      expect(response.body).toHaveProperty('requestId');
    });
  });

  describe('Users Routes', () => {
    test('GET /users returns users list', async () => {
      const response: ApiResponse = await request(app)
        .get('/users')
        .expect(200)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('users');
      expect(response.body.users).toHaveLength(2);
      expect(response.body.users[0]).toHaveProperty('id', 1);
      expect(response.body.users[0]).toHaveProperty('name', 'John Doe');
      expect(response.body).toHaveProperty('total', 2);
      expect(response.body).toHaveProperty('gateway');
      expect(response.body.gateway).toHaveProperty('requestId');
      expect(response.body.gateway).toHaveProperty('serviceName', 'users');

      // Verify exec was called correctly with JSON format
      expect(mockExec).toHaveBeenCalledWith(
        expect.stringContaining('users_service'),
        expect.any(Object),
        expect.any(Function)
      );

      // Verify JSON Event/Context format
      const calls = mockExec.mock.calls;
      const lastCall = calls[calls.length - 1];
      expect(lastCall[0]).toMatch(/'\{.*\}' '\{.*\}'$/);
    });

    test('GET /users/:id with valid ID', async () => {
      const response: ApiResponse = await request(app)
        .get('/users/1')
        .expect(200)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('user');
      expect(response.body.user).toHaveProperty('id', 1);
      expect(response.body.user).toHaveProperty('name', 'John Doe');
      expect(response.body).toHaveProperty('gateway');
      expect(response.body.gateway).toHaveProperty('serviceName', 'users');

      // Verify exec was called with correct JSON Event
      expect(mockExec).toHaveBeenCalledWith(
        expect.stringContaining('users_service'),
        expect.any(Object),
        expect.any(Function)
      );

      // Verify Event contains correct path
      const calls = mockExec.mock.calls;
      const lastCall = calls[calls.length - 1];
      expect(lastCall[0]).toContain('"path":"/1"');
    });

    test('GET /users/:id with invalid ID returns 400', async () => {
      const response: ApiResponse = await request(app)
        .get('/users/invalid')
        .expect(400)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('error', true);
      expect(response.body).toHaveProperty('message');
      expect(response.body.message).toContain('must be a number');

      // Verify exec was not called for invalid input
      expect(mockExec).not.toHaveBeenCalled();
    });

    test('GET /users handles service error gracefully', async () => {
      // Mock service error
      mockExec.mockImplementation((command: string, options: any, callback?: any) => {
        if (typeof options === 'function') {
          callback = options;
        }
        setTimeout(() => {
          callback(new Error('Users service unavailable'));
        }, 10);
        return {} as any;
      });

      const response: ApiResponse = await request(app)
        .get('/users')
        .expect(500)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('error', true);
      expect(response.body).toHaveProperty(
        'message',
        'Service users error: Users service unavailable'
      );
      expect(response.body).toHaveProperty('requestId');
    });
  });

  describe('Orders Routes', () => {
    test('GET /orders returns orders list', async () => {
      const response: ApiResponse = await request(app)
        .get('/orders')
        .expect(200)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('orders');
      expect(response.body.orders).toHaveLength(2);
      expect(response.body.orders[0]).toHaveProperty('id', 1);
      expect(response.body.orders[0]).toHaveProperty('product', 'Laptop');
      expect(response.body).toHaveProperty('total', 2);
      expect(response.body).toHaveProperty('gateway');
      expect(response.body.gateway).toHaveProperty('serviceName', 'orders');

      // Verify exec was called correctly with JSON format
      expect(mockExec).toHaveBeenCalledWith(
        expect.stringContaining('orders_service'),
        expect.any(Object),
        expect.any(Function)
      );

      // Verify JSON Event/Context format
      const calls = mockExec.mock.calls;
      const lastCall = calls[calls.length - 1];
      expect(lastCall[0]).toMatch(/'\{.*\}' '\{.*\}'$/);
    });

    test('GET /orders/:id with valid ID', async () => {
      const response: ApiResponse = await request(app)
        .get('/orders/1')
        .expect(200)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('order');
      expect(response.body.order).toHaveProperty('id', 1);
      expect(response.body.order).toHaveProperty('product', 'Laptop');
      expect(response.body).toHaveProperty('gateway');
      expect(response.body.gateway).toHaveProperty('serviceName', 'orders');

      // Verify exec was called with correct JSON Event
      expect(mockExec).toHaveBeenCalledWith(
        expect.stringContaining('orders_service'),
        expect.any(Object),
        expect.any(Function)
      );

      // Verify Event contains correct path
      const calls = mockExec.mock.calls;
      const lastCall = calls[calls.length - 1];
      expect(lastCall[0]).toContain('"path":"/1"');
    });

    test('GET /orders/:id with invalid ID returns 400', async () => {
      const response: ApiResponse = await request(app)
        .get('/orders/invalid')
        .expect(400)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('error', true);
      expect(response.body.message).toContain('must be a number');

      // Verify exec was not called for invalid input
      expect(mockExec).not.toHaveBeenCalled();
    });

    test('GET /users/:userId/orders with valid user ID', async () => {
      const response: ApiResponse = await request(app)
        .get('/users/1/orders')
        .expect(200)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('userId', 1);
      expect(response.body).toHaveProperty('orders');
      expect(response.body.orders).toHaveLength(1);
      expect(response.body.orders[0]).toHaveProperty('product', 'Laptop');
      expect(response.body).toHaveProperty('gateway');

      // Verify exec was called with correct JSON Event
      expect(mockExec).toHaveBeenCalledWith(
        expect.stringContaining('orders_service'),
        expect.any(Object),
        expect.any(Function)
      );

      // Verify Event contains correct path
      const calls = mockExec.mock.calls;
      const lastCall = calls[calls.length - 1];
      expect(lastCall[0]).toContain('"path":"/1/orders"');
    });

    test('GET /users/:userId/orders with invalid user ID returns 400', async () => {
      const response: ApiResponse = await request(app)
        .get('/users/invalid/orders')
        .expect(400)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('error', true);
      expect(response.body.message).toContain('must be a number');

      // Verify exec was not called for invalid input
      expect(mockExec).not.toHaveBeenCalled();
    });

    test('GET /orders handles service timeout error', async () => {
      // Mock service timeout
      mockExec.mockImplementation((command: string, options: any, callback?: any) => {
        if (typeof options === 'function') {
          callback = options;
        }
        setTimeout(() => {
          const error: any = new Error('Command failed');
          error.killed = true;
          error.signal = 'SIGTERM';
          callback(error);
        }, 10);
        return {} as any;
      });

      const response: ApiResponse = await request(app)
        .get('/orders')
        .expect(408)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('error', true);
      expect(response.body.message).toContain('timed out');
      expect(response.body).toHaveProperty('requestId');
    });
  });

  describe('Error Handling', () => {
    test('404 for non-existent routes', async () => {
      const response: ApiResponse = await request(app)
        .get('/non-existent-route')
        .expect(404)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('error', true);
      expect(response.body).toHaveProperty('message');
      expect(response.body.message).toContain('not found');
      expect(response.body).toHaveProperty('availableRoutes');
      expect(response.body.availableRoutes).toContain('/health');
      expect(response.body.availableRoutes).toContain('/users');
      expect(response.body.availableRoutes).toContain('/orders');
    });

    test('Response includes request ID', async () => {
      const response: ApiResponse = await request(app).get('/api-docs').expect(200);

      expect(response.body).toHaveProperty('requestId');
      expect(response.headers).toHaveProperty('x-request-id');
      expect(response.body.requestId).toBe(response.headers['x-request-id']);
    });

    test('Service not found error returns 404', async () => {
      // Mock executable not found error
      mockExec.mockImplementation((command: string, options: any, callback?: any) => {
        if (typeof options === 'function') {
          callback = options;
        }
        setTimeout(() => {
          const error: any = new Error('Command failed');
          error.code = 'ENOENT';
          callback(error);
        }, 10);
        return {} as any;
      });

      const response: ApiResponse = await request(app)
        .get('/users')
        .expect(404)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('error', true);
      expect(response.body.message).toContain('executable not found');
    });

    test('Invalid JSON error returns 502', async () => {
      // Mock invalid JSON response
      mockExec.mockImplementation((command: string, options: any, callback?: any) => {
        if (typeof options === 'function') {
          callback = options;
        }
        setTimeout(() => {
          callback(null, { stdout: 'invalid json', stderr: '' });
        }, 10);
        return {} as any;
      });

      const response: ApiResponse = await request(app)
        .get('/users')
        .expect(502)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('error', true);
      expect(response.body.message).toContain('invalid JSON');
    });

    test('Unknown service error returns 500', async () => {
      // Mock generic service error
      mockExec.mockImplementation((command: string, options: any, callback?: any) => {
        if (typeof options === 'function') {
          callback = options;
        }
        setTimeout(() => {
          callback(new Error('Unknown service error'));
        }, 10);
        return {} as any;
      });

      const response: ApiResponse = await request(app)
        .get('/users')
        .expect(500)
        .expect('Content-Type', /json/);

      expect(response.body).toHaveProperty('error', true);
      expect(response.body.message).toContain('Unknown service error');
    });
  });

  describe('CORS and Security', () => {
    test('CORS headers are present', async () => {
      const response: ApiResponse = await request(app).get('/health').expect(200);

      expect(response.headers).toHaveProperty('access-control-allow-origin');
    });

    test('Security headers are present', async () => {
      const response: ApiResponse = await request(app).get('/health').expect(200);

      // Helmet should add security headers
      expect(response.headers).toHaveProperty('x-content-type-options', 'nosniff');
    });

    test('OPTIONS request for CORS preflight', async () => {
      const response: ApiResponse = await request(app).options('/users').expect(204);

      expect(response.headers).toHaveProperty('access-control-allow-origin');
    });
  });

  describe('Microservice Mock Behavior', () => {
    test('Mock verifies exec call parameters', async () => {
      // This should return 404 because ID 123 is not in our mock data (Service not found)
      const response = await request(app).get('/users/123');

      expect(response.status).toBe(404); // Service not found in mock
      expect(response.body).toHaveProperty('error', true);
      expect(response.body.message).toContain('Service users not found');

      expect(mockExec).toHaveBeenCalledWith(
        expect.stringContaining('users_service'),
        expect.any(Object),
        expect.any(Function)
      );

      // Verify the command contains the correct JSON Event with path
      const calls = mockExec.mock.calls;
      const lastCall = calls[calls.length - 1];
      const command = lastCall[0];
      
      // Extract and parse the Event JSON from the command (more flexible regex)
      const jsonMatches = command.match(/'(\{.*?\})'/g);
      expect(jsonMatches).toBeTruthy();
      
      if (jsonMatches) {
        expect(jsonMatches.length).toBeGreaterThanOrEqual(2);
        
        const eventJson = jsonMatches[0].slice(1, -1); // Remove quotes
        const lambdaEvent = JSON.parse(eventJson);
        expect(lambdaEvent.path).toBe('/123');
        expect(lambdaEvent.httpMethod).toBe('GET');
        expect(lambdaEvent.pathParameters).toHaveProperty('id', '123');
      }
    });

    test('Mock can simulate different response times', async () => {
      // Mock slow response by adding delay
      mockExec.mockImplementation((command: string, options: any, callback?: any) => {
        if (typeof options === 'function') {
          callback = options;
        }

        setTimeout(() => {
          const stdout = JSON.stringify({
            ...mockUsersResponse,
            executionTime: 2000, // Custom execution time
          });
          callback(null, { stdout, stderr: '' });
        }, 100); // Longer delay to simulate slow response

        return {} as any;
      });

      const startTime = Date.now();
      const response: ApiResponse = await request(app).get('/users').expect(200);

      const duration = Date.now() - startTime;
      expect(duration).toBeGreaterThan(90); // Should take at least 90ms due to our mock delay
      expect(response.body).toHaveProperty('gateway');
    });

    test('Mock handles concurrent requests properly', async () => {
      const requests = [
        request(app).get('/users'),
        request(app).get('/orders'),
        request(app).get('/users/1'),
      ];

      const responses = await Promise.all(requests);

      responses.forEach(response => {
        expect(response.status).toBe(200);
        expect(response.body).toHaveProperty('gateway');
      });

      // Verify exec was called for each request
      expect(mockExec).toHaveBeenCalledTimes(3);
    });
  });
});
