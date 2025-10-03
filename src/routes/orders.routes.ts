/**
 * Orders Microservice Router
 * 
 * Handles all routes related to orders management
 */

import { Router, Request, Response } from 'express';
import { BaseRouter } from './BaseRouter';
import { RouteConfig, CallMicroserviceFunction, HandleServiceErrorFunction } from '../types';

export class OrdersRouter extends BaseRouter {
    public routeConfig: RouteConfig = {
        serviceName: 'orders',
        basePath: '/orders',
        endpoints: [
            {
                path: '',
                method: 'GET',
                handler: 'getAllOrders'
            },
            {
                path: '/:id',
                method: 'GET',
                handler: 'getOrderById',
                validation: (req: Request) => this.validateNumericId(req.params.id, 'Order ID')
            }
        ]
    };

    public setupRoutes(
        callMicroservice: CallMicroserviceFunction,
        handleServiceError: HandleServiceErrorFunction
    ): Router {
        // GET /orders - Get all orders
        this.router.get('/', async (req: Request, res: Response): Promise<void> => {
            const handler = this.createRouteHandler('GET', '/orders');
            await handler(req, res, callMicroservice, handleServiceError);
        });

        // GET /orders/:id - Get order by ID
        this.router.get('/:id', async (req: Request, res: Response): Promise<void> => {
            const handler = this.createRouteHandler(
                'GET', 
                '/orders/:id',
                (req: Request) => this.validateNumericId(req.params.id, 'Order ID')
            );
            await handler(req, res, callMicroservice, handleServiceError);
        });

        return this.router;
    }
}

/**
 * User-Orders Cross-Service Router
 * 
 * Handles routes that involve both users and orders (cross-service operations)
 */
export class UserOrdersRouter extends BaseRouter {
    public routeConfig: RouteConfig = {
        serviceName: 'orders', // Uses orders service but accessible via users path
        basePath: '/users',
        endpoints: [
            {
                path: '/:userId/orders',
                method: 'GET',
                handler: 'getUserOrders',
                validation: (req: Request) => this.validateNumericId(req.params.userId, 'User ID')
            }
        ]
    };

    public setupRoutes(
        callMicroservice: CallMicroserviceFunction,
        handleServiceError: HandleServiceErrorFunction
    ): Router {
        // GET /users/:userId/orders - Get orders for a specific user
        this.router.get('/:userId/orders', async (req: Request, res: Response): Promise<void> => {
            const handler = this.createRouteHandler(
                'GET', 
                '/users/:userId/orders',
                (req: Request) => this.validateNumericId(req.params.userId, 'User ID')
            );
            await handler(req, res, callMicroservice, handleServiceError);
        });

        return this.router;
    }
}