/**
 * API Gateway Tests
 * 
 * Tests the API Gateway functionality including routing,
 * error handling, and integration with microservices.
 */

const request = require('supertest');
const { app } = require('../api-gateway');

describe('API Gateway', () => {
    describe('Health Check', () => {
        test('GET /health returns status information', async () => {
            const response = await request(app)
                .get('/health')
                .expect('Content-Type', /json/);
            
            expect(response.body).toHaveProperty('status');
            expect(response.body).toHaveProperty('timestamp');
            expect(response.body).toHaveProperty('requestId');
            expect(response.body).toHaveProperty('services');
            
            // Should have users and orders services
            expect(response.body.services).toHaveProperty('users');
            expect(response.body.services).toHaveProperty('orders');
        });
    });

    describe('API Documentation', () => {
        test('GET /api-docs returns documentation', async () => {
            const response = await request(app)
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
            const response = await request(app)
                .get('/users')
                .expect('Content-Type', /json/);
            
            // Should either succeed or return a service error
            if (response.status === 200) {
                expect(response.body).toHaveProperty('users');
                expect(response.body).toHaveProperty('total');
                expect(response.body).toHaveProperty('gateway');
                expect(response.body.gateway).toHaveProperty('requestId');
                expect(response.body.gateway).toHaveProperty('serviceName', 'users');
            } else {
                expect(response.body).toHaveProperty('error', true);
                expect(response.body).toHaveProperty('requestId');
            }
        });

        test('GET /users/:id with valid ID', async () => {
            const response = await request(app)
                .get('/users/1')
                .expect('Content-Type', /json/);
            
            // Should either succeed or return a service error
            if (response.status === 200) {
                expect(response.body).toHaveProperty('gateway');
            } else {
                expect(response.body).toHaveProperty('error', true);
            }
        });

        test('GET /users/:id with invalid ID returns 400', async () => {
            const response = await request(app)
                .get('/users/invalid')
                .expect(400)
                .expect('Content-Type', /json/);
            
            expect(response.body).toHaveProperty('error', true);
            expect(response.body).toHaveProperty('message');
            expect(response.body.message).toContain('must be a number');
        });
    });

    describe('Orders Routes', () => {
        test('GET /orders returns orders list', async () => {
            const response = await request(app)
                .get('/orders')
                .expect('Content-Type', /json/);
            
            // Should either succeed or return a service error
            if (response.status === 200) {
                expect(response.body).toHaveProperty('orders');
                expect(response.body).toHaveProperty('total');
                expect(response.body).toHaveProperty('gateway');
                expect(response.body.gateway).toHaveProperty('serviceName', 'orders');
            } else {
                expect(response.body).toHaveProperty('error', true);
            }
        });

        test('GET /orders/:id with valid ID', async () => {
            const response = await request(app)
                .get('/orders/1')
                .expect('Content-Type', /json/);
            
            // Should either succeed or return a service error
            if (response.status === 200) {
                expect(response.body).toHaveProperty('gateway');
            } else {
                expect(response.body).toHaveProperty('error', true);
            }
        });

        test('GET /orders/:id with invalid ID returns 400', async () => {
            const response = await request(app)
                .get('/orders/invalid')
                .expect(400)
                .expect('Content-Type', /json/);
            
            expect(response.body).toHaveProperty('error', true);
            expect(response.body.message).toContain('must be a number');
        });

        test('GET /users/:userId/orders with valid user ID', async () => {
            const response = await request(app)
                .get('/users/1/orders')
                .expect('Content-Type', /json/);
            
            // Should either succeed or return a service error
            if (response.status === 200) {
                expect(response.body).toHaveProperty('gateway');
            } else {
                expect(response.body).toHaveProperty('error', true);
            }
        });

        test('GET /users/:userId/orders with invalid user ID returns 400', async () => {
            const response = await request(app)
                .get('/users/invalid/orders')
                .expect(400)
                .expect('Content-Type', /json/);
            
            expect(response.body).toHaveProperty('error', true);
            expect(response.body.message).toContain('must be a number');
        });
    });

    describe('Error Handling', () => {
        test('404 for non-existent routes', async () => {
            const response = await request(app)
                .get('/non-existent-route')
                .expect(404)
                .expect('Content-Type', /json/);
            
            expect(response.body).toHaveProperty('error', true);
            expect(response.body).toHaveProperty('message');
            expect(response.body.message).toContain('not found');
            expect(response.body).toHaveProperty('availableRoutes');
        });

        test('Response includes request ID', async () => {
            const response = await request(app)
                .get('/api-docs')
                .expect(200);
            
            expect(response.body).toHaveProperty('requestId');
            expect(response.headers).toHaveProperty('x-request-id');
        });
    });

    describe('CORS and Security', () => {
        test('CORS headers are present', async () => {
            const response = await request(app)
                .get('/health')
                .expect(200);
            
            expect(response.headers).toHaveProperty('access-control-allow-origin');
        });

        test('Security headers are present', async () => {
            const response = await request(app)
                .get('/health')
                .expect(200);
            
            // Helmet should add security headers
            expect(response.headers).toHaveProperty('x-content-type-options');
        });
    });
});