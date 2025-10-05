/**
 * Products Microservice Router
 * 
 * Example of how to add a new microservice to the gateway
 * Handles all routes related to products management
 */

import { Router, Request, Response } from 'express';
import { BaseRouter } from './BaseRouter';
import { RouteConfig, CallMicroserviceFunction, HandleServiceErrorFunction } from '../types';

export class ProductsRouter extends BaseRouter {
    public routeConfig: RouteConfig = {
        serviceName: 'products',
        basePath: '/products',
        endpoints: [
            {
                path: '',
                method: 'GET',
                handler: 'getAllProducts'
            },
            {
                path: '/:id',
                method: 'GET',
                handler: 'getProductById',
                validation: (req: Request) => this.validateNumericId(req.params.id, 'Product ID')
            },
            {
                path: '/category/:categoryId',
                method: 'GET',
                handler: 'getProductsByCategory',
                validation: (req: Request) => this.validateNumericId(req.params.categoryId, 'Category ID')
            }
        ]
    };

    public setupRoutes(
        callMicroservice: CallMicroserviceFunction,
        handleServiceError: HandleServiceErrorFunction
    ): Router {
        // GET /products - Get all products
        this.router.get('/', async (req: Request, res: Response): Promise<void> => {
            const handler = this.createRouteHandler('GET', '/products');
            await handler(req, res, callMicroservice, handleServiceError);
        });

        // GET /products/:id - Get product by ID
        this.router.get('/:id', async (req: Request, res: Response): Promise<void> => {
            const handler = this.createRouteHandler(
                'GET', 
                '/products/:id',
                (req: Request) => this.validateNumericId(req.params.id, 'Product ID')
            );
            await handler(req, res, callMicroservice, handleServiceError);
        });

        // GET /products/category/:categoryId - Get products by category
        this.router.get('/category/:categoryId', async (req: Request, res: Response): Promise<void> => {
            const handler = this.createRouteHandler(
                'GET', 
                '/products/category/:categoryId',
                (req: Request) => this.validateNumericId(req.params.categoryId, 'Category ID')
            );
            await handler(req, res, callMicroservice, handleServiceError);
        });

        return this.router;
    }
}